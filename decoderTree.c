#include <assert.h>
#include "spacedef.h"
#include "debug.h"
#include "decoderTree.h"
#include "alpha.h"
#include "arithmetic/coder.h"
#include "arithmetic/bitio.h"
#ifndef WIN32
#include <obstack.h>
#endif

/** Flag to indicate a node is the root. */
#define ROOT -1

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

/** Number of internal nodes in the tree. Equals the number of ones in the natural code file. */
static Ushort internalNodes;

/** Total number of nodes in the tree. */
static Uint totalNodes;

#ifndef WIN32
static struct obstack nodeStack;
#endif

static void  updateChildrenTransitions(decoderTree_t node, Uint cidx, decoderTree_t origTrans, decoderTree_t newTrans) {
  Uint i;

  for (i=0; i<alphasize; i++) {
    if (node->children[i]) {
      if (node->children[i]->transitions[cidx] == origTrans) {
	node->children[i]->transitions[cidx] = newTrans;
      }
      updateChildrenTransitions(node->children[i], cidx, origTrans, newTrans);
    }
  }
}

/** 
 * Calculates the canonical decomposition of a string in a faster way. It is only possible to use this variant in some special cases.
 * @param[in] tree node from where to start the search.
 * @param[in] xLeft left index in the input data of the string to canonize.
 * @param[in] xRight right index in the input data of the string to canonize.
 * @param[in] node tree node with a pointer to the data string. Used by GETINDEX2.
 * @param[out] r node whose label is the longest prefix of the string in the tree.
 * @param[out] uLeft index of the leftmost character of the string <i>v</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] uRight index of the leftmost character of the string <i>v</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] vLeft index of the leftmost character of the remaining part of the string (after <i>ru</i>).
 * @param[out] vRight index of the rightmost character of the remaining part of the string (after <i>ru</i>).
 */
