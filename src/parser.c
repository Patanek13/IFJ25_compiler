/**
 * @file parser.c
 * @author Šimon Čorej (xcorejs00)
 * @brief Main Parser Function
 * @version 0.1
 * @date 2025-10-12
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"


// TODO: vlozit do expression() moznost kontroly spravnosti (a+5 == 10)
// TODO: doplnit moznost ! v parametroch expression ternary atd
// TODO: doplnit do funkcii assign params return case OPERATOR: return expression
// TODO: doplnit do tych istych funkcii case QUESTION: return ternary()
//      -> ? nesmie byt prvy operator a kontrola ci token pred nim je spravnej hodnoty bude na semantike
//      -> kontrola a : b bude na ternary()
//      -> ternary() bude kontrolovat iba ( ) ? :
//      -> to co bolo pred ? ci je spravne napisane skontroluje dana funkcia ostatne je na ternary()

FILE* in;
FILE* out;

/* condition state 0 = IF 1 = WHILE */
bool cond_state = 0;

TokenType list_param_ops[] = {IS, EQUAL_EQUAL, LESS, LESS_EQUAL, MORE, MORE_EQUAL, NOT, NOT_EQUAL, AND, OR};
char* built_in_string[] = {"read_str", "read_num", "write", "floor", "str", "length", "substring", "strcmp", "ord", "chr", "read_bool"};

bool match_token(TokenType type){
    token = get_token();
    return (token.type == type);
}

// debug token
bool check_and_take_token(TokenType type, int *error_code){
    if (token.type != type){
        fprintf(out, "ERROR: Expected token type %d but found %d\n", type, token.type);
        *error_code = SYNTAX_ERROR;
        return false;
    }
    token = get_token();
    return true;
}

bool is_built_in_func(){
    for (int i = 0; i < 11; i++){
        if (strcmp(token.value.string, built_in_string[i]) == 0){ return true; }
    }
    return false;
}

// bool is_operator(){
//     return ((token.type == PLUS) || (token.type == MINUS) || (token.type == MULTIPLY) || (token.type == DIVIDE));
// }

bool is_operand(){
    return ((token.type == ID) || (token.type == GLOBAL_ID) || (token.type == STRING)
            || (token.type == FLOATING) || (token.type == BOOLEAN) || (token.type == INTEGER));
}

bool is_param_expr(){
    for (int i = 0; i < 10; i++){
        if (list_param_ops[i] == token.type){ return true; }
    }
    return false;
}

int expression(){ /* pozor pri assign konci az po nacitani ")" chyba, a = a + 5*/
    while(token.type != BRACKET_END){
        token = get_token();
    }
    // token = get_token();
    fprintf(out, "___________\n expression OK return \n_____________\n");
    return ERR_OK;
}

// int ternary_arg_check(){
//     switch(token.type){

//     }
// }

