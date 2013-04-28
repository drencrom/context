#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_math.h>
#include <sys/types.h>
#include "types.h"
#include "debug.h"
#include "spacedef.h"
#include "fsmTree.h"
#include "stack.h"
#include "alpha.h"
#include "gammaFunc.h"
#include "statistics.h"
#include "text.h"

/** Number of bits in Uint */
#define INTWORDSIZE (UintConst(1) << LOGWORDSIZE)    

/** Most significative bit of a Uint. */
#define FIRSTBIT (UintConst(1) << (INTWORDSIZE-1)) 

/** Second most significative bit of a Uint. */
#define SECONDBIT (FIRSTBIT >> 1)                  

/** Number of array buckets per node. If more information is needed per node this value should be adjusted accordingly.*/
#define BRANCHWIDTH UintConst(3) 

/** Maximum allowed height for the tree */
#define MAX_HEIGHT 30

/**
 * Given a pointer to the tree array returns the index in the array of that pointer.
 * @param[in] N pointer to the tree array.
 * @returns index of the pointer in the array.
 */
#define NODEINDEX(N) ((Uint) ((N) - streetab))

/** Bit used to indicate that a node is a leaf. */
#define LEAFBIT FIRSTBIT 

/** Bit used to indicate that a node is the last (rightmost) child of a node. */
#define RIGHTMOSTCHILDBIT SECONDBIT 

/** Bit used to indicate that a node has not been evaluated. */
#define UNEVALUATEDBIT FIRSTBIT 

/**
 * Indicates if a node of the tree is a leaf checking the corresponding bit.
 * @param[in] P pointer to the first index on a node in the tree array.
 * @returns True if the node is a leaf.
 */
#define ISLEAF(P) ((*(P)) & LEAFBIT)

/**
 * Indicates if a node is the last (rightmost) child of its parent checking the corresponding bit.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @returns True if the node is the last child.
 */
#define ISRIGHTMOSTCHILD(P) ((*(P)) & RIGHTMOSTCHILDBIT)

/**
 * Indicates if a node has not been evaluated checking the corresponding bit.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @return True if the node has not been evaluated.
 */
#define ISUNEVALUATED(P) ((*((P)+1)) & UNEVALUATEDBIT)

/**
 * Returns the index in the input string of the leftmost character in this node label.
 * First removes the all bit flags.
 * @param P pointer to the first index of a node in the tree array.
 * @returns the index of the leftmost character.
 */
#define GETLP(P) ((*(P)) & ~(LEAFBIT | RIGHTMOSTCHILDBIT))

/**
 * Returns the index in the tree array of the first child of this node.
 * This is the second integer of the three reserved for each internal node.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @returns the index of the first child.
 */
#define GETFIRSTCHILD(P) (*((P)+1))

/**
 * Sets the index in the input string of leftmost character of this node label.
 * Preserves the rightmost child bit setting.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @param[in] LP new index of the leftmost character.
 */
#define SETLP(P,LP) *(P) = (*(P) & RIGHTMOSTCHILDBIT) | (LP)

/**
 * Sets the index in the tree array of the first child of this node.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @param[in] C index of the first child.
 */
#define SETFIRSTCHILD(P,C) *((P)+1) = C

/**
 * Adds a new leaf to the tree.
 * @param[out] P pointer to the tree array where the new leaf will be set.
 * @param[in] L index in the input string of the leftmost character of this leaf label.
 */
#define SETLEAF(P,L)        *(P) = (L) | LEAFBIT

/**
 * Returns the statistics for a node. This is the third integer of the three reserved for each internal node.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @returns pointer to the node statistics.
 */
#define GETSTATS(P)         (*((P)+2))

/**
 * Sets the statistics for a node.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @param[in] C pointer to the node statistics.
 */
#define SETSTATS(P,C)       *((P)+2) = C

/** Undefined reference. */
#define UNDEFREFERENCE      (UINT_MAX)   

/**
 * Given a pointer to the input text returns the index in the array of such pointer.
 * This is used as the start position of a certain suffix.
 * @param[in] L pointer to the input text
 * @returns the index of the pointed char in the array.
 */
#define SUFFIXNUMBER(L)     ((Uint) (*(L) - text))  

