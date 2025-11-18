/**
 * @file generator.c
 * @author Sebastián Kuchta (xkuchts00)
 * @date 2025-11-09
 *
 *
 */

#include "generator.h"
#include "ast.h"
#include "error.h"
#include "stackframe.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>


ErrorCode generate_code(ASTNode* node, Frame* gf, FrameStack* fs){
  if (!node) return 0;

  switch (node->type) {
    case NODE_FUNCTION: {
      int numOfParams = node->children[0]->child_count;
      fprintf(stdout, "LABEL $%s$%i\n",node->value, numOfParams);
      Frame tf;
      F_init(&tf, TF);
      fprintf(stdout, "CREATEFRAME\n");
      FS_Push(fs, tf);
      fprintf(stdout, "PUSHFRAME\n");
      if (numOfParams > 0) {
        for (int i = 0; i < numOfParams; i++) {
          ASTNode* param = node->children[0]->children[i];
          F_insert(&tf, param->value, param->data_type);
          fprintf(stdout, "DEFVAR LF@%s", node->children[0]->children[i]->value);
        }

        for (int i = numOfParams - 1; i >= 0; i--) {
          fprintf(stdout, "POPS LF@%s", node->children[0]->children[i]->value);
        }
      }
      fprintf(stdout, "DEFVAR LF@retVal$"); // sem sa ulozi vysledok tela
      //generuj telo funkcie
      generate_code(node->children[1], gf, fs);

      F_cleanup(&tf);
      return ERR_OK;
    }
    case NODE_SETTER:
    case NODE_GETTER:
    case NODE_ID: {
      Frame* lf = FS_Top(fs);
      FrameEntry* fe = F_lookup(lf, node->value);
      if (fe == NULL) {
        F_insert(lf, node->value, node->data_type);
        fprintf(stdout, "DEFVAR LF@%s", node->value);
      }
    }
    case NODE_BLOCK:

    case NODE_RETURN: {
      Frame* lf = FS_Top(fs);
      fprintf(stdout, "PUSHS LF@%s", node->value);

    }
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
    case NODE_PROGRAM:
      return ERR_INTERNAL;
  }
  return ERR_OK;
}

ErrorCode generate_program(ASTNode* root, SymTable** symTableArray, bool debug) {
  if (root == NULL) return ERR_INTERNAL;
  //fixed prologue
  printf(".IFJcode25\n");
  printf("JUMP $main$0");
  Frame gf;
  FrameStack fs;
  F_init(&gf, GF);
  FS_init(&fs);
  ErrorCode state;

  //generate all functions
  ASTNode* mainBlock = root->children[0];
  for (size_t i = 0; i < mainBlock->child_count; i++) {
    state = generate_code(mainBlock->children[i], &gf, &fs);
    if (state != ERR_OK) {
      return ERR_INTERNAL;
    }
  }

  return state;
}
