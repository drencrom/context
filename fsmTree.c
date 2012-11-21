#include <assert.h>
#include "fsmTree.h"
#include "alpha.h"
#include "text.h"
#include "spacedef.h"
#include "debug.h"
#include "arithmetic/coder.h"
#include "arithmetic/bitio.h"

/** Flag to indicate a node is the root. */
#define ROOT -1

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

/** Current position in the buffer. */
static short pos;

/** Number of internal nodes in the tree. Equals the number of ones in the natural code file. */
static Ushort internalNodes; 

/** Total number of nodes in the tree. */
static Uint totalNodes;

/** 
 * Calculates the canonical decomposition of a string in a faster way. It is only possible to use this variant in some special cases.
 * @param[in] tree node from where to start the search.
 * @param[in] xLeft left index in the input data of the string to canonize.
 * @param[in] xRight right index in the input data of the string to canonize.
 * @param[out] r node whose label is the longest prefix of the string in the tree.
 * @param[out] uLeft index of the leftmost character of the string <i>v</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] uRight index of the leftmost character of the string <i>v</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] vLeft index of the leftmost character of the remaining part of the string (after <i>ru</i>).
 * @param[out] vRight index of the rightmost character of the remaining part of the string (after <i>ru</i>).
 */
static void fastCanonize(fsmTree_t tree, Uint xLeft, Uint xRight, fsmTree_t *r, Uint *uLeft, Uint *uRight, Uint *vLeft, Uint *vRight) {
  BOOL end;
  fsmTree_t child = NULL;

  *vLeft = 1;
  *vRight = 0; /* v is empty */
  
  if (tree->left != ROOT) {
    xLeft += tree->length + tree->right - tree->left + 1;
  }
  end = xLeft > xRight;
  
  while (!end) {
    child = tree->children[GETINDEX(xLeft)];
    if ((child->right - child->left) <= (xRight - xLeft)) {
      xLeft += child->right - child->left + 1;
      tree = child;
      end = xLeft > xRight;
    }
    else {
      end = True;
    }
  }

  *r = tree;
  /*TODO: esto se arregla con *uleft=xleft y *uRight=xRight y sin if */
  if (xLeft <= xRight) {
    *uLeft = child->left;
    *uRight = child->left + xRight - xLeft;
  }
  else {
    *uLeft = 1;
    *uRight = 0; /* u is empty */
  }
}


/** 
 * Calculates the canonical decomposition of a string.
 * @param[in] tree node from where to start the search.
 * @param[in] xLeft left index in the input data of the string to canonize.
 * @param[in] xRight right index in the input data of the string to canonize.
 * @param[out] r node whose label is the longest prefix of the string in the tree.
 * @param[out] uLeft index of the leftmost character of the string <i>u</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] uRight index of the rightmost character of the string <i>u</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] vLeft index of the leftmost character of the remaining part of the string (after <i>ru</i>).
 * @param[out] vRight index of the rightmost character of the remaining part of the string (after <i>ru</i>).
 */
static void canonize(fsmTree_t tree, Uint xLeft, Uint xRight, fsmTree_t *r, Uint *uLeft, Uint *uRight, Uint *vLeft, Uint *vRight) {
  BOOL end = False;
  Uint i;
  fsmTree_t child;

  if (xLeft > xRight) {
    *uLeft = 1;
    *uRight = 0; /* u is empty */
    end = True;
  }
  else if (tree->left != ROOT) {
    xLeft += tree->length + tree->right - tree->left + 1;
  }

  while (!end) {
    child = tree->children[GETINDEX(xLeft)]; 
    if (child) { /* there is an edge in the direction of xLeft */
      for (i=child->left; (i<=child->right) && (xLeft<=xRight) && (text[i]==text[xLeft]); i++, xLeft++); 
      if (i > child->right) { /* all the edge is in x */
	tree = child;
	if (xLeft > xRight) {
	  end = True;
	  *uLeft = 1;
	  *uRight = 0; /* u is empty */
	}
      }
      else {
	end = True;
	*uLeft = child->left;
	*uRight = i-1;
      }
    }
    else {
      end = True;
      *uLeft = 1;
      *uRight = 0; /* u is empty */
    }
  }

  *r = tree;
  *vLeft = xLeft;
  *vRight = xRight;
}


