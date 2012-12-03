#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> 
#include "encoder.h"
#include "alpha.h"
#include "debug.h"
#include "text.h"
#include "spacedef.h"
#include "arithmetic/coder.h"
#include "arithmetic/bitio.h"
#include "see.h"

static Uint MAX_COUNT;

Uint findSymbolCount (fsmTree_t tree, Uchar sym) {
  Uint i;

  for (i=0; i<tree->totalSyms; i++) {
    if (tree->symbols[i] == sym) {
      return tree->count[i];
    }
  }
  return 0;
}

static void printStats (fsmTree_t origTree, Uint *maskedChars, Uint i) {
  int j;

  printf ("## ");
  for(j=0; j<origTree->totalSyms; j++) {
    printf("count[%d] = %d %s", origTree->symbols[j], origTree->count[j], 
	   (maskedChars[alphaindex[origTree->symbols[j]]] == i+1 ? "(masked) " : ""));
  }
  printf("Total: %d %d (%p)\n", origTree->totalCount, origTree->totalSyms, (void *)origTree);
  /*printf("Total: %d %d\n", origTree->totalCount, origTree->totalSyms);*/

  /*if (!isRootFsmTree(origTree)) {
    for (j=origTree->left; j< origTree->right; j++) {
      printf("%d-", *(text + j));
    }
    if (origTree->right == textlen) {
      printf("$\n");
    }
    else {
      printf("%d\n", *(text + origTree->right));
    }
  }  
  else printf("\n");*/
}

/**
 * Remove symbols from the statistics of the ancestors of this node in case that is necessary.
 * Symbols are removed from the ancestors if they only have the same symbols as the child node
 * (including the ones indicated in deletedChars).
 * @param[in] tree node of the tree.
 * @param[in] deletedChars bit vector indicating which characters have been erased from the node statistics.
 */
static void fixParents (fsmTree_t tree, BOOL *deletedChars) {
  Uint i; /*, numEscapes;*/
  Uchar sym;
  fsmTree_t parTree = tree;
  BOOL newChars;

  while (!isRootFsmTree(parTree)) {
    parTree = parTree->parent->origin;
    newChars = False;

    for (i=0; i<parTree->totalSyms && !newChars; i++) {
      sym = parTree->symbols[i];
      newChars |= (parTree->count[i] > 0) && (findSymbolCount(tree, sym) == 0) && (!deletedChars[alphaindex[sym]]);
    }

    if (!newChars) { /* there are no new symbols */
      DEBUGCODE(printf("fixing %p\n", (void *)parTree));
      /*numEscapes = parTree->totalCount;*/
      parTree->totalCount = 0;
      for (i=0; i<parTree->totalSyms; i++) {  
	/*numEscapes -= parTree->count[i];*/
	if (deletedChars[alphaindex[parTree->symbols[i]]]) {
	  parTree->count[i] = 0;
	}
	else {
	  parTree->totalCount += parTree->count[i];
	}
      }

      /*numEscapes = numEscapes >> 1;
      if (numEscapes == 0) numEscapes = 1;  es necesario esto? 
      parTree->totalCount += numEscapes;*/
      parTree->totalCount *= 2;
    }
    else break;
  }
}
    
static void rescale (fsmTree_t tree) {
  BOOL *charFlags, needToFix = False;
  Uint numEscapes, i;
  DEBUGCODE(printf("rescalo %p\n", (void *)tree));

  numEscapes = tree->totalCount; 
  tree->totalCount = 0; 
  CALLOC(charFlags, BOOL, alphasize);    

  for (i=0; i<tree->totalSyms; i++) {  
    if (tree->count[i] > 0) {
      numEscapes -= tree->count[i];
      tree->count[i] = tree->count[i] >> 1;
      if (tree->count[i] == 0) {
	charFlags[alphaindex[tree->symbols[i]]] = True;
	needToFix = True;
      }
      else {
	tree->totalCount += tree->count[i];
      }
    }
  }

  if (needToFix) {
    fixParents(tree, charFlags);
  }
  FREE(charFlags)
  numEscapes = numEscapes >> 1;
  if (numEscapes == 0) numEscapes = 1; 
  tree->totalCount += numEscapes;
}


/**
 * @param[in] tree model tree.
 * @param[in] compressedFile file to output the compressed data.
 * @param[in] text text to compress.
 * @param[in] textlen length of the text to compress
 * @param[in] useSee if see will be used
 */
