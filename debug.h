#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

/** 
 * If debug is enabled executes the input code.
 * @param[in] S the code to execute.
 */
#define DEBUGCODE(S) S;

#else

#define DEBUGCODE(S) /* nothing */

#endif  /* DEBUG */

#endif
