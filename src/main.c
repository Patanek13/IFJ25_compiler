/**
 * @file main.c
 * @author Sebastián Kuchta (xkuchts00)
 * @author Patrik Lošťák (xlostap00)
 * @brief Main function with basic logic
 * @date 2025-10-01
 *
 */
#include "ast.h"
#include "error.h"
#include "generator.h"
#include "parser.h"
#include "scanner.h"
#include "semantic.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  bool debug = false;
  bool scan_test = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      debug = true;
    }
    if (strcmp(argv[i], "-s") == 0) {
      scan_test = true;
    }
  }

  // Scanner only test mode
  if (scan_test) {
    scanner_init(stdin, stdout);
    return parser_function(true);
  }

  // Parser initialization
  ASTNode *ast_root = NULL;
  int parse_error = ERR_OK;

  // Run the parser
  ast_root = run_parser(stdin, stderr, &parse_error);

  // Check for parsing errors
  if (ast_root == NULL) {
    if (debug) {
      fprintf(stderr, "Parsing failed with error code: %d\n", parse_error);
    }
    ast_free(ast_root);
    return parse_error; // Return the parsing error code
  } else {
    if (debug) {
      fprintf(stderr, "Parsing completed successfully.\n");
    }
  }

  // Semantic analysis
  int semantic_error = semantic_analysis(ast_root, debug);
  if (semantic_error != ERR_OK) {
    if (debug) {
      fprintf(stderr, "Semantic analysis failed with error code: %d\n", semantic_error);
    }
    ast_free(ast_root);
    return semantic_error; // Return the semantic error code
  } else {
    if (debug) {
      fprintf(stderr, "Semantic analysis completed successfully.\n");
    }
  }

  ErrorCode generatorError = generate_program(ast_root, false);
  fprintf(stderr, "Generator status: %i\n", generatorError);

  if (debug) {
    fprintf(stderr, "<AST representation>\n");
    ast_fprint_debug(ast_root, stderr);
  }

  // Free AST
  ast_free(ast_root);

  if (debug) {
    fprintf(stderr, "Program ended successfully\n");
  }

  return ERR_OK; // Compilation successful
}