/**
 * Store the boundaries of the portion of the text that needs to be read when evaluating this node.
 * This includes setting the unevaluated bit for that node.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @param[in] L the left boundary.
 * @param[in] R the right boundary.
 */
#define STOREBOUNDARIES(P,L,R) *(P) = (Uint) ((L) - suffixbase);\
                               *((P)+1) = ((R) - suffixbase) | UNEVALUATEDBIT

/**
 * Given a pointer to an unevaluated node returns the stored left boundary. 
 * @param[in] P pointer to the first index of a node in the tree array.
 * @returns the left boundary.
 */
#define GETLEFTBOUNDARY(P)  (suffixbase + *(P))

/**
 * Given a pointer to an unevaluated node returns the stored right boundary.  
 * This is stored in the second integer of the three reserved for each internal node.
 * @param[in] P pointer to the first index of a node in the tree array.
 * @returns the right boundary.
 */
#define GETRIGHTBOUNDARY(P) (suffixbase + ((*((P)+1)) & ~UNEVALUATEDBIT))

/** Undefined successor */
#define UNDEFINEDSUCC  (UINT_MAX)    

Uchar *sentinel,               /**< Points to text[textlen] which is undefined. */
      **suffixes,              /**< Array of pointers to suffixes of t. */
      **suffixbase,            /**< Pointers into suffixes are considered w.r.t.\ this pointer. */
      **sbuffer,               /**< Buffer to sort suffixes in sortByChar. */
      **sbufferspace = NULL,   /**< Space to be used by sbuffer. */
      **bound[UCHAR_MAX+1];    /**< Pointers into sbuffer while sorting. */

Uint  occurrence[UCHAR_MAX+1], /**< Number of occurrences of each character. */
      *streetab = NULL,        /**< Array that holds the suffix tree representation. */
      streetabsize,            /**< Number of integers in the allocated streetab memory. */
      *nextfreeentry,          /**< Pointer to next unused element in streetab. */
      sbufferwidth,            /**< Number of elements in sbufferspace. */
      maxsbufferwidth,         /**< Maximal number of elements in sbufferspace. */
      suffixessize,            /**< Number of unprocessed suffixes. */
      maxunusedsuffixes,       /**< When reached, then move and halve space for suffixes. */
      rootchildtab[UCHAR_MAX+1]; /**< Constant time access to successors of root. */

/** Flag indicating that the root has been evaluated. */
BOOL  rootevaluated;   


/**
 * Does space management for the <i>sbuffer</i> array. If there is unused space in the <i>suffixes</i> buffer it is
 * used here, otherwise the <i>sbufferspace</i> array is used (and possibly enlarged if neccesary).
 * @param[in] left left boundary of the portion of the suffixes that will be processed.
 * @param[in] right right boundary of the portion of the suffixes that will be processed.
 * @returns a pointer to the selected memory area.
 */
static Uchar **getsbufferspaceeager(Uchar **left,Uchar **right)
{
  Uint width = (Uint) (right-left+1);

  if(width > (Uint) (left-suffixes))
  {
    if(width > sbufferwidth)
    {
      sbufferwidth = width;
      REALLOC(sbufferspace,sbufferspace,Uchar *,sbufferwidth);
    }
    return sbufferspace;
  }
  return left - width;
}


/** 
 * Maximum number of extra array nodes needed for the children of a node. 
 * The extra 1 is for the <i>$</i> edge that always leads to a leaf.
*/
#define MAXSUCCSPACE (BRANCHWIDTH * alphasize + 1)


/**
 * Enlarges the tree array structure if the current free space is not enough for the evaluation of a new node. 
 */
static void allocstreetab()
{
  Uint tmpindex = NODEINDEX(nextfreeentry); 
  if (tmpindex + MAXSUCCSPACE >= streetabsize) {
    REALLOC(streetab,streetab,Uint,streetabsize + MAXSUCCSPACE);
    streetabsize += MAXSUCCSPACE;
    /* update necessary, since streetab may have been moved. */
    nextfreeentry = streetab + tmpindex;
  }
}


