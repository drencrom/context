#include "reset.h"
#include "alpha.h"

Uint MAX_COUNT;

void setMaxCount() {
  MAX_COUNT = (alphasize <= 100 ? 450 : 200);
}
