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
 
#ifndef SEE_H
#define SEE_H

#include "types.h"
#include "fsmTree.h"
#include "decoderTree.h"

/** SEE table structure */
Uint See[1<<14][2];

/** Returns the contents of the SEE table for the encoder */
int getSeeStateEncoder (fsmTree_t tree, Uint allCount, Uint pos, Uint numMasked, const Uchar * text, Uint alphasize);

/** Returns the contents of the SEE table for the encoder */
int getSeeStateDecoder (decoderTree_t tree, Uint allCount, Uint pos, Uint numMasked, const Uchar * text, Uint alphasize);

/** Updates the SEE table */
void updateSee (Uint state, BOOL escape, Uint alphasize);

/** Initalizes the SEE table */
void initSee ();

#endif