/**
 * Inserts new nodes in the FSM closure of the tree. Inserts nodes <i>ru</i> and <i>ruv</i> doing edge splits if necessary.
 * @param[in] r parent of the new node to be added.
 * @param[in] uLeft index of the leftmost character of the <i>u</i> string.
 * @param[in] uRight index of the rightmost character of the <i>u</i> string.
 * @param[in] vLeft index of the leftmost character of the <i>v</i> string.
 * @param[in] vRight index of the rightmost character of the <i>v</i> string.
 * @returns a pointer to the new added node.
 */
static fsmTree_t insert (fsmTree_t r, Uint uLeft, Uint uRight, Uint vLeft, Uint vRight) {
  fsmTree_t new  = initFsmTree(), newLeaf, ret;

  if (uLeft > uRight) {
    /* add */
    new->left = vLeft;
    new->right = vRight;
    if (r->left != ROOT) {
      new->length = r->length + r->right - r->left + 1;
    }
    new->parent = r; 
    r->children[GETINDEX(vLeft)] = new;

    new->origin = r->origin;
    ret = new;
  }
  else {
    /* split */
    new->left = uLeft;
    new->right = uRight;
    new->length = r->children[GETINDEX(uLeft)]->length;
    new->children[GETINDEX(uRight+1)] = r->children[GETINDEX(uLeft)]; /* TODO: uRight+1 == vLeft?? */
    new->children[GETINDEX(uRight+1)]->left = uRight+1;
    new->children[GETINDEX(uRight+1)]->length = new->length + uRight - uLeft + 1;
    new->children[GETINDEX(uRight+1)]->parent = new; 
    new->parent = r; 
    r->children[GETINDEX(uLeft)] = new;

    new->origin = r->origin;
    new->traversed[GETINDEX(uRight+1)] = r->traversed[GETINDEX(uLeft)];

    if (vLeft <= vRight) {
      newLeaf = initFsmTree();
      /* add */
      newLeaf->left = vLeft;
      newLeaf->right = vRight;
      newLeaf->length = new->length + new->right - new->left + 1;
      newLeaf->parent = new; 
      new->children[GETINDEX(vLeft)] = newLeaf;

      newLeaf->origin = new->origin;
      ret = newLeaf;
    }
    else {
      ret = new;
    }
  }
  return ret; 
}


/**
 * Verifies that the tail of <i>root</i> is in the tree, adding it if necessary and continuing recursively on the children of <i>root</i>.
 * @param[in] root node to use as the start point in the search of <i>canonize</i>.
 * @param[in] node node to verify.
 * @param[in] fast flag to indicate that it is possible to use <i>fastCanonize</i> in this invocation of <i>verify</i>.
 */
static void verify(const fsmTree_t root, fsmTree_t node, BOOL fast) {
  Uint xLeft, uLeft, uRight, vLeft, vRight, i, cidx;
  fsmTree_t r, x;
 
  if (node->left != ROOT) {
    cidx = GETINDEX(node->left - node->length);
    xLeft = node->left - node->length + 1;

    if (fast) {
      /*printf("%p - %d - %d\n", (void*)root, xLeft, node->right);*/
      fastCanonize(root, xLeft, node->right, &r, &uLeft, &uRight, &vLeft, &vRight);
    } 
    else {
      canonize(root, xLeft, node->right, &r, &uLeft, &uRight, &vLeft, &vRight);
    }

    if ((uLeft <= uRight) || (vLeft <= vRight)) {
      x = insert(r, uLeft, uRight, vLeft, vRight);
      if (uLeft <= uRight) {
	if (r->traversed[GETINDEX(uLeft)]) {
	  verify((r->left == ROOT ? r : r->tail), r->children[GETINDEX(uLeft)], True);
	  /*verify((r->left == ROOT ? r : r->tail), r->children[GETINDEX(uLeft)], False);*/
	}
      }
      else if (r->traversed[GETINDEX(vLeft)]) {
	verify((r->left == ROOT ? r : r->tail), r->children[GETINDEX(vLeft)], False);
      }
    }
    else { /* the node already exists */
      x = r;
    }
    node->tail = x;
    x->transitions[cidx] = node;
  } 
  for (i=0; i<alphasize; i++) {
    if (!node->traversed[i]) {
      node->traversed[i] = True;
      if (node->children[i]) {
	verify((node->left == ROOT ? node : node->tail), node->children[i], False);
      }
    }
  }  
}


/**
 * Adds to the FSM a set of transitions originating from tree if they were not defined by <i>verify</i>.
 * @param[in] transitions set of transitions.
 * @param[in] tree origin node of the transitions.
 */
