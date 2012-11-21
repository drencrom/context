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
