/*
 * @file ast.c
 * @brief Abstract Syntax Tree (AST) implementation
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains the implementation of the Abstract Syntax Tree (AST)
 */

#include "ast.h"
#include "strutils.h"

ASTNode *ast_create_node(ASTNodeType type, const char *value, ASTDataType data_type) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        return NULL; // Memory allocation failed
    }
    node->type = type;
    node->data_type = data_type;

    if (value) {
        node->value = str_dup(value);
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

    // Print the data type of the node
    static const char *data_type_strings[] = {
        "UNKNOWN",
        "NULL",
        "BOOL",
        "INT",
        "FLOAT",
        "STRING"
    };


    // Type of node
    static const char *node_type_strings[] = {
        "PROGRAM",
        "FUNCTION",
        "SETTER",
        "GETTER",
        "ID",
        "BLOCK",
        "RETURN",
        "ASSIGN",
        "CALL",
        "BINOP",
        "LITERAL",
        "IF",
        "WHILE",
        "PARAM_LIST",
        "ARG_LIST",
        "UNOP",
        "TERNARY"
    };

    // Print node type and check enums bounds
    if (node->type < 0 || node->type >= sizeof(node_type_strings) / sizeof(node_type_strings[0])) {
        printf("UNKNOWN_NODE_TYPE");
    } else {
        printf("%s", node_type_strings[node->type]);
    }

    // Print the value of the node
    if (node->value) {
        printf(" (%s)", node->value);
    }

    // Print the data type of the node and check enums bounds
    if (node->data_type < 0 || node->data_type >= sizeof(data_type_strings) / sizeof(data_type_strings[0])) {
        printf(" [UNKNOWN_DATA_TYPE]");
    } else {
        printf(" [%s]", data_type_strings[node->data_type]);
    }
    
    printf("\n");

    // Recursively print child nodes
    for (size_t idx = 0; idx < node->child_count; idx++) {
        ast_print_debug(node->children[idx], depth + 1);
    }
}


