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
 
#ifndef SPACEDEF_H
#define SPACEDEF_H
#include <string.h>
#include "types.h"


/** 
 * Resizes a block of memory.
 * @param[in] S pointer to the memory to resize.
 * @param[in] T type of the objects that are stored in the memory to resize.
 * @param[in] N number of objects of type <i>T</i> that will be possible to store in the resized memory.
 * @returns a pointer to the resized memory.
 */
#define REALLOCSPACE(S,T,N)\
        (T *) realloc(S,sizeof(T) * (size_t) (N))


/** 
 * Resizes a block of memory and prints an error message if necessary.
 * @param[out] V pointer that will point to the resized memory.
 * @param[in] S pointer to the memory to resize.
 * @param[in] T type of the objects that are stored in the memory to resize.
 * @param[in] N number of objects of type <i>T</i> that will be possible to store in the resized memory.
 */
#define REALLOC(V,S,T,N)\
        V = REALLOCSPACE(S,T,N);\
        if((V) == NULL)\
        {\
          fprintf(stderr,"file %s, line %lu: realloc(%lu) failed\n",\
                  __FILE__,(Showuint) __LINE__,\
                  (Showuint) (sizeof(T) * (size_t) (N)));\
          exit(EXIT_FAILURE);\
        }


/** 
 * Frees a block of memory and makes the pointer NULL. 
 * @param[in] P pointer to the memory to free.
 */
#define FREE(P)\
        if((P) != NULL)\
        {\
          free(P);\
          P = NULL;\
        }


/** 
 * Allocs a new block of memory and sets all its bits to zero. 
 * @param[out] S pointer to the new allocated memory.
 * @param[in] T type of the objects that will be stored in this memory area.
 * @param[in] N number of objects of type <i>T</i> that will be possible to store in the new memory. 
 */
#define CALLOC(S,T,N)\
        S = calloc(sizeof(T), N);\
        if ((S) == NULL) {\
          fprintf(stderr,"file %s, line %lu: calloc(%lu) failed\n",\
                  __FILE__,(Showuint) __LINE__,\
                  (Showuint) (sizeof(T) * (size_t) (N)));\
          exit(EXIT_FAILURE);\
        }
        
#define MALLOC(S,T,N)\
        S = malloc(sizeof(T) * N);\
        if ((S) == NULL) {\
          fprintf(stderr,"file %s, line %lu: calloc(%lu) failed\n",\
                  __FILE__,(Showuint) __LINE__,\
                  (Showuint) (sizeof(T) * (size_t) (N)));\
          exit(EXIT_FAILURE);\
        }

#endif
