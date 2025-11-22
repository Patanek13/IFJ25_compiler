/**
 * @file generator.h
 * @author Sebastián Kuchta (xkuchts00)
 * @brief
 * @date 2025-11-09
 *
 *
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include "ast.h"
#include "error.h"
#include "stackframe.h"
#include "symtable.h"

typedef struct {
  int labelCounter;
} labelManager;

int getUniqueLabelId(labelManager* lm);

/**
 * @brief Entry point for code generation
 *
 * @param root AST root node
 * @param symTableArray Array of symbolic tables
 * @return ErrorCode
 */
ErrorCode generate_code(ASTNode* node, Frame* gf, FrameStack* fs);
ErrorCode generate_program(ASTNode* root, SymTable** symTableArray, bool debug);
#endif
