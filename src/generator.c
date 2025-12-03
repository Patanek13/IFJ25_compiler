/**
 * @file generator.c
 * @author Sebastián Kuchta (xkuchts00)
 * @brief Implementation of IFJcode25 code generator
 *
 */

#include "generator.h"
#include "ast.h"
#include "error.h"
#include "stackframe.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int labelCounter = 0;

ErrorCode generate_code(ASTNode *node, Frame *gf);

char *data_type_to_str(DataType type) {
  char *dataType;
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

char *get_frame_from_id(char *id) {
  if (strncmp(id, "__", 2) == 0) {
    return "GF";
  } else {
    return "LF";
  }
}

void populate_var_definitions(ASTNode *node, Frame *lf) {
  if (!node)
    return;
  if (node->type == NODE_VAR_DECL) {
    fprintf(stdout, "DEFVAR %s@%s\n", get_frame_from_id(node->children[0]->value), node->children[0]->value);
  }

  for (size_t i = 0; i < node->child_count; i++) {
    populate_var_definitions(node->children[i], lf);
  }
}

void populate_global_vars(ASTNode *node, Frame *gf) {
  if (!node)
    return;

  if (node->type == NODE_ID && strncmp(node->value, "__", 2) == 0) {
    if (F_lookup(gf, node->value) == NULL) {
      fprintf(stdout, "DEFVAR GF@%s\n", node->value);
      fprintf(stdout, "MOVE GF@%s nil@nil\n", node->value);
      F_insert(gf, node->value, node->data_type);
    }
  }

  for (size_t i = 0; i < node->child_count; i++) {
    populate_global_vars(node->children[i], gf);
  }
}

void print_formated_string(char *in) {
  if (!in)
    return;

  int inLen = strlen(in);
  for (int i = 0; i < inLen; i++) {
    unsigned char c = (unsigned char)in[i];

    if (c <= 32 || c == 35 || c == 92) {
      fprintf(stdout, "\\%03d", c);
    } else {
      fputc(c, stdout);
    }
  }
}

ErrorCode generate_is_operator(ASTNode *node, Frame *gf) {
  generate_code(node->children[0], gf); // lava strana

  fprintf(stdout, "POPS GF@__$temp1\n");
  fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp1\n");

  char *rhT = node->children[1]->value;
  if (strcmp(rhT, "Num") == 0) {
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "PUSHS string@int\n");
    fprintf(stdout, "EQS\n");

    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "PUSHS string@float\n");
    fprintf(stdout, "EQS\n");

    fprintf(stdout, "ORS\n");
    return ERR_OK;
  } else if (strcmp(rhT, "String") == 0) {
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
    fprintf(stdout, "EXIT int@26\n");
    return ERR_OK;
  }
}

void convert_to_float() {
  int uniqueId = labelCounter++;

  fprintf(stdout, "POPS GF@__$temp1\n");
  fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp1\n");

  fprintf(stdout, "JUMPIFNEQ $skiptofloat$%i GF@__$tempRes string@int\n", uniqueId);

  fprintf(stdout, "INT2FLOAT GF@__$temp1 GF@__$temp1\n");

  fprintf(stdout, "LABEL $skiptofloat$%i\n", uniqueId);
  fprintf(stdout, "PUSHS GF@__$temp1\n");
}

void verify_operands() {
  int uniqueId = labelCounter++;
  fprintf(stdout, "POPS GF@__$temp2\n");
  fprintf(stdout, "POPS GF@__$temp1\n");

  fprintf(stdout, "TYPE GF@__$tempJ GF@__$temp2\n");
  fprintf(stdout, "TYPE GF@__$tempI GF@__$temp1\n");

  fprintf(stdout, "JUMPIFEQ $inttofloat$%i GF@__$tempJ string@float\n", uniqueId);
  fprintf(stdout, "JUMPIFEQ $inttofloat$%i GF@__$tempI string@float\n", uniqueId);

  fprintf(stdout, "JUMP $skiptofloat$%i\n", uniqueId);

  fprintf(stdout, "LABEL $inttofloat$%i\n", uniqueId);
  fprintf(stdout, "JUMPIFNEQ $endconvop1$%i GF@__$tempJ string@int\n", uniqueId);
  fprintf(stdout, "INT2FLOAT GF@__$temp2 GF@__$temp2\n");
  fprintf(stdout, "LABEL $endconvop1$%i\n", uniqueId);

  fprintf(stdout, "JUMPIFNEQ $skiptofloat$%i GF@__$tempI string@int\n", uniqueId);
  fprintf(stdout, "INT2FLOAT GF@__$temp1 GF@__$temp1\n");

  fprintf(stdout, "LABEL $skiptofloat$%i\n", uniqueId);
  fprintf(stdout, "PUSHS GF@__$temp1\n");
  fprintf(stdout, "PUSHS GF@__$temp2\n");
}

