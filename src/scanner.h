#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

// Token types enum prototype
typedef enum {

} TokenType;

// Token structure prototype
typedef struct {
    TokenType type;
    char value[128];
} Token;

#endif // SCANNER_H