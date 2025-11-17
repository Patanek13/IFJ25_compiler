/**
 * @file generator.c
 * @author Sebastián Kuchta (xkuchts00)
 * @date 2025-11-09
 *
 *
 */

#include "generator.h"
#include "error.h"
#include "stackframe.h"
#include <stdio.h>

int generate(ASTNode* node, SymTable* symbolTable){
  if (!node) return 0;
  for (size_t i = 0; i < node->child_count; i++) {
    generate(node->children[i], symbolTable);
  }

  switch (node->type) {
    case NODE_PROGRAM:
    case NODE_FUNCTION:
    case NODE_SETTER:
    case NODE_GETTER:
    case NODE_ID:
    case NODE_BLOCK:
    case NODE_RETURN:
    case NODE_ASSIGN:
    case NODE_CALL:
    case NODE_BINOP:
    case NODE_LITERAL:
    case NODE_IF:
    case NODE_WHILE:
    case NODE_PARAM_LIST:
    case NODE_ARG_LIST:
    case NODE_UNOP:
    case NODE_TERNARY:
      break;
  }
  return 1;
}

ErrorCode generate_code(ASTNode* root, SymTable** symTableArray) {
  //fixed prologue
  printf(".IFJcode25\n");
  printf("JUMP $main");
  Frame gf;
  FrameStack fs;
  F_init(&gf, GF);
  FS_init(&fs);

  int state = generate(root, *symTableArray);

  return state;
}
