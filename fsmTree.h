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
 
#ifndef FSM_TREE_H
#define FSM_TREE_H

#ifndef WIN32
#include <obstack.h>
#endif
#include "types.h"

/** Encoder context tree structure. */
typedef struct fsmTree {
  Uint left, /**< Index of the leftmost character of this node label in the input string. */
       right, /**< Index of the rightmost character of this node label in the input string. */
       length, /**< Distance from this node to the root of the tree. */
       *count, /**< List containing the number of occurrences of each character in this state. */
       totalSyms, /**< Total number of symbols occuring at this state. */
       totalCount; /*suma de counts*/
    /*numEscapes;*/ /*suma de escapes*/

  Uchar *symbols; /* simbolo en cada posicion */

  struct fsmTree *tail, /**< Pointer to the node whose label is the tail of this one. */
                 *origin, /**< Pointer to the original node this one descends from. */
                 *parent, /**< Pointer to the parent of this node. */
                 **children, /**< List of pointers to all the children of this node. */
                 **transitions; /**< List of FSM transitions from this state. */

  BOOL *traversed, /**< List of flags indicating an attempt was made to traverse each child edge. */
       used; /**< Flag that indicates if this node has been used to encode a symbol. */
  
#ifndef WIN32
  struct obstack nodeStack; /**< Obstack used to allocate memory for this tree. */
#endif
} *fsmTree_t;

/** Creates and initializes a new fsm tree structure instance. */
fsmTree_t initFsmTree();

/** Deletes a fsm tree structure instance. */
void freeFsmTree(fsmTree_t);

/** Adds a new symbol to this tree node */
void addSymbol(fsmTree_t, const Uchar);

/** Calculates the FSM closure of this tree. */
void makeFsm(fsmTree_t);

/** Writes this tree into a file. */
void writeFsmTree(const fsmTree_t, FILE *);

/** Indicates if the parameter node is the root of the tree */
BOOL isRootFsmTree(const fsmTree_t);

Uint getHeight (const fsmTree_t);

void printContext (fsmTree_t);

void compareTrees (const fsmTree_t, const fsmTree_t);

void copyStatistics (const fsmTree_t orig, fsmTree_t dest, const Uchar * text2);

#ifdef DEBUG

/** Prints this tree data to the standard output */
void printFsmTree(const fsmTree_t);

#endif 

#endif
