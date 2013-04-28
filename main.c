/* Copyright 2013 Jorge Merlino

   This file is part of Context.

   Context is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Context is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Context.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include <assert.h>
#include <math.h>
#include "types.h"
#include "spacedef.h"
#include "mapfile.h"
#include "reverse.h"
#include "wotd.h"
#include "fsmTree.h"
#include "suffixTree.h"
#include "decoderTree.h"
#include "encoder.h"
#include "decoder.h"
#include "debug.h"
#include "alpha.h"
#include "text.h"
#include "reset.h"
#include "arithmetic/coder.h"
#include "arithmetic/bitio.h"

#ifdef DEBUG
#include "gammaFunc.h" 
#endif

/** Ukkonen linear suffix tree contruction algorithm. */
#define UKKONEN 1

/** Kurtz suffix tree contruction algorithm. */
#define KURTZ 2

/** Magic number to detect valid ctx files */
#define MAGIC 0x3276

/**
 * Writes a byte to the output file using the arithmetic encoder
 * @param[in] byte the data to write
 * @param[in] file the output file
 */
static void writeByte(int byte, FILE *file) {
  SYMBOL s;

  byte = byte & 0x000000FF;
  s.scale = 256; 
  s.low_count = byte;
  s.high_count = byte + 1;
  encode_symbol(file, &s);
}

/**
 * Reads a byte using the arithmetic decoder
 * @param[in] file the input file
 * @returns the readed byte
 */
static int readByte(FILE *file) {
  SYMBOL s;
  int ret;

  s.scale = 256; 
  ret = get_current_count(&s);
  s.low_count = ret;
  s.high_count = ret + 1;
  remove_symbol_from_stream(file, &s);

  return ret;
}

/**
 * Writes the alphabet to the output file.
 * @param[in] file the output file.
 */
static void writeAlphabet(FILE *file) {
  int i, j, last = UCHAR_MAX + 1;
  SYMBOL s;
  long cost;

  /* write alphabet size */
  writeByte(alphasize, file);

  cost = bit_ftell_output(file);

  if (alphasize <= UCHAR_MAX) {
    if (alphasize < 128) { /* send alphabet */
      for (i=alphasize-1; i>=0; i--) {
	s.scale = last;
	s.low_count = characters[i];
	s.high_count = characters[i] + 1;
	encode_symbol(file, &s);
	last = characters[i];
      }
    }
    else { /* send complement of alphabet */
      for (i=UCHAR_MAX, j=alphasize-1; i>=0; i--) {
	if (j<0 || characters[j] != i) {
	  s.scale = last;
	  s.low_count = i;
	  s.high_count = i+1;
	  encode_symbol(file, &s);
	  last = i;
	}
	else {
	  j--;
	}
      }
    }
  }
  printf("Alphabet cost: %ld\n", bit_ftell_output(file) - cost);
}


/**
 * Reads the alphabet from the input file.
 * @param[in] file input file.
 */
static void readAlphabet(FILE *file) {
  SYMBOL s;
  int i, j, k, last = UCHAR_MAX + 1, count;
  
  /* read alphabet size */
  alphasize = readByte(file);

  if (alphasize == 0) alphasize = 256;
  printf("Alphasize: %ld\n", alphasize);

  if (alphasize <= UCHAR_MAX) {
    if (alphasize < 128) { /* read alphabet */
      for (i=alphasize-1; i>=0; i--) {
	s.scale = last;
	count = get_current_count(&s);
	characters[i] = count;
	alphaindex[characters[i]] = i;
	last = count;

	s.low_count = count;
	s.high_count = count + 1;
	remove_symbol_from_stream(file, &s);
      }
    }
    else { /* read complement of alphabet */
      for (i=alphasize-1, j=UCHAR_MAX, k=UCHAR_MAX - alphasize; k>=0; k--) {
	s.scale = last;
	count = get_current_count(&s);
	last = count;

	if (j>count) {
	  for (; j>count; i--, j--) {
	    characters[i] = j;
	    alphaindex[characters[i]] = i;
	  }
	}
	j = count-1;

	s.low_count = count;
	s.high_count = count + 1;
	remove_symbol_from_stream(file, &s);
      }
      for (; j>0; i--, j--) {
	characters[i] = j;
	alphaindex[characters[i]] = i;
      }
    }
  }
  else {
    for (i=0; i<alphasize; i++) {
      characters[i] = i;
      alphaindex[i] = i;
    }
  }
}

