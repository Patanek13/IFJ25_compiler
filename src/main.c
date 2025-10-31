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

int main(int argc, char** argv) {
  bool debug = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      debug = true;
    }
  }

  parser_function(debug);

  ErrorCode result;

  return ERR_OK;
}
