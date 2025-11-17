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

//DJBHash
unsigned int F_hash(char* id) {
  unsigned long hash = 5381;
  int c;
  while ((c = *id++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % MAX_FRAME_SIZE;
}

FrameEntry* F_lookup(Frame* frame, char* id) {
  unsigned int key = F_hash(id);
  if (frame != NULL && frame->entryTable[key] != NULL) {
    return frame->entryTable[key];
  } else {
    return NULL;
  }
}

ErrorCode F_insert(Frame* frame, char* id, FrameData data) {
  unsigned int key = F_hash(id);
  if (frame == NULL || frame->entryTable[key] != NULL) {
    return ERR_INTERNAL;
  } else {
    FrameEntry* entry = malloc(sizeof(FrameEntry));
    entry->data = data;
    frame->entryTable[key] = entry;
    return ERR_OK;
  }
}



ErrorCode FS_init(FrameStack *fs) {
  if (fs != NULL) {
    fs->topIndex = -1;
    fs->stack = malloc(MAX_STACK_SIZE*sizeof(Frame));
    if (fs->stack == NULL) {
      return ERR_INTERNAL;
    } else {
      return ERR_OK;
    }
  } else {
    return ERR_INTERNAL;
  }
}

bool FS_IsEmpty(FrameStack *fs) {
  return (fs->topIndex == -1);
}

bool FS_IsFull(FrameStack *fs) {
  return (fs->topIndex == (MAX_STACK_SIZE-1));
}

Frame* FS_Pop(FrameStack* fs) {
  if (!FS_IsEmpty(fs)) {
    fs->topIndex--;
    return &fs->stack[(fs->topIndex++)];
  } else {
    return NULL;
  }
}

ErrorCode FS_Push(FrameStack *fs, Frame frame) {
  if (!FS_IsFull(fs)) {
    fs->topIndex++;
    fs->stack[fs->topIndex] = frame;
    return ERR_OK;
  } else {
    return ERR_INTERNAL;
  }
}

void FS_Dispose(FrameStack *fs) {
  free(fs->stack);
  fs->stack = NULL;
  fs->topIndex = -1;
}
