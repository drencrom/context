#ifndef SEE_H
#define SEE_H

#include "types.h"
#include "fsmTree.h"
#include "decoderTree.h"

/** SEE table structure */
Uint See[1<<13][2];

/** Returns the contents of the SEE table for the encoder */
int getSeeStateEncoder (fsmTree_t tree, Uint allCount, Uint pos, Uint numMasked, const Uchar * text, Uint alphasize);

/** Returns the contents of the SEE table for the encoder */
int getSeeStateDecoder (decoderTree_t tree, Uint allCount, Uint pos, Uint numMasked, const Uchar * text, Uint alphasize);

/** Updates the SEE table */
void updateSee (Uint state, BOOL escape, Uint alphasize);

/** Initalizes the SEE table */
void initSee ();

#endif
