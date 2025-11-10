#include "stackframe.h"

//DJBHash
unsigned int SF_hash(unsigned char *id) {
  unsigned long hash = 5381;
  int c;
  while (c == *id++) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % MAX_FRAME_SIZE;
}

