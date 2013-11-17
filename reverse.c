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
 
#include "types.h"

/**
 * @param[in] s string to reverse.
 * @param[in] len length of the string to reverse.
 */
void reverseinplace(Uchar *s,Uint len) {
  Uchar *front, *back, tmp;

  for(front = s, back = s + len - 1; front < back; front++, back--) {
    tmp = *front;
    *front = *back;
    *back = tmp;
  }
}


/**
 * @param[in] s string to reverse.
 * @param[in] len length of the string to reverse.
 * @param[out] sreverse reversed string.
 */
void reversestring(Uchar *s, Uint len, Uchar *sreverse) {
  Uchar *sp;
  Uint pos;

  for(pos = 0, sreverse += len-1, sp = s; pos < len; *sreverse-- = *sp++, pos++);
  sreverse++;
}


/**
 * @param[in] s string to reverse.
 * @param[in] len length of the string to reverse.
 * @param[out] sreverse reversed string.
 * @param[in] revLength length of the reversed string.
 */
void reverseString2Binary(Uchar *s, Uint len, Uchar *sreverse, Uint revLength) {
  char ch;
  int pos, i;

  for(pos = 0, sreverse += revLength-1; pos != len; s++, pos++){
    ch = *s;
    for (i = 0; i < 8; i++) {
      *sreverse-- = ((ch & 0x80) >> 7 ? '1' : '0');
      ch = ch << 1;
    }
  }
}
