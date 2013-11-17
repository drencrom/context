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
