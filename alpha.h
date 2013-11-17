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
 
#ifndef ALPHA_H
#define ALPHA_H

#include "types.h"

/** Alphabet size. */
Uint alphasize;

/** Characters in text in alphabetical order. */
Uchar characters[UCHAR_MAX+1]; 

/** Index of each alphabet character */
Uint alphaindex[UCHAR_MAX+1]; 

/** 
 * Returns the index of the Nth character in the input text. Used in encoder. 
 * @param[in] N position of the character in the input text.
 * @return The index number of the input character.
*/
#define GETINDEX(N) (N == textlen ? alphasize : alphaindex[text[N]])

#define GETINDEX3(N) (N == textlen ? alphasize : alphaindex[text2[N]])

/** 
 * Returns the index of the Nth character in the input text. Used in decoder. 
 * @param[in] N position of the character in the input text.
 * @return The index number of the input character.
*/
#define GETINDEX2(N) (alphaindex[(*node->text)[N]])

/** Reads input text and initializes alphabet variables */
void buildAlpha(Uchar *text, const Uint textlen);

#endif
