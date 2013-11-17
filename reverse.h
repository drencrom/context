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
 
#ifndef REVERSE_H
#define REVERSE_H

/** Reverses a string without using extra memory. */
void reverseinplace(Uchar *s,Uint len);

/** Reverses a string and returns the reverse in a new string. */
void reversestring(Uchar *s, Uint len, Uchar *sreverse);

/** Reverses a string bit by bit and returns a new bit string with the reversed bits. */
void reverseString2Binary(Uchar *s, Uint len, Uchar *sreverse, Uint revLength);

#endif
