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
