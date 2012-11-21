#ifndef ENCODER_H
#define ENCODER_H

#include "fsmTree.h"

/** Encode the input data into the output file using the tree as model. */
void encode (fsmTree_t, FILE *, const Uchar *text, const Uint textlen, const BOOL useSee);

/** Encode the binary input data into the output file using the tree as model. */
void encodeBin (fsmTree_t, FILE *, const Uchar *text, const Uint textlen);

#endif
