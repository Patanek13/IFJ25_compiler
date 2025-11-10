/**
 * @file stackframe.h
 * @author Sebastián Kuchta (xkuchts00)
 * @brief
 * @date 2025-11-09
 *
 */

#ifndef STACKFRAME_H
#define STACKFRAME_H

#define MAX_FRAME_SIZE 127
#include "ast.h"

typedef enum {
  GF,
  TF,
  LF
} FrameType;

typedef struct FrameData {
  DataType type;
  union {
    int i;
    double f;
    char *s;
    bool b;
  } data;
} FrameData;

typedef struct FrameEntry {
  FrameData data;
  int initialized;
  struct FrameEntry* next;
} FrameEntry;

typedef struct Frame {
  FrameType FrameType;
  FrameEntry EntryTable[MAX_FRAME_SIZE];
} Frame;

unsigned int SF_hash(unsigned char* id);
FrameEntry* SF_lookup(Frame frame, char* id);

#endif