static void propagateTransitions(fsmTree_t *transitions, fsmTree_t tree) {
  Uint i;

  for (i=0; i<alphasize; i++) {
    if (!tree->transitions[i]) {
      tree->transitions[i] = transitions[i];
    }
  }

  for (i=0; i<alphasize; i++) {
    if (tree->children[i]) {
      propagateTransitions(tree->transitions, tree->children[i]);
    }
  }
}

/**
 * Auxiliary function to write a tree node to a file using an arithmetic encoder.
 * @param[in] internal flag indicating if the node to write is internal or a leaf.
 * @param[in] file file where the tree is written.
 * @returns True if there is no need to write any more data to the file.
 */
static BOOL writeEncoder(BOOL internal, FILE *file) {
  SYMBOL s;
  Uint shift = 0, totalN = totalNodes, internalN = internalNodes;

  while ((totalN & 0xFFFFC000) != 0) { /* some of the most significative 18 bits on */
    totalN >>= 1;
    shift++;
  }

  if (shift > 0) {
    internalN >>= shift;
    totalN = (internalN * alphasize) + 1;
  }

  s.scale = totalN;
  if (internal) {
    s.low_count = 0;
    s.high_count = internalN;
    internalNodes--;
  }
  else {
    s.low_count = internalN;
    s.high_count = totalN;
  }
  totalNodes--;
  /* DEBUGCODE(printf("low:%d high:%d scale:%d length:%ld\n", s.low_count, s.high_count, s.scale, bit_ftell_output(file))); */
  encode_symbol(file, &s);

  return ((totalNodes == internalNodes) || internalNodes == 0);
}


/**
 * Auxiliary recursive function to write a tree to a file using an arithemtic encoder.
 * @param[in] tree node to write.
 * @param[in] offset index of the node label string current being processed. 
 * This is needed in order to write a full tree. 
 * @param[in] file file where the tree is written.
 */
static BOOL writeFsmTreeRec(const fsmTree_t tree, Uint offset, FILE *file) {
  Uint i;
  BOOL leaf = True, stop;

  if (tree->left + offset == tree->right) {
    for (i=0; i<alphasize && leaf; i++) {
      leaf = !tree->children[i];
    }
    if (leaf) {
      return writeEncoder(0, file);
    }
    else {
      stop = writeEncoder(1, file);
      if (stop) return True;
      for (i=0; i<alphasize; i++) {
	if (tree->children[i]) {
	  stop = writeFsmTreeRec(tree->children[i], 0, file);
	  if (stop) return True;
	}
	else {
	  stop = writeEncoder(0, file);
	  if (stop) return True;
	}
      }
    }
  }
  else {
    stop = writeEncoder(1, file);
    if (stop) return True;
    for (i=0; i<alphasize; i++) {
      if (GETINDEX(tree->left + offset + 1) == i) {
	stop = writeFsmTreeRec(tree, offset+1, file);
	if (stop) return True;
      }
      else {
	stop = writeEncoder(0, file);
	if (stop) return True;
      }
    }
  }
  return False;
}


/**
 * Counts the number of internal nodes in a full tree. 
 * If it is not full counts also the nodes needed to make it full.
 * @param[in] tree tree to count nodes from.
 * @param[in] offset index of the node label string current being processed. 
 * This is needed in order to simulate a full tree. 
 */
static Ushort getInternalNodeCount (const fsmTree_t tree, const Uint offset) {
  Uint i, count = 0;
  BOOL leaf = True;

  if (tree->left + offset == tree->right) {
    for (i=0; i<alphasize; i++) {
      if (tree->children[i]) {
	count += getInternalNodeCount(tree->children[i], 0);
	leaf = False;
      }
    }
    if (!leaf) count ++;
  }
  else {
    count = getInternalNodeCount(tree, offset+1) + 1;
  }
  return count;
}

