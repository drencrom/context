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
