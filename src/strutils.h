/*
  * @file strutils.h
  * @brief String utility functions
  * @author Patrik Lošťák (xlostap00)
  * @details This file contains declarations for string utility functions
*/

#ifndef IFJ_STRUTILS_H
#define IFJ_STRUTILS_H

#include <stdlib.h>
#include <string.h>

/*
 * @brief Duplicate a string (similar to strdup)
 * @param s Pointer to the source string
 * @return Pointer to the newly allocated duplicate string, or NULL on failure
 */
char *str_dup(const char *s);

#endif // IFJ_STRUTILS_H
