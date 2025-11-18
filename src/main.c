/**
 * @file main.c
 * @author Sebastián Kuchta
 * @author Patrik Lošťák (xlostap00)
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
    parser_function(true);
    return ERR_OK;
  }

  // Parser initialization
  ASTNode* ast_root = NULL;
  int parse_error = ERR_OK;

  // Run the parser
  ast_root = run_parser(stdin, stderr, &parse_error);

  // Check for parsing errors
  if (ast_root == NULL) {
    if (debug) {
      fprintf(stdout, "Parsing failed with error code: %d\n", parse_error);
    }
    ast_free(ast_root);
    return parse_error; // Return the parsing error code
  }

  // Semantic analysis
  int semantic_error = semantic_analysis(ast_root, debug);
  if (semantic_error != ERR_OK) {
    if (debug) {
      fprintf(stdout, "Semantic analysis failed with error code: %d\n", semantic_error);
    }
    ast_free(ast_root);
    return semantic_error; // Return the semantic error code
  }
  // Generate code would go here

  if (debug) {
    fprintf(stdout, "<AST representation>\n");
    ast_fprint_debug(ast_root, stdout);
  }


  // Free AST
  ast_free(ast_root);

  if (debug) {
    fprintf(stdout, "Program ended successfully\n");
  }

  return ERR_OK; // Compilation successful
}
