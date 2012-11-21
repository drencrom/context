#ifndef MAPFILE_H
#define MAPFILE_H

#include "types.h"

#ifdef WIN32

/** Opens a file and maps it into memory. Windows version. */
void * file2String(char *name, Uint *textlen, HANDLE *hndl);

/** Opens a file and maps it into memory. Windows version. */
void freetextspace(Uchar *text, HANDLE hndl);

#else

/** Opens a file and maps it into memory. UNIX version. */
void * file2String(char *name, Uint *textlen);

/** Frees the memory used by the mapping of a file. UNIX version. */ 
void freetextspace(Uchar *text, Uint textlen);

#endif /* WIN32 */

#endif
