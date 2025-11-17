/*
 * @file ast.c
 * @brief Abstract Syntax Tree (AST) implementation
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains the implementation of the Abstract Syntax Tree
 * (AST)
 */

#include "ast.h"
#include "strutils.h"

ASTNode *ast_create_node(ASTNodeType type, const char *value,
                         DataType data_type) {
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

  parent->children =
      realloc(parent->children, (parent->child_count + 1) * sizeof(ASTNode *));
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


// Helper function to convert ASTNodeType to string
const char *ast_node_type_to_string(ASTNodeType type) {
  switch (type) {
  case NODE_PROGRAM:
    return "NODE_PROGRAM";
  case NODE_BLOCK:
    return "NODE_BLOCK";
  case NODE_IF:
    return "NODE_IF";
  case NODE_WHILE:
    return "NODE_WHILE";
  case NODE_FUNCTION:
    return "NODE_FUNCTION";
  case NODE_RETURN:
    return "NODE_RETURN";
  case NODE_SETTER:
    return "NODE_SETTER";
  case NODE_GETTER:
    return "NODE_GETTER";
  case NODE_ASSIGN:
    return "NODE_ASSIGN";
  case NODE_BINOP:
    return "NODE_BINOP";
  case NODE_UNOP:
    return "NODE_UNOP";
  case NODE_CALL:
    return "NODE_CALL";
  case NODE_LITERAL:
    return "NODE_LITERAL";
  case NODE_ID:
    return "NODE_ID";
  case NODE_ARG_LIST:
    return "NODE_ARG_LIST";
  case NODE_PARAM_LIST:
    return "NODE_PARAM_LIST";
  case NODE_TERNARY:
    return "NODE_TERNARY";
  default:
    return "NODE_UNKNOWN";
  }
}

// Helper function to convert DataType to string
const char *data_type_to_string(DataType data_type) {
  switch (data_type) {
  case TYPE_UNKNOWN:
    return "TYPE_UNKNOWN";
  case TYPE_NULL:
    return "TYPE_NULL";
  case TYPE_BOOL:
    return "TYPE_BOOL";
  case TYPE_INT:
    return "TYPE_INT";
  case TYPE_FLOAT:
    return "TYPE_FLOAT";
  case TYPE_STRING:
    return "TYPE_STRING";
  default:
    return "TYPE_?";
  }
}

// Function to print the AST for debugging to a given FILE*
// When 'out' is NULL, defaults to stdout
void ast_fprint_debug(const ASTNode *root, FILE *out) {
  if (!root)
    return;
  if (!out)
    out = stdout;
  ast_fprint_debug_inner(root, out, "", true);
}

// Function to print the AST for debugging to stdout
void ast_print_debug_stdout(const ASTNode *root) {
  ast_fprint_debug(root, stdout);
}

// Function to print the AST for debugging to a file (returns 0 on success, -1 on error)
int ast_print_debug_to_file(const ASTNode *root, const char *filename) {
  if (!filename)
    return -1;
  FILE *f = fopen(filename, "w");
  if (!f)
    return -1;
  ast_fprint_debug(root, f);
  fclose(f);
  return 0;
}

// Recursive helper function for ast_fprint_debug
// Prints the AST in a tree-like structure
// Parameters:
//   node     - current AST node to print
//   out      - output FILE*
//   prefix   - string prefix for current level (for tree structure)
//   is_last  - whether this node is the last child of its parent
/*
  ASCII tree style:
        Root
        ├── Child1
        │   ├── Grandchild1
        │   └── Grandchild2
        └── Child2
             └── Grandchild3

  When is_last is true, use "└── " for the branch, otherwise "├── ".
  Prefix decides what to print before the branch (e.g., "│   " or "    ").
*/
void ast_fprint_debug_inner(const ASTNode *node, FILE *out,
                                   const char *prefix, bool is_last) {
  if (!node)
    return;

  // Print the prefix and branch
  if (prefix && prefix[0] != '\0') {
    fprintf(out, "%s", prefix);
  }

  // Print branch character
  if (is_last) {
    fprintf(out, "└── ");
  } else {
    fprintf(out, "├── ");
  }

  // Node header: type, optional value, and data type
  const char *node_name = ast_node_type_to_string(node->type);
  // Print node type and value if present
  if (node->value && node->value[0] != '\0') {
    fprintf(out, "%s (\"%s\") [", node_name, node->value);
  } else {
    fprintf(out, "%s [", node_name);
  }

  // Print data type
  const char *data_name = data_type_to_string(node->data_type);
  fprintf(out, "%s]", data_name);
  fprintf(out, "\n");

  // Calculate length for new prefix string
  // If this is the last child, add spaces; otherwise, add vertical line
  size_t new_pref_len = 0;
  const char *add = is_last ? " " : "│ ";
  if (prefix)
    new_pref_len = strlen(prefix) + strlen(add) + 1;
  // Allocate new prefix string
  char *new_prefix = (char *)malloc(new_pref_len);

  if (!new_prefix)
    return; // Memory allocation failed
  // Build new prefix string
  if (prefix)
    strcpy(new_prefix, prefix);
  else
    new_prefix[0] = '\0';
  strcat(new_prefix, add);

  // Recursively print child nodes
  for (size_t i = 0; i < node->child_count; ++i) {
    int last = (i == node->child_count - 1);
    ast_fprint_debug_inner(node->children[i], out, new_prefix, last);
  }
  // Free the allocated prefix string
  free(new_prefix);
}