static void copyStatisticsRec (const fsmTree_t orig, const int offsetOrig, fsmTree_t dest, const int offsetDest, const Uchar * text2) {
  int i, posDest, posOrig;
  BOOL end;
 
  posDest = dest->left + offsetDest;
  posOrig = orig->left + offsetOrig;
  end = False;
  while (posOrig <= orig->right && posDest <= dest->right && !end) {
    if (text2[posOrig] == text[posDest]) {
      posOrig++;
      posDest++;
    }
    else {
      end = True;
    }
  }

  if (!end) {
    if (posDest > dest->right) { /* reached a node in dest */
      dest->origin->totalSyms = orig->origin->totalSyms;
      dest->origin->totalCount = orig->origin->totalCount;
      for (i=0; i<alphasize; i++) {
	dest->origin->symbols[i] = orig->origin->symbols[i];
	dest->origin->count[i] = orig->origin->count[i];
      }

      if (posOrig > orig->right) { /* reached a node in orig also */
	for (i=0; i<alphasize; i++) {
	  if (orig->children[i] && dest->children[i]) {
	    copyStatisticsRec(orig->children[i], 0, dest->children[i], 0, text2);
	  }
	}
      }
      else {
	if (dest->children[GETINDEX3(posOrig)]) {
	  copyStatisticsRec(orig, posOrig, dest->children[GETINDEX3(posOrig)], 0, text2);
	}
      }
	
    }
    else { /* reached a node in orig */
      if (orig->children[GETINDEX(posDest)]) 
	copyStatisticsRec(orig->children[GETINDEX(posDest)], 0, dest, posDest, text2);
    }
  }
}

#ifdef DEBUG

/**
 * Auxiliary recursive function to write a tree into the standard output.
 * @param[in] tree node of the tree to write.
 * @param[in] level depth of the node.
 */
static void printRec(const fsmTree_t tree, int level) {
  int i;

  if (level > 0) {
    for (i=1; i<=level; i++) {
      printf("-");
    }

    /*printf("  %d - %d \n", text[tree->left], text[tree->right]);*/
    assert (tree->right < textlen);
    assert (tree->left < textlen);
    printf("%d - %d (%p) parent: %p orig: %p\n", tree->left, tree->right, (void*)tree, (void*)tree->parent, (void*)tree->origin); 
    for (i=tree->left; i< tree->right; i++) {
      printf("%d-", *(text + i));
    }
    if (tree->right == textlen) {
      printf("$\n");
    }
    else {
      printf("%d\n", *(text + tree->right));
    }
  }
  else {
    printf("(root)\n");
  }
  
  level++;
  for (i=0; i<alphasize; i++) {
    if (tree->children[i]) {
      printf ("hijo %d ", i);
      printRec(tree->children[i], level);
    }
  }
}

#endif

/**
 * @returns a new fsm tree insance. 
 */
fsmTree_t initFsmTree() {
  fsmTree_t ret;

  CALLOC(ret, struct fsmTree, 1);
  memset(ret, 0, sizeof(struct fsmTree));

  obstack_init (&(ret->nodeStack));
  if (obstack_chunk_size (&(ret->nodeStack)) < 16384) {
    obstack_chunk_size (&(ret->nodeStack)) = 16384;
  }
  
  ret->children = (fsmTree_t *)obstack_alloc(&(ret->nodeStack), sizeof(struct fsmTree *) * alphasize);
  memset(ret->children, 0, sizeof(struct fsmTree *) * alphasize);
  
  ret->transitions = (fsmTree_t *)obstack_alloc(&(ret->nodeStack), sizeof(struct fsmTree *) * alphasize);
  memset(ret->transitions, 0, sizeof(struct fsmTree *) * alphasize);
  
  ret->traversed = (BOOL *)obstack_alloc(&(ret->nodeStack), sizeof(BOOL) * alphasize);
  memset(ret->traversed, 0, sizeof(BOOL) * alphasize);
  
  ret->count = (Uint *)obstack_alloc(&(ret->nodeStack), sizeof(Uint) * alphasize);
  ret->symbols = (Uchar *)obstack_alloc(&(ret->nodeStack), sizeof(Uchar) * alphasize);


  /*CALLOC(ret, struct fsmTree, 1);
  CALLOC(ret->children, struct fsmTree *, alphasize);
  CALLOC(ret->transitions, struct fsmTree *, alphasize);
  CALLOC(ret->traversed, BOOL, alphasize);
  MALLOC(ret->count, Uint, alphasize); 
  MALLOC(ret->symbols, Uchar, alphasize); */

  /* not always true but makes sense as init */
  ret->left = ret->right = ROOT;
  ret->origin = ret;
  ret->used = False;
  return ret;
}


/**
 * @param[in] tree tree to delete. 
 */
void freeFsmTree(fsmTree_t tree) {
  /*FREE(tree->children);
  FREE(tree->transitions);
  FREE(tree->traversed);
  FREE(tree->count);
  FREE(tree->symbols);
  FREE(tree);*/
  obstack_free(&(tree->nodeStack), NULL);
} 

