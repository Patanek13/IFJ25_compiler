/**
 * @file stackframe.c
 * @author Sebastián Kuchta (xkuchts00)
 * @brief Implementation of frame datatype as hashtable
 *
 */

#include "stackframe.h"
#include "error.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void F_init(Frame *frame, FrameType type) {
  if (frame != NULL) {
    frame->frameType = type;
    for (unsigned int i = 0; i < MAX_FRAME_SIZE; i++) {
      frame->entryTable[i] = NULL;
    }
  }
}

// DJBHash
unsigned int F_hash(char *id) {
  unsigned long hash = 5381;
  int c;
  while ((c = *id++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % MAX_FRAME_SIZE;
}

FrameEntry *F_lookup(Frame *frame, char *id) {
  unsigned int key = F_hash(id);
  if (frame != NULL && frame->entryTable[key] != NULL) {
    FrameEntry *item = frame->entryTable[key];
    while (item != NULL) { // prechadzanie synonymami
      if (strcmp(item->id, id) == 0) {
        return item;
      } else {
        item = item->next;
      }
    }
    return NULL;
  } else {
    return NULL;
  }
}

ErrorCode F_insert(Frame *frame, char *id, DataType type) {
  FrameEntry *item = F_lookup(frame, id);
  if (item != NULL) {
    item->type = type;
    return ERR_OK;
  } else if (frame != NULL) {
    unsigned int key = F_hash(id);
    item = malloc(sizeof(FrameEntry));
    if (item == NULL)
      return false;
    item->id = id;
    item->type = type;
    item->initialized = 0;
    item->next = NULL;
    if (frame->entryTable[key] != NULL)
      item->next = frame->entryTable[key];
    frame->entryTable[key] = item;
    return ERR_OK;
  }
  return ERR_INTERNAL;
}

void F_cleanup(Frame *frame) {
  if (frame != NULL) {
    for (int i = 0; i < MAX_FRAME_SIZE; i++) {
      FrameEntry *item = frame->entryTable[i];
      FrameEntry *nextItem = NULL;
      while (item != NULL) {
        nextItem = item->next;
        free(item);
        item = nextItem;
      }
      frame->entryTable[i] = NULL;
    }
  }
}
