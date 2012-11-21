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
