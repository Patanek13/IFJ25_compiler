/**
 * @file generator.h
 * @author Sebastián Kuchta (xkuchts00)
 * @brief Header file for code generator
 *
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include "ast.h"
#include "error.h"

typedef struct {
  int labelCounter;
} labelManager;

/**
 * @brief Entry point for code generation, should receive AST from parser after semantic
    verification
 *
 * @param root Semantically verified AST root node
 * @param debug Toggle for debug mode
 * @return ErrorCode
 */
ErrorCode generate_program(ASTNode *root, bool debug);
#endif