void encode (fsmTree_t tree, FILE *compressedFile, const Uchar *text, const Uint textlen, const BOOL useSee) {
  SYMBOL s, escape;
  Uint i, j, k, numMasked, low, pos, allCount, noMask, *maskedChars, state;
  long cost;
  BOOL found;
  fsmTree_t origTree;
  Uchar sym;

  MAX_COUNT = 25000;
  
  printf("MAX_COUNT: %d\n", MAX_COUNT);
  CALLOC(maskedChars, Uint, alphasize);
  cost = bit_ftell_output(compressedFile);

  if (useSee) {
    initSee();
  }

  for (i=0; i<textlen; i++) { 
    sym = text[i];
    pos = GETINDEX(i);
    DEBUGCODE(printf("index: %d\n", i));
    origTree = tree->origin;
    numMasked = 0;
    assert(origTree != NULL);

    if (origTree->totalCount > MAX_COUNT && origTree->used) {
      rescale (origTree);
    }
    DEBUGCODE(printStats(origTree, maskedChars, i));
    found = False;
    do {
      noMask = -1;
      if (origTree->totalSyms > numMasked) { /* found a parent with more data */
	/*printf("encontre estado con mas simbolos\n");*/
	low = j = allCount = 0;
	do {
	  allCount += origTree->count[j];
	  if (maskedChars[alphaindex[origTree->symbols[j]]] != i+1) {
	    found = (origTree->symbols[j] == sym) && (origTree->count[j] > 0);
	    if (origTree->symbols[j] == sym && origTree->count[j] == 0) {
	      noMask = j;
	    }

	    if (!found) {
	      low += origTree->count[j];
	    }
	  }
	}
	while (!found && ++j < origTree->totalSyms);

	if (found) {
	  s.low_count = low;
	  s.high_count = low += origTree->count[j];
	  /* compute the new scale */
	  for (k=j+1; k < origTree->totalSyms; k++) {
	    allCount += origTree->count[k];
	    if (maskedChars[alphaindex[origTree->symbols[k]]] != i+1) {
	      low += origTree->count[k];
	    }
	  }
	  s.scale = low + origTree->totalCount - allCount; /* low + escapes */

	  if (low > 0 && useSee) {
	    state = getSeeStateEncoder(origTree, allCount, i, numMasked, text, alphasize);
	    if (state != -1) {
	      /*DEBUGCODE(printf("-- state %d %d\n", See[state][0], See[state][1]));*/
	      escape.scale = escape.high_count = See[state][1];
	      escape.low_count = See[state][0];

	      DEBUGCODE(printf("--scale: %d diff: %d symbol: %d\n", escape.scale, escape.high_count - escape.low_count, -2));
	      encode_symbol(compressedFile, &escape);
	      
	      s.scale = low;
	      updateSee(state, False, alphasize);
	    }
	  }

	  /*DEBUGCODE(printf("++scale: %d high: %d low: %d symbol: %d\n", s.scale, s.high_count, s.low_count, sym));*/
	  DEBUGCODE(printf("++scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, sym));

	  encode_symbol(compressedFile, &s);
	  origTree->totalCount += 2;
	  origTree->count[j] += 2; 
	  origTree->used = True;
	}
	else {
	  s.low_count = low;
	  s.high_count = s.scale = low + origTree->totalCount - allCount; /* low + escapes */

	  if (origTree->totalCount > 0 && low > 0 && useSee) {
	    state = getSeeStateEncoder(origTree, allCount, i, numMasked, text, alphasize);
	    /*DEBUGCODE(printf("-- state %d\n", state));*/
	    if (state != -1) {
	      /*DEBUGCODE(printf("-- state %d %d\n", See[state][0], See[state][1]));*/
	      escape.scale = See[state][1];
	      escape.low_count = 0;
	      escape.high_count = See[state][0];

	      DEBUGCODE(printf("--scale: %d diff: %d symbol: %d\n", escape.scale, escape.high_count - escape.low_count, -1));
	      assert(See[state][1] > 0);
	      encode_symbol(compressedFile, &escape);
	      
	      s.scale = 0;
	      updateSee(state, True, alphasize);
	    }
	  }

	  /*if (s.scale > 0 && s.low_count > 0) {*/
	  if (s.scale > 0) {
	    DEBUGCODE(printf("--scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, -1));
	    encode_symbol(compressedFile, &s);
	  }

	  for (j--; j != -1; j--) {
	    if (origTree->count[j] > 0 && (maskedChars[alphaindex[origTree->symbols[j]]] != i+1)) {
	      maskedChars[alphaindex[origTree->symbols[j]]] = i+1;
	      numMasked++;
	    }
	  }

	  if (noMask == -1) {
	    addSymbol(origTree, sym);
	  }
	  else {
	    origTree->count[noMask] = 1;
	    origTree->totalCount++;
	  }
	  origTree->totalCount++; /* escape */
	}
      } /* if (origTree->totalSyms > numMasked) */
      else {
	addSymbol(origTree, sym);
	origTree->totalCount++; /* escape */
	DEBUGCODE(printf("--escape\n"));
      }

      if (isRootFsmTree(origTree) && !found) {
	found = True; 
	for (j=0, allCount=0, low=0; j<alphasize; j++) {
	  if (maskedChars[j] != i+1) {
	    allCount++;
	    if (j < pos) { 
	      low++;
	    }
	  }
	}
	
	/* do not reserve probability for symbols already seen */
	s.scale = allCount;
	s.low_count = low;
	s.high_count = low + 1;
	encode_symbol(compressedFile, &s);
	
	/*DEBUGCODE(printf("++scale: %d high: %d low: %d symbol: %d\n", s.scale, s.high_count, s.low_count, sym));*/
	DEBUGCODE(printf("++scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, sym));
      }

      if (!found) {
	origTree = origTree->parent->origin;
	  
	if (origTree->totalCount > MAX_COUNT && origTree->used) {
	  rescale (origTree);
	}
	DEBUGCODE(printStats(origTree, maskedChars, i));

	/* search for a parent with more information */
	while (!isRootFsmTree(origTree) && origTree->totalSyms == numMasked) {
	  addSymbol(origTree, sym);
	  origTree->totalCount++; /* escape */

	  DEBUGCODE(printf("--escape\n"));
	  origTree = origTree->parent->origin;  

	  if (origTree->totalCount > MAX_COUNT && origTree->used) {
	    rescale (origTree);
	  }
	  DEBUGCODE(printStats(origTree, maskedChars, i));
	  DEBUGCODE(printf("--escape\n"));
	}
      }
    }
    while (!found);
    DEBUGCODE(printf("\n"));
    tree = tree->transitions[pos];
  }
  FREE(maskedChars);
}