ASTNode* parse_expression(int *error_code) {
    error_code = ERR_OK;
    fprintf(out, "DEBUG: parse_expression volani\n");
    // precedencni analyza

    ASTNode* node = NULL;
    fprintf(out, "DEBUG: parse_expression() s tokenem: \n");
    print_token(token);
    fprintf(out, "\n");

    // Unarni operator na zacatku
    if (token.type == OPERATOR && strcmp(token.value.string, "-") == 0) {
      node = ast_create_node(NODE_UNOP, token.value.string, TYPE_UNKNOWN);
      if (node == NULL) {
          *error_code = ERR_INTERNAL;
          return NULL;
      }

      // Nacteni dalsiho tokenu
      token = get_token();
      ASTNode* operand = parse_expression(error_code);
      if (*error_code != ERR_OK) {
          ast_free(node);
          return NULL;
      }
      ast_add_child(node, operand);
    } else {
      // zpracujem termy
      switch (token.type) {
        case ID: {
            Token id_token = token; // Ulozim si token id
            token = get_token();
            if (token.type == BRACKET_START) {
                // Je to volani funkce
                token = id_token; // Vratim se k id tokenu pro func_call
                node = func_call(error_code);
                if (*error_code != ERR_OK) {
                    return NULL;
                }
            } else {
                // Neni to volani, vytvorim uzel pro ID
                node = ast_create_node(NODE_ID, id_token.value.string, TYPE_UNKNOWN);
                if (node == NULL) {
                    *error_code = ERR_INTERNAL;
                    return NULL;
                }
                // token je uz nacteny
            }
            break;
        }
        case IFJ:
            node = built_in_call(error_code);
            if (*error_code != ERR_OK) {
                return NULL;
            }
            token = get_token();
            break;
        case GLOBAL_ID:
            node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);
            if (node == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            token = get_token();
            break;
        case STRING:
            node = ast_create_node(NODE_LITERAL, token.value.string, TYPE_STRING);
            if (node == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            token = get_token();
            break;
        case INTEGER: {
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "%d", token.value.integer);
            node = ast_create_node(NODE_LITERAL, buffer, TYPE_INT);
            if (node == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            token = get_token();
            break;
        }
        case FLOATING: {
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "%f", token.value.floating);
            node = ast_create_node(NODE_LITERAL, buffer, TYPE_FLOAT);
            if (node == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            token = get_token();
            break;
        }
        case BOOLEAN: {
            char* bool_str = token.value.boolean ? "true" : "false";
            node = ast_create_node(NODE_LITERAL, bool_str, TYPE_BOOL);
            if (node == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            token = get_token();
            break;
        }
        case NULL_KEYWORD:
            node = ast_create_node(NODE_LITERAL, "null", TYPE_NULL);
            if (node == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            token = get_token();
            break;
        default:
            fprintf(out, "ERROR: Neznamy token v parse_expression: ");
            *error_code = SYNTAX_ERROR;
            return NULL;
            break;
      }
    }

    // poresit ternarni operator
    if (token.type == QUESTION) {
        // vytvorit ternarni uzel
        ASTNode* ternary_node = ast_create_node(NODE_TERNARY, NULL, TYPE_UNKNOWN);
        if (ternary_node == NULL) {
            ast_free(node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }

        // pridat podminku jako prvni child
        ast_add_child(ternary_node, node);

        // zpracovat vyraz mezi ? a :
        token = get_token();
        int expr_error = ERR_OK;
        ASTNode* true_expr = parse_expression(&expr_error);
        if (expr_error != ERR_OK) {
            ast_free(ternary_node);
            *error_code = expr_error;
            return NULL;
        }
        ast_add_child(ternary_node, true_expr);

        // ocekavat dvojtecku
        if (token.type != COLON) {
            ast_free(ternary_node);
            *error_code = SYNTAX_ERROR;
            return NULL;
        }

        // zpracovat vyraz za :
        token = get_token();
        ASTNode* false_expr = parse_expression(&expr_error);
        if (expr_error != ERR_OK) {
            ast_free(ternary_node);
            *error_code = expr_error;
            return NULL;
        }
        ast_add_child(ternary_node, false_expr);
      }

    fprintf(out, "___________\n parse_expression OK return \n_____________\n");
    return node;
}

// parsujeme seznam paramteru pri deklaraci funkce
// <PARAM_LIST> --> ID (,ID)*
ASTNode* param_list(int* error_code) {
  fprintf(out, "nasli sme token v param_list: ");
  print_token(token);
  fprintf(out, "\n");
  *error_code = ERR_OK;

  ASTNode* param_list_node = ast_create_node(NODE_PARAM_LIST, NULL, TYPE_UNKNOWN);
  if (param_list_node == NULL) {
      *error_code = ERR_INTERNAL;
      return NULL;
  }

  if (token.type == BRACKET_END) {
      // prazdny seznam parametru
      fprintf(out, "___________\n param_list OK return \n_____________\n");
      return param_list_node;
  }

  // zpracovat parametry ve while
  while (true) {
      // musi byt ID
      if (token.type != ID) {
          fprintf(out, "ERROR: Ocekavane ID v seznamu parametru, nasli sme: ");
          print_token(token);
          fprintf(out, "\n");
          ast_free(param_list_node);
          *error_code = SYNTAX_ERROR;
          return NULL;
      }

      // vytvorit uzel pro ID
      ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);
      if (id_node == NULL) {
          ast_free(param_list_node);
          *error_code = ERR_INTERNAL;
          return NULL;
      }

      // pridat id jako child k param_list_node
      ast_add_child(param_list_node, id_node);

      // nacist dalsi token
      token = get_token();

      if (token.type == COMMA) {
          // dalsi parametr
          token = get_token();

          // zustala carka bez dalsiho parametru
          if (token.type == BRACKET_END) {
              fprintf(out, "ERROR: Zbyla carka bez dalsiho parametru\n");
              ast_free(param_list_node);
              *error_code = SYNTAX_ERROR;
              return NULL;
          }

      } else if (token.type == BRACKET_END) {
          // konec seznamu parametru
          break;
      } else {
          // neocekavany token
          fprintf(out, "ERROR: Neocekavany token v seznamu parametru: ");
          print_token(token);
          fprintf(out, "\n");
          ast_free(param_list_node);
          *error_code = SYNTAX_ERROR;
          return NULL;
      }
  }
  return param_list_node;
}

ASTNode* params(int* error_code) {
    fprintf(out, "nasli sme token v params: ");
    print_token(token);
    fprintf(out, "\n");

    *error_code = ERR_OK;
    ASTNode* params_node = ast_create_node(NODE_ARG_LIST, NULL, TYPE_UNKNOWN);
    if (params_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // parametry nebudou, takze ), vratim prazdny params_node
    if (token.type == BRACKET_END) {
        fprintf(out, "___________\n params OK return \n_____________\n");
        return params_node;
    }

    // zpracovat parametry ve whilu
    while (true) {
        ASTNode* arg_node = parse_expression(error_code);

        // zkontrolovat chyby pri tvorbe argumentu
        if (error_code != ERR_OK) {
            ast_free(params_node);
            ast_free(arg_node);
            return NULL;
        }

        // pridat argument jako child k params_node
        if (arg_node != NULL) {
            ast_add_child(params_node, arg_node);
        }

        // nacist dalsi token, pokud je carka
        if (token.type == COMMA) {
            token = get_token();

            // zustala carka bez dalsiho parametru
            if (token.type == BRACKET_END) {
                fprintf(out, "ERROR: Zbyla carka bez dalsiho parametru\n");
                ast_free(params_node);
                *error_code = SYNTAX_ERROR;
                return NULL;
            }
            // dalsi parametr
            continue;
          } else if (token.type == BRACKET_END) {
            // konec seznamu parametru
            break;
          } else {
            // neocekavany token
            fprintf(out, "ERROR: Neocekavany token v seznamu parametru: ");
            print_token(token);
            fprintf(out, "\n");
            ast_free(params_node);
            *error_code = SYNTAX_ERROR;
            return NULL;
          }
    }

    // vsechno v poradku
    fprintf(out, "___________\n params OK return \n_____________\n");
    *error_code = ERR_OK;
    return params_node;
}


/**
 * @brief Function <BLOCK> on default calls command function
 *
 * @return SYNTAX_ERROR or OK if finished
 */
ASTNode* block(int* error_code) {
    fprintf(out, "nasli sme token v block: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BLOCK> -> <{> <NEWLINE> <COMMANDS> <}> <ELSE>*/

    // program() zere prvni {
    // block() chce command dokud neskonci }

    *error_code = ERR_OK;
    ASTNode* block_node = ast_create_node(NODE_BLOCK, NULL, TYPE_UNKNOWN);
    if (block_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // poresit optional newline po {
    if (token.type != NEW_LINE) {
        // nemuze byt prazdny blok {}
        if (token.type == BLOCK_END) {
            fprintf(out, "ERROR: Prazdny blok neni povoleny\n");
            ast_free(block_node);
            *error_code = SYNTAX_ERROR;
            return NULL;
        }
        fprintf(out, "Neni NEW_LINE po BLOCK_START\n");
        *error_code = SYNTAX_ERROR;
        ast_free(block_node);
        return NULL;
    }
    // nacist dalsi token po newline
    token = get_token();

    // zpracovat prikazy v bloku, dokud neskonci }
    while (token.type != BLOCK_END) {
        int cmd_error = ERR_OK;
        ASTNode* cmd_node = command(&cmd_error);

        if (cmd_error != ERR_OK) {
            *error_code = cmd_error;
            ast_free(block_node);
            return NULL;
        }
        // command() vraci NULL pro newline
        if (cmd_node != NULL) {
            ast_add_child(block_node, cmd_node);
        }

        // EOF nastane pred koncem bloku
        if (token.type == EOF_TOKEN) {
            *error_code = SYNTAX_ERROR;
            ast_free(block_node);
            return NULL;
        }
    }

    // konec bloku
    token = get_token();
    fprintf(out, "___________\n block OK return \n_____________\n");
    return block_node;
}

/**
 * @brief Function to call user made functions
 *
 * @return Function return ASTNode* of <func_call> or NULL on error
 */
ASTNode* func_call(int *error_code) {
    fprintf(out, "nasli sme token v func_call: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <FUNC_CALL> -> ID <(>  <PARAMS>  <)>*/

    *error_code = ERR_OK;
    // pokud neni ID, chyba
    if (token.type != ID) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // vytvorit uzel pro volani funkce
    ASTNode* call_node = ast_create_node(NODE_CALL, NULL, TYPE_UNKNOWN);

    if (call_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // ulozit jmeno funkce jako identifikator
    ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);

    if (id_node == NULL) {
        ast_free(call_node);
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // pridat id jako child ke call node
    ast_add_child(call_node, id_node);

    // nacist dalsi token, musi byt (
    if (!match_token(BRACKET_START)) {
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // zpracovat parametry
    token = get_token();

    int params_error = ERR_OK;
    ASTNode* args_node = params(&params_error);

    if (params_error != ERR_OK) {
        ast_free(call_node);
        *error_code = params_error;
        return NULL;
    }

    // pridat args_node jako child ke call_node
    ast_add_child(call_node, args_node);

    // nacist dalsi token, musi byt )
    if (token.type != BRACKET_END) {
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    fprintf(out, "___________\n func_call OK return \n_____________\n");
    return call_node;
}

/**
 * @brief Function to call built in functions
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
ASTNode* built_in_call(int* error_code) {
    fprintf(out, "nasli sme token v b_i_c: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <BUILT_IN_CALL> -> <IFJ> <.> <KW> <(> <PARAMS> <)> */

    *error_code = ERR_OK;
    // pokud neni IFJ, chyba
    if (token.type != IFJ) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // nacist dalsi token, musi byt .
    if (!match_token(DOT)) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // nacist dalsi token, musi byt id funkce
    if (!match_token(ID) || !is_built_in_func()) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // potrebuju ulozit jmeno funkce
    char* func_name = str_dup(token.value.string);
    if (func_name == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // vytvorit uzel pro volani funkce
    ASTNode* call_node = ast_create_node(NODE_CALL, NULL, TYPE_UNKNOWN);
    if (call_node == NULL) {
        free(func_name);
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // ulozit jmeno funkce jako id
    ASTNode* id_node = ast_create_node(NODE_ID, func_name, TYPE_UNKNOWN);
    free(func_name);
    if (id_node == NULL) {
        ast_free(call_node);
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // pridat id jako child ke call_node
    ast_add_child(call_node, id_node);
    // nacist dalsi token, musi byt (
    if (!match_token(BRACKET_START)) {
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // zpracovat parametry
    token = get_token();

    int params_error = ERR_OK;
    ASTNode* args_node = params(&params_error);
    if (params_error != ERR_OK) {
        ast_free(call_node);
        *error_code = params_error;
        return NULL;
    }

    // pridat args_node jako child ke call_node
    ast_add_child(call_node, args_node);

    // nacist dalsi token, musi byt )
    if (token.type != BRACKET_END) {
        ast_free(call_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    fprintf(out, "___________\n b_i_c OK return \n_____________\n");
    return call_node;
}

/**
 * @brief Function to use conditions and while loops
 * @todo doplnit kde mozu byt newlines
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
ASTNode* cond_loop(int* error_code) {
    fprintf(out, "nasli sme token v cond_loop cond_state: %s: ", (cond_state)? "while":"if");
    print_token(token);
    fprintf(out, "\n");

    *error_code = ERR_OK;
    // kontrola jestli je to if nebo while
    bool is_if = false;
    // vytvorit uzel pro podminku nebo cyklus
    ASTNode *cond_node = NULL;
    // vytvorit parent uzel pro pridani podminky/cyklu
    if (token.type == IF) {
        cond_node = ast_create_node(NODE_IF, NULL, TYPE_UNKNOWN);
    } else if (token.type == WHILE) {
        cond_node = ast_create_node(NODE_WHILE, NULL, TYPE_UNKNOWN);
    } else {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    if (cond_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // nacist dalsi token, musi byt (
    token = get_token();
    if (!check_and_take_token(BRACKET_START, error_code)) {
        ast_free(cond_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // zpracovat vyraz podminky
    ASTNode* expr_node = parse_expression(error_code);
    if (*error_code != ERR_OK) {
        ast_free(cond_node);
        return NULL;
    }
    ast_add_child(cond_node, expr_node);

    // dalsi token musi byt )
    if (!check_and_take_token(BRACKET_END, error_code)) {
        ast_free(cond_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // dalsi token, musi byt {
    if (!check_and_take_token(BLOCK_START, error_code)) {
        ast_free(cond_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    // zpracovat blok prikazu
    ASTNode* block_node = block(error_code);
    if (*error_code != ERR_OK) {
        ast_free(cond_node);
        return NULL;
    }
    ast_add_child(cond_node, block_node);

    if (is_if) {
        if (token.type == ELSE) {
            token = get_token();

            if (token.type == IF) {
                // else if
                ASTNode* else_if_node = cond_loop(error_code); // rekurzivni zavolam
                if (*error_code != ERR_OK) {
                    ast_free(cond_node);
                    return NULL;
                }
                ast_add_child(cond_node, else_if_node);
            } else if (token.type == BLOCK_START) {
                // klasika else
                token = get_token(); // nacist {
                ASTNode* else_block_node = block(error_code);
                if (*error_code != ERR_OK) {
                    ast_free(cond_node);
                    return NULL;
                }
                ast_add_child(cond_node, else_block_node);
            } else {
                // chyba
                fprintf(out, "ERROR: Expected IF or BLOCK after ELSE\n");
                ast_free(cond_node);
                *error_code = SYNTAX_ERROR;
                return NULL;
            }
        } else {
            // else neni, vsechno ok
            // pridat prazdny else do AST
            ASTNode* empty_else_node = ast_create_node(NODE_BLOCK, NULL, TYPE_UNKNOWN);
            if (empty_else_node == NULL) {
                ast_free(cond_node);
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            ast_add_child(cond_node, empty_else_node);
        }
    }

    // cekam newline po celym if/while
    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(cond_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }

    fprintf(out, "___________\n cond_loop OK return \n_____________\n");
    return cond_node;
}

/**
 * @brief Function to assign value or expression
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
ASTNode* assign(int *error_code) { /* TODO poriadne otestovat nove riadky kde mozu a nemozu byt */
    fprintf(out, "nasli sme token v assign: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <ASSIGN> -> <ID> <=> <LITERAL> (or) <EXPRESSION> */

    *error_code = ERR_OK;
    ASTNode* assign_node = ast_create_node(NODE_ASSIGN, NULL, TYPE_UNKNOWN);
    if (assign_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // musi byt ID nebo GLOBAL_ID
    if ((token.type != ID) && (token.type != GLOBAL_ID)) {
        ast_free(assign_node);
        *error_code = SYNTAX_ERROR;
        return NULL;
}

    // vytvorit uzel pro ID
    ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);
    if (id_node == NULL) {
        ast_free(assign_node);
        *error_code = ERR_INTERNAL;
        return NULL;
    }
    // pridat id jako child k assign_node
    ast_add_child(assign_node, id_node);

    // nacist dalsi token, musi byt =
    token = get_token();
    if (!check_and_take_token(EQUAL, error_code)) {
        ast_free(assign_node);
        return NULL;
    }

    // nacist dalsi token, musi byt literal nebo expression
    ASTNode* expr_node = parse_expression(error_code);
    if (*error_code != ERR_OK) {
        ast_free(assign_node);
        return NULL;
    }

    // pridat expr_node jako child k assign_node
    ast_add_child(assign_node, expr_node);

    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(assign_node);
        return NULL;
    }

    fprintf(out, "___________\n assign OK return \n_____________\n");
    return assign_node;
}

/**
 * @brief Function to declare user made functions
 *
 * @return ERR_OK or SYNTAX_ERROR
 */
ASTNode* func_decl(int *error_code) {
    fprintf(out, "nasli sme token v func_decl: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE:  <FUNC_DECL> -> <STATIC> <ID> <=?> <BRACKETS> <BLOCK> */

    *error_code = ERR_OK;
    // token je STATIC
    token = get_token();
    // musi byt ID
    if (token.type != ID) {
        *error_code = SYNTAX_ERROR;
        return NULL;
    }
    // potrebuju ulozit nazov funkce
    char *func_name = str_dup(token.value.string);
    if (func_name == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // dalsi token musi byt (
    token = get_token();
    ASTNode* func_node = NULL;

    if (token.type == BRACKET_START) {
        // vytvorit uzel pro deklaraci funkce
        func_node = ast_create_node(NODE_FUNCTION, func_name, TYPE_UNKNOWN);
        if (func_node == NULL) {
            free(func_name);
            *error_code = ERR_INTERNAL;
            return NULL;
        }

        // zpracovat seznam parametru
        token = get_token();
        ASTNode* params_node = param_list(error_code);
        if (*error_code != ERR_OK) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }
        // pridat params_node jako child k func_node
        ast_add_child(func_node, params_node);

        // dalsi token musi byt )
        if (!check_and_take_token(BRACKET_END, error_code)) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }

    } else if (token.type == EQUAL) {
        // vytvorit uzel pro deklaraci setter funkce
        func_node = ast_create_node(NODE_SETTER, func_name, TYPE_UNKNOWN);
        if (func_node == NULL) {
            free(func_name);
            *error_code = ERR_INTERNAL;
            return NULL;
        }

        token = get_token(); // musi byt =
        if (!check_and_take_token(BRACKET_START, error_code)) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }

        // setter musi mit jeden parameter
        ASTNode* params_node = ast_create_node(NODE_PARAM_LIST, NULL, TYPE_UNKNOWN);
        if (params_node == NULL) {
            free(func_name);
            ast_free(func_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        // musi byt ID
        if (token.type != ID) {
            free(func_name);
            ast_free(func_node);
            ast_free(params_node);
            *error_code = SYNTAX_ERROR;
            return NULL;
        }
        // vytvorit uzel pro ID
        ASTNode* id_node = ast_create_node(NODE_ID, token.value.string, TYPE_UNKNOWN);
        if (id_node == NULL) {
            free(func_name);
            ast_free(func_node);
            ast_free(params_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        // pridat id jako child k params_node
        ast_add_child(params_node, id_node);
        // pridat params_node jako child k func_node
        ast_add_child(func_node, params_node);

        token= get_token(); // nacist dalsi token
        if (!check_and_take_token(BRACKET_END, error_code)) {
            free(func_name);
            ast_free(func_node);
            return NULL;
        }
      } else if (token.type == BLOCK_START) {
        // vytvorit uzel pro deklaraci getter funkce
        func_node = ast_create_node(NODE_GETTER, func_name, TYPE_UNKNOWN);
        if (func_node == NULL) {
            free(func_name);
            *error_code = ERR_INTERNAL;
            return NULL;
        }

        // getter nema parametry, pridat prazdny param_list
        ASTNode* params_node = ast_create_node(NODE_PARAM_LIST, NULL, TYPE_UNKNOWN);
        if (params_node == NULL) {
            free(func_name);
            ast_free(func_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        ast_add_child(func_node, params_node);
        // nic vic cist, pokracujem na block
    } else {
        free(func_name);
        *error_code = SYNTAX_ERROR;
        return NULL;
    }
    // zpracovat blok prikazu
    free(func_name);
    if (!check_and_take_token(BLOCK_START, error_code)) {
        ast_free(func_node);
        return NULL;
    }

    ASTNode* block_node = block(error_code);
    if (*error_code != ERR_OK) {
        ast_free(func_node);
        return NULL;
    }
    ast_add_child(func_node, block_node);

    // cekam newline po celem func_decl
    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(func_node);
        return NULL;
    }

    fprintf(out, "___________\n func_decl OK return \n_____________\n");
    return func_node;
}

ASTNode* return_func(int *error_code) {
    fprintf(out, "nasli sme token v return_func: ");
    print_token(token);
    fprintf(out, "\n");

    *error_code = ERR_OK;
    // token je RETURN

    ASTNode* return_node = ast_create_node(NODE_RETURN, NULL, TYPE_UNKNOWN);
    if (return_node == NULL) {
        *error_code = ERR_INTERNAL;
        return NULL;
    }

    // zkontrolovat return bez vyrazu
    if (token.type == NEW_LINE) {
        // pro semantiku return null
        ASTNode* null_node = ast_create_node(NODE_LITERAL, "null", TYPE_NULL);
        if (null_node == NULL) {
            ast_free(return_node);
            *error_code = ERR_INTERNAL;
            return NULL;
        }
        ast_add_child(return_node, null_node);
        fprintf(out, "___________\n return_func OK return \n_____________\n");
        token= get_token(); // nacist dalsi token
        return return_node;
    }

    // zpracovat vyraz za return
    ASTNode* expr_node = parse_expression(error_code);
    if (*error_code != ERR_OK) {
        ast_free(return_node);
        return NULL;
    }
    ast_add_child(return_node, expr_node);
    // cekam newline po return
    if (!check_and_take_token(NEW_LINE, error_code)) {
        ast_free(return_node);
        return NULL;
    }

    fprintf(out, "___________\n return_func OK return \n_____________\n");
    return return_node;
}


ASTNode* command(int *error_code) {
    fprintf(out, "nasli sme token v command: ");
    print_token(token);
    fprintf(out, "\n");
    /* RULE: <COMMAND> -> <FUNC_CALL> <NEW_LINE>
                      | <ASSIGN> <NEW_LINE>
                      | <BUILT_IN_CALL> <NEW_LINE>
                      | <RETURN> <NEW_LINE>
                      | <VAR> <ID> <NEW_LINE>
                      | <COND_LOOP>
                      | <FUNC_DECL>
                      | <NEW_LINE>
                      | <BLOCK_END>
    */

    *error_code = ERR_OK;
    ASTNode* command_node = NULL;

    switch(token.type){
        case ID: {
          Token id_token = token; // Ulozim si token id
          token = get_token(); // Nactu dalsi token

          if (token.type == BRACKET_START) {
              // Je to volani funkce
              token = id_token; // Vratim se k id tokenu pro func_call
              command_node = func_call(error_code);
              if (*error_code != ERR_OK) {
                  return NULL;
              }

              // func_call necha token )
              // cekam NEW_LINE
              if (!check_and_take_token(BRACKET_END, error_code)) {
                  ast_free(command_node);
                  return NULL;
              }

              if (!check_and_take_token(NEW_LINE, error_code)) {
                  ast_free(command_node);
                  return NULL;
              }

          } else if (token.type == EQUAL) {
              // Neni to volani, takze prirazeni
              token = id_token; // Vratim se k id tokenu pro assign
              command_node = assign(error_code);
              if (*error_code != ERR_OK) {
                  return NULL;
              }
            // assign necha token za vyrazem
          } else {
              fprintf(out, "ERROR: Neocekavany token po ID: ");
              print_token(token);
              fprintf(out, "\n");
              *error_code = SYNTAX_ERROR;
              return NULL;
          }
          break;
        }


        case IFJ:
            command_node = built_in_call(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // built_in_call necha token )
            if (!check_and_take_token(BRACKET_END, error_code)) {
                ast_free(command_node);
                return NULL;
            }
            // cekam NEW_LINE
            if (!check_and_take_token(NEW_LINE, error_code)) {
                ast_free(command_node);
                return NULL;
            }
            break;

        case RETURN:
            token = get_token(); // vezmu dalsi token
            command_node = return_func(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // uz vzal newline
            break;

        case GLOBAL_ID: /* chyba var a global id sa nevolaju rovnako*/
            // tu je prirazeni
            command_node = assign(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // vzal newline
            break;

        case VAR: {
            // deklarace promenne
            token = get_token(); // nacist dalsi token, musi byt ID
            if (token.type != ID) {
                *error_code = SYNTAX_ERROR;
                return NULL;
            }

            // ulozit jmeno promenne
            char* var_name = str_dup(token.value.string);
            if (var_name == NULL) {
                *error_code = ERR_INTERNAL;
                return NULL;
            }

            // vytvorit uzel pro deklaraci promenne
            command_node = ast_create_node(NODE_ASSIGN, NULL, TYPE_UNKNOWN);
            if (command_node == NULL) {
                *error_code = ERR_INTERNAL;
                free(var_name);
                return NULL;
            }

            // vytvorit uzel pro ID
            ASTNode* id_node = ast_create_node(NODE_ID, var_name, TYPE_UNKNOWN);
            free(var_name);
            if (id_node == NULL) {
                ast_free(command_node);
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            // pridat id jako child k assign_node
            ast_add_child(command_node, id_node);

            // promenna inicializovana na null
            ASTNode* null_node = ast_create_node(NODE_LITERAL, "null", TYPE_NULL);
            if (null_node == NULL) {
                ast_free(command_node);
                *error_code = ERR_INTERNAL;
                return NULL;
            }
            // pridat null jako child k assign_node
            ast_add_child(command_node, null_node);
            // cekam NEW_LINE
            token = get_token();
            if (!check_and_take_token(NEW_LINE, error_code)) {
                ast_free(command_node);
                return NULL;
            }
            break;
        }

        case IF:
        case WHILE:
            command_node = cond_loop(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // cond_loop uz vzal newline
            break;

        case STATIC:
            command_node = func_decl(error_code);
            if (*error_code != ERR_OK){ return NULL; }
            // func_decl uz vzal newline
            break;

        case NEW_LINE:
            token = get_token();
            return NULL; // vratim NULL a nepridam do AST
            break;

        case BLOCK_END:
            fprintf(out, "___________\n command OK return \n_____________\n");
            return NULL; // vratim NULL a nepridam do AST
            break;

        default:
            *error_code = SYNTAX_ERROR;
            return NULL;
            break;
   }

   if (*error_code != ERR_OK) {
        ast_free(command_node);
        return NULL;
   }

  return command_node;
}

int valid(){
    fprintf(out, "sme v valid\n");
    fprintf(out, "nasli sme token v valid: ");
    print_token(token);
    fprintf(out, "\n");

    switch(token.type){
        case IMPORT:
            if ((!match_token(STRING)) && (token.type != NEW_LINE)){ return SYNTAX_ERROR; }
            return valid();
            break;

        case STRING:
            if (strcmp(token.value.string, "ifj25") == 0){
                if (!match_token(FOR)){ return SYNTAX_ERROR; }
                return valid();
            } else {
                return SYNTAX_ERROR;
            }
            break;

        case FOR:
            if (!match_token(IFJ) && (token.type != NEW_LINE)){ return SYNTAX_ERROR; }
            return valid();
            break;

        case IFJ:
            if (match_token(NEW_LINE)){
                // musim mit novy token, protoze match_token() posunul na NEW_LINE
                token = get_token();
                return ERR_OK;
            }
            return SYNTAX_ERROR;
            break;

        case NEW_LINE:
            token = get_token();
            return valid();
            break;

        default:
            return SYNTAX_ERROR;
            break;
    }
    return SYNTAX_ERROR;
}

ASTNode* program(int* error_code){
    fprintf(out, "nasli sme token v prog: ");
    print_token(token);
    fprintf(out, "\n");

    if (token.type != CLASS) {
      fprintf(out, "ERROR: Musi byt 'class'\n");
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    if (!match_token(ID) || strcmp(token.value.string, "Program") != 0) {
      fprintf(out, "ERROR: Musi byt 'Program'\n");
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    if (!match_token(BLOCK_START)) {
      fprintf(out, "ERROR: Musi byt '{'\n");
      *error_code = SYNTAX_ERROR;
      return NULL;
    }

    ASTNode* program_node = ast_create_node(NODE_PROGRAM, NULL, TYPE_UNKNOWN);

    if (!program_node) {
      fprintf(out, "ERROR: Failed to create AST node\n");
      *error_code = ERR_INTERNAL;
      return NULL;
    }

    token = get_token(); // Move to the next token after '{'

    int child_error = ERR_OK;
    ASTNode* block_node = block(&child_error); // Parse the block

    if (child_error != ERR_OK) {
      fprintf(out, "ERROR: Failed to parse block\n");
      ast_free(program_node);
      *error_code = child_error;
      return NULL;
    }

    // block() took '}'
    ast_add_child(program_node, block_node);

    if (token.type != EOF_TOKEN) {
      fprintf(out, "ERROR: Expected end of file\n");
    }

    *error_code = ERR_OK;
    return program_node;
}

// ===========================================DEBUG PRECEDENCE=====================================================
void tok_print(TokenType tok){
    switch(tok){
        case LESS:            fprintf(out, "<  |  "); break;
        case MORE:            fprintf(out, ">  |  "); break;
        case EQUAL:            fprintf(out, "= |  "); break;
        case ERROR:            fprintf(out, "ERR  |   "); break;

        default:
            break;
    }
}
void precedence_check(){
    Token token1;
    Token token2;
    fprintf(out, "table: \n");
    for (int i = 0; i < 43; i++){
        token1 = get_token();

        // if (token1.type == EOF_TOKEN){ break; }

        if (token1.type == NEW_LINE){ fprintf(out, "\n"); continue; }

        token2 = get_token();

        fprintf(out, "%s,", token1.value.string);

        if (token2.type == INTEGER){
            fprintf(out, "%d |  ", token2.value.integer);
        }
        else if (token2.type == FLOATING){
            fprintf(out, "%f |  ", token2.value.floating);
        }
        else if (token2.type == BRACKET_START){
            fprintf(out, "( |  ");
        }
        else if (token2.type == BRACKET_END){
            fprintf(out, ") |  ");
        }
        else {
            fprintf(out, "%s |  ", token2.value.string);
        }

        tok_print(precedence_table[token_to_int(token1)][token_to_int(token2)]);
    }
}
// ===============================================================================================================


int main (int argc, char** argv){
    (void)argc;
    (void)argv;

    in = fopen("../samples/test.wren", "r");
    if (!in){ printf("nejde otvorit\n"); return ERR_INTERNAL; }

    out = fopen("../samples/outfile.txt", "w");
    if (!out){ return ERR_INTERNAL; }

    scanner_init(in, out);

    int ok;
    // ok = 1;

    token = get_token();

    ASTNode* ast_root = NULL;
    int parse_error = ERR_OK;


    // precedence_check();
    // if (token.type == EQUAL){
    //     Stack stack;
    //     if (stack_init(&stack) != ERR_OK){ return SYNTAX_ERROR; }

    //     stack_push(&stack, END_EXPR);

    //     int value;
    //     token = get_token();
    //     value = expression_val(&stack);
    //     fprintf(out, "calculated value: %d\n", value);
    // }



    if (valid() == ERR_OK){
        fprintf(out, "VALID OK\n");

        ast_root = program(&parse_error);

        if (parse_error == ERR_OK){
            ok = 1;
            fprintf(out, "\n<AST representation>\n");
            ast_print_debug(ast_root, 0);
        } else {
            ok = 0;
            fprintf(out, "PARSE ERROR\n");
        }
    } else {
        fprintf(out, "VALID NOT OK\n");
        parse_error = SYNTAX_ERROR;
    }

    fprintf(out, "Program ended %s\n", (ok == 1) ? "successfully" : "with error");
    fclose(in);
    fclose(out);


    return (ok == 1) ? ERR_OK : parse_error;
}
