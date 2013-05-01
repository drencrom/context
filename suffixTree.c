#include <stdlib.h>
#include "suffixTree.h"
#include "stack.h"
#include "alpha.h"
#include "text.h"
#include "gammaFunc.h"
#include "spacedef.h"
#include "debug.h"

/** Flag to indicate that the rightmost index of a label is the index of the last 
 * processed character of the input string. */
#define INFINITY -3

/** Flag to indicate a node is bottom. */
#define BOTTOM -2

/** Flag to indicate a node is the root. */
#define ROOT -1

/** 
 * Gets the index of the rightmost character of this node label in the input string. 
 * @param[out] R the rightmost index.
 * @param[in] T tree node.
*/
#define GET_RIGHT(R, T)\
        if (T->child) {\
          R = T->child->left-1;\
        }\
        else {\
          R = INFINITY;\
        }\


/**
 * Given a reference for a (possibly implicit) state returns the canonical reference to the same state.
 * @param[in] s explicit state of the tree.
 * @param[in] k index of the leftmost character of the string from <i>s</i> to the state.
 * @param[in] p index of the rightmost character of the string from <i>s</i> to the state. 
 * @param[out] retS new explicit state that is the closest ancestor of the state to canonize.
 * @param[out] retK index of the leftmost character of the string from <i>retS</i> to the state to canonize.
*/
static void canonize (suffixTree_t s, Uint k, Uint p, suffixTree_t *retS, Uint *retK) {
  Uint kPrime, pPrime;
  suffixTree_t sPrime;

  if (p == -1 || p < k) {
    *retS = s;
    *retK = k;
  }
  else {
    sPrime = s;
    if (s->left == BOTTOM) {
      sPrime = s->child; /* ROOT is the t-transition of BOTTOM for every t */
    }
    else {
      for (sPrime=sPrime->child; sPrime && text[sPrime->left]!=text[k]; sPrime=sPrime->sibling);
    }
    kPrime = sPrime->left;
    GET_RIGHT(pPrime, sPrime);
    while ((k <= p) && (pPrime - kPrime <= p - k)) {
      k = k + pPrime - kPrime +1;
      s = sPrime;
      if (k <= p) {
	for (sPrime=sPrime->child; sPrime && text[sPrime->left]!=text[k]; sPrime=sPrime->sibling);
	kPrime = sPrime->left;
	GET_RIGHT(pPrime, sPrime);
      }
    }
    *retS = s;
    *retK = k;
  }
}


/**
 * Tests if the input state is the end point.\ If it is not the end point, and it is not an explicit state
 * it is made explicit by splitting a transition.
 * @param[in] s explicit state of the tree.
 * @param[in] k index of the leftmost character of the string from <i>s</i> to the state.
 * @param[in] p index of the rightmost character of the string from <i>s</i> to the state. 
 * @param[out] r new explicit state created or <i>s</i> if a new node was not needed.
 * @returns True if the input state is the end point.
 */
static BOOL testAndSplit(suffixTree_t s, Uint k, Uint p, suffixTree_t *r) {
  suffixTree_t sPrime = s, new;
  Uint kPrime, pPrime;

  if (p != -1 && k <= p) {
    for (sPrime=sPrime->child; sPrime && text[sPrime->left]!=text[k]; sPrime=sPrime->sibling);
    kPrime = sPrime->left;
    GET_RIGHT(pPrime, sPrime);
    if ((p+1 < textlen) && (text[kPrime+p-k+1] == text[p+1])) {
      *r = s;
      return True;
    }
    else {
      /* split transition */
      CALLOC(new, struct suffixTree, 1);
      new->left = sPrime->left;
      new->child = sPrime;
      new->sibling = sPrime->sibling;
      new->parent = sPrime->parent;

      sPrime->left = kPrime + p - k + 1;
      sPrime->parent = new;
      sPrime->sibling = NULL;

      /* fix the parent */
      if (new->parent->child == sPrime) {
	new->parent->child = new;
      }
      else {
	for (s=new->parent->child; s->sibling != sPrime; s=s->sibling);
	s->sibling = new;
      }

      *r = new;
      return False;
    }
  }
  else { /* s is explicit */
    *r = s;
    if (s->left == BOTTOM) {
      return True;
    }
    if (p+1 < textlen) {
      for (sPrime=sPrime->child; sPrime && text[sPrime->left]!=text[p+1]; sPrime=sPrime->sibling);
    }
    else {
      sPrime=NULL;
    }
    return sPrime != NULL;
  }
}


/**
 * Creates a new leaf on the tree.
 * @param[in] s parent of the new leaf to create.
 * @param[in] i index of the leftmost character of the new node label.
 */
