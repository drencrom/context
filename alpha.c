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