ErrorCode generate_binop(ASTNode *node, Frame *gf) {
  if (strcmp(node->value, "is") == 0) {
    ErrorCode state = generate_is_operator(node, gf);
    return state;
  }

  generate_code(node->children[0], gf); // Operand 1
  generate_code(node->children[1], gf); // Operand 2
  verify_operands();

  char *op = node->value;

  if (strcmp(op, "+") == 0) {
    int uniqueId = labelCounter++;

    fprintf(stdout, "POPS GF@__$temp2\n");
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempI GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempJ GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $binop$p$error$%i GF@__$tempI string@nil\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $binop$p$error$%i GF@__$tempJ string@nil\n", uniqueId);

    fprintf(stdout, "JUMPIFEQ $binop$p$checkS$%i GF@__$tempI string@string\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $binop$p$error$%i GF@__$tempJ string@string\n", uniqueId);

    fprintf(stdout, "JUMPIFEQ $binop$p$float$%i GF@__$tempI string@float\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $binop$p$float$%i GF@__$tempJ string@float\n", uniqueId);

    fprintf(stdout, "ADD GF@__$tempRes GF@__$temp1 GF@__$temp2\n");
    fprintf(stdout, "JUMP $binop$p$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$p$float$%i\n", uniqueId);

    fprintf(stdout, "JUMPIFNEQ $binop$p$c1$%i GF@__$tempI string@int\n", uniqueId);
    fprintf(stdout, "INT2FLOAT GF@__$temp1 GF@__$temp1\n");
    fprintf(stdout, "LABEL $binop$p$c1$%i\n", uniqueId);

    fprintf(stdout, "JUMPIFNEQ $binop$p$c2$%i GF@__$tempJ string@int\n", uniqueId);
    fprintf(stdout, "INT2FLOAT GF@__$temp2 GF@__$temp2\n");
    fprintf(stdout, "LABEL $binop$p$c2$%i\n", uniqueId);

    fprintf(stdout, "ADD GF@__$tempRes GF@__$temp1 GF@__$temp2\n");
    fprintf(stdout, "JUMP $binop$p$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$p$checkS$%i\n", uniqueId);
    fprintf(stdout, "JUMPIFNEQ $binop$p$error$%i GF@__$tempJ string@string\n", uniqueId);
    fprintf(stdout, "CONCAT GF@__$tempRes GF@__$temp1 GF@__$temp2\n");
    fprintf(stdout, "JUMP $binop$p$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$p$error$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");

    fprintf(stdout, "LABEL $binop$p$end$%i\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
  } else if (strcmp(op, "-") == 0) {
    int uniqueId = labelCounter++;
    fprintf(stdout, "POPS GF@__$temp2\n");
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempI GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempJ GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $binop$s$error$%i GF@__$tempI string@nil\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $binop$s$error$%i GF@__$tempJ string@nil\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$temp1\n");
    fprintf(stdout, "PUSHS GF@__$temp2\n");
    fprintf(stdout, "SUBS\n");
    fprintf(stdout, "JUMP $binop$s$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$s$error$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");

    fprintf(stdout, "LABEL $binop$s$end$%i\n", uniqueId);
  } else if (strcmp(op, "*") == 0) {
    int uniqueId = labelCounter++;

    fprintf(stdout, "POPS GF@__$temp2\n");
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempI GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempJ GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $binop$m$error$%i GF@__$tempI string@nil\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $binop$m$error$%i GF@__$tempJ string@nil\n", uniqueId);

    fprintf(stdout, "JUMPIFEQ $binop$m$string$%i GF@__$tempI string@string\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $binop$m$error$%i GF@__$tempJ string@string\n", uniqueId);

    fprintf(stdout, "PUSHS GF@__$temp1\n");
    fprintf(stdout, "PUSHS GF@__$temp2\n");

    convert_to_float();
    fprintf(stdout, "POPS GF@__$temp3\n");
    convert_to_float();
    fprintf(stdout, "PUSHS GF@__$temp3\n");
    fprintf(stdout, "MULS\n");
    fprintf(stdout, "JUMP $binop$m$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$m$string$%i\n", uniqueId);

    fprintf(stdout, "ISINT GF@__$tempK GF@__$temp2\n");
    fprintf(stdout, "JUMPIFNEQ $binop$m$error$%i GF@__$tempK bool@true\n", uniqueId);

    fprintf(stdout, "JUMPIFNEQ $binop$m$skipf2i$%i GF@__$tempJ string@float\n", uniqueId);
    fprintf(stdout, "FLOAT2INT GF@__$temp2 GF@__$temp2\n");

    fprintf(stdout, "LABEL $binop$m$skipf2i$%i\n", uniqueId);
    fprintf(stdout, "MOVE GF@__$tempRes string@\n");
    fprintf(stdout, "LABEL $binop$m$stringloop$%i\n", uniqueId);
    fprintf(stdout, "GT GF@__$tempK GF@__$temp2 int@0\n");
    fprintf(stdout, "JUMPIFEQ $binop$m$endloop$%i GF@__$tempK bool@false\n", uniqueId);

    fprintf(stdout, "CONCAT GF@__$tempRes GF@__$tempRes GF@__$temp1\n");
    fprintf(stdout, "SUB GF@__$temp2 GF@__$temp2 int@1\n");

    fprintf(stdout, "JUMP $binop$m$stringloop$%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$m$endloop$%i\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "JUMP $binop$m$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$m$error$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");

    fprintf(stdout, "LABEL $binop$m$end$%i\n", uniqueId);
  } else if (strcmp(op, "/") == 0) {
    // dynamicke rozhodnutie medzi DIV a IDIV
    // int uniqueId = labelCounter++;
    int uniqueId = labelCounter++;

    fprintf(stdout, "POPS GF@__$temp2\n");
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempI GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempJ GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $binop$d$error$%i GF@__$tempI string@nil\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $binop$d$error$%i GF@__$tempJ string@nil\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$temp1\n");
    fprintf(stdout, "PUSHS GF@__$temp2\n");

    convert_to_float();
    fprintf(stdout, "POPS GF@__$temp3\n");
    convert_to_float();
    fprintf(stdout, "PUSHS GF@__$temp3\n");

    fprintf(stdout, "DIVS\n");
    fprintf(stdout, "JUMP $binop$d$end%i\n", uniqueId);

    fprintf(stdout, "LABEL $binop$d$error$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");

    fprintf(stdout, "LABEL $binop$d$end%i\n", uniqueId);
  } else if (strcmp(op, "==") == 0)
    fprintf(stdout, "EQS\n");
  else if (strcmp(op, "<") == 0)
    fprintf(stdout, "LTS\n");
  else if (strcmp(op, ">") == 0)
    fprintf(stdout, "GTS\n");
  else if (strcmp(op, "!=") == 0) {
    fprintf(stdout, "EQS\n");
    fprintf(stdout, "NOTS\n");
  } else if (strcmp(op, "<=") == 0) {
    fprintf(stdout, "GTS\n");
    fprintf(stdout, "NOTS\n");
  } else if (strcmp(op, ">=") == 0) {
    fprintf(stdout, "LTS\n");
    fprintf(stdout, "NOTS\n");
  } else if (strcmp(op, "&&") == 0) {
    int uniqueId = labelCounter++;
    generate_code(node->children[0], gf); // left side
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "JUMPIFEQ $and$false$end$%i GF@__$temp1 bool@false\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $and$false$end$%i GF@__$temp1 nil@nil\n", uniqueId);

    generate_code(node->children[1], gf); // right side
    fprintf(stdout, "JUMP $and$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $and$false$end$%i\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$temp1\n");

    fprintf(stdout, "LABEL $and$end$%i\n", uniqueId);
  } else if (strcmp(op, "||") == 0) {
    int uniqueId = labelCounter++;
    generate_code(node->children[0], gf); // left side
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "JUMPIFEQ $or$right$%i GF@__$temp1 bool@false\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $or$right$%i GF@__$temp1 nil@nil\n", uniqueId);

    fprintf(stdout, "PUSHS GF@__$temp1\n");
    fprintf(stdout, "JUMP $or$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $or$right$%i\n", uniqueId);
    generate_code(node->children[1], gf); // right side

    fprintf(stdout, "LABEL $or$end$%i\n", uniqueId);
  }
  return ERR_OK;
}