static void addChild (suffixTree_t s, Uint i) {
  suffixTree_t new;

  CALLOC(new, struct suffixTree, 1);
  new->left = i;
  new->parent = s;
  
  if (!s->child) {
    s->child = new;
  }
  else {
    for (s=s->child; s->sibling; s=s->sibling);
    s->sibling = new;
  }
}


/**
 * Adds a new character of the input string to the evolving suffix tree.
 * @param s explicit node on the tree that is the closest ancestor to the active point.
 * @param k index of the leftmost character of the string from <i>s</i> to the active point. 
 * @param i integer such that (i-1) is the index of the rightmost character of the string from <i>s</i> to the active point. 
 * @param retS explicit node on the tree that is an ancestor of the end point.
 * @param retK index of the leftmost character of the string from <i>retS</i> to the end point.
 */
static void update (suffixTree_t s, Uint k, Uint i, suffixTree_t *retS, Uint *retK) {
  suffixTree_t oldr=NULL, r;

  while (!testAndSplit(s, k, i-1, &r)) {
    /* add new leaf */
    addChild(r, i);
    if (oldr) {
      /* create suffix link */
      oldr->suffix = r;
    }
    oldr = r;
    canonize(s->suffix, k, i-1, &s, &k);
  }

  if (oldr && oldr->left!=ROOT) { 
    oldr->suffix = r;
  }
  *retS = s;
  *retK = k;
}


/**
 * Delete a subtree from the complete tree. 
 * @param[in] tree subtree to delete.
 */
static void pruneSubTree(suffixTree_t tree) {
  Uint stacktop=0, stackalloc=0, *stack = NULL, treePtr;

  PUSHNODE((Uint)tree);
  while(NOTSTACKEMPTY) {
    POPNODE(treePtr);
    tree = (suffixTree_t)treePtr;
    if (tree->sibling) {
      PUSHNODE((Uint)tree->sibling);
    }
    if (tree->child) {
      PUSHNODE((Uint)tree->child);
    }
    freeSuffixTree(tree);
  }
  FREE(stack);
}

#ifdef DEBUG 

/** 
 * Auxiliary function to print a node of the tree to the standard output.
 * @param[in] tree tree node to print.
 */
static void printNode(suffixTree_t tree) {
  Uint right;

  if (tree->left == BOTTOM) {
    /* nothing */
  }
  else if (tree->left == ROOT) {
    printf("root");
  }
  else {
    GET_RIGHT(right, tree);
    if (right == INFINITY) right = textlen;
    printf("%ld - %ld", tree->left, right);
    /*for (i=tree->left; i<= tree->right; i++) {
      if (i == textlen) {
	printf("$");
      }
      else { 
	printf("%c", text[i]);
      }
    }*/
  }
}


/**
 * Auxiliary recursive function to write a tree into the standard output.
 * @param[in] tree the tree to print
 * @param[in] level depth of this node in the whole tree.
 */
static void printRec(suffixTree_t tree, Uint level) {
  int i;

  if (level > 0) {
    for (i=1; i<level; i++) {
      printf("-");
    }
    printNode (tree);
    /*if (tree->suffix) {
      printf("  (");
      printNode (tree->suffix, text, textlen);
       printf(")");
    }*/
    printf("\n");
  }
  
  level++;
  if (tree->child) {
    for (tree=tree->child; tree; tree=tree->sibling) {
      printRec(tree, level);
    }
  }
}


/**
 * Returns the number of nodes on this tree. 
 * @param[in] tree the tree (or subtree) to count.
 * @returns the number of nodes in the tree.
 */
static int treeSize(suffixTree_t tree) {
  int sum = 1;

  if (tree->child) {
    for (tree=tree->child; tree; tree=tree->sibling) {
      sum += treeSize(tree);
    }
  }
  return sum;
}

#endif


/**
 * @returns a new an initilized tree. 
 */
suffixTree_t initSuffixTree() {
  suffixTree_t tree;

  CALLOC(tree, struct suffixTree, 1);
  CALLOC(tree->child, struct suffixTree, 1);

  tree->left = BOTTOM;
  
  tree->child->left = ROOT;
  tree->child->suffix = tree;
  tree->child->parent = tree;

  return tree;
}


/**
 * @param[in,out] tree the tree to delete.
 */
void freeSuffixTree(suffixTree_t tree) {
  FREE(tree);
}


/**
 * @param[in,out] tree an empty and initialized suffix tree. 
 */