/**
 * Sorts lexicographically a portion of the <i>suffixes</i> array.
 * @param[in] left left boundary of the portion of the suffixes that will be sorted.
 * @param[in] right right boundary of the portion of the suffixes that will be sorted.
 * @param[in] prefixlen all pointers to the prefixes are advanced this number of chars because it is known that 
 * are all equal for all suffixes.
 */
static void sortByChar(Uchar **left, Uchar **right, Uint prefixlen)
{
  Uchar **i, **j, **nextFree = sbuffer;
  Uint a;

  if(*right + prefixlen == sentinel)  /* shortest suffix is sentinel: skip */
  {
    *right += prefixlen;
    right--;
  }
  for(i=left; i<=right; i++) /* determine size for each group */
  {
    *i += prefixlen;         /* drop the common prefix */
    occurrence[(Uint) **i]++;
  }
  for(i=left; i<=right; i++) /* determine right bound for each group */
  {
    a = (Uint) **i;
    if(occurrence[a] > 0)
    {
      bound[a] = nextFree+occurrence[a]-1;
      nextFree = bound[a]+1;
      occurrence[a] = 0;
    }
  }
  for(i=right; i>=left; i--) /* insert suffixes into buffer */
  {
    *(bound[(Uint) **i]--) = *i;
  }
  for(i=left,j=sbuffer; i<=right; i++,j++) /* copy grouped suffixes back */
  {
    *i = *j;
  }
}


/**
 * Sorts lexicographically a portion of the <i>suffixes</i> array. It is used the first time
 * to sort the whole array because it can use the <i>suffixes</i> array instead of <i>sbuffer</i>.
 */
static void sortByChar0()
{
  Uchar *cptr, **nextFree = suffixes;
  Uint a;

  for(cptr=text; cptr < text+textlen; cptr++) /* determine size for each group */
  {
    occurrence[(Uint) *cptr]++;
  }
  for(cptr=characters; cptr < characters+alphasize; cptr++)
  {
    a = (Uint) *cptr;
    bound[a] = nextFree+occurrence[a]-1;
    nextFree = bound[a]+1;
    occurrence[a] = 0;
  }
  for(cptr=text+textlen-1; cptr>=text; cptr--) /* insert suffixes into array */
  {
   *(bound[(Uint) *cptr]--) = cptr;
  }
  suffixes[textlen] = sentinel;  /* suffix $ is the largest suffix */
}


/**
 * Finds the longest common prefix (LCP) length of a number of consecutive prefixes.
 * @param[in] left left boundary of the portion of the suffixes that will be examined.
 * @param[in] right right boundary of the portion of the suffixes that will be examined.
 * @returns the LCP length.
 */
static Uint grouplcp(Uchar **left,Uchar **right)
{
  Uchar cmpchar, **i;
  Uint j;

  for(j=UintConst(1); /* nothing */; j++)
  {
    if(*right+j == sentinel)
    {
      return j;
    }
    cmpchar = *(*left+j);
    for(i=left+1; i<=right; i++)
    {
      if(*(*i+j) != cmpchar)
      {
        return j;
      }
    }
  }
}


/**
 * Given the boundaries of an unevaluated node, evaluates it and reserves space for all its children.
 * @param[in] left the left boundary of the portion of the text that needs to be read for evaluation.
 * @param[in] right the right boundary of the portion of the text that needs to be read for evaluation.
 * @returns the index in the tree array of the first branching child of the now evaluated node.
 */
static Uint evalsuccedges(Uchar **left,Uchar **right)
{
  Uchar firstchar, **r, **l;
  Uint leafnum, firstbranch = UNDEFREFERENCE, *previousnode = NULL;
  BOOL sentineledge = False;

  allocstreetab();
  if(*right == sentinel)
  {
    right--;  /* skip the smallest suffix */
    sentineledge = True;
  }
  for(l=left; l<=right; l=r+1)
  {
    for(firstchar=**l,r=l; r<right && **(r+1)==firstchar; r++)
    {
      /* nothing */ ;
    }
    previousnode = nextfreeentry;
    if(r > l) /* create branching node */
    {
      if(firstbranch == UNDEFREFERENCE)
      {
        firstbranch = NODEINDEX(nextfreeentry);
      }
      STOREBOUNDARIES(nextfreeentry,l,r);
      /* store l and r. resume later with this unevaluated node */
      nextfreeentry += BRANCHWIDTH;
    } else /* create leaf */
    {
      leafnum = SUFFIXNUMBER(l);
      SETLEAF(nextfreeentry,leafnum);
      nextfreeentry++;
    }
  }
  if(sentineledge)
  {
    leafnum = SUFFIXNUMBER(right+1);
    SETLEAF(nextfreeentry,leafnum);
    previousnode = nextfreeentry++;
  }
  assert(previousnode != NULL);
  *previousnode |= RIGHTMOSTCHILDBIT;
  return firstbranch;
}

