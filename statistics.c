#include "statistics.h"
#include "alpha.h"
#include "spacedef.h"
#include "stack.h"

static Uint stacktop=0, stackalloc=0, *stack = NULL;

/**
 * @returns a new statistics instance.
 */
statistics_t allocStatistics() {
  statistics_t st;

  CALLOC(st, struct statistics, 1);
  CALLOC(st->count, Uint, alphasize);
  CALLOC(st->symbols, Uchar, alphasize);
  return st;
}


/**
 * @param[in] st statistics to delete.
 */
void freeStatistics(statistics_t st) {
  FREE(st->count);
  FREE(st->symbols);
  FREE(st);
}

/**
 * @returns a statistics instace
 */
statistics_t getStatistics() {
  Uint statsPtr;

  if (NOTSTACKEMPTY) {
    POPNODE(statsPtr);
    return (statistics_t)statsPtr;
  }
  else {
    return allocStatistics();
  }
}

/**
 * @param[in] st the statistics to return to the buffer
 */
void returnStatistics(statistics_t st) {
  memset(st->count, 0, alphasize * sizeof(Uint));
  memset(st->symbols, 0, alphasize * sizeof(Uchar));
  st->symbolCount = 0;
  st->cost = 0;
  PUSHNODE((Uint)st);
}

void freeBuffer() {
  Uint statsPtr;

  while (NOTSTACKEMPTY) {
    POPNODE(statsPtr);
    freeStatistics((statistics_t)statsPtr);
  }
  FREE(stack);
  stacktop = stackalloc = 0;
}
