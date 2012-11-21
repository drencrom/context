#ifndef STATISTICS_H
#define STATISTICS_H

#include "types.h"

/** Structure that saves statistics for each tree node in the encoder. */
typedef struct statistics {
  Uint *count; /**< List with the number of occurrences of each character in this state. */
  Uchar *symbols; /**< List of occuring symbols */
  Uint symbolCount; /**< Number of occuring symbols */
  double cost; /**< Cost assigned to this subtree. */
} *statistics_t;

/** Creates and initializes a new statistics structure instance. */
statistics_t allocStatistics();

/** Deletes a statistics structure instance. */
void freeStatistics(statistics_t);

/** Returns a new statistics structure, can be created or obtained from the buffer */
statistics_t getStatistics();

/** Puts a statistics structure that is no longer used into the buffer */
void returnStatistics(statistics_t);

/** Deletes all statistics stored in the buffer */
void freeBuffer();

#endif