ErrorCode generate_builtin(ASTNode *node, Frame *gf) {
  char *funId = node->children[0]->value;
  if (strcmp(funId, "Ifj.write") == 0) {
    for (size_t i = 0; i < node->children[1]->child_count; i++) {
      int uniqueId = labelCounter++;
      generate_code(node->children[1]->children[i], gf);
      fprintf(stdout, "POPS GF@__$tempRes\n");
      fprintf(stdout, "TYPE GF@__$temp3 GF@__$tempRes\n");
      fprintf(stdout, "JUMPIFNEQ $intcheckskip$%i GF@__$temp3 string@float\n", uniqueId);
      fprintf(stdout, "ISINT GF@__$temp3 GF@__$tempRes\n");
      fprintf(stdout, "JUMPIFEQ $intcheckskip$%i GF@__$temp3 bool@false\n", uniqueId);
      fprintf(stdout, "FLOAT2INT GF@__$tempRes GF@__$tempRes\n");

      fprintf(stdout, "LABEL $intcheckskip$%i\n", uniqueId);
      fprintf(stdout, "WRITE GF@__$tempRes\n");
      fprintf(stdout, "PUSHS nil@nil\n");
    }
    // fprintf(stdout, "PUSHS nil@nil\n");
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.read_num") == 0) {
    fprintf(stdout, "READ GF@__$tempRes int\n");
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.read_str") == 0) {
    fprintf(stdout, "READ GF@__$tempRes string\n");
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.read_bool") == 0) {
    fprintf(stdout, "READ GF@__$tempRes bool\n");
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.length") == 0) {
    generate_code(node->children[1], gf);
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "STRLEN GF@__$tempRes GF@__$temp1\n");
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.floor") == 0) {
    generate_code(node->children[1], gf);
    int uniqueId = labelCounter++;
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp1\n");
    fprintf(stdout, "JUMPIFEQ $ifjfloor$skip$%i GF@__$tempRes string@int\n", uniqueId);
    fprintf(stdout, "FLOAT2INT GF@__$temp1 GF@__$temp1\n");
    // fprintf(stdout, "INT2FLOAT GF@__$temp1 GF@__$temp1\n");
    fprintf(stdout, "LABEL $ifjfloor$skip$%i\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$temp1\n");
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.str") == 0) {
    int uniqueId = labelCounter++;
    generate_code(node->children[1], gf);
    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp1\n");

    fprintf(stdout, "JUMPIFEQ $ifjstr$nil$%i GF@__$tempRes string@nil\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $ifjstr$bool$%i GF@__$tempRes string@bool\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $ifjstr$int$%i GF@__$tempRes string@int\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $ifjstr$float$%i GF@__$tempRes string@float\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $ifjstr$string$%i GF@__$tempRes string@string\n", uniqueId);

    fprintf(stdout, "PUSHS string@\n");
    fprintf(stdout, "JUMP $ifjstr$end$%i\n", uniqueId);

    // nil
    fprintf(stdout, "LABEL $ifjstr$nil$%i\n", uniqueId);
    fprintf(stdout, "PUSHS string@null\n");
    fprintf(stdout, "JUMP $ifjstr$end$%i\n", uniqueId);

    // bool
    fprintf(stdout, "LABEL $ifjstr$bool$%i\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $ifjstr$bool$true$%i GF@__temp1 bool@true\n", uniqueId);
    fprintf(stdout, "PUSHS string@false\n");
    fprintf(stdout, "JUMP $ifjstr$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $ifjstr$bool$true$%i\n", uniqueId);
    fprintf(stdout, "PUSHS string@true\n");
    fprintf(stdout, "JUMP $ifjstr$end$%i\n", uniqueId);

    // int
    fprintf(stdout, "LABEL $ifjstr$int$%i\n", uniqueId);
    fprintf(stdout, "INT2STR GF@__$temp2 GF@__$temp1\n");
    fprintf(stdout, "PUSHS GF@__$temp2\n");
    fprintf(stdout, "JUMP $ifjstr$end$%i\n", uniqueId);

    // float
    fprintf(stdout, "LABEL $ifjstr$float$%i\n", uniqueId);
    fprintf(stdout, "FLOAT2STR GF@__$temp2 GF@__$temp1\n");
    fprintf(stdout, "PUSHS GF@__$temp2\n");
    fprintf(stdout, "JUMP $ifjstr$end$%i\n", uniqueId);

    // string
    fprintf(stdout, "LABEL $ifjstr$string$%i\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$temp1\n");
    fprintf(stdout, "JUMP $ifjstr$end$%i\n", uniqueId);
    // end
    fprintf(stdout, "LABEL $ifjstr$end$%i\n", uniqueId);
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.substring") == 0) {
    int uniqueId = labelCounter++;
    generate_code(node->children[1], gf);
    fprintf(stdout, "POPS GF@__$temp3\n"); // j
    fprintf(stdout, "POPS GF@__$temp2\n"); // i
    fprintf(stdout, "POPS GF@__$temp1\n"); // s

    // i a j typechecks
    fprintf(stdout, "ISINT GF@__$tempRes GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$jtype$%i GF@__$tempRes bool@true\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");

    fprintf(stdout, "LABEL $ifjsub$jtype$%i\n", uniqueId);
    fprintf(stdout, "ISINT GF@__$tempRes GF@__$temp3\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$toint$%i GF@__$tempRes bool@true\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");

    // i,j to int
    fprintf(stdout, "LABEL $ifjsub$toint$%i\n", uniqueId);
    fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp2\n");
    fprintf(stdout, "JUMPIFNEQ $ifjsub$skipf2i1$%i GF@__$tempRes string@float\n", uniqueId);
    fprintf(stdout, "FLOAT2INT GF@__$tempI GF@__$temp2\n"); // i
    fprintf(stdout, "JUMP $ifjsub$tointj$%i\n", uniqueId);
    fprintf(stdout, "LABEL $ifjsub$skipf2i1$%i\n", uniqueId);
    fprintf(stdout, "MOVE GF@__$tempI GF@__$temp2\n");

    fprintf(stdout, "LABEL $ifjsub$tointj$%i\n", uniqueId);
    fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp3\n");
    fprintf(stdout, "JUMPIFNEQ $ifjsub$skipf2i2$%i GF@__$tempRes string@float\n", uniqueId);
    fprintf(stdout, "FLOAT2INT GF@__$tempJ GF@__$temp3\n"); // j
    fprintf(stdout, "JUMP $ifjsub$scheck$%i\n", uniqueId);
    fprintf(stdout, "LABEL $ifjsub$skipf2i2$%i\n", uniqueId);
    fprintf(stdout, "MOVE GF@__$tempJ GF@__$temp3\n");

    fprintf(stdout, "LABEL $ifjsub$scheck$%i\n", uniqueId);
    fprintf(stdout, "TYPE GF@__$tempRes GF@__$temp1\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$strlen$%i GF@__$tempRes string@string\n", uniqueId);
    fprintf(stdout, "EXIT int@25\n");
    // dlzka retazca
    fprintf(stdout, "LABEL $ifjsub$strlen$%i\n", uniqueId);
    fprintf(stdout, "STRLEN GF@__$tempK GF@__$temp1\n");

    // i < 0
    fprintf(stdout, "LT GF@__$tempRes GF@__$tempI int@0\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$null$%i GF@__$tempRes bool@true\n", uniqueId);

    // j < 0
    fprintf(stdout, "LT GF@__$tempRes GF@__$tempJ int@0\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$null$%i GF@__$tempRes bool@true\n", uniqueId);
    // i > j
    fprintf(stdout, "GT GF@__$tempRes GF@__$tempI GF@__$tempJ\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$null$%i GF@__$tempRes bool@true\n", uniqueId);
    // i >= len (negácia i < len)
    fprintf(stdout, "LT GF@__$tempRes GF@__$tempI GF@__$tempK\n");
    fprintf(stdout, "JUMPIFNEQ $ifjsub$null$%i GF@__$tempRes bool@true\n", uniqueId);
    // j > len
    fprintf(stdout, "GT GF@__$tempRes GF@__$tempJ GF@__$tempK\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$null$%i GF@__$tempRes bool@true\n", uniqueId);

    // while i < j
    fprintf(stdout, "MOVE GF@__$tempRes string@\n");
    fprintf(stdout, "LABEL $ifjsub$loopCond$%i\n", uniqueId);
    fprintf(stdout, "LT GF@__$temp2 GF@__$tempI GF@__$tempJ\n");
    fprintf(stdout, "JUMPIFEQ $ifjsub$loopEnd$%i GF@__$temp2 bool@false\n", uniqueId);

    fprintf(stdout, "GETCHAR GF@__$tempK GF@__$temp1 GF@__$tempI\n");
    fprintf(stdout, "CONCAT GF@__$tempRes GF@__$tempRes GF@__$tempK\n");
    fprintf(stdout, "ADD GF@__$tempI GF@__$tempI int@1\n");
    fprintf(stdout, "JUMP $ifjsub$loopCond$%i\n", uniqueId);

    fprintf(stdout, "LABEL $ifjsub$loopEnd$%i\n", uniqueId);
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "JUMP $ifjsub$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $ifjsub$null$%i\n", uniqueId);
    fprintf(stdout, "PUSHS nil@nil\n");

    fprintf(stdout, "LABEL $ifjsub$end$%i\n", uniqueId);
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.strcmp") == 0) {
    int uniqueId = labelCounter++;

    generate_code(node->children[1], gf);
    fprintf(stdout, "POPS GF@__$temp2\n"); // s2
    fprintf(stdout, "POPS GF@__$temp1\n"); // s1

    // s1,s2 typecheck
    fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp1\n");
    fprintf(stdout, "JUMPIFNEQ $ifjstrcmp$typeerr$%i GF@__$temp3 string@string\n", uniqueId);
    fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp2\n");
    fprintf(stdout, "JUMPIFNEQ $ifjstrcmp$typeerr$%i GF@__$temp3 string@string\n", uniqueId);

    // testy rovnosti
    fprintf(stdout, "LT GF@__$tempRes GF@__$temp1 GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $ifjstrcmp$less$%i GF@__$tempRes bool@true\n", uniqueId);
    fprintf(stdout, "GT GF@__$tempRes GF@__$temp1 GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $ifjstrcmp$greater$%i GF@__$tempRes bool@true\n", uniqueId);

    fprintf(stdout, "PUSHS int@0\n");
    fprintf(stdout, "JUMP $ifjstrcmp$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $ifjstrcmp$less$%i\n", uniqueId);
    fprintf(stdout, "PUSHS int@-1\n");
    fprintf(stdout, "JUMP $ifjstrcmp$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $ifjstrcmp$greater$%i\n", uniqueId);
    fprintf(stdout, "PUSHS int@1\n");
    fprintf(stdout, "JUMP $ifjstrcmp$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $ifjstrcmp$typeerr$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@25\n");

    fprintf(stdout, "LABEL $ifjstrcmp$end$%i\n", uniqueId);
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.ord") == 0) {
    int uniqueId = labelCounter++;
    generate_code(node->children[1], gf);

    fprintf(stdout, "POPS GF@__$temp2\n"); // i
    fprintf(stdout, "POPS GF@__$temp1\n"); // s

    // type checks
    fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp1\n");
    fprintf(stdout, "JUMPIFNEQ $ifjord$errtype$&$%i GF@__$temp3 string@string\n", uniqueId);

    fprintf(stdout, "ISINT GF@__$temp3 GF@__$temp2\n");
    fprintf(stdout, "JUMPIFNEQ $ifjord$errval$%i GF@__$temp3 bool@true\n", uniqueId);

    fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp2\n");
    fprintf(stdout, "JUMPIFEQ $ifjord$float2intskip$%i GF@__$temp3 string@int\n", uniqueId);
    fprintf(stdout, "FLOAT2INT GF@__$tempI GF@__$temp2\n");
    fprintf(stdout, "JUMP $ifjord$strlen$%i\n", uniqueId);
    fprintf(stdout, "LABEL $ifjord$float2intskip$%i\n", uniqueId);
    fprintf(stdout, "MOVE GF@__$tempI GF@__$temp2\n");

    fprintf(stdout, "LABEL $ifjord$strlen$%i\n", uniqueId);
    fprintf(stdout, "STRLEN GF@__$tempJ GF@__$temp1\n");

    // i < 0 return 0
    fprintf(stdout, "LT GF@__$tempRes GF@__$tempI int@0\n");
    fprintf(stdout, "JUMPIFEQ $ifjord$zero$%i GF@__$tempRes bool@true\n", uniqueId);
    // i >= 0 return 0
    fprintf(stdout, "LT GF@__$tempRes GF@__$tempI GF@__$tempJ\n");
    fprintf(stdout, "JUMPIFNEQ $ifjord$zero$%i GF@__$tempRes bool@true\n", uniqueId);

    fprintf(stdout, "STRI2INT GF@__$tempRes GF@__$temp1 GF@__$tempI\n");
    // fprintf(stdout, "INT2FLOAT GF@__$tempRes GF@__$tempRes\n");
    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "JUMP $ifjord$end$%i\n", uniqueId);

    // return 0
    fprintf(stdout, "LABEL $ifjord$zero$%i\n", uniqueId);
    fprintf(stdout, "PUSHS int@0\n");
    fprintf(stdout, "JUMP $ifjord$end$%i\n", uniqueId);

    // errors, end
    fprintf(stdout, "LABEL $ifjord$errtype$&$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@25\n");

    fprintf(stdout, "LABEL $ifjord$errval$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");

    fprintf(stdout, "LABEL $ifjord$end$%i\n", uniqueId);
    return ERR_OK;
  } else if (strcmp(funId, "Ifj.chr") == 0) {
    int uniqueId = labelCounter++;

    generate_code(node->children[1], gf);
    fprintf(stdout, "POPS GF@__$temp1\n");

    // type checks
    fprintf(stdout, "ISINT GF@__$tempRes GF@__$temp1\n");
    fprintf(stdout, "JUMPIFNEQ $ifjchr$valerr$%i GF@__$tempRes bool@true\n", uniqueId);

    // float to int to char
    fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp1\n");
    fprintf(stdout, "JUMPIFNEQ $ifjchr$skipf2i$%i GF@__$temp3 string@float\n", uniqueId);
    fprintf(stdout, "FLOAT2INT GF@__$tempI GF@__$temp1\n");
    fprintf(stdout, "JUMP $ifjchar$i2c$%i\n", uniqueId);
    fprintf(stdout, "LABEL $ifjchr$skipf2i$%i\n", uniqueId);
    fprintf(stdout, "MOVE GF@__$tempI GF@__$temp1\n");

    fprintf(stdout, "LABEL $ifjchar$i2c$%i\n", uniqueId);
    fprintf(stdout, "INT2CHAR GF@__$tempRes GF@__$tempI\n");

    fprintf(stdout, "PUSHS GF@__$tempRes\n");
    fprintf(stdout, "JUMP $ifjchr$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $ifjchr$valerr$%i\n", uniqueId);
    fprintf(stdout, "EXIT int@26\n");
    fprintf(stdout, "LABEL $ifjchr$end$%i\n", uniqueId);
    return ERR_OK;
  }
  return ERR_OK;
}

