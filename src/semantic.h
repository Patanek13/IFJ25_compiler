/**
 * @file semantic.h
 * @author Patrik Lošťák (xlostap00)
 * @brief Semantic Analysis header file
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "error.h"
#include "strutils.h"
#include "symtable.h"

/**
 * @brief Performs semantic analysis on the given AST.
 * @param root Pointer to the root ASTNode of the program.
 * @param debug Boolean flag to enable debug output.
 * @return Integer indicating success or specific semantic error code.
 */
int semantic_analysis(ASTNode *root, bool debug);

#endif // SEMANTIC_H
