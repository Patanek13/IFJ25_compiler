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

char* getFrameFromId(char* id) {
  if (strncmp(id, "__", 2) == 0) {
    return "GF";
  } else {
    return "LF";
  }
}

void populateVarDefinitions(ASTNode* node, Frame* lf) {
  if (!node) return;

  if (node->type == NODE_VAR_DECL) {
    fprintf(stdout, "DEFVAR %s@%s\n", getFrameFromId(node->value),node->value);
  }

  for (int i = 0; i < node->child_count; i++) {
    populateVarDefinitions(node->children[i], lf);
  }
}

char* cleanString(char* in) {
  if (!in) return "";
  size_t inLen = strlen(in);
  char* out = malloc(inLen*4+1);
  if (!out) return NULL;

  char* ptr = out;
  for (size_t i = 0; i < inLen; i++) {
    char c = (char)in[i];
    if (c <= 32 || c == 35 || c == 92) {
      ptr += sprintf(out, "\\%03d", c);
    } else {
      *ptr = c;
      ptr++;
    }
  }
  *ptr = '\0';
  return out;
}

ErrorCode generate_IsOperator(ASTNode* node, Frame* gf, FrameStack* fs){
  generate_code(node->children[0], gf, fs); // lava strana

  fprintf(stdout, "POPS GF@__$temp1\n");
  fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp1\n");
  fprintf(stdout, "PUSHS GF@__$tempRes\n");


  char* rhT = node->children[1]->value;
  if (strcmp(rhT, "Num") == 0) {
    fprintf(stdout, "PUSHS string@int\n");
    fprintf(stdout, "EQS\n");

    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "PUSHS string@float\n");

    fprintf(stdout, "ORS\n");
    return ERR_OK;
  } else if (strcmp(rhT, "String") == 0){
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "PUSHS string@string\n");
    fprintf(stdout, "EQS\n");
    return ERR_OK;
  } else if (strcmp(rhT, "Null") == 0) {
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "PUSHS string@nil\n");
    fprintf(stdout, "EQS\n");
    return ERR_OK;
  } else {
    fprintf(stdout, "PUSHS bool@false");
    return ERR_OK;
  }
}

ErrorCode generate_binOp(ASTNode* node, Frame* gf, FrameStack* fs){
  if (strcmp(node->value, "is") == 0) {
    ErrorCode state = generate_IsOperator(node, gf, fs);
    return state;
  }

  generate_code(node->children[0], gf, fs);
  generate_code(node->children[1], gf, fs);

  char* op = node->value;

  if (strcmp(op, "+") == 0) {
    if (node->data_type == TYPE_STRING) {
      fprintf(stdout, "POPS GF@__$temp1\n");
      fprintf(stdout, "POPS GF@__$temp2\n");
      fprintf(stdout, "CONCAT GF@__$tempRes GF@__$temp1 GF@__$temp2\n");

      fprintf(stdout, "PUSHS GF@__$tempRes\n");
    } else {
      fprintf(stdout, "ADDS\n");
    }
  }
  else if (strcmp(op, "-") == 0) fprintf(stdout, "SUBS\n");
  else if (strcmp(op, "*") == 0) fprintf(stdout, "MULS\n");
  else if (strcmp(op, "/") == 0) fprintf(stdout, "DIVS\n");
  else if (strcmp(op, "==") == 0) fprintf(stdout, "EQS\n");
  else if (strcmp(op, "<") == 0) fprintf(stdout, "LTS\n");
  else if (strcmp(op, ">") == 0) fprintf(stdout, "GTS\n");
  else if (strcmp(op, "!=") == 0) {
    fprintf(stdout, "EQS\n");
    fprintf(stdout, "NOTS\n");
  } else if (strcmp(op, "<=") == 0) {
    fprintf(stdout, "GTS\n");
    fprintf(stdout, "NOTS\n");
  } else if (strcmp(op, ">=") == 0) {
    fprintf(stdout, "LTS\n");
    fprintf(stdout, "NOTS\n");
  }
  return ERR_OK;
}

