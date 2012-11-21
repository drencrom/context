#ifndef REVERSE_H
#define REVERSE_H

/** Reverses a string without using extra memory. */
void reverseinplace(Uchar *s,Uint len);

/** Reverses a string and returns the reverse in a new string. */
void reversestring(Uchar *s, Uint len, Uchar *sreverse);

/** Reverses a string bit by bit and returns a new bit string with the reversed bits. */
void reverseString2Binary(Uchar *s, Uint len, Uchar *sreverse, Uint revLength);

#endif
