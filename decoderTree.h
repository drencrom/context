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
 
#ifndef DECODER_TREE_H
#define DECODER_TREE_H

#include "types.h"

/** Decoder context tree structure. */
typedef struct decoderTree {
  Uint left, /**< Index of the leftmost character of the label of this node. (left == length) */
       right, /**< Index of the rightmost character of the label of this node.*/
       *count, /**< Counts of the number of occurrences of each symbol in this state. */
       totalSyms, /** < Total number of symbols occuring at this state. */
       totalCount; /*suma de counts*/
   
  struct decoderTree *tail, /**< Pointer to the node whose label is the tail of this one. */
                     *origin, /**< Pointer to the original node this one descends from. */
                     *parent, /**< Pointer to the parent of this node. */
                     **children, /**< List of pointers to all the children of this node. */
                     **transitions; /**< List of FSM transitions from this state. */

  BOOL *traversed, /**< List of flags indicating an attempt was made to traverse each child edge. */
       used, /**< Flag that indicates if this node has been used to decode eny symbols. */
       internal, /** < Flag that indicates if this node is an internal node of T(x) */
       internalFSM; /** < Flag that indicates if this node is an internal node of the FSM closure of T(x) */

  Uchar **text; /**< Pointer to the input text. */
  Uchar *symbols; /* simbolo en cada posicion */
} *decoderTree_t;

void initDecoderTreeStack();

/** Creates and initializes a new decoder tree structure instance. */
decoderTree_t initDecoderTree(BOOL);

/** Deletes a decoder tree structure instance. */
void freeDecoderTree(decoderTree_t, BOOL);

/** Calculates the FSM closure of this tree. */
void makeDecoderFsm(decoderTree_t);

/** Creates a new decoder tree reading it from a file. */
decoderTree_t readDecoderTree(FILE *);

/** Indicates if the parameter node is the root of the tree */
BOOL isRootDecoderTree(decoderTree_t);

/** Verify*, only called by the decoder routine */
void verifyDecoder(const decoderTree_t root, decoderTree_t node);

#ifdef DEBUG

/** Prints this tree data to the standard output */
void printDecoderTree(decoderTree_t);

#endif

#endif
