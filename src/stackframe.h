/**
 * @file stackframe.h
 * @author Sebastián Kuchta (xkuchts00)
 * @brief
 * @date 2025-11-09
 *
 */

#ifndef STACKFRAME_H
#define STACKFRAME_H

#include "error.h"
#define MAX_FRAME_SIZE 127
#define MAX_STACK_SIZE 512
#include "ast.h"

typedef enum {
  GF,
  TF,
  LF
} FrameType;

typedef struct FrameEntry {
  char* id;
  DataType type;
  union {
    int i;
    float f;
    char *s;
    bool b;
  } data;
  int initialized;
  struct FrameEntry* next;
} FrameEntry;

typedef struct Frame {
  FrameType frameType;
  FrameEntry* entryTable[MAX_FRAME_SIZE];
} Frame;

typedef struct {
  Frame* stack;
  int topIndex;
} FrameStack;

void F_init(Frame* frame, FrameType type);
unsigned int F_hash(char* id);
FrameEntry* F_lookup(Frame* frame, char* id);
ErrorCode F_insert(Frame* frame, char* id, DataType type);
void F_cleanup(Frame* frame);

ErrorCode FS_init(FrameStack* fs);
bool FS_IsEmpty(FrameStack* fs);
bool FS_IsFull(FrameStack* fs);
Frame* FS_Pop(FrameStack* fs);
ErrorCode FS_Push(FrameStack* fs, Frame frame);
void FS_Dispose(FrameStack* fs);
Frame* FS_Top(FrameStack* fs);

#endif
