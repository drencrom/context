#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include "spacedef.h"
#include "debug.h"

/** 
 * Indicates if the stack is empty.
 * @returns True if the stack is not empty.
 */
#define NOTSTACKEMPTY (stacktop > 0)


/**
 * Pushes a new item to the stack. 
 * @param[in] N new item to add to the stack.
 */
#define PUSHNODE(N)\
        if(stacktop >= stackalloc)\
        {\
          stackalloc += 100;\
          REALLOC(stack,stack,Uint,stackalloc);\
        }\
        assert(stack != NULL);\
        stack[stacktop++] = N


/**
 * Pops an element out of the stack.
 * @param[out] N the retrieved element.
 */
#define POPNODE(N)\
        N = stack[--stacktop]

#endif
