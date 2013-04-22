#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> 
#include "decoder.h"
#include "debug.h"
#include "alpha.h"
#include "spacedef.h"
#include "stack.h"
#include "see.h"
#include "reset.h"
#include "arithmetic/coder.h"
#include "arithmetic/bitio.h"

static Uint findSymbolCount (decoderTree_t tree, Uchar sym) {
  Uint i;

  for (i=0; i<tree->totalSyms; i++) {
    if (tree->symbols[i] == sym) {
      return tree->count[i];
    }
  }
  return 0;
}

/**
 * Remove symbols from the statistics of the ancestors of this node in case that is necessary.
 * Symbols are removed from the ancestors if they only have the same symbols as the child node
 * (including the ones indicated in deletedChars).
 * @param[in] tree node of the tree.
 * @param[in] deletedChars bit vector indicating which characters have been erased from the node statistics.
 */
static void fixParents (decoderTree_t tree, BOOL *deletedChars) {
  Uint i; /*, numEscapes;*/
  Uchar sym;
  decoderTree_t parTree = tree;
  BOOL newChars;

  while (!isRootDecoderTree(parTree)) {
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
    
static void rescale (decoderTree_t tree) {
  BOOL *charFlags, needToFix = False;
  Uint numEscapes, i;
  DEBUGCODE(printf("rescalo %p\n", (void *)tree));

  numEscapes = tree->totalCount; /* - tree->count[0];*/
  /*tree->count[0] = tree->count[0] >> 1;*/
  tree->totalCount = 0; /*tree->count[0];*/
  CALLOC(charFlags, BOOL, alphasize);    

  for (i=0; i<tree->totalSyms; i++) {  
    if (tree->count[i] > 0) {
      numEscapes -= tree->count[i];
      tree->count[i] = tree->count[i] >> 1;
      if (tree->count[i] == 0) {
	/*DEBUGCODE(printf("needtofix %d\n", tree->symbols[i]));*/
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
  FREE(charFlags);
  numEscapes = numEscapes >> 1;
  if (numEscapes == 0) numEscapes = 1; /* es necesario esto? */
  tree->totalCount += numEscapes;
}

static void printStats (decoderTree_t origTree, Uint *maskedChars, Uint i) {
  int j;

  printf ("## ");
  for(j=0; j<origTree->totalSyms; j++) {
    printf("count[%d] = %ld %s", origTree->symbols[j], origTree->count[j], 
	   (maskedChars[alphaindex[origTree->symbols[j]]] == i+1 ? "(masked) " : ""));
  }
  printf("Total: %ld %ld (%p)\n", origTree->totalCount, origTree->totalSyms, (void *)origTree);
  /*printf("Total: %d %d\n", origTree->totalCount, origTree->totalSyms);*/
  
  /*if (!isRootDecoderTree (origTree)) {
    for (j=0; j<origTree->right; j++) {
      printf("%d-", *(*(origTree->text) + j));
    }
    printf("%d\n", *(*(origTree->text) + origTree->right));
  }
  else printf("\n");*/
}

static void addNodes (Uchar * text, Uchar sym, decoderTree_t * tree, decoderTree_t * prevTree, Uint i, Uint * zPrevLeft, Uint * zPrevRight) {
  decoderTree_t sNext, child, newChild, new, newLeaf;
  Uint zLeft = 1, zRight = 0, uSize, uLeft, zNextLeft, j, k, b;
  BOOL end;

  text[i] = sym; /* TODO: ver si se puede usar mmap */
  sNext = (*tree)->transitions[alphaindex[sym]];
  new = NULL;

  /* calculo u */
  if (!isRootDecoderTree(sNext) && sNext->right >= (*tree)->right + 1) {
    /* u is empty */
    if (*zPrevLeft <= *zPrevRight) {
      child = sNext->children[alphaindex[(*(*prevTree)->text)[*zPrevLeft]]];
      if (child) {
	*zPrevLeft = child->left;
	zLeft = child->left;
	zRight = child->left + *zPrevRight - *zPrevLeft + 1;
      }
      else {
	zLeft = 1;
	zRight = 0;
      }
    }
    else {
      zLeft = 1;
      zRight = 0;
    }
  }
  else {
    if (isRootDecoderTree(sNext)) {
      uSize = (*tree)->right + 1;
      uLeft = 0;
      child = sNext->children[alphaindex[sym]];
    }
    else {
      uSize = (*tree)->right - sNext->right;
      uLeft = sNext->right + 1; /* points to tree->text */
      child = sNext->children[alphaindex[(*(*tree)->text)[sNext->right]]];
    }

    if (child) {
      j = 0;
      end = False;
      zNextLeft = child->left; /* will be zPrevLeft when the do-while loop ends */
      do {
	for (k=child->left+1; j < uSize && k <= child->right && (*(*tree)->text)[uLeft+j] == (*child->text)[k]; j++, k++);
	if (j == uSize) { /* all u belongs to WORD T */
	  if (k > child->right) { /* sz already is a node = child */
	    new = child;
	  }
	  else {
	    if ((*zPrevLeft <= *zPrevRight) && 
		((*(*prevTree)->text)[*zPrevLeft] == (*child->text)[k])) { /* head z belongs to WORD T */
	      if (k == child->right) {
		new = child;
		/* TODO: esto es dudoso, ¿hay que seguir bajando? */
	      }
	      else {
		zLeft = child->left;
		zRight = k + *zPrevRight - *zPrevLeft;
		/* TODO: esto es dudoso, ¿hay que seguir bajando? */
	      }
	    }
	    else {
	      zLeft = child->left;
	      zRight = k - 1;
	    }
	  }
	  end = True;
	}
	else { 
	  if (k > child->right) { /* standing in a node */
	    newChild = child->children[alphaindex[(*(*tree)->text)[uLeft+j]]];
	    if (newChild) {
	      sNext = child;
	      child = newChild;
	      j++;
	    }
	    else {
	      new = child;
	      end = True;
	    }
	  }
	  else { /* fell off the tree -> found z */
	    zLeft = child->left;
	    zRight = k - 1;
	    end = True;
	  }
	}
      } while (!end); 
      *zPrevLeft = zNextLeft;
    }
    else { /* z empty */
      zLeft = 1;
      zRight = 0;
    }
  }

  /* insert z if necessary */
  if (new == NULL) {
    if (zLeft <= zRight) {
      new  = initDecoderTree(False);
      DEBUGCODE(printf("New node4: %p\n", (void *)new));
      new->internal = child->internal;
      new->internalFSM = child->internalFSM;
      new->left = (!isRootDecoderTree(sNext) ? sNext->right + 1 : 0);
      new->right = new->left + zRight - zLeft;

      new->children[alphaindex[(*child->text)[new->right+1]]] = child;
      child->left = new->right + 1;
      child->parent = new; 
      new->text = child->text;

      new->parent = sNext;
      if (isRootDecoderTree(sNext)) {
	sNext->children[alphaindex[sym]] = new;
      }
      else {
	sNext->children[alphaindex[(*(*tree)->text)[sNext->right]]] = new;
      }

      new->totalSyms = child->totalSyms;
      new->totalCount = 0;

      memcpy(new->transitions, sNext->transitions, alphasize * sizeof(decoderTree_t));
      for (j=0; j < new->totalSyms; j++) {
	new->count[j] = (child->count[j] == 0 ? 0 : 1);
	new->totalCount += new->count[j];
	new->symbols[j] = child->symbols[j];
      }
      new->totalCount *= 2; /* symbols and escapes */

      if (sNext->internal) {
	new->origin = new;
      }
      else {
	new->origin = child->origin;
      }

      verifyDecoder((isRootDecoderTree(sNext) ? sNext : sNext->tail), new);
    }
    else {
      /* Hay que llamar a verify? */
      new = sNext;
      *zPrevLeft = new->right + 1; /* empty */
    }
  }

  *prevTree = new;
  *zPrevRight = new->right;

  /* get b */
  b = -1; /* lambda */ 
  if (new->internalFSM) {
    /*if (new->internal) {*/
    if (!isRootDecoderTree(new) && new->right < i) {
      b = text[i - new->right - 1];
    }
    else if (isRootDecoderTree(new)) {
      b = text[i];
    }
  }

  if (b != -1) {
    /* insert b */
    newLeaf = initDecoderTree(False);
    DEBUGCODE(printf("New node5: %p\n", (void *)newLeaf));
    newLeaf->internal = False;
    newLeaf->internalFSM = False;
    newLeaf->left = newLeaf->right = new->right + 1;
    newLeaf->parent = new; 
    new->children[alphaindex[b]] = newLeaf;

    MALLOC(newLeaf->text, Uchar *, 1);
    MALLOC(*newLeaf->text,Uchar, newLeaf->right + 1);
    if (newLeaf->right > 0) {
      memcpy(*newLeaf->text, *new->text, newLeaf->left);
    }
    (*newLeaf->text)[newLeaf->left] = b;      

    memcpy(newLeaf->transitions, new->transitions, alphasize * sizeof(decoderTree_t));
    /*for (j=0; j<alphasize; j++) {
      newLeaf->transitions[j] = new->transitions[j];
      }*/

    if (new->internal) {
      newLeaf->origin = newLeaf;
    }
    else {
      newLeaf->origin = new->origin;
    }

    verifyDecoder((isRootDecoderTree(new) ? new : new->tail), newLeaf);
    *tree = newLeaf;
  }
  else {
    *tree = new;
  }
}

void decode (decoderTree_t tree, const Uint textlen, FILE *compressedFile, FILE *output, const BOOL useSee) {
  Uint i, j, k, length, count, numMasked, prevCount, allCount, low, *maskedChars, state;
  Uint zPrevLeft = 1, zPrevRight = 0;
  Uchar sym = 0;
  SYMBOL s;
  decoderTree_t origTree, prevTree = NULL;
  BOOL found, escape;
  Uchar * text;

  printf("MAX_COUNT: %ld\n", MAX_COUNT);
  MALLOC(text, Uchar, textlen);
  CALLOC(maskedChars, Uint, alphasize);

  if (useSee) {
    initSee();
  }

  for (i=0; i<textlen; i++) {
    origTree = tree->origin;
    numMasked = 0;
    DEBUGCODE(printf("index: %ld\n", i));
    /*DEBUGCODE(printf("orig: "); printStats(tree, maskedChars));*/

    if (origTree->totalCount > MAX_COUNT && origTree->used) {
      rescale (origTree);
    }

    DEBUGCODE(printStats(origTree, maskedChars, i));
    length = 0;
    found = False;
    do {
      escape = False;
      if (origTree->totalSyms > numMasked && origTree->totalCount > 0) { /* found a parent with more data */
	low = allCount = 0;
	for (j=0; j < origTree->totalSyms; j++) {
	  allCount += origTree->count[j];
	  if (maskedChars[alphaindex[origTree->symbols[j]]] != i+1) {
	    low += origTree->count[j];
	  }
	}

	if (low > 0 && useSee) {
	  state = getSeeStateDecoder(origTree, allCount, i, numMasked, text, alphasize);
	  if (state != -1) {
	    /*DEBUGCODE(printf("-- state %d %d\n", See[state][0], See[state][1]));*/
	    s.scale = See[state][1];
	    count = get_current_count(&s);
	    if (count < See[state][0]) { /* escape */
	      s.low_count = 0;
	      s.high_count = See[state][0];
	      DEBUGCODE(printf("--scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, -1));
	      remove_symbol_from_stream(compressedFile, &s);
	      escape = True;
	      
	      for(j=0; j < origTree->totalSyms; j++) {
		if (origTree->count[j] > 0 && maskedChars[alphaindex[origTree->symbols[j]]] != i+1) {
		  maskedChars[alphaindex[origTree->symbols[j]]] = i+1;
		  numMasked++;
		}
	      }
	    }
	    else {
	      s.low_count = See[state][0];
	      s.high_count = See[state][1];
	      DEBUGCODE(printf("--scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, -2));
	      remove_symbol_from_stream(compressedFile, &s);
	      s.scale = low;
	    }
	    
	    updateSee(state, escape, alphasize);
	  }
	  else {
	    s.scale = low + origTree->totalCount - allCount; /* low + escapes */
	  }
	}
	else {
	  s.scale = low + origTree->totalCount - allCount; /* low + escapes */
	}
	
	if (!escape) {
	  count = get_current_count(&s);
	  /*DEBUGCODE(printf("scale: %d count: %d\n", s.scale, count));*/

	  for (j=0; maskedChars[alphaindex[origTree->symbols[j]]] == i+1; j++);
	  s.high_count = origTree->count[j];
	  for (j++; (j < origTree->totalSyms) && (s.high_count <= count); j++) {
	    if (maskedChars[alphaindex[origTree->symbols[j]]] != i+1) {
	      s.high_count += origTree->count[j];
	    }
	  }

	  if ((j == origTree->totalSyms) && (s.high_count <= count)) { /* not found */
	    s.low_count = s.high_count;
	    s.high_count = s.scale;
	    DEBUGCODE(printf("--scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, -1));

	    for(j--; j != -1; j--) {
	      if (origTree->count[j] > 0 && maskedChars[alphaindex[origTree->symbols[j]]] != i+1) {
		maskedChars[alphaindex[origTree->symbols[j]]] = i+1;
		numMasked++;
	      }
	    }
	  }
	  else { /* found */
	    found = True;
	    j--;
	    s.low_count = s.high_count - origTree->count[j] ;
	    sym = origTree->symbols[j];
	    putc(sym, output);
	    origTree->used = True;
	    DEBUGCODE(printf("++scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, sym));
	    origTree->totalCount+=2;
	    origTree->count[j]+=2;
	  }
	  remove_symbol_from_stream(compressedFile, &s);
	}
      }
      else {
	DEBUGCODE(printf("--escape\n"));
      }

      if (isRootDecoderTree(origTree) && !found) {
	found = True;
	for (j=0, count=0, prevCount=0; j<alphasize; j++) {
	  if (maskedChars[j] != i+1) {
	    count++;
	  }
	}
	s.scale = count;
	count = get_current_count(&s);
	/*DEBUGCODE(printf("scale: %d count: %d\n", s.scale, count));*/

	s.high_count=0;
	for (j=0; (j < alphasize) && (s.high_count <= count); j++) {
	  if (maskedChars[j] != i+1) {
	    s.high_count ++;
	  }
	} 
	s.low_count = s.high_count - 1;
	sym = characters[j-1];
	remove_symbol_from_stream(compressedFile, &s);
	putc(sym, output);

	origTree->totalCount += 2; /* symbol and escape */

	for (k=0; (k < origTree->totalSyms) && (origTree->symbols[k] != sym); k++);
	if (k == origTree->totalSyms) {
	  origTree->totalSyms++;	    
	}
	origTree->count[k] = 1;
	origTree->symbols[k] = sym;

	DEBUGCODE(printf("++scale: %d diff: %d symbol: %d\n", s.scale, s.high_count - s.low_count, sym));
      }

      if (!found) {
	origTree = origTree->parent->origin;
	length++;

	if (origTree->totalCount > MAX_COUNT && origTree->used) {
	  rescale (origTree);
	}
	DEBUGCODE(printStats(origTree, maskedChars, i));

	/* search for a parent with more information */
	while (!isRootDecoderTree(origTree) && origTree->totalSyms == numMasked) {
	  DEBUGCODE(printf("--escape\n"));
	  origTree = origTree->parent->origin;  
	  length++;
      
	  if (origTree->totalCount > MAX_COUNT && origTree->used) {
	    rescale (origTree);
	  }
	  DEBUGCODE(printStats(origTree, maskedChars, i));
	  DEBUGCODE(printf("--escape\n"));
	}
      }
    }
    while (!found);

    origTree = tree->origin;
    for (j=0; j<length; j++) {
      origTree->totalCount += 2; /* symbol and escape */
      for (k=0; (k < origTree->totalSyms) && (origTree->symbols[k] != sym); k++);
      if (k == origTree->totalSyms) {
	origTree->totalSyms++;	    
      }

      origTree->count[k] = 1;
      origTree->symbols[k] = sym;
      assert(origTree->parent);
      origTree = origTree->parent->origin;  
    }

    DEBUGCODE(printf("\n"));
    addNodes(text, sym, &tree, &prevTree, i, &zPrevLeft, &zPrevRight);
  } /* for */
  FREE(maskedChars);
  FREE(text);
}