void buildSuffixTree(suffixTree_t tree) {
  Uint k=0, i=0;
  suffixTree_t s = tree->child; /* ROOT */

  while (i <= textlen) {
    update(s, k, i, &s, &k);
    canonize(s, k, i, &s, &k);
    i++;
    if (i%1000 == 0)  printf("%ld/%ld\r", i, textlen);
  }
  printf("%ld/%ld\n", textlen, textlen);
  DEBUGCODE(printf("Original tree size: %d\n", treeSize(tree)));
}


/**
 * @param[in,out] tree the tree to prune.
 */
void pruneSuffixTree(suffixTree_t tree) {
  Uint stacktop=0, stackalloc=0, *stack = NULL, treePtr, length, i, right;
  suffixTree_t child;
  double est;

  tree = tree->child; /* ROOT */
  PUSHNODE((Uint)tree);
  PUSHNODE(0);
  while(NOTSTACKEMPTY) {
    POPNODE(length);
    POPNODE(treePtr);
    tree = (suffixTree_t)treePtr;
    if (!tree->child) { /* is a leaf */
      tree->stats = allocStatistics();
      if (tree->left > length) { /* this is not the full string prefix */
	i = GETINDEX(tree->left-length-1);
	tree->stats->count[i] = 1;
	tree->stats->cost = log2Alpha();
      }
      else {
	tree->stats->cost = log2Alpha();
      }
    }
    else { /* is a branching node */
      for (child=tree->child; child && child->stats; child=child->sibling);
      if (child) { /* there is one child to evaluate */
	PUSHNODE((Uint)tree);
	PUSHNODE(length);
	PUSHNODE((Uint)child);
	if (tree->left != ROOT) {
	  GET_RIGHT(right, tree);
	  PUSHNODE(length + right - tree->left + 1);
	}
	else {
	  PUSHNODE(0);
	}
      } 
      else { /* all children are already evaluated */
	tree->stats = allocStatistics();
	for (child=tree->child; child; child=child->sibling) {
	  for (i=0; i<alphasize; i++) {
	    tree->stats->count[i] += child->stats->count[i];
	  }
	  tree->stats->cost += child->stats->cost;
	  freeStatistics(child->stats);
	}

	GET_RIGHT(right, tree);
	tree->stats->cost += hAlpha() * alphasize * (right - tree->left + 1); 

	/*est = kt(tree->stats);*/
	est = nodeCost(tree->stats);
	if (est <= tree->stats->cost) { /* we have to prune */
	  tree->stats->cost = est;
	  pruneSubTree(tree->child);
	  tree->child = NULL;
	}
      }
    }
  }
  freeStatistics(tree->stats);
  FREE(stack);
}


/**
 * @param[in] tree the tree to transform.
 * @returns a new fsm tree equivalent tho the input one.
 */
fsmTree_t fsmSuffixTree(suffixTree_t tree) {
  Uint stacktop=0, stackalloc=0, *stack = NULL, sfxPtr, fsmPtr, pos, right;
  fsmTree_t ret = initFsmTree(), fsmNode;

  if (tree->child->child) { /* if root has children */
    PUSHNODE((Uint)tree->child->child); 
    PUSHNODE((Uint)ret); /* ROOT */
    while(NOTSTACKEMPTY) {
      POPNODE(fsmPtr);
      POPNODE(sfxPtr);
      fsmNode = (fsmTree_t)fsmPtr;
      tree = (suffixTree_t)sfxPtr;
      /*if (tree->left == textlen) {
	pos = alphasize;
      }
      else {
	pos = GETINDEX(tree->left);   
      }*/
      if (tree->left != textlen) { /* ignore $ leaves */
	pos = GETINDEX(tree->left);   
	fsmNode->children[pos] = initFsmTree();
	fsmNode->children[pos]->parent = fsmNode; 
	fsmNode->children[pos]->left = tree->left; 
	GET_RIGHT(right, tree);
	if (right == INFINITY) {
	  fsmNode->children[pos]->right = tree->left; /* shorten leaf */
	}
	else {
	  fsmNode->children[pos]->right = right; 
	}
	if (fsmNode->left != ROOT) {
	  fsmNode->children[pos]->length = fsmNode->length + fsmNode->right - fsmNode->left + 1;
	}

	if (tree->sibling) {
	  PUSHNODE((Uint)tree->sibling);
	  PUSHNODE(fsmPtr);
	}
	if (tree->child) {
	  PUSHNODE((Uint)tree->child);
	  PUSHNODE((Uint)fsmNode->children[pos]);
	}
      }
      FREE(tree);
    }
    FREE(stack);
  }
  return ret;
}

#ifdef DEBUG

/**
 * @param[in] tree the tree to print.
 */
void printSuffixTree(const suffixTree_t tree) {
  printRec(tree, 0);
}

#endif