/**
 * Evaluates all the edges outgoing from the root of the tree. It is a specialization of <i>evaluatesuccedges</i> for the root node.
 * @param[in] left the left boundary of the portion of the text that needs to be read for evaluation.
 * @param[in] right the right boundary of the portion of the text that needs to be read for evaluation.
 * @returns the index in the tree array of the first branching child of the now evaluated node.
 * @see evaluatesuccedges
 */
static Uint evalrootsuccedges(Uchar **left,Uchar **right)
{
  Uchar firstchar, **r, **l;
  Uint *rptr, leafnum, firstbranch = UNDEFREFERENCE;

  for(rptr = rootchildtab; rptr <= rootchildtab + UCHAR_MAX; rptr++)
  {
    *rptr = UNDEFINEDSUCC;
  }
  for(l=left; l<=right; l=r+1) /* first phase */
  {
    for(firstchar=**l,r=l; r<right && **(r+1)==firstchar; r++)
    {
      /* nothing */ ;
    }
    if(r > l) /* create branching node */
    {
      if(firstbranch == UNDEFREFERENCE)
      {
        firstbranch = NODEINDEX(nextfreeentry);
      }
      STOREBOUNDARIES(nextfreeentry,l,r);
      /* store l and r. resume later with this unevaluated branch node */
      rootchildtab[firstchar] = NODEINDEX(nextfreeentry);
      nextfreeentry += BRANCHWIDTH;
    } else /* create leaf */
    {
      leafnum = SUFFIXNUMBER(l);
      SETLEAF(nextfreeentry,leafnum);
      rootchildtab[firstchar] = leafnum | LEAFBIT;
      nextfreeentry++;
    }
  }
  SETLEAF(nextfreeentry,textlen | RIGHTMOSTCHILDBIT);
  nextfreeentry++;
  return firstbranch;
}


/**
 * Extracts the boundaries of an unevaluated node, evaluates it and creates all its children.
 * @param[in] node index of the node to evaluate in the tree array representation.
 * @param[out] length length of the label of this node.
 * @returns the index in the tree array of the first branching child of the now evaluated node.
 */
static Uint evaluatenodeeager(Uint node, Uint *length)
{
  Uint prefixlen, *nodeptr, unusedsuffixes;
  Uchar **left, **right;

  nodeptr = streetab + node;
  left = GETLEFTBOUNDARY(nodeptr);
  right = GETRIGHTBOUNDARY(nodeptr);
  SETLP(nodeptr,SUFFIXNUMBER(left));
  SETFIRSTCHILD(nodeptr,NODEINDEX(nextfreeentry));

  unusedsuffixes = (Uint) (left - suffixes);
  if(suffixessize > UintConst(10000) && unusedsuffixes > maxunusedsuffixes)
  {
    Uint tmpdiff, width = (Uint) (right - left + 1);
    Uchar **i, **j;
    for(i=left, j=suffixes; i<suffixes+suffixessize; i++, j++)
    {
      *j = *i;  /* move remaining suffixes to the left */
    }
    suffixessize -= unusedsuffixes;
    maxunusedsuffixes = suffixessize >> 1;
    tmpdiff = (Uint) (suffixes - suffixbase);
    REALLOC(suffixes,suffixes,Uchar *,suffixessize);
    suffixbase = suffixes - (tmpdiff + unusedsuffixes);
    left = suffixes;
    right = suffixes + width - 1;
  }
  sbuffer = getsbufferspaceeager(left,right);
  prefixlen = grouplcp(left,right);
  *length = prefixlen;
  sortByChar(left,right,prefixlen);
  return evalsuccedges(left,right);
}


