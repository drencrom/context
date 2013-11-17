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
