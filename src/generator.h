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
#include "symtable.h"

/**
 * @brief Entry point for code generation
 *
 * @param root AST root node
 * @param symTableArray Array of symbolic tables
 * @return int
 */
int generate_code(ASTNode* root, SymTable** symTableArray);


#endif