/**
 * Returns the next brother of this node that is not a leaf.
 * @param[in] previousbranch index of the original node in the tree array representation.
 * @returns the index of the next branching brother of the node in the tree array representation.
 */
static Uint getnextbranch(Uint previousbranch)
{
  Uint *nodeptr = streetab + previousbranch;

  if(ISRIGHTMOSTCHILD(nodeptr))
  {
    return UNDEFREFERENCE;
  }
  nodeptr += BRANCHWIDTH;
  while(True)
  {
    if(ISLEAF(nodeptr))
    {
      if(ISRIGHTMOSTCHILD(nodeptr))
      {
        return UNDEFREFERENCE;
      }
      nodeptr++;
    } else
    {
      return NODEINDEX(nodeptr);
    }
  }
}

/**
 * Specifically tests if it is necessary to prune the tree at the root and does it if necessary .
 */
static void pruneRoot() {
  Uint *nodeptr;
  Uint pos, i, idx;
  Uint end;
  statistics_t stats, childStats; 
  double est, auxx;

  Uint * distinct;
  CALLOC(distinct, Uint, alphasize);

  nodeptr = streetab;

  /* counters */
  stats = getStatistics();

  do {
    if (ISLEAF(nodeptr)) {
      if (GETLP(nodeptr) > 0) { /* if false the previous character is outside of string */
	pos = GETLP(nodeptr) - 1; /* position where leaf starts minus previous length */
	idx = alphaindex[*(text+pos)];
	if (stats->count[idx] == 0) {
	  stats->symbols[stats->symbolCount++] = idx;
	}
	stats->count[idx]++;
	distinct[idx]++;
      }
      stats->cost += log2Alpha();
    }
    else {
      childStats = (statistics_t)GETSTATS(nodeptr);
      for (i=0; i<childStats->symbolCount; i++) {
	if (stats->count[childStats->symbols[i]] == 0) {
	  stats->symbols[stats->symbolCount++] = childStats->symbols[i];
	} 
	stats->count[childStats->symbols[i]] += childStats->count[childStats->symbols[i]];
	distinct[childStats->symbols[i]]++;
      }
      stats->cost += childStats->cost;
      returnStatistics(childStats);
    }

    end = ISRIGHTMOSTCHILD(nodeptr);
    if (!end) {
      if (ISLEAF(nodeptr)) {
	nodeptr++;
      }
      else {
	nodeptr += BRANCHWIDTH;
      }
    }
  } while (!end);

  stats->cost += hAlpha() * alphasize;

  auxx = escapeCost(stats, distinct);
  stats->cost += auxx;
  assert(auxx >= 0);

  free(distinct);

  est = nodeCost(stats);

  if (est <= stats->cost) { /* pruning needed */
    nextfreeentry = 0; /* indicates an empty tree */
  }
  freeStatistics(stats);
}


/**
 * Evaluates the cost function to decide if we have to prune this node and does it if necessary. 
 * @param[in] node index of the node in the array tree.
 * @param[in] length length of the label of this node (from the root of the tree).
 * @param[in] branchLength length of the label of this node alone.
 */