ErrorCode generate_code(ASTNode *node, Frame *gf) {
  if (!node)
    return 0;

  ErrorCode returnError = ERR_OK;

  switch (node->type) {
  case NODE_FUNCTION: {
    int numOfParams = node->children[0]->child_count;
    fprintf(stdout, "LABEL $%s$%i\n", node->value, numOfParams);

    fprintf(stdout, "PUSHFRAME\n");
    populate_var_definitions(node->children[1], gf);

    if (numOfParams > 0) { // ziskaj argumenty zo zasobnika
      for (int i = numOfParams - 1; i >= 0; i--) {
        fprintf(stdout, "DEFVAR LF@%s\n", node->children[0]->children[i]->value);
        fprintf(stdout, "POPS LF@%s\n", node->children[0]->children[i]->value);
      }
    }

    // generuj telo funkcie
    returnError = generate_code(node->children[1], gf);
    if (strcmp(node->value, "main") == 0) {
      fprintf(stdout, "PUSHS nil@nil\n");
      fprintf(stdout, "POPFRAME\n");
      fprintf(stdout, "RETURN\n\n");
    } else {
      fprintf(stdout, "PUSHS nil@nil\n");
      fprintf(stdout, "POPFRAME\n");
      fprintf(stdout, "RETURN\n\n");
    }
    return returnError;
  }
  case NODE_SETTER: {
    int idLen = strlen(node->value);
    char setterId[idLen];
    strncpy(setterId, node->value, idLen - 1); // setterId bez "="
    setterId[idLen - 1] = '\0';
    fprintf(stdout, "LABEL $set$%s\n", setterId);
    fprintf(stdout, "PUSHFRAME\n");

    ASTNode *setParam = node->children[0]->children[0]; // NODE_PARAM_LIST->NODE_ID
    fprintf(stdout, "DEFVAR LF@%s\n", setParam->value);
    fprintf(stdout, "POPS LF@%s\n", setParam->value);

    ASTNode *codeBlock = node->children[1];
    populate_var_definitions(codeBlock, gf);
    returnError = generate_code(codeBlock, gf);

    fprintf(stdout, "PUSHS nil@nil\n");
    fprintf(stdout, "POPFRAME\n");
    fprintf(stdout, "RETURN\n\n");
    return returnError;
  }

  case NODE_GETTER: { // generuje getter, chova sa ako funkcia bez parametrov
    fprintf(stdout, "LABEL $get$%s\n", node->value);
    fprintf(stdout, "PUSHFRAME\n");
    populate_var_definitions(node->children[1], gf);
    returnError = generate_code(node->children[1], gf);

    fprintf(stdout, "PUSHS nil@nil\n");
    fprintf(stdout, "POPFRAME\n");
    fprintf(stdout, "RETURN\n\n");
    return ERR_OK;
  }
  case NODE_TYPE_ID:
    return ERR_OK;
  case NODE_ID: {
    fprintf(stdout, "PUSHS %s@%s\n", get_frame_from_id(node->value), node->value);
    return ERR_OK;
  }
  case NODE_BLOCK: { // postupny chod prikazmi v code blocku
    for (size_t i = 0; i < node->child_count; i++) {
      returnError = generate_code(node->children[i], gf);
      if (returnError != ERR_OK) {
        return returnError;
      }
    }
    returnError = ERR_OK;
    return ERR_OK;
  }

  case NODE_RETURN: {
    if (node->child_count > 0) {
      generate_code(node->children[0], gf); // generuje vyraz v return na vrchol zasobniku
    } else {
      fprintf(stdout, "PUSHS nil@nil\n");
    }
    fprintf(stdout, "POPFRAME\n");
    fprintf(stdout, "RETURN\n\n");
    return ERR_OK;
  }
  case NODE_ASSIGN: {
    int idLen = strlen(node->children[0]->value);
    if (node->children[0]->value[idLen - 1] == '=') { // identify setter
      char setterId[idLen];
      strncpy(setterId, node->children[0]->value, idLen - 1);
      setterId[idLen - 1] = '\0';

      returnError = generate_code(node->children[1], gf); //generate rhs and put on stack as argument for setter
      fprintf(stdout, "CREATEFRAME\n");
      fprintf(stdout, "CALL $set$%s\n", setterId);
    } else {
      returnError = generate_code(node->children[1], gf); // left side generate and put on stack
      fprintf(stdout, "POPS %s@%s\n", get_frame_from_id(node->children[0]->value), node->children[0]->value);
    }
    return returnError;
  }

  case NODE_CALL: {
    char *funcId = node->children[0]->value;
    //fprintf(stderr, "genNodeCall:%s\n", funcId);

    if (strncmp(funcId, "Ifj.", 4) == 0) {
      returnError = generate_builtin(node, gf); // inline generate built-in function
    } else if (node->frame == FRAME_GLOBAL) { // identify getter (distinguished from function by frame type)
      fprintf(stdout, "CREATEFRAME\n");
      fprintf(stdout, "CALL $get$%s\n", funcId);
    } else {
      int numOfArgs = node->children[1]->child_count;
      if (numOfArgs > 0) {
        ASTNode *args = node->children[1];
        for (size_t i = 0; i < args->child_count; i++) {
          generate_code(args->children[i], gf); // push arguments on stack
        }
      }
      fprintf(stdout, "CREATEFRAME\n");
      fprintf(stdout, "CALL $%s$%i\n", funcId, numOfArgs);
    }
    return ERR_OK;
  }
  case NODE_BINOP: {
    returnError = generate_binop(node, gf);
    return returnError;
  }
  case NODE_LITERAL: {
    if (node->data_type == TYPE_FLOAT) {
      fprintf(stdout, "PUSHS float@%a\n", strtod(node->value, NULL));
    } else if (node->data_type == TYPE_INT) {
      fprintf(stdout, "PUSHS int@%d\n", atoi(node->value));
    } else if (node->data_type == TYPE_NULL) {
      fprintf(stdout, "PUSHS nil@nil\n");
    } else if (node->data_type == TYPE_STRING) {
      fprintf(stdout, "PUSHS string@");
      print_formated_string(node->value);
      fprintf(stdout, "\n");
    } else {
      char *dataType = data_type_to_str(node->data_type);
      if (dataType != NULL) {
        fprintf(stdout, "PUSHS %s@%s\n", dataType, node->value);
      } else {
        return ERR_INTERNAL;
      }
    }
    return ERR_OK;
  }
  case NODE_VAR_DECL: { //local variable already defined at start of function, just insert nil
    fprintf(stdout, "MOVE %s@%s nil@nil\n", get_frame_from_id(node->children[0]->value), node->children[0]->value);
    return ERR_OK;
  }
  case NODE_IF: {
    int uniqueId = labelCounter++;
    generate_code(node->children[0], gf); // evaluate condition body (should return bool value)

    fprintf(stdout, "POPS GF@__$temp1\n");
    fprintf(stdout, "JUMPIFEQ $if$else$%i GF@__$temp1 nil@nil\n", uniqueId);

    fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp1\n");
    fprintf(stdout, "JUMPIFNEQ $if$skipcheck$%i GF@__$temp3 string@bool\n", uniqueId);

    fprintf(stdout, "JUMPIFEQ $if$else$%i GF@__$temp1 bool@false\n", uniqueId);

    fprintf(stdout, "LABEL $if$skipcheck$%i\n", uniqueId);

    generate_code(node->children[1], gf); // then
    fprintf(stdout, "JUMP $if$end$%i\n", uniqueId);

    fprintf(stdout, "LABEL $if$else$%i\n", uniqueId);
    if (node->child_count > 2) {
      generate_code(node->children[2], gf); // else body
    }

    fprintf(stdout, "LABEL $if$end$%i\n", uniqueId);
    return ERR_OK;
  }
  case NODE_WHILE: {
    int uniqueId = labelCounter++;
    fprintf(stdout, "LABEL $while$start$%i\n", uniqueId);
    generate_code(node->children[0], gf);
    fprintf(stdout, "PUSHS bool@false\n");
    fprintf(stdout, "JUMPIFEQS $while$end$%i\n", uniqueId);
    generate_code(node->children[1], gf);
    fprintf(stdout, "JUMP $while$start$%i\n", uniqueId);
    fprintf(stdout, "LABEL $while$end$%i\n", uniqueId);
    return ERR_OK;
  }
  case NODE_PARAM_LIST:
    return ERR_OK;
  case NODE_ARG_LIST: {
    for (size_t i = 0; i < node->child_count; i++) {
      ASTNode *arg = node->children[i];
      generate_code(arg, gf);
    }
    return ERR_OK;
  }
  case NODE_UNOP: {
    int uniqueId = labelCounter++;
    generate_code(node->children[0], gf);

    if (strcmp(node->value, "-") == 0) {
      fprintf(stdout, "POPS GF@__$temp1\n");
      fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp1\n");

      fprintf(stdout, "JUMPIFEQ $unop$minus$int$%i GF@__$temp3 string@int\n", uniqueId);
      fprintf(stdout, "JUMPIFEQ $unop$minus$float$%i GF@__$temp3 string@float\n", uniqueId);
      fprintf(stdout, "EXIT int@26\n");

      fprintf(stdout, "LABEL $unop$minus$int$%i\n", uniqueId);
      fprintf(stdout, "PUSHS int@0\n");
      fprintf(stdout, "PUSHS GF@__$temp1\n");
      fprintf(stdout, "SUBS\n");
      fprintf(stdout, "JUMP $unop$end$%i\n", uniqueId);

      fprintf(stdout, "LABEL $unop$minus$float$%i\n", uniqueId);
      fprintf(stdout, "PUSHS float@0x0p+0\n");
      fprintf(stdout, "PUSHS GF@__$temp1\n");
      fprintf(stdout, "SUBS\n");
      fprintf(stdout, "JUMP $unop$end$%i\n", uniqueId);
    } else if (strcmp(node->value, "!") == 0) {
      fprintf(stdout, "POPS GF@__$temp1\n");
      fprintf(stdout, "TYPE GF@__$temp3 GF@__$temp1\n");

      fprintf(stdout, "JUMPIFEQ $unop$not$nil$%i GF@__$temp3 string@nil\n", uniqueId);
      fprintf(stdout, "JUMPIFEQ $unop$not$bool$%i GF@__$temp3 string@bool\n", uniqueId);

      fprintf(stdout, "PUSHS bool@false\n");
      fprintf(stdout, "JUMP $unop$end$%i\n", uniqueId);

      fprintf(stdout, "LABEL $unop$not$nil$%i\n", uniqueId);
      fprintf(stdout, "PUSHS bool@true\n");
      fprintf(stdout, "JUMP $unop$end$%i\n", uniqueId);

      fprintf(stdout, "LABEL $unop$not$bool$%i\n", uniqueId);
      fprintf(stdout, "PUSHS GF@__$temp1\n");
      fprintf(stdout, "NOTS\n");
      fprintf(stdout, "JUMP $unop$end$%i\n", uniqueId);
    }
    fprintf(stdout, "LABEL $unop$end$%i\n", uniqueId);
    return ERR_OK;
  }
  case NODE_TERNARY: {
    int uniqueId = labelCounter++;
    generate_code(node->children[0], gf);
    fprintf(stdout, "POPS GF@__$temp1\n");

    // if
    fprintf(stdout, "JUMPIFEQ $terelse$%i GF@__$temp1 nil@nil\n", uniqueId);
    fprintf(stdout, "JUMPIFEQ $terelse$%i GF@__$temp1 bool@false\n", uniqueId);

    // then
    generate_code(node->children[1], gf);
    fprintf(stdout, "JUMP $terend$%i\n", uniqueId);

    // else
    fprintf(stdout, "LABEL $terelse$%i\n", uniqueId);
    generate_code(node->children[2], gf);

    fprintf(stdout, "$terend$%i\n", uniqueId);
    return ERR_OK;
  }
  case NODE_PROGRAM: {
    fprintf(stdout, ".IFJcode25\n");

    //prepare global helper variables
    fprintf(stdout, "DEFVAR GF@__$temp1\n");
    fprintf(stdout, "DEFVAR GF@__$temp2\n");
    fprintf(stdout, "DEFVAR GF@__$temp3\n");
    fprintf(stdout, "DEFVAR GF@__$tempI\n");
    fprintf(stdout, "DEFVAR GF@__$tempJ\n");
    fprintf(stdout, "DEFVAR GF@__$tempK\n");
    fprintf(stdout, "DEFVAR GF@__$tempRes\n");

    populate_global_vars(node, gf);

    fprintf(stdout, "CREATEFRAME\n");
    fprintf(stdout, "CALL $main$0\n");
    fprintf(stdout, "JUMP $program$end$\n");
    // generate all program functions
    ASTNode *mainBlock = node->children[0];
    for (size_t i = 0; i < mainBlock->child_count; i++) {
      returnError = generate_code(mainBlock->children[i], gf);
      if (returnError != ERR_OK) {
        return ERR_INTERNAL;
      }
    }

    fprintf(stdout, "LABEL $program$end$\n");
    fprintf(stdout, "CLEARS\n");
    fprintf(stdout, "EXIT int@0\n");
    return ERR_OK;
  }
  default:
    return ERR_OK;
  }
  return ERR_OK;
}

ErrorCode generate_program(ASTNode *root, bool debug) {
  if (root == NULL)
    return ERR_INTERNAL;

  Frame gf;
  F_init(&gf, GF);
  ErrorCode state;

  if (debug) {
    fprintf(stderr, "Debug mode ON\n");
  }

  state = generate_code(root, &gf);
  F_cleanup(&gf);

  return state;
}
