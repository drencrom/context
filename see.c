#include <assert.h>
#include "see.h"
#include "debug.h"
#include <math.h> /* for log */

static int get3ByteRepresentation (const Uint num, const Uint alphasize) {
  if (num <= 0) return 0;
  else if (num <= 1) return 1;
  else if (num <= 2) return 2;
  else if (num <= 4) return 3;
  else if (num <= (alphasize >= 150 ? 8 : 6)) return 4; 
  else if (num <= 9) return 5;
  else if (num <= (alphasize >= 150 ? 25 : 15)) return 6; 
  else return 7;
}

/**
 * @param[in] tree current node of the tree
 * @param[in] allCount total number of symbols observed in this state 
 * @param[in] pos position of the current character in the input text
 * @param[in] numMasked number of masked characters in the current state
 * @param[in] text input text
 * @param[in] alphasize text alphabet size
 * @returns 
 */
int getSeeStateEncoder (fsmTree_t tree, Uint allCount, Uint pos, Uint numMasked, const Uchar * text, Uint alphasize) {
  Uint state, syms; 

  if ((tree->totalCount - allCount > 3) || (allCount >= (alphasize >= 150 ? 128 : 30))) {
    return -1;
  }

  state = tree->totalCount - allCount; /* # escapes */

  state <<= 3;
  state |= get3ByteRepresentation(allCount, alphasize);

  state <<= 3;
  state |= get3ByteRepresentation(numMasked, alphasize);

  /*state <<= 3;
    state |= get3ByteRepresentation(tree->totalSyms, alphasize);*/

  syms = tree->totalSyms;
  /************/
  if (alphasize < 150) {
    while (!isRootFsmTree(tree) && tree->totalSyms == syms) {
      tree = tree->parent->origin;
    }
    state <<=2;
    if (tree->totalSyms > 3) {
      state |= 3;
    }
    else {
      state |= tree->totalSyms;
    }
  }
  /************/

  state <<=3;
  state |= (int)(log(text[pos-1])+0.5);

  return state;
}

int getSeeStateDecoder (decoderTree_t tree, Uint allCount, Uint pos, Uint numMasked, const Uchar * text, Uint alphasize) {
  Uint state, syms;

  if ((tree->totalCount - allCount > 3) || (allCount >= (alphasize >= 150 ? 128 : 30))) {
    return -1;
  }

  state = tree->totalCount - allCount;

  state <<= 3;
  state |= get3ByteRepresentation(allCount, alphasize);

  state <<=3;
  state |= get3ByteRepresentation(numMasked, alphasize);
  
  syms = tree->totalSyms;
  /************/
  if (alphasize < 150) {
    while (!isRootDecoderTree(tree) && tree->totalSyms == syms) {
      tree = tree->parent->origin;
    }
    state <<=2;
    if (tree->totalSyms > 3) {
      state |= 3;
    }
    else {
      state |= tree->totalSyms;
    }
  }
  /************/

  state <<=3;
  state |= (int)(log(text[pos-1])+0.5);

  return state;
}
  
void updateSee (Uint state, BOOL escape, Uint alphasize) {
  if (escape) {
    See[state][0] += 17;
    See[state][1] += 18;
  }
  else {
    if (See[state][0] >= (alphasize >= 150 ? 500 : 4000)) {
      See[state][0] = (See[state][0] >> 1) + 1;
      See[state][1] = (See[state][1] >> 1) + 1;
    }
    See[state][1] += (alphasize >= 150 ? 16 : 17);
  }

  if (See[state][1] >= (alphasize >= 150 ? 800 : 8000)) {
    See[state][0] = (See[state][0] >> 1) + 1;
    See[state][1] = (See[state][1] >> 1) + 1;
  }
}

void initSee () {
  Uint allCount, allScaled, state, i;

  for (allScaled = 0; allScaled < 4; allScaled++) {
    switch(allScaled) {
    case 0: 
      allCount = 20;
      break;
    case 1: 
      allCount = 30;
      break;
    case 2:
      allCount = 40;
      break;
    case 3: 
      allCount = 50;
      break;
    default:
      allCount = 0;
      break;
    }

    state = allScaled << 11;
    for(i=0; i<(1<<11); i++) {
      See[state][0] =  20;
      See[state][1] = allCount + 20;
      state++;
    }
  }
}
