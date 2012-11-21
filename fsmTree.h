#ifndef FSM_TREE_H
#define FSM_TREE_H

#include <obstack.h>
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
  
  struct obstack nodeStack; /**< Obstack used to allocate memory for this tree. */
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
