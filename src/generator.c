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
#include "symtable.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tmp_counter = 0;
int scope_counter = 0;

char* dataTypeToStr(DataType type) {
  char* dataType;
  switch (type) {
    case TYPE_UNKNOWN:
      return NULL;
    case TYPE_NULL:
      dataType = "nil";
      break;
    case TYPE_BOOL:
      dataType = "bool";
      break;
    case TYPE_INT:
      dataType = "int";
      break;
    case TYPE_FLOAT:
      dataType = "float";
      break;
    case TYPE_STRING:
      dataType = "string";
      break;
  }
  return dataType;
}

ErrorCode generate_code(ASTNode* node, Frame* gf, FrameStack* fs){
  if (!node) return 0;

  ErrorCode returnError = ERR_OK;

  switch (node->type) {
    case NODE_FUNCTION: {
      int numOfParams = node->children[0]->child_count;
      scope_counter = 0;
      fprintf(stdout, "LABEL $%s$%i\n",node->value, numOfParams);
      Frame* tf = malloc(sizeof(Frame));
      F_init(tf, TF);
      fprintf(stdout, "CREATEFRAME\n");
      FS_Push(fs, tf);
      fprintf(stdout, "PUSHFRAME\n");
      if (numOfParams > 0) {
        for (int i = 0; i < numOfParams; i++) {
          ASTNode* param = node->children[0]->children[i];
          F_insert(tf, param->value, param->data_type);
          fprintf(stdout, "DEFVAR LF@%s\n", node->children[0]->children[i]->value);
        }

        for (int i = numOfParams - 1; i >= 0; i--) {
          fprintf(stdout, "POPS LF@%s\n", node->children[0]->children[i]->value);
        }
      }
      //fprintf(stdout, "DEFVAR LF@retVal$"); // sem sa ulozi vysledok tela
      //generuj telo funkcie
      returnError = generate_code(node->children[1], gf, fs);


      return returnError;
    }
    case NODE_SETTER:
    case NODE_GETTER:
    case NODE_TYPE_ID:
    case NODE_ID: {
      Frame* lf = FS_Top(fs);
      FrameEntry* fe = F_lookup(lf, node->value);
      if (fe == NULL) {
        F_insert(lf, node->value, node->data_type);
        fprintf(stdout, "DEFVAR LF@%s$%i\n", node->value, scope_counter);
      }

      return ERR_OK;
    }
    case NODE_BLOCK: { // postupny chod prikazmi v code block
      scope_counter++;
      for (int i = 0; i < node->child_count; i++) {
        returnError = generate_code(node->children[i], gf, fs);
        if (returnError != ERR_OK) {
          scope_counter--;
          return returnError;
        }
      }
      scope_counter--;
      returnError = ERR_OK;
    }

    case NODE_RETURN: {
      //Frame* lf = FS_Top(fs);
      generate_code(node, gf, fs); //generuje vyraz v return
      fprintf(stdout, "PUSHS LF@%s\n", node->children[0]->value);
      Frame* lf = FS_Pop(fs);
      F_cleanup(lf);
      free(lf);
      fprintf(stdout, "POPFRAME\n");
      fprintf(stdout, "RETURN\n\n");
      return ERR_OK;
    }
    case NODE_ASSIGN: {
      //fprintf(stdout, "MOVE %s@%s %s@%s", );
    }

    case NODE_CALL: {
      returnError = generate_code(node->children[1], gf, fs); //ARG_LIST

    }
    case NODE_BINOP: {
      fprintf(stdout, "DEFVAR LF@tmpVar$%i\n", tmp_counter);
      tmp_counter++;


    }
    case NODE_LITERAL: {
      if (node->data_type == TYPE_FLOAT) {
        fprintf(stdout, "PUSHS float@%a\n", strtod(node->value, NULL));
      } else if (node->data_type == TYPE_INT) {
        fprintf(stdout, "PUSHS int@%d\n", atoi(node->value));
      } else {
        fprintf(stdout, "PUSHS %s@%s\n", dataTypeToStr(node->data_type),node->value);
      }
      return ERR_OK;
    }
    case NODE_IF:
    case NODE_WHILE:{
      fprintf(stdout, "LABEL while$%d", scope_counter);
    }
    case NODE_PARAM_LIST:
      return ERR_OK;
    case NODE_ARG_LIST:
      for (int i = 0; i < node->child_count; i++) {
        ASTNode* arg = node->children[i];
        fprintf(stdout, "PUSHS %s@%s\n", dataTypeToStr(arg->data_type), arg->value);
      }
    case NODE_UNOP:
    case NODE_TERNARY:
    case NODE_PROGRAM:
      return ERR_INTERNAL;
    default:
      return ERR_OK;
  }
  return ERR_OK;
}

ErrorCode generate_program(ASTNode* root, SymTable** symTableArray, bool debug) {
  if (root == NULL) return ERR_INTERNAL;
  //fixed prologue
  printf(".IFJcode25\n");
  printf("JUMP $main$0\n");
  Frame gf;
  FrameStack fs;
  F_init(&gf, GF);
  FS_init(&fs);
  ErrorCode state;

  if (debug) {
    fprintf(stderr, "Debug mode ON\n");
  }
  if (symTableArray == NULL) {
    return ERR_INTERNAL;
  }
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
