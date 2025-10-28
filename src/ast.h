/**
 * @file ast.h
 * @brief Abstract Syntax Tree (AST) definitions
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains the definitions and declarations for the Abstract Syntax Tree (AST)
 */

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ==== AST Node Types =====================
typedef enum {
    NODE_PROGRAM, // Root node of the program
    NODE_FUNCTION, // Function node (static foo(a,b){...})
    NODE_SETTER,   // Setter node
    NODE_GETTER,   // Getter node
    NODE_ID, // Name of variable or function
    NODE_BLOCK, // Block of statements {...}
    NODE_RETURN, // Return statement
    NODE_ASSIGN, // x = expr
    NODE_CALL, // Function call foo(a,b)
    NODE_BINOP, // Binary operation (+, -, *, /...)
    NODE_LITERAL, // Literal value (num, string, bool, null)
} ASTNodeType;


// ==== AST Node Structure =====================
typedef struct ASTNode {
    ASTNodeType type; // Type of the AST node
    struct ASTNode **children; // Array of pointers to child nodes
    size_t child_count; // Number of child nodes
    struct ASTNode *parent; // Pointer to parent node
} ASTNode;



#endif // AST_H
