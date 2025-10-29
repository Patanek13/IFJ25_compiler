/*
 * @file ast.c
 * @brief Abstract Syntax Tree (AST) implementation
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains the implementation of the Abstract Syntax Tree (AST)
 */

#include "ast.h"
// For strdup because C17 does not guarantee its declaration in string.h
extern char* strdup(const char*);

ASTNode *ast_create_node(ASTNodeType type, const char *value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        return NULL; // Memory allocation failed
    }
    node->type = type;
    if (value) {
        node->value = strdup(value);
        if (!node->value) {
            free(node);
            return NULL; // Memory allocation failed
        }
    } else {
        node->value = NULL;
    }
    node->children = NULL;
    node->child_count = 0;
    node->parent = NULL;
    return node;
}


void ast_add_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) {
        return; // Invalid input
    }

    parent->children = realloc(parent->children, (parent->child_count + 1) * sizeof(ASTNode *));
    if (!parent->children) {
        return; // Memory allocation failed
    }

    parent->children[parent->child_count] = child;
    parent->child_count++;
    child->parent = parent;
}

void ast_free(ASTNode *root) {
    if (!root) {
        return; // Nothing to free
    }

    // Recursively free child nodes
    for (size_t idx = 0; idx < root->child_count; idx++) {
        ast_free(root->children[idx]);
    }

    // Free the children array
    free(root->children);

    // Free the node's value because of strdup
    free(root->value);

    // Free the root node
    free(root);
}


void ast_print_debug(const ASTNode *node, int depth) {
    if (!node) {
        return;
    }

    // Indentation for better look
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    // Print node information
    printf("Node Type: %d", node->type);
    if (node->value) {
        printf(", Value: %s", node->value);
    }
    printf("\n");

    // Recursively print child nodes
    for (size_t idx = 0; idx < node->child_count; idx++) {
        ast_print_debug(node->children[idx], depth + 1);
    }
}