static void prune(Uint node, Uint length, Uint branchLength) {
  Uint *nodeptr, *nodeptrPar;
  Uint pos, i, idx;
  statistics_t stats=NULL, childStats; 
  Uint end;
  double est, auxx;

  Uint * distinct;
  CALLOC(distinct, Uint, alphasize);

  nodeptrPar = streetab + node;
  nodeptr = streetab + GETFIRSTCHILD(nodeptrPar);

  do {
    if (ISLEAF(nodeptr)) {
      if (stats == NULL) {
	stats = getStatistics();
      }
      if (GETLP(nodeptr) > length) { /* if false the previous character is outside the string */
	pos = GETLP(nodeptr) - length - 1; /* position where the leaf begins minus previous length */
	idx = alphaindex[*(text+pos)];
	if (stats->count[idx] == 0) {
	  stats->symbols[stats->symbolCount++] = idx;
	}
	stats->count[idx]++;
	distinct[idx]++;
      }
    }
    else {
      childStats = (statistics_t)GETSTATS(nodeptr);

      if (stats == NULL) {
	stats = childStats;
	for (i=0; i<stats->symbolCount; i++) {
	  distinct[stats->symbols[i]]++;
	} 
      }
      else {
	for (i=0; i<childStats->symbolCount; i++) {
	  if (stats->count[childStats->symbols[i]] == 0) {
	    stats->symbols[stats->symbolCount++] = childStats->symbols[i];
	  } 
	  stats->count[childStats->symbols[i]] += childStats->count[childStats->symbols[i]];
	  distinct[childStats->symbols[i]]++;
	}
	stats->cost += childStats->cost;
	returnStatistics(childStats);
      }
    }

    end = ISRIGHTMOSTCHILD(nodeptr);
    if (!end) {
      if (ISLEAF(nodeptr)) {
	nodeptr++;
      }
      else {
	nodeptr += BRANCHWIDTH;
      }
    }
  } while (!end);

  stats->cost += hAlpha() * alphasize * branchLength;
  
  auxx = escapeCost(stats, distinct);
  stats->cost += auxx;
  assert(auxx >= 0);

  free(distinct);
  if ((length-branchLength) < MAX_HEIGHT) {
    est = nodeCost(stats);
    assert(est >= 0);
 
    if (est <= stats->cost) { /* pruning needed */
      nextfreeentry = streetab + GETFIRSTCHILD(nodeptrPar);
      SETFIRSTCHILD(nodeptrPar, UNDEFREFERENCE);
      stats->cost = est;
    }
  }
  else {   
    stats->cost = 0xFFFFFFFF;
    nextfreeentry = streetab + GETFIRSTCHILD(nodeptrPar);
    SETFIRSTCHILD(nodeptrPar, UNDEFREFERENCE);
  }

  SETSTATS(nodeptrPar, (Uint)stats);
}

/**
 * Prunes a node inconditionally
 * @param [in] node index of the node in the array tree
 * @param [in] length length of the label of this node (from the root of the tree).
 */
static void prune2 (Uint node, Uint length) {
  Uint *nodeptr, idx;
  Uchar **left, **right, **i;
  statistics_t stats;

  stats = getStatistics();

  nodeptr = streetab + node;
  left = GETLEFTBOUNDARY(nodeptr);
  right = GETRIGHTBOUNDARY(nodeptr);

  for (i = left; i <= right; i++) {
    if (SUFFIXNUMBER(i) > length) {
      idx = alphaindex[text[SUFFIXNUMBER(i) - length - 1]];
      if (stats->count[idx] == 0) {
	stats->symbols[stats->symbolCount++] = idx;
      }
      stats->count[idx]++;
    }
  }
  stats->cost = 0xFFFFFFFF;
  SETSTATS(nodeptr, (Uint)stats);
  SETFIRSTCHILD(nodeptr, UNDEFREFERENCE);
}

/**
 * Main method to build and prune the tree.
 */
static void evaluateeager() {
  Uint firstbranch, nextbranch, node, newNode, length, branchLength, stacktop=0, stackalloc=0, *stack = NULL;

  sortByChar0();
  firstbranch = evalrootsuccedges(suffixes,suffixes+textlen-1);

  if(firstbranch != UNDEFREFERENCE)
  {
    PUSHNODE(firstbranch);
    PUSHNODE(0);
    PUSHNODE(0);
    while(NOTSTACKEMPTY)
    {
      POPNODE(branchLength);
      POPNODE(length);
      POPNODE(node);
      while(node != UNDEFREFERENCE)
      {
	if (!ISUNEVALUATED(streetab+node)) { /* the node has already been evaluated */
	  prune(node, length, branchLength);
	  node = UNDEFREFERENCE; /* to continue the while */
	}
	else {
	  if((nextbranch = getnextbranch(node)) != UNDEFREFERENCE) {
	    PUSHNODE(nextbranch);
	    PUSHNODE(length); /* my parent's length */
	    PUSHNODE(0); /* does not matter, will be branchLenth when evaluated */
	  }

	  if (length < MAX_HEIGHT) {
	    newNode = evaluatenodeeager(node, &branchLength);
	    if (newNode == UNDEFREFERENCE) { /* all children are leaves */
	      prune(node, length + branchLength, branchLength);
	    }
	    else {
	      PUSHNODE(node);
	      PUSHNODE(branchLength + length);
	      PUSHNODE(branchLength);
	    }
	    node = newNode;
	    length += branchLength; /* is the length of the father */
	  }
	  else {
	    prune2(node, length);  
	    node = UNDEFREFERENCE; 
	  }
	}
      }
    }
    FREE(stack);
  }
}


