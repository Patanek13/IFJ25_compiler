/**
 * @file stackframe.c
 * @author Sebastián Kuchta (xkuchts00)
 * @brief
 * @version 0.1
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 *
 */

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

