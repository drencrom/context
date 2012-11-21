#ifndef SUFFIX_TREE_H
#define SUFFIX_TREE_H

#include "types.h"
#include "statistics.h"
#include "fsmTree.h"

/** Suffix tree structure. Based on Ukkonen paper. */
typedef struct suffixTree {
  Uint left; /**< Index of the leftmost character of this node label in the input string. */
  struct suffixTree *suffix, /**< Pointer to the node whose label is the tail of this node's label. */
                    *child, /**< Pointer to the first child of this node. */
                    *sibling, /**< Pointer to the next sibling of this node. */
                    *parent; /**< Pointer to the parent of this node. */
  statistics_t stats; /**< Pointer to the statistics for this node context. */
} *suffixTree_t;


/** Creates and initializes a new suffix tree structure instance. */
suffixTree_t initSuffixTree();

/** Deletes a suffix tree structure instance. */
void freeSuffixTree(suffixTree_t);

/** Builds a suffix tree based on the input string. */
void buildSuffixTree(suffixTree_t);

/** Prunes this suffix tree according to some cost function. */
void pruneSuffixTree(suffixTree_t);

/** Transforms this tree in an equivalent fsm tree structure. */
fsmTree_t fsmSuffixTree(suffixTree_t);


#ifdef DEBUG

/** Prints a suffix tree to the standard output. */
void printSuffixTree(const suffixTree_t);

#endif

#endif