/**
 * Initializes the tree array structure and ancillary structures.
 */
static void inittree()
{
  Uint i;

  sentinel = text+textlen;
  REALLOC(streetab,streetab,Uint,BRANCHWIDTH + MAXSUCCSPACE);
  streetabsize = BRANCHWIDTH + MAXSUCCSPACE;
  nextfreeentry = streetab;

  /* no partition */
  suffixessize = textlen+1;
  maxunusedsuffixes = suffixessize >> 1;
  CALLOC(suffixes, Uchar *, suffixessize);
  suffixbase = suffixes;

  sbufferwidth = 0;
  maxsbufferwidth = textlen >> 8;
  rootevaluated = False;
  for(i=0; i<=UCHAR_MAX; i++)
  {
    occurrence[i] = 0;
  }
}


/**
 * Builds a pruned context tree.
 */
static void wotd()
{
  inittree();
  evaluateeager();
  FREE(suffixes);
  FREE(sbufferspace);  
}


/** 
 * Returns the next sibling of a node.
 * @param[in] node index of the node in the tree array.
 * @returns index of the first sibling in the tree array.
 */
Uint getnextsibling(Uint node) {
  Uint *nodeptr = streetab + node;

  if(ISRIGHTMOSTCHILD(nodeptr))
  {
    return UNDEFREFERENCE;
  }
  else {
    if(ISLEAF(nodeptr)) {
      return NODEINDEX(nodeptr + 1);
    }
    else {
      return NODEINDEX(nodeptr + BRANCHWIDTH);
    }
  }  
}


/** Builds a new fsm tree from the pruned tree array representation.
 * @returns a new fsm tree.
 */
static fsmTree_t buildTree () {
  Uint stacktop=0, stackalloc=0, *stack = NULL, sibling, child, pos, node, *nodeptr, parentptr;
  fsmTree_t ret = initFsmTree(), parent;

  if (nextfreeentry == 0) { /* only root */
    return ret;
  }

  PUSHNODE(0);
  PUSHNODE((Uint)ret);

  while(NOTSTACKEMPTY) {
    POPNODE(parentptr);
    POPNODE(node);
    parent = (fsmTree_t)parentptr;
    nodeptr = streetab + node;

    if (GETLP(nodeptr) != textlen) { /* ignore $ leaves */
      pos = GETINDEX(GETLP(nodeptr));
      parent->children[pos] = initFsmTree();
      parent->children[pos]->parent = parent;
      parent->children[pos]->left = GETLP(nodeptr); 

      /* push the next sibling */
      sibling = getnextsibling(node);
      if (sibling != UNDEFREFERENCE) {
	PUSHNODE(sibling);
	PUSHNODE(parentptr); 
      }

      if (parent->left != -1) { /* != ROOT */
	parent->children[pos]->length = parent->length + parent->right - parent->left + 1;
      }
      if (ISLEAF(nodeptr) || GETFIRSTCHILD(nodeptr) == UNDEFREFERENCE) { /* is a leaf or has been pruned */
	parent->children[pos]->right = GETLP(nodeptr); /* shorten leaf */
      }
      else {
	child = GETFIRSTCHILD(nodeptr);
	parent->children[pos]->right = GETLP(streetab + child) - 1;

	/* push this child */
	PUSHNODE(child);
	PUSHNODE((Uint)parent->children[pos]);
      }
    }
  }

  FREE(stack);
  FREE(streetab);
  return ret;
}


/**
 * @returns a new fsm tree.
 */
fsmTree_t buildSTree ()
{
  wotd();
  /* as the representation has no root it has to be done specifically */
  pruneRoot();
  freeBuffer();

  return buildTree();
}