ErrorCode generate_code(ASTNode* node, Frame* gf, FrameStack* fs){
  if (!node) return 0;

  ErrorCode returnError = ERR_OK;

  switch (node->type) {
    case NODE_FUNCTION: {
      labelManager counter;
      counter.labelCounter = 0;
      int numOfParams = node->children[0]->child_count;
      fprintf(stdout, "LABEL $%s$%i\n",node->value, numOfParams);
      Frame* lf = malloc(sizeof(Frame));
      F_init(lf, LF);
      fprintf(stdout, "CREATEFRAME\n");
      FS_Push(fs, lf);
      fprintf(stdout, "PUSHFRAME\n");
      populateVarDefinitions(node, lf);
      if (numOfParams > 0) {
        for (int i = numOfParams - 1; i >= 0; i--) {
          ASTNode* param = node->children[0]->children[i];
          F_insert(lf, param->value, param->data_type);
          fprintf(stdout, "DEFVAR LF@%s\n", node->children[0]->children[i]->value);
          fprintf(stdout, "POPS LF@%s\n", node->children[0]->children[i]->value);
        }
      }
      //fprintf(stdout, "DEFVAR LF@retVal$"); // sem sa ulozi vysledok tela
      //generuj telo funkcie
      returnError = generate_code(node->children[1], gf, fs);

      if (strcmp(node->value, "main") != 0) {
        fprintf(stdout, "PUSHS nil@nil\n");
        FS_Pop(fs);
        F_cleanup(lf);
        fprintf(stdout, "POPFRAME\n");
        fprintf(stdout, "RETURN\n");
      }
      return returnError;
    }
    case NODE_SETTER: {
      fprintf(stdout, "LABEL $set$%s", node->value);
      fprintf(stdout, "CREATEFRAME");
      fprintf(stdout, "PUSHFRAME");

      ASTNode* setParam = node->children[0]->children[0]; // NODE_PARAM_LIST->NODE_ID
      fprintf(stdout, "DEFVAR LF@%s", setParam->value);
      fprintf(stdout, "POPS LF@%s", setParam->value);

      ASTNode* codeBlock = node->children[1];
      populateVarDefinitions(codeBlock, gf);
      generate_code(codeBlock, gf, fs);

      fprintf(stdout, "PUSHS nil@nil");
      fprintf(stdout, "POPFRAME");
      fprintf(stdout, "RETURN");
    }

    case NODE_GETTER:{
      fprintf(stdout, "LABEL $get$%s", node->value);
      fprintf(stdout, "CREATEFRAME");
      fprintf(stdout, "PUSHFRAME");
      populateVarDefinitions(node->children[1], gf);
      returnError = generate_code(node->children[1], gf, fs);

      fprintf(stdout, "PUSHS nil@nil");
      fprintf(stdout, "POPFRAME");
      fprintf(stdout, "RETURN");
      return ERR_OK;
    }
    case NODE_TYPE_ID:
      return ERR_OK;
    case NODE_ID: {
      /*Frame* lf = FS_Top(fs);
      FrameEntry* fe = F_lookup(lf, node->value);
      if (fe == NULL) {
        F_insert(lf, node->value, node->data_type);
        fprintf(stdout, "DEFVAR LF@%s\n", node->value);
      }
      */
      fprintf(stdout, "PUSHS %s@%s\n", getFrameFromId(node->value), node->value);
      return ERR_OK;
    }
    case NODE_BLOCK: { // postupny chod prikazmi v code block
      for (size_t i = 0; i < node->child_count; i++) {
        returnError = generate_code(node->children[i], gf, fs);
        if (returnError != ERR_OK) {
          return returnError;
        }
      }
      returnError = ERR_OK;
      return ERR_OK;
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
      int idLen = strlen(node->value);
      if (node->value[idLen-1] == '=') { //identify setter
        char setterId[idLen-1];
        strncpy(setterId, node->value, idLen-1);
        generate_code(node->children[1], gf, fs); // prava cast na zasobik

        fprintf(stdout, "CALL $set$%s\n", setterId);
      } else {
        generate_code(node->children[1], gf, fs); // prava cast na zasobnik
        fprintf(stdout, "POPS %s@%s\n", getFrameFromId(node->children[0]->value), node->children[0]->value);
      }
    }

    case NODE_CALL: {
      returnError = generate_code(node->children[1], gf, fs); //ARG_LIST
      return ERR_OK;
    }
    case NODE_BINOP: {
      returnError = generate_binOp(node, gf, fs);
      return returnError;
    }
    case NODE_LITERAL: {
      if (node->data_type == TYPE_FLOAT) {
        fprintf(stdout, "PUSHS float@%a\n", strtod(node->value, NULL));
      } else if (node->data_type == TYPE_INT) {
        fprintf(stdout, "PUSHS int@%d\n", atoi(node->value));
      } else if (node->data_type == TYPE_NULL) {
        fprintf(stdout, "PUSHS %s@nil\n", dataTypeToStr(node->data_type));
      } else if (node->data_type == TYPE_STRING) {
        fprintf(stdout, "PUSHS %s@%s\n", dataTypeToStr(node->data_type), cleanString(node->value));
      } else {
        fprintf(stdout, "PUSHS %s@%s\n", dataTypeToStr(node->data_type), node->value);
      }
      return ERR_OK;
    }
    case NODE_VAR_DECL: {
      fprintf(stdout, "MOVE %s@%s nil@nil\n", getFrameFromId(node->children[0]->value), node->children[0]->value);
      return ERR_OK;
    }
    case NODE_IF:
    case NODE_WHILE:{
      //fprintf(stdout, "LABEL while$%d", scope_counter);
      return ERR_OK;
    }
    case NODE_PARAM_LIST:
      return ERR_OK;
    case NODE_ARG_LIST: {
      for (size_t i = 0; i < node->child_count; i++) {
        ASTNode* arg = node->children[i];
        fprintf(stdout, "PUSHS %s@%s\n", dataTypeToStr(arg->data_type), arg->value);
      }
      return ERR_OK;
    }
    case NODE_UNOP:
    case NODE_TERNARY:
    case NODE_PROGRAM: {
      fprintf(stdout, ".IFJcode25\n");
      fprintf(stdout, "DEFVAR GF@__$temp1\n");
      fprintf(stdout, "DEFVAR GF@__$temp2\n");
      fprintf(stdout, "DEFVAR GF@__$tempRes\n");
      fprintf(stdout, "JUMP $main$0\n");
      //generate all functions
      ASTNode* mainBlock = node->children[0];
      for (size_t i = 0; i < mainBlock->child_count; i++) {
        returnError = generate_code(mainBlock->children[i], gf, fs);
        if (returnError != ERR_OK) {
          return ERR_INTERNAL;
        }
      }
      return ERR_OK;
    }
    default:
      return ERR_OK;
  }
  return ERR_OK;
}

ErrorCode generate_program(ASTNode* root, SymTable** symTableArray, bool debug) {
  if (root == NULL) return ERR_INTERNAL;

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

  state = generate_code(root, &gf, &fs);


  return state;
}