/**
 * Compresses the input text and writes the compressed data to a file.
 * @param[in] filename name and path of the file to compress.
 * @param[in] compressed name and path of the compressed output file.
 * @param[in] algorithm the algorithm that will be used to build the suffix tree (Ukkonnen or Kurtz).
 * @param[in] see if see will be used.
 */
static void zip(char *filename, char *compressed, BOOL algorithm, int parts, BOOL see) {
  Uchar *origText, *prevText = NULL;
  Uint origTextLen, partTextLen, currentTextLen;
  FILE *compressed_file;
  int i, part;
  fsmTree_t stree = NULL, prevTree = NULL;
  BOOL alloc = False;

#ifdef WIN32
  HANDLE hndl;
  origText = (Uchar *) file2String(filename, &origTextLen, &hndl);
#else
  origText = (Uchar *) file2String(filename, &origTextLen);
#endif

  if(origText == NULL) {
    fprintf(stderr,"Cannot open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  if (!compressed) {
    CALLOC(compressed, Uchar, strlen(filename) + 5);
    strcpy(compressed, filename);
    strcat(compressed, ".ctx");
    alloc = True;
  }

  compressed_file = fopen(compressed, "wb");
  if (!compressed_file) {
    printf( "Could not open output file");
    exit(1);
  }
  if (alloc) FREE(compressed);

  buildAlpha(origText, origTextLen);
  printf ("Alphasize: %ld\n", alphasize);
  printf("Algorithm %d\n", algorithm);

  setMaxCount();

  /* write magic number */
  putc(MAGIC >> 8, compressed_file);
  putc(MAGIC, compressed_file);
  /* write # of parts */
  putc(parts, compressed_file);

  initialize_output_bitstream();
  initialize_arithmetic_encoder();

  writeAlphabet(compressed_file);

  currentTextLen = 0;
  for (part = 1; part <= parts; part++) {
    printf("---------- part %d ---------------\n", part);
    if (part != parts) {
      partTextLen = floor(origTextLen / parts);
    }
    else {
      partTextLen = origTextLen - (floor(origTextLen / parts) * (parts - 1));
    }
 
    if (part > 1) {
      prevText = text;
      prevTree = stree;
    }

    textlen = partTextLen;
    CALLOC(text, Uchar, textlen);
    reversestring(origText + currentTextLen, textlen, text);
    
    if (algorithm == UKKONEN) {
      suffixTree_t tree = initSuffixTree();
      buildSuffixTree(tree);
      printf("Tree built\n");
      pruneSuffixTree(tree);
      stree = fsmSuffixTree(tree);   
    }
    else {
      stree = buildSTree();
      printf("Tree built\n");
    }

    /*if (part > 1) {
      copyStatistics(prevTree, stree, prevText);
      FREE(prevText);
      freeFsmTree(prevTree);
    }*/

    DEBUGCODE(printf("gamma hits: %d gamma Misses: %d\n", getHits(), getMisses()));
    printf("height: %ld\n", getHeight(stree));

    /* write textlen */
    for (i=3; i>=0; i--) {
      writeByte(textlen >> (8 * i), compressed_file);
    }
    printf ("Textlen: %ld\n", textlen);
    writeFsmTree(stree, compressed_file);
    printf("FSM...\n");
    makeFsm(stree);
    DEBUGCODE(printFsmTree(stree));
    printf("Encoding...\n");

    encode(stree, compressed_file, origText + currentTextLen, partTextLen, see);
    
    currentTextLen += partTextLen;
  }

  FREE(text);
  freeFsmTree(stree);

  flush_arithmetic_encoder(compressed_file);
  flush_output_bitstream(compressed_file);

#ifdef WIN32
  freetextspace(origText, hndl);
#else
  freetextspace(origText, origTextLen);
#endif

  fclose(compressed_file);
}


/**
 * Decompresses the data in an input file and writes it in a new file.
 * @param[in] filename name and path of the input file.
 * @param[in] output name and path of the output file.
 * @param[in] see if see will be used.
 */
static void unzip(char *filename, char *output, BOOL see) {
  FILE *output_file, *compressed_file;
  int i, header, parts, part;
  Uint textlen = 0;
  decoderTree_t tree;

  initDecoderTreeStack();

  compressed_file = fopen(filename, "rb");
  if (!compressed_file) {
    perror( "Could not open input file");
    exit(1);
  }

  if (!output) {
    output = strrchr(filename, '.');
    if (output) {
      *output = '\0';
    } 
    else {
      strcat(filename, ".out");
    }
    output = filename;
  }

  output_file = fopen(output, "wb");
  if (!output_file) {
    perror( "Could not open output file");
    exit(1);
  }

  /* check magic */
  header = getc(compressed_file) << 8;
  header += getc(compressed_file);
  if (header != MAGIC) {
    fprintf(stderr, "Invalid compressed file\n");
    exit(1);
  }

  /* read parts */
  parts = getc(compressed_file);

  initialize_input_bitstream();
  initialize_arithmetic_decoder(compressed_file);

  readAlphabet(compressed_file);

  setMaxCount();

  for (part = 1; part <= parts; part++) {  
    printf("---------- part %d ---------------\n", part);
    /* read textlen */
    for (textlen=0, i=3; i>=0; i--) {
      textlen += readByte(compressed_file) << (8 * i);
    }

    tree = readDecoderTree(compressed_file);

    printf("Tree built\n");
    printf("Textlen: %ld\n", textlen);
    printf("FSM...\n"); 

    DEBUGCODE(printDecoderTree(tree));
    makeDecoderFsm(tree);
    DEBUGCODE(printDecoderTree(tree));

    printf("Decoding...\n");
    decode(tree, textlen, compressed_file, output_file, see);
  }
  fclose(compressed_file);
  fclose(output_file);
}


/**
 * Prints program usage instructions in the standard error output.
 * @param[in] progname name of the program as invoked by the user.
 */
static void printUsage (char *progname) {
  fprintf(stderr, "Usage: %s [options] input_file [output_file]\n\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t-z: compress (default)\n"); 
  fprintf(stderr, "\t-d: decompress (default if invoked as hpunzip)\n"); 
  fprintf(stderr, "\t-s: use secondary espace estimation (SEE)\n"); 
  fprintf(stderr, "\t-h: show this message\n"); 

  fprintf(stderr, "\nOptions for compression only:\n");
  fprintf(stderr, "\t-k: use Kurtz algorithm (default)\n"); 
  fprintf(stderr, "\t-u: use Ukkonnen algorithm (default with -b)\n"); 
  fprintf(stderr, "\t-p <num>: number of parts to partition the file\n");
}


/**
 * Parses input parameters and calls the appropriate function.
 * @param[in] argc number of input parameters.
 * @param[in] argv input parameters.
 * @returns program return code.
 */
int main(int argc,char *argv[])
{
  int i, algorithm = 0, parts = 1;
  BOOL compress, see = True;
  char *error = NULL, *pos;

#ifdef WIN32  
  pos = strrchr(argv[0], '\\');
#else
  pos = strrchr(argv[0], '/');
#endif

  if (!pos) {
    pos = argv[0];
  }
  else {
    pos++;
  }
  compress = !(strncmp(pos, "uncontext", 9) == 0);
    
  i = 1;
  while (i < argc && argv[i][0] == '-') {
    switch (argv[i][1]) {
    case 'd':
      compress = False;
      break;
    case 'z':
      compress = True;
      break;
    case 'k':
      algorithm = KURTZ;
      break;
    case 'u':
      algorithm = UKKONEN;
      break;
    case 's':
      see = True;
      break;
    case 'p':
      i++;
      parts = atoi(argv[i]);
      assert(parts < 256);
      break;
    case 'h':
      printUsage(argv[0]);
      printf("\n");
      return EXIT_SUCCESS;
    default:
      error = "Invalid option parameter";
    }
    i++;
  }

  if (parts < 1) {
    error = "Invalid number of parts";
  }

  MAX_COUNT = 300;

  if (!error && argc > i) {
    if (algorithm == 0) {
      algorithm = KURTZ;
    }

    if (compress) {
      if (argc > i+1) {
	zip(argv[i], argv[i+1], algorithm, parts, see);
      }
      else {
	zip(argv[i], NULL, algorithm, parts, see);
      }
    }
    else {
      if (argc > i+1) {
	unzip(argv[i], argv[i+1], see);
      }
      else {
	unzip(argv[i], NULL, see);
      }
    }

    printf("\n");
    return EXIT_SUCCESS;
  }
  else {
    if (!error) {
      error = "Not enough parameters";
    }

    fprintf(stderr, "%s\n\n", error);
    printUsage(argv[0]);

    printf("\n");
    return EXIT_FAILURE;
  }
}
