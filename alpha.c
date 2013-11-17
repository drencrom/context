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
 
#include "alpha.h"

void buildAlpha(Uchar *text, const Uint textlen) {
  Uint occ[UCHAR_MAX+1] = {0}, i, j;
  Uchar *tptr;

  for (tptr = text; tptr < text+textlen; tptr++)
  {
    occ[(Uint) *tptr]++;
  }
  for (j=0, i=0; i<=UCHAR_MAX; i++)
  {
    if(occ[i] > 0)
    {
      characters[j++] = (Uchar) i;
    }
  }
  alphasize = j;
  
  for(i=0; i<alphasize; i++)
  {
    alphaindex[(Uint) characters[i]] = i;
  }
}
