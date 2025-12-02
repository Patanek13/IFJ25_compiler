/**
 * @file strutils.c
 * @brief String utility functions implementation
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains implementations of string utility functions
 */

#include "strutils.h"

char *str_dup(const char *s) {
  if (s == NULL)
    return NULL;
  size_t len = strlen(s) + 1;
  char *d = malloc(len);
  if (d)
    memcpy(d, s, len);
  return d;
}