static void fastCanonize(decoderTree_t tree, Uint xLeft, Uint xRight, decoderTree_t node, decoderTree_t *r, Uint *uLeft, Uint *uRight, Uint *vLeft, Uint *vRight) {
  BOOL end;
  decoderTree_t child = NULL;

  *vLeft = 1;
  *vRight = 0; /* v is empty */

  if (tree->left != ROOT) {
    xLeft += tree->right + 1;
  }
  end = xLeft > xRight;

  while (!end) {
    child = tree->children[GETINDEX2(xLeft)];
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
  if (xLeft <= xRight) {
    *uLeft = xLeft;
    *uRight = xRight;
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
 * @param[in] node tree node with a pointer to the data string. Used by GETINDEX2.
 * @param[out] r node whose label is the longest prefix of the string in the tree.
 * @param[out] uLeft index of the leftmost character of the string <i>u</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] uRight index of the rightmost character of the string <i>u</i> in the input data. 
 * <i>ru</i> is the longest prefix of the string which is a word of the tree.
 * @param[out] vLeft index of the leftmost character of the remaining part of the string (after <i>ru</i>).
 * @param[out] vRight index of the rightmost character of the remaining part of the string (after <i>ru</i>).
 */
static void canonize(decoderTree_t tree, Uint xLeft, Uint xRight, decoderTree_t node, decoderTree_t *r, Uint *uLeft, Uint *uRight, Uint *vLeft, Uint *vRight) {
  decoderTree_t child;
  BOOL end = False;
  Uint i, xLeftStart;

  if (xLeft > xRight) {
    *uLeft = 1;
    *uRight = 0; /* u is empty */
    end = True;
    /* FIXME: es necesario esto? */
  }
  else if (tree->left != ROOT) {
    xLeft += tree->right + 1;
  }
  
  while (!end) {
    child = tree->children[GETINDEX2(xLeft)];
    if (child) { /* there is an edge in the direction of xLeft */
      xLeftStart = xLeft;
      for (i=child->left; (i<=child->right) && (xLeft<=xRight) && ((*child->text)[i] == (*node->text)[xLeft]); i++, xLeft++); 
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
	*uLeft = xLeftStart;
	*uRight = xLeft-1;
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
 * Inserts a new node in the FSM closure of the tree.
 * @param[in] r parent of the new node to be added.
 * @param[in] node tree node with a pointer to the data string. Used by GETINDEX2.
 * @param[in] uLeft index of the leftmost character of the <i>u</i> string.
 * @param[in] uRight index of the rightmost character of the <i>u</i> string.
 * @param[in] vLeft index of the leftmost character of the <i>v</i> string.
 * @param[in] vRight index of the rightmost character of the <i>v</i> string.
 * @param[in] decoder flag to indicate insert is called from the decoder routine
 * @returns a pointer the new added node.
 */
static decoderTree_t insert (decoderTree_t r, decoderTree_t node, Uint uLeft, Uint uRight, Uint vLeft, Uint vRight, BOOL decoder) {
  decoderTree_t new  = initDecoderTree(False), newLeaf, ret, child;
  BOOL leaf;
  Uint i,j, cidx = GETINDEX2(0);

  DEBUGCODE(printf("New node1: %p\n", (void *)new));
  if (uLeft > uRight) {
    for (i=0; i<alphasize && !r->children[i]; i++);
    leaf = i == alphasize;

    if (r->internal && vRight > vLeft) { /* if a non atomical node is added we have to insert the leaf of T(x) which is the parent of this node */
      newLeaf = initDecoderTree(False);
      DEBUGCODE(printf("New node3: %p\n", (void *)newLeaf));
      newLeaf->left = newLeaf->right = (r->right != ROOT ? r->right + 1 : 0);
      newLeaf->internal = False;
      newLeaf->internalFSM = !decoder;

      if (leaf && (r->left != ROOT)) {
	newLeaf->text = r->text;
	REALLOC(*newLeaf->text, *newLeaf->text, Uchar, newLeaf->right + 1);
      }
      else {
	MALLOC(newLeaf->text, Uchar *, 1);
	CALLOC(*newLeaf->text,Uchar, newLeaf->right + 1);
	if (newLeaf->left > 0) {
	  memcpy(*newLeaf->text, *r->text, newLeaf->left); 
	}
      }
      (*newLeaf->text)[newLeaf->left] = (*node->text)[vLeft];
      r->children[GETINDEX2(vLeft)] = newLeaf;
      newLeaf->parent = r;
      newLeaf->origin = newLeaf;

      if (decoder) {
	for (i=0; i<alphasize; i++) {
	  if (i != cidx) {
	    newLeaf->transitions[i] = r->transitions[i];
	  }
	}
      }
      vLeft++;
      leaf = True;
      r = newLeaf;
    }
 
    new->left = (r->right != ROOT ? r->right + 1 : 0);
    new->right = new->left + vRight - vLeft;
    new->internal = False;  
    new->internalFSM = !decoder;  

    if (leaf && (r->left != ROOT)) {
      new->text = r->text;
      REALLOC(*new->text, *new->text, Uchar, new->right + 1);
    }
    else {
      MALLOC(new->text, Uchar *, 1);
      CALLOC(*new->text,Uchar, new->right + 1);
      /*printf("left: %d, right: %d isRoot: %d\n", new->left, new->right, isRootDecoderTree(r));*/
      if (new->left > 0) {
        memcpy(*new->text, *r->text, new->left); /* new->left = r->right + 1 */
      }
    }
    for (i=vLeft, j=new->left; i<=vRight; i++, j++) {
      (*new->text)[j] = (*node->text)[i];
    }
    r->children[GETINDEX2(vLeft)] = new;
    new->parent = r;

    if (r->internal/* && decoder*/) {
      new->origin = new;
    }
    else {
      new->origin = r->origin;
    }

    if (decoder) {
      for (i=0; i<alphasize; i++) {
	if (i != cidx) {
	  new->transitions[i] = r->transitions[i];
	}
      }
    }

    ret = new;
  }
  else {
    /* split */
    new->left = (r->right != ROOT ? r->right + 1 : 0);
    new->right = new->left + uRight - uLeft;

    child = r->children[GETINDEX2(uLeft)];
    new->internal = child->internal;
    new->internalFSM = !decoder || child->internalFSM;
    
    new->children[alphaindex[(*child->text)[new->right+1]]] = child;
    child->left = new->right + 1;
    child->parent = new; 
    new->text = child->text;

    new->parent = r; 
    r->children[GETINDEX2(uLeft)] = new;

    /*if (!decoder) {
      new->origin = r->origin;
    }
    else*/ if (r->internal) {
      new->origin = new;
    }
    else {
      new->origin = child->origin;
    }

    if (decoder) {
      new->totalSyms = child->totalSyms;
      new->totalCount = 0;
      for (i=0; i<alphasize; i++) {
	if (i != cidx) {
	  new->transitions[i] = r->transitions[i];
	}

	if (i < new->totalSyms) {
	  new->count[i] = (child->count[i] == 0 ? 0 : 1);
	  new->totalCount += new->count[i];
	  new->symbols[i] = child->symbols[i];
        }
      }
      new->totalCount *= 2; /* symbols and escapes */
    }

    new->traversed[alphaindex[(*new->text)[new->right+1]]] = r->traversed[GETINDEX2(uLeft)];

    if (vLeft <= vRight) {

      /******/
      if (new->internal && vRight > vLeft) { /* if a non atomical node is added we have to insert the leaf of T(x) which is the parent of this node */
	newLeaf = initDecoderTree(False);
	DEBUGCODE(printf("New node6: %p\n", (void *)newLeaf));
	newLeaf->left = newLeaf->right = (new->right != ROOT ? new->right + 1 : 0);
	newLeaf->internal = False;
	newLeaf->internalFSM = !decoder;

	MALLOC(newLeaf->text, Uchar *, 1);
	CALLOC(*newLeaf->text,Uchar, newLeaf->right + 1);
	if (newLeaf->left > 0) {
	  memcpy(*newLeaf->text, *new->text, newLeaf->left); 
	}

	(*newLeaf->text)[newLeaf->left] = (*node->text)[vLeft];
	new->children[GETINDEX2(vLeft)] = newLeaf;
	newLeaf->parent = new;
	newLeaf->origin = newLeaf;

	if (decoder) {
	  for (i=0; i<alphasize; i++) {
	    if (i != cidx) {
	      newLeaf->transitions[i] = r->transitions[i];
	    }
	  }
	}
	vLeft++;
	new = newLeaf;
      }
      /******/

      newLeaf = initDecoderTree(False);
      DEBUGCODE(printf("New node2: %p\n", (void *)newLeaf));
      /* add */
      newLeaf->internal = False;
      newLeaf->internalFSM = !decoder;
      newLeaf->left = new->right + 1;
      newLeaf->right = newLeaf->left + vRight - vLeft;
      newLeaf->parent = new; 
      new->children[GETINDEX2(vLeft)] = newLeaf;

      MALLOC(newLeaf->text, Uchar *, 1);
      CALLOC(*newLeaf->text,Uchar, newLeaf->right + 1);
      memcpy(*newLeaf->text, *new->text, newLeaf->left);
      for (i=vLeft, j=newLeaf->left; i<=vRight; i++, j++) {
	(*newLeaf->text)[j] = (*node->text)[i];
      }

      if (new->internal) {
	newLeaf->origin = newLeaf;
      }
      else {
	newLeaf->origin = new->origin;
      }

      if (decoder) {
	for (i=0; i<alphasize; i++) {
	  if (i != cidx) {
	    newLeaf->transitions[i] = new->transitions[i];
	  }
	}
      }

      ret = newLeaf;
    }
    else {
      ret = new;
    }
  }
  
  return ret; 
}


/**
 * Verifies that the tail of <i>node</i> is in the tree, adding it if necessary and continuing recursively on the children of <i>root</i>.
 * @param[in] root node to use as the start point in the search of <i>canonize</i>.
 * @param[in] node node to verify.
 * @param[in] fast flag to indicate that it is possible to use <i>fastCanonize</i> in this invocation of <i>verify</i>.
 * @param[in] decoder flag to indicate verify is called from the decoder routine
 */
static void verify(const decoderTree_t root, decoderTree_t node, BOOL fast, BOOL decoder) {
  Uint xLeft, uLeft, uRight, vLeft, vRight, i, cidx;
  decoderTree_t r, x;

  if (node->left != ROOT) {
    cidx = GETINDEX2(0);
    xLeft = 1;
    
    if (fast) {
      fastCanonize(root, xLeft, node->right, node, &r, &uLeft, &uRight, &vLeft, &vRight);
    } 
    else {
      canonize(root, xLeft, node->right, node, &r, &uLeft, &uRight, &vLeft, &vRight);
    }

    if ((uLeft <= uRight) || (vLeft <= vRight)) {
      x = insert(r, node, uLeft, uRight, vLeft, vRight, decoder);
      if (uLeft <= uRight) {
	if (r->traversed[GETINDEX2(uLeft)]) {
	  verify((r->left == ROOT ? r : r->tail), r->children[GETINDEX2(uLeft)], True, decoder);
	}
      }
      else if (r->traversed[GETINDEX2(vLeft)]) {
	verify((r->left == ROOT ? r : r->tail), r->children[GETINDEX2(vLeft)], False, decoder);
      }
    }
    else { /* the node already exists */
      x = r;
    }
    node->tail = x;

    /*if (decoder) {
      updateChildrenTransitions(x, cidx, x->transitions[cidx], node);
      }*/
    x->transitions[cidx] = node;
  } 
  
  for (i=0; i<alphasize; i++) {
    if (!node->traversed[i]) {
      node->traversed[i] = True;
      if (node->children[i]) {
	verify((node->left == ROOT ? node : node->tail), node->children[i], False, decoder);
      }
    }
  }  
}

void verifyDecoder(const decoderTree_t root, decoderTree_t node) {
  verify(root, node, False, True);
}

/**
 * Adds to the FSM a set of transitions originating from <i>tree</i> if thew were not defined by <i>verify</i>.
 * @param[in] transitions set of transitions.
 * @param[in] tree origin node of the transitions.
 */
static void propagateTransitions(decoderTree_t *transitions, decoderTree_t tree) {
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
 * Reads a bit from the file using an arithmetic decoder. Each bit indicates if a node of the tree is internal (True) or a leaf (False).
 * @param[in] file input file pointer.
 * @returns the readed bit.
 */
static BOOL readEncoder(FILE *file) {
  SYMBOL s;
  Uint count, totalN = totalNodes, internalN = internalNodes, shift = 0;
  BOOL internal;
  
  if (totalNodes == internalNodes) {
    return True;
  }
  else if (internalNodes == 0) {
    return False;
  }
  else {
    while ((totalN & 0xFFFFC000) != 0) { /* some of the most significative 18 bits on */
      totalN >>= 1;
      shift++;
    }

    if (shift > 0) {
      internalN >>= shift;
      totalN = (internalN * alphasize) + 1;
    }

    s.scale = totalN;
    count = get_current_count(&s);
    internal = count < internalN;

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
    remove_symbol_from_stream(file, &s);

    /* printf("low:%d high:%d scale:%d symbol:%d\n", s.low_count, s.high_count, s.scale, internal); */
    return internal;
  }
}


/**
 * Reads a decoder tree from its encoded representation on a file.
 * @param[in] t root of the tree.
 * @param[in] file input file pointer.
 */
static int readDecoderTreeRec (decoderTree_t t, FILE *file) {
  Uint i;
  int onlyChild = -1 /* no children */, childStatus;
  decoderTree_t child;
  BOOL internal;

  for (i=0; i<alphasize; i++) {
    internal = readEncoder(file);
    if (internal) {
      if (onlyChild == -1) {
	onlyChild = i; /* one child */
      }
      else if (onlyChild >= 0) {
	onlyChild = -2; /* more than one children */
      }
      /*onlyChild = -2;*/
      
      t->children[i] = initDecoderTree(True);
      child = t->children[i];
      child->parent = t;
      child->internal = True;
      child->internalFSM = True;
      if (t->left == ROOT) {
	child->left = child->right = 0;
	MALLOC(child->text, Uchar *, 1);
	CALLOC(*child->text, Uchar, 1);
	(*child->text)[0] = characters[i];
      }
      else {
	child->left = child->right = t->left + 1;
	/*if (i == 0) {*/
	if (onlyChild >= 0) { /* first child */
	  child->text = t->text;
	  REALLOC(*child->text, *child->text, Uchar, child->left + 1);
	}
	else {
	  MALLOC(child->text, Uchar *, 1);
	  CALLOC(*child->text, Uchar, child->left + 1);
	  memcpy(*child->text, *t->text, child->left);
	}
	(*child->text)[child->left] = characters[i];
      }
      childStatus = readDecoderTreeRec(child, file);
      if (childStatus >= 0) { /* this child has outgoing degree = 1 */
	child->children[childStatus]->left = child->left;
	t->children[i] = child->children[childStatus];
	t->children[i]->parent = t;
	freeDecoderTree(child, child->children[childStatus]->text != child->text);
      }
    }
  }
  return onlyChild;
}

#ifdef DEBUG

/**
 * Auxiliary recursive function to print a tree to the standard output
 * @param[in] tree the node of the tree to print.
 * @param[in] level depth of the node.
 */
static void printRec(const decoderTree_t tree, int level) {
  int i;

  if (level > 0) {
    for (i=1; i<=level; i++) {
      printf("-");
    } 

    printf("%ld - %ld (%p) parent: %p orig: %p\n", tree->left, tree->right, (void*)tree, (void*)tree->parent, (void*)tree->origin); 
    for (i=0; i<tree->right; i++) {
      printf("%d-", *(*(tree->text) + i));
    }
    printf("%d\n", *(*(tree->text) + tree->right));

    /*printf("  %d %d (%p)\n", tree->left, tree->right, (void*)tree);
      printf(" %d\n", **tree->text);*/
    /*for (i=0; i<tree->left; i++) {
      printf("%c", (*tree->text)[i]);
    }
    printf("%c\n", (*tree->text)[tree->left]);*/
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

void initDecoderTreeStack() {
#ifndef WIN32
  obstack_init (&nodeStack);
  if (obstack_chunk_size (&nodeStack) < 16384) {
    obstack_chunk_size (&nodeStack) = 16384;
  }
#endif
}


/**
 * @returns a new decoder tree instance. 
 */
decoderTree_t initDecoderTree(BOOL useMalloc) {
  decoderTree_t ret;

  if (useMalloc) {
    CALLOC(ret, struct decoderTree, 1);
    CALLOC(ret->children, struct decoderTree *, alphasize);
    CALLOC(ret->transitions, struct decoderTree *, alphasize);
    CALLOC(ret->traversed, BOOL, alphasize);
    MALLOC(ret->count, Uint, alphasize); 
    MALLOC(ret->symbols, Uchar, alphasize);
  }
  else {
#ifndef WIN32
    ret = (decoderTree_t)obstack_alloc(&nodeStack, sizeof(struct decoderTree));
    memset(ret, 0, sizeof(struct decoderTree));

    ret->children = (decoderTree_t *)obstack_alloc(&nodeStack, sizeof(struct decoderTree *) * alphasize);
    memset(ret->children, 0, sizeof(struct decoderTree *) * alphasize);

    ret->transitions = (decoderTree_t *)obstack_alloc(&nodeStack, sizeof(struct decoderTree *) * alphasize);
    memset(ret->transitions, 0, sizeof(struct decoderTree *) * alphasize);

    ret->traversed = (BOOL *)obstack_alloc(&nodeStack, sizeof(BOOL) * alphasize);
    memset(ret->traversed, 0, sizeof(BOOL) * alphasize);

    ret->count = (Uint *)obstack_alloc(&nodeStack, sizeof(Uint) * alphasize);
    ret->symbols = (Uchar *)obstack_alloc(&nodeStack, sizeof(Uchar) * alphasize);
#else
    CALLOC(ret, struct decoderTree, 1);
    CALLOC(ret->children, struct decoderTree *, alphasize);
    CALLOC(ret->transitions, struct decoderTree *, alphasize);
    CALLOC(ret->traversed, BOOL, alphasize);
    MALLOC(ret->count, Uint, alphasize); 
    MALLOC(ret->symbols, Uchar, alphasize);
#endif
  }

  /* not always true but makes sense as init */
  ret->left = ROOT;
  ret->right = ROOT;
  ret->origin = ret;
  ret->used = False;

  return ret;
}


/**
 * @param[in,out] tree the tree to delete. 
 * @param[in] deleteText if the string pointed by this node must be deleted
 */
void freeDecoderTree(decoderTree_t tree, BOOL deleteText) {
  FREE(tree->children);
  FREE(tree->transitions);
  FREE(tree->traversed);
  FREE(tree->count);
  FREE(tree->symbols);
  if (deleteText) {
    FREE(*(tree->text));
    FREE(tree->text);
  }
  FREE(tree);
}


/**
 * @param[in] tree the tree to process.
 */
void makeDecoderFsm(decoderTree_t tree) {
  /* decoderTree_t transitions [alphasize]; */
  decoderTree_t *transitions;
  Uint i;

  CALLOC(transitions, decoderTree_t, alphasize);
  verify(tree, tree, False, False);
  for (i=0; i<alphasize; i++) {
    transitions[i] = tree; /* ROOT */
  }
  propagateTransitions (transitions, tree);
  FREE(transitions);
}


/**
 * @param[in] file file to read the tree from.
 * @returns a new decoder tree.
 */
decoderTree_t readDecoderTree(FILE *file) {
  decoderTree_t ret;
  SYMBOL s;

  ret = initDecoderTree(True); /* ROOT */
  ret->internal = True;
  ret->internalFSM = True;
  ret->used = True;

  s.scale = 16383;
  internalNodes = get_current_count(&s);
  s.low_count = internalNodes;
  s.high_count = internalNodes + 1;
  remove_symbol_from_stream(file, &s);
  /* DEBUGCODE(printf("%d %d %d\n", s.scale, s.low_count, s.high_count)); */
    
  totalNodes = (internalNodes * alphasize) + 1;
  printf("Nodes: internal %d total %ld\n", internalNodes, totalNodes);

  if (internalNodes > 0) {
    /*FIXME: nunca guardar el nodo root*/
    readEncoder(file); /* leo el root */
    readDecoderTreeRec(ret, file);
  }
  else {
    ret->internalFSM = False;
  }

  return ret;
}


/** 
 * @param[in] tree tree node.
 * @returns True if the node is the root, False otherwise.
*/
BOOL isRootDecoderTree(decoderTree_t tree) {
  return tree->left == ROOT;
}


#ifdef DEBUG

/**
 * @param[in] tree tree to print.
 */
void printDecoderTree(decoderTree_t tree) {
  printRec(tree, 0);
}

#endif
