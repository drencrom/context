#ifndef DECODER_H
#define DECODER_H

#include "decoderTree.h"

/** Decodes the compressed file data to the output file (for non-binary alphabet). */
void decode (decoderTree_t, const Uint textlen, FILE *compressed_file, FILE *output, const BOOL useSee);

/** Decodes the compressed file data to the output file (for binary alphabet). */
void decodeBin (decoderTree_t, const Uint textlen, FILE *compressed_file, FILE *output);

#endif