void addSymbol(fsmTree_t tree, const Uchar sym) {
  tree->symbols[tree->totalSyms] = sym;
  tree->count[tree->totalSyms] = 1;
  tree->totalSyms++;
  tree->totalCount++;
}

/**
 * @param[in] tree tree to process.
 */
void makeFsm(fsmTree_t tree) {
  fsmTree_t *transitions;
  Uint i;

  CALLOC(transitions, fsmTree_t, alphasize);
  tree->used = True;
  verify(tree, tree, False);
  for (i=0; i<alphasize; i++) {
    transitions[i] = tree; /* ROOT */
  }
  propagateTransitions (transitions, tree);
  FREE(transitions);
}


/**
 * @param[in] tree tree to write.
 * @param[in] file output file to write the data.
 */
void writeFsmTree(const fsmTree_t tree, FILE *file) {
  Uint total, internal;
  SYMBOL s;
  long cost;

  pos = 0;
  
  internalNodes = internal = getInternalNodeCount(tree, 0);
  totalNodes = total = (internalNodes * alphasize) + 1;

  cost = bit_ftell_output(file);

  assert(internalNodes < 16383);
  s.scale = 16383;
  s.low_count = internalNodes;
  s.high_count = internalNodes + 1;
  encode_symbol(file, &s);
  /* DEBUGCODE(printf("%d %d %d\n", s.scale, s.low_count, s.high_count)); */
  
  if (internalNodes > 0) {
    writeFsmTreeRec(tree, 0, file);
  }
  printf("Representantion cost: %ld (internal: %d, total: %d)\n", 
	 bit_ftell_output(file) - cost, internal, total);
}


/** 
 * @param[in] tree tree node.
 * @returns True if the node is the root, False otherwise.
*/
BOOL isRootFsmTree(const fsmTree_t tree) {
  return tree->left == ROOT;
}

Uint getHeight (const fsmTree_t tree) {
  Uint i, max = 0, h;

  for (i=0; i<alphasize; i++) {
    if (tree->children[i]) {
      h = getHeight(tree->children[i]);
      if (h > max) max = h;
    }
  }

  return max + tree->right - tree->left + 1;
}

void printContext (fsmTree_t tree) {
  int i;
  
  while (tree->left != ROOT) {
    for (i=tree->left; i< tree->right; i++) {
      printf("%c", *(text + i));
      /*printf("%d", i);*/
    }
    if (tree->right == textlen) {
      printf("$\n");
    }
    else {
      printf("%c\n", *(text + tree->right));
      /*printf("%d\n", tree->right);*/
    }
    tree = tree->parent;
  }
  printf("root\n");
}

static int compareTreesRec (const fsmTree_t treeA, const fsmTree_t treeB, int level) {
  int i, minLevel, actLevel;

  if ((treeA == NULL && treeB != NULL) || (treeA != NULL && treeB == NULL)) {
    return level;
  }
  else if (treeA == NULL && treeB == NULL) {
    return -1;
  }
  else {
    minLevel = compareTreesRec(treeA->children[0], treeB->children[0], level+1);
    for (i=1; (i<alphasize) && (minLevel != level+1); i++) {
      actLevel = compareTreesRec(treeA->children[i], treeB->children[i], level+1);
      if ((actLevel > 0) && (minLevel > actLevel)) {
	minLevel = actLevel;
      }
    }

    return minLevel;
  }
}

void compareTrees (const fsmTree_t treeA, const fsmTree_t treeB) {
  int level = compareTreesRec(treeA, treeB, 0);
  printf("First different level = %d\n", level);
}

void copyStatistics (const fsmTree_t orig, fsmTree_t dest, const Uchar * text2) {
  int i;

  dest->totalSyms = orig->totalSyms;
  dest->totalCount = orig->totalCount;
  for (i=0; i<alphasize; i++) {
    dest->symbols[i] = orig->symbols[i];
    dest->count[i] = orig->count[i];
  }
  
  for (i=0; i<alphasize; i++) {
    if (orig->children[i] && dest->children[i]) {
      copyStatisticsRec(orig->children[i], 0, dest->children[i], 0, text2);
    }
  }
}

#ifdef DEBUG

/**
 * @param[in] tree tree to print.
 */
void printFsmTree(const fsmTree_t tree) {
  printRec(tree, 0);
}

#endif
