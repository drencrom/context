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
