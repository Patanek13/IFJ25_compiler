/**
 * @file main.c
 * @author Sebastián Kuchta
 * @brief Main function with basic logic
 * @date 2025-10-01
 *
 */
#include "error.h"
#include "scanner.h"
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "semantic.h"
#include "ast.h"


int main(int argc, char** argv) {
  bool debug = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      debug = true;
    }
  }

  // Initialize scanner with standard input and error output
  scanner_init(stdin, stderr);

  ASTNode* ast_root = NULL;
  int error_code = ERR_OK;

  // Load first token for the parser
  token = get_token();

  if (valid() != ERR_OK) {
    fprintf(stderr, "ERROR: Validation of prolog (import...) failed.\n");
    error_code = SYNTAX_ERROR;
  } else {
    ast_root = program(&error_code);
  }

  // Check for errors during parsing
  if (error_code != ERR_OK) {
    if (debug) {
      fprintf(stderr, "DEBUG: Parsing failed with error code %d\n", error_code);
    }
    ast_free(ast_root);
    return error_code;
  }

  if (debug) {
    fprintf(stdout, "<AST representation>\n");
    ast_fprint_debug(ast_root, stdout);
  }

  // Semantic analysis
  // Code generation

  // Free AST
  ast_free(ast_root);

  if (debug) {
    fprintf(stdout, "Program ended successfully\n");
  }

  return ERR_OK; // Compilation successful
}
