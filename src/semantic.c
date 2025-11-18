/*
* @file semantic.c
* @author Patrik Lošťák (xlostap00)
* @brief Semantic Analysis implementation file
*/

#include "semantic.h"

// Max depth for block scope tracking
#define SCOPE_STACK_MAX 100

/*========= STACK of symbol tables for block scopes =========*/

typedef struct {
    SymTable* tables[SCOPE_STACK_MAX];
    int topIndex;
} ScopeStack;

/* @brief Initializes the scope stack
  * @param stack Pointer to the scope stack
*/
static void init_Scope_Stack(ScopeStack* stack) {
    stack->topIndex = -1;
}

/* @brief Pushes a symbol table onto the scope stack
  * @param stack Pointer to the scope stack
  * @param table Pointer to the symbol table to push
  * @return true if push was successful, false if stack overflow
 */
static bool push_Scope(ScopeStack* stack, SymTable* table) {
    if (stack->topIndex >= SCOPE_STACK_MAX - 1) {
        fprintf(stderr, "Semantic Error: Scope stack overflow\n");
        return false;
    }
    stack->tables[++stack->topIndex] = table;
    return true;
}

/* @brief Pops a symbol table from the scope stack
  * @param stack Pointer to the scope stack
  * @return Pointer to the popped symbol table, or NULL if stack is empty
*/
static SymTable* pop_Scope(ScopeStack* stack) {
    if (stack->topIndex < 0) {
        return NULL;
    }
    return stack->tables[stack->topIndex--];
}

/* @brief Returns the top symbol table on the scope stack
  * @param stack Pointer to the scope stack
  * @return Pointer to SymTable if found, NULL otherwise
*/
static SymTable* top_Scope(ScopeStack* stack) {
    if (stack->topIndex < 0) {
        return NULL; // Stack is empty
    }
    // Return the top symbol table on the scope stack
    return stack->tables[stack->topIndex];
}

/* @brief Search symbol in all scopes from top to bottom
  * @param stack Pointer to the scope stack
  * @param key Symbol name to search for
  * @return Pointer to SymbolData if found, NULL otherwise
*/
static SymbolData* scope_lookup(ScopeStack* stack, const char* key) {
    for (int symbol_idx = stack->topIndex; symbol_idx >= 0; symbol_idx--) {
        SymbolData* data = symtable_lookup(stack->tables[symbol_idx], key);
        if (data != NULL) {
            return data; // Found in this scope
        }
    }
    return NULL; // Not found in any scope
}

/* @brief Search symbol only in the current scope
  * @param stack Pointer to the scope stack
  * @param key Symbol name to search for
  * @return Pointer to SymbolData if found, NULL otherwise
*/
static SymbolData* scope_lookup_current(ScopeStack* stack, const char* key) {
    if (stack->topIndex < 0) {
        return NULL;
    }
    return symtable_lookup(stack->tables[stack->topIndex], key);
}

/*========= SEMANTIC ANALYSIS IMPLEMENTATION =========*/

/*========= Analysis Context Structure =========*/
// Context structure to hold analysis state
typedef struct {
    ScopeStack* scope_stack; // Stack of symbol tables for scopes
    bool debug;              // Debug flag
    int* error_code;        // Pointer to error code
    SymTable* global_table; // Global symbol table
    ASTNode* current_function; // Pointer to AST node of current function
} AnalysisContext;

/*================================================================*/

/*======== Prototypes of Semantic Analysis Functions =============*/
static void analyze_node(ASTNode* node, AnalysisContext* context);
static void fill_global_table(SymTable* global_table, int* error_code);

static void analyze_program(ASTNode* node, AnalysisContext* context);
static void analyze_function(ASTNode* node, AnalysisContext* context);
static void analyze_block(ASTNode* node, AnalysisContext* context);
static void analyze_return(ASTNode* node, AnalysisContext* context);
static void analyze_assign(ASTNode* node, AnalysisContext* context);
static void analyze_call(ASTNode* node, AnalysisContext* context);
static void analyze_if(ASTNode* node, AnalysisContext* context);
static void analyze_while(ASTNode* node, AnalysisContext* context);
static DataType analyze_expression(ASTNode* node, AnalysisContext* context);
static void analyze_setter(ASTNode* node, AnalysisContext* context);
static void analyze_getter(ASTNode* node, AnalysisContext* context);
static bool func_exists(SymTable* table, const char* base_key);
/*===========================================================================*/

/* @brief Checks if the given DataType is numeric (INT or FLOAT)
  * @param type DataType to check
  * @return true if numeric, false otherwise
*/
static bool is_num_type(DataType type) {
    return type == TYPE_INT || type == TYPE_FLOAT;
}

/* @brief Checks if a function with the given base key exists in the symbol table
  * @param table Pointer to the symbol table
  * @param base_key Base key of the function (without parameter signature)
  * @return true if function exists, false otherwise
*/
static bool func_exists(SymTable* table, const char* base_key) {
    for (size_t item_idx = 0; item_idx < table->size; item_idx++) {
        if (table->items[item_idx].state == SLOT_OCCUPIED) {
            const char* key = table->items[item_idx].key;
            // Check if key starts with base_key followed by '#'
            if (strncmp(key, base_key, strlen(base_key)) == 0 && key[strlen(base_key)] == '#') {
                return true; // Function with matching base key exists
            }
        }
    }
    return false; // No matching function found
}

/* @brief Fills up the global table with built-in functions
  * @param global_table Pointer to the global symbol table
*/
static void fill_global_table(SymTable* global_table, int* error_code) {
    char* key = NULL;
    SymbolData data;

    // Built-in function: Ifj.read_str() -> String | Null
    key = make_function_key("Ifj.read_str", 0);
    data = create_function_symbol(TYPE_UNKNOWN, 0);
    data.defined = true;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.read_str\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.read_num() -> Num | Null
    key = make_function_key("Ifj.read_num", 0);
    data = create_function_symbol(TYPE_UNKNOWN, 0);
    data.defined = true;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.read_num\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.write(value: Any) -> Null
    key = make_function_key("Ifj.write", 1);
    data = create_function_symbol(TYPE_NULL, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_UNKNOWN; // Any type
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.write\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.length(str: String) -> Num
    key = make_function_key("Ifj.length", 1);
    data = create_function_symbol(TYPE_INT, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.length\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.floor(num: Num) -> Num
    key = make_function_key("Ifj.floor", 1);
    data = create_function_symbol(TYPE_INT, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_FLOAT;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.floor\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.substring(str: String, start: Num, length: Num) -> String | Null
    key = make_function_key("Ifj.substring", 3);
    data = create_function_symbol(TYPE_UNKNOWN, 3);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    data.info.function.param_types[1] = TYPE_INT;
    data.info.function.param_types[2] = TYPE_INT;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.substring\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.str(value: Any) -> String
    key = make_function_key("Ifj.str", 1);
    data = create_function_symbol(TYPE_STRING, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_UNKNOWN; // Any type
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.str\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.strcmp(str1: String, str2: String) -> Num
    key = make_function_key("Ifj.strcmp", 2);
    data = create_function_symbol(TYPE_INT, 2);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    data.info.function.param_types[1] = TYPE_STRING;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.strcmp\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.ord(str: String, index: Num) -> Num
    key = make_function_key("Ifj.ord", 2);
    data = create_function_symbol(TYPE_INT, 2);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    data.info.function.param_types[1] = TYPE_INT;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.ord\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.chr(ASCI code: Num) -> String
    key = make_function_key("Ifj.chr", 1);
    data = create_function_symbol(TYPE_STRING, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_INT;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.chr\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

    // Built-in function: Ifj.read_bool() -> Bool | Null
    key = make_function_key("Ifj.read_bool", 0);
    data = create_function_symbol(TYPE_BOOL, 0);
    data.defined = true;
    if(symtable_insert(global_table, key, data) == ERR_INTERNAL) {
        fprintf(stderr, "Semantic Error: Failed to insert built-in function Ifj.read_bool\n");
        *error_code = ERR_INTERNAL;
        free(key);
        return;
    }
    free(key);

}

/* @brief Calls the right semantic analysis function based on the AST node type
  * @param node Pointer to the AST node
  * @param context Pointer to the analysis context (scope stack, debug flag, error code)
*/
static void analyze_node(ASTNode* node, AnalysisContext* context) {
    if (node == NULL || *context->error_code != ERR_OK) {
        return;
    }

    switch (node->type) {
        case NODE_PROGRAM:
            analyze_program(node, context);
            break;
        case NODE_FUNCTION:
            analyze_function(node, context);
            break;
        case NODE_SETTER:
            //analyze_setter(node, context);
            break;
        case NODE_GETTER:
            //analyze_getter(node, context);
            break;
        case NODE_BLOCK:
            analyze_block(node, context);
            break;
        case NODE_RETURN:
            analyze_return(node, context);
            break;
        case NODE_ASSIGN:
            analyze_assign(node, context);
            break;
        case NODE_CALL:
            analyze_call(node, context);
            break;
        case NODE_IF:
            analyze_if(node, context);
            break;
        case NODE_WHILE:
            analyze_while(node, context);
            break;
        // Expressions are handled in analyze_expression
        case NODE_ID:
        case NODE_BINOP:
        case NODE_LITERAL:
        case NODE_UNOP:
        case NODE_TERNARY:
            analyze_expression(node, context);
            break;
        // These nodes are handled as part of function declarations/calls
        case NODE_PARAM_LIST:
        case NODE_ARG_LIST:
            break; // Nothing to do here
    }
}

/*
* @brief Analyzes the root AST node (program)
* @param root Pointer to the root ASTNode of the program.
* @param pointer to the analysis context
*/
static void analyze_program(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing program node\n");
    }

    // Expecting program node to have one child: the main block
    if (node->child_count > 0 && node->children[0] != NULL) {
        analyze_node(node->children[0], context);
    }

    // After analyzing the program, check for undefined main function
    char* key = make_function_key("main", 0);
    SymbolData* main_func = symtable_lookup(context->global_table, key);

    if (main_func == NULL || !main_func->defined) {
        *context->error_code = ERR_SEMANTIC_UNDEFINED;
        if (context->debug) {
            fprintf(stderr, "Semantic Error: 'main' function is undefined\n");
        }
    }
    free(key);
}

/*
* @brief Analyzes the block AST node
* @param node Pointer to the block ASTNode.
* @param pointer to the analysis context
*/
static void analyze_block(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
      fprintf(stderr, "Semantic Analysis: Analyzing block node\n");
    }

    bool new_scope_created = false;

    // Blocks inside functions (IF, WHILE,...) create new local scope
    // The function body block scope is handled in analyze_function
    if (node->parent->type != NODE_FUNCTION) {
      SymTable* local_table = malloc(sizeof(SymTable));
      if (local_table == NULL || symtable_init(local_table) != ERR_OK) {
        *context->error_code = ERR_INTERNAL;
        return;
      }

      if (!push_Scope(context->scope_stack, local_table)) {
        *context->error_code = ERR_INTERNAL;
        return;
      }
      new_scope_created = true;
    }

    // Analyze all child nodes in the block
    for (size_t child_idx = 0; child_idx < node->child_count; child_idx++) {
        analyze_node(node->children[child_idx], context);
        if (*context->error_code != ERR_OK) {
            break; // Stop on first error
        }
    }

    // Pop the local scope after analyzing the block
    if (new_scope_created) {
        SymTable* old_table = pop_Scope(context->scope_stack);
        symtable_free(old_table);
        free(old_table);
    }
  }

/*
* @brief Analyzes the function AST node
* @param node Pointer to the function ASTNode.
* @param pointer to the analysis context
*/
static void analyze_function(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing function node\n");
    }

    // Get information about the function from the AST node
    const char* func_name = node->value;
    ASTNode* param_list = node->children[0]; // First child is parameter list
    ASTNode* func_block = node->children[1]; // Second child is function body
    size_t param_count = param_list->child_count;

    // Create function key (overloading supported)
    char* key = make_function_key(func_name, param_count);
    if (key == NULL) {
        *context->error_code = ERR_INTERNAL;
        return;
    }

    // Create and insert function symbol into the global table
    SymbolData func_data = create_function_symbol(TYPE_UNKNOWN, param_count);
    func_data.defined = true; // Function is now defined

    // Check for redefinition in the global scope
    ErrorCode insert_result = symtable_insert(context->global_table, key, func_data);
    if (insert_result == ERR_SEMANTIC_REDEFINITION) {
        *context->error_code = ERR_SEMANTIC_REDEFINITION;
        if (context->debug) {
            fprintf(stderr, "Semantic Error: Redefinition of function '%s'\n", func_name);
        }
        free(key);
        return; // Stop on redefinition error
    } else if (insert_result == ERR_INTERNAL) {
        *context->error_code = ERR_INTERNAL;
        free(key);
        return; // Stop on internal error
    }

    free(key);

    // Create a new symbol table for the function scope
    SymTable* func_table = malloc(sizeof(SymTable));
    if (func_table == NULL || symtable_init(func_table) != ERR_OK) {
        *context->error_code = ERR_INTERNAL;
        return;
    }

    // Push the function scope onto the scope stack
    if (!push_Scope(context->scope_stack, func_table)) {
        *context->error_code = ERR_INTERNAL;
        return;
    }

    // Insert parameters into the function's symbol table
    for (size_t param_idx = 0; param_idx < param_count; param_idx++) {
        ASTNode* param_node = param_list->children[param_idx];
        const char* param_name = param_node->value;


        // Create variable symbol for the parameter
        SymbolData param_data = create_variable_symbol(TYPE_UNKNOWN);
        // Insert parameter into function scope and check for redefinition
        insert_result = symtable_insert(func_table, param_name, param_data);
        if (insert_result == ERR_SEMANTIC_REDEFINITION) {
            *context->error_code = ERR_SEMANTIC_REDEFINITION;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Redefinition of parameter '%s' in function '%s'\n",
                        param_name, func_name);
            }
            return; // Stop on redefinition error
        } else if (insert_result == ERR_INTERNAL) {
            *context->error_code = ERR_INTERNAL;
            return; // Stop on internal error
        }


    }
    // Analyze the function body block
    context->current_function = node; // Set current function context
    analyze_node(func_block, context);
    context->current_function = NULL; // Clear current function context

    // Pop the function scope after analysis
    SymTable* old_table = pop_Scope(context->scope_stack);
    symtable_free(old_table);
    free(old_table);
}

/*
* @brief Analyzes the assign AST node (also var declaration)
*/
static void analyze_assign(ASTNode *node, AnalysisContext* context) {
    ASTNode* id_node = node->children[0]; // First child is the ID
    ASTNode* expr_node = node->children[1]; // Second child is the expression
    const char* var_name = id_node->value; // Variable name

    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing assign node for variable '%s'\n", var_name);
    }

    // Analyze the expression to get its type
    DataType expr_type = analyze_expression(expr_node, context);
    if (*context->error_code != ERR_OK) {
        return;
    }

    // Check if it is a declaration or assign
    // Parser creates for var id ASSIGN_NODE with literal null as expr
    bool is_declaration = false;
    if (expr_node->type == NODE_LITERAL && expr_node->data_type == TYPE_NULL) {
        is_declaration = true;
    }

    // Get the current scope symbol table
    SymTable* current_scope = top_Scope(context->scope_stack);

    // Declaration
    if (is_declaration) {
        // Check for redefinition in the current scope
        SymbolData var_data = create_variable_symbol(TYPE_NULL);
        ErrorCode insert_result = symtable_insert(current_scope, var_name, var_data);

        if (insert_result == ERR_SEMANTIC_REDEFINITION) {
            *context->error_code = ERR_SEMANTIC_REDEFINITION;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Redefinition of variable '%s'\n", var_name);
            }
            return;
        } else if (insert_result == ERR_INTERNAL) {
            *context->error_code = ERR_INTERNAL;
            return;
        } else {
            // Successfully declared variable with NULL type
            id_node->data_type = TYPE_NULL;
            return;
          }
    } else { // This is assign x = ...
        // Lookup the variable in all scopes
        SymbolData* var_symbol = scope_lookup(context->scope_stack, var_name);

        if (var_symbol == NULL) {
          // Is var global? (starts with __)
          if (strncmp(var_name, "__", 2) == 0) {
              // Is global variable, lookup or create in global table
              var_symbol = symtable_lookup(context->global_table, var_name);
              if (var_symbol == NULL) {
                // Create global variable
                SymbolData global_var_data = create_variable_symbol(expr_type);
                symtable_insert(context->global_table, var_name, global_var_data);
            } else {
                var_symbol->type = expr_type; // Update type
            }
            id_node->data_type = expr_type; // Set AST node type
        } else {
            // It is a local variable, which must be already declared
            *context->error_code = ERR_SEMANTIC_UNDEFINED;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Assignment to undefined variable '%s'\n", var_name);
            }
        }
    } else {
        // Symbol found, which one ?
        if (var_symbol->kind == SYM_SETTER) {
            // It is setter, check param type with expr_type
            if (var_symbol->info.setter.param_type != expr_type &&
                var_symbol->info.setter.param_type != TYPE_UNKNOWN &&
                expr_type != TYPE_UNKNOWN) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Type mismatch in setter '%s' assignment\n", var_name);
                }
            }
            id_node->data_type = TYPE_NULL; // Setter nothing returns
        } else if (var_symbol->kind == SYM_VARIABLE) {
            // It is var
            var_symbol->type = expr_type; // Update type
            id_node->data_type = expr_type; // Set AST node type
        } else {
            // cannot assign to function/getter
            *context->error_code = ERR_SEMANTIC_OTHER;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Invalid assignment target '%s'\n", var_name);
            }
        }
    }
  }
}

/* @brief Analyzes the call AST node
  * @param node Pointer to the call ASTNode.
  * @param pointer to the analysis context
*/
static void analyze_call(ASTNode* node, AnalysisContext* context) {
    const char* func_name = node->children[0]->value; // First child is function ID
    ASTNode* arg_list = node->children[1]; // Second child is argument list
    size_t arg_count = arg_list->child_count;

    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing call node for function '%s'\n", func_name);
    }

    // Analyze all argument expressions to get their types (recursive)
    for (size_t arg_idx = 0; arg_idx < arg_count; arg_idx++) {
        analyze_expression(arg_list->children[arg_idx], context);
        if (*context->error_code != ERR_OK) {
            return;
        }
    }

    // Create function key for lookup
    char* func_key = make_function_key(func_name, arg_count);
    if (func_key == NULL) {
        *context->error_code = ERR_INTERNAL;
        return;
    }

    // Lookup the function in the global symbol table
    SymbolData* func_data = symtable_lookup(context->global_table, func_key);
    free(func_key);

    // If not found, set error 3 or 5
    if (func_data == NULL) {
        if (func_exists(context->global_table, func_name)) {
            *context->error_code = ERR_SEMANTIC_FUNCTION; // Invalid argument count
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Function '%s' called with invalid count of arguments (%zu)\n", func_name, arg_count);
            }
        } else {
            *context->error_code = ERR_SEMANTIC_UNDEFINED; // Undefined function
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Undefined function call '%s'\n", func_name);
            }
        }
        return;
    }

    // Built-in func? Static checks
    if (strncmp(func_name, "Ifj.", 4) == 0) {
        // Built-in functions have been pre-defined in the global table
        // Check for specific built-in functions
        // Ifj.length(str: String) -> Num | Null
        if (strcmp(func_name, "Ifj.length") == 0) {
            ASTNode* arg = arg_list->children[0];
            // Argument must be String
            if (arg->type == NODE_LITERAL && arg->data_type != TYPE_STRING) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Argument type mismatch in call to 'Ifj.length'\n");
                }
            }
        } else if (strcmp(func_name, "Ifj.substring") == 0) {
            // Ifj.substring(str: String, start: Num, length: Num) -> String | Null
            ASTNode* arg1 = arg_list->children[0];
            ASTNode* arg2 = arg_list->children[1];
            ASTNode* arg3 = arg_list->children[2];
            // First argument must be String
            if (arg1->type == NODE_LITERAL && arg1->data_type != TYPE_STRING) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: First argument type mismatch in call to 'Ifj.substring'\n");
                }
            }
            // Second and third arguments must be Int
            if (arg2->type == NODE_LITERAL && arg2->data_type != TYPE_INT) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Second argument type mismatch in call to 'Ifj.substring'\n");
                }
            }

            if (arg3->type == NODE_LITERAL && arg3->data_type != TYPE_INT) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Third argument type mismatch in call to 'Ifj.substring'\n");
                }
            }
        } else if (strcmp(func_name, "Ifj.ord") == 0) {
            // Ifj.ord(str: String, index: Num) -> Num
            ASTNode* arg1 = arg_list->children[0];
            ASTNode* arg2 = arg_list->children[1];
            // First argument must be String
            if (arg1->type == NODE_LITERAL && arg1->data_type != TYPE_STRING) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: First argument type mismatch in call to 'Ifj.ord'\n");
                }
            }
            // Second argument must be Int
            if (arg2->type == NODE_LITERAL && arg2->data_type != TYPE_INT) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Second argument type mismatch in call to 'Ifj.ord'\n");
                }
            }
        } else if (strcmp(func_name, "Ifj.chr") == 0) {
            // Ifj.chr(ASCI code: Num) -> String
            ASTNode* arg1 = arg_list->children[0];
            // Argument must be Int
            if (arg1->type == NODE_LITERAL && arg1->data_type != TYPE_INT) {
                *context->error_code = ERR_SEMANTIC_TYPE;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Argument type mismatch in call to 'Ifj.chr'\n");
                }
            }
        }
    }
    // Set the call node's data type to the function's return type
    node->data_type = func_data->type;
}

/* @brief Analyzes the return AST node
  * @param node Pointer to the return ASTNode.
  * @param pointer to the analysis context
*/
static void analyze_return(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing return node\n");
    }

    // Get the current function's expected return type
    if (context->current_function == NULL) {
        *context->error_code = ERR_SEMANTIC_OTHER;
        if (context->debug) {
            fprintf(stderr, "Semantic Error: 'return' statement outside of function\n");
        }
        return;
    }

    const char* func_name = context->current_function->value;
    size_t param_count = context->current_function->children[0]->child_count;
    char* func_key = make_function_key(func_name, param_count);
    if (func_key == NULL) {
        *context->error_code = ERR_INTERNAL;
        return;
    }

    SymbolData* func_data = symtable_lookup(context->global_table, func_key);
    free(func_key);

    if (func_data == NULL) {
        *context->error_code = ERR_INTERNAL;
        return;
    }

    DataType expected_return_type = func_data->type;

    // Analyze the return expression to get its type
    ASTNode* expr_node = node->children[0]; // First child is the return expression
    DataType return_type = analyze_expression(expr_node, context);
    if (*context->error_code != ERR_OK) {
        return;
    }

    // Check if the return type matches the expected return type
    if (expected_return_type != TYPE_UNKNOWN && return_type != expected_return_type && return_type != TYPE_UNKNOWN) {
        *context->error_code = ERR_SEMANTIC_TYPE;
        if (context->debug) {
            fprintf(stderr, "Semantic Error: Return type mismatch in function '%s'\n", func_name);
        }
    }
}

static void analyze_if(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing if node\n");
    }

    // First child is the cond expression
    analyze_expression(node->children[0], context);
    if (*context->error_code != ERR_OK) {
        return;
    }

    // Second child is the if block
    analyze_node(node->children[1], context);
    if (*context->error_code != ERR_OK) {
        return;
      }

      // Optional third child is the else block (empty if not present)
      if (node->child_count > 2) {
          analyze_node(node->children[2], context);
      }
}

/* @brief Analyzes the while AST node
  * @param node Pointer to the while ASTNode.
  * @param pointer to the analysis context
*/
static void analyze_while(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing while node\n");
    }

    // First child is the cond expression
    analyze_expression(node->children[0], context);
    if (*context->error_code != ERR_OK) {
        return;
    }

    // Second child is the while block
    analyze_node(node->children[1], context);
}


static DataType analyze_expression(ASTNode* node, AnalysisContext* context) {
    // Base case: null node or error already occurred
    if (node == NULL || *context->error_code != ERR_OK) {
        return TYPE_UNKNOWN;
    }

    DataType result_type = TYPE_UNKNOWN;

    switch (node->type) {
        case NODE_LITERAL:
            // Literal node: type is directly available
            result_type = node->data_type;
            break;
        case NODE_ID: {
            // ID node, lookup in scope
            const char* var_name = node->value;
            SymbolData* var_symbol = scope_lookup(context->scope_stack, var_name);
            if (var_symbol == NULL) {
                // Is var global? (starts with __)
                if (strncmp(var_name, "__", 2) == 0) {
                    // Is global variable, lookup in global table
                    var_symbol = symtable_lookup(context->global_table, var_name);
                    if (var_symbol == NULL) {
                        // Global variable undefined, set type null
                        result_type = TYPE_NULL;
                    } else {
                        result_type = var_symbol->type;
                    }
                  } else {
                    // Undefined variable
                    *context->error_code = ERR_SEMANTIC_UNDEFINED;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Undefined variable '%s'\n", var_name);
                    }
                    return TYPE_UNKNOWN;
                  }
              } else {
                // Symbol found
                if (var_symbol->kind == SYM_GETTER) {
                    // getter, result type is getter return type
                    result_type = var_symbol->type;
                } else if (var_symbol->kind == SYM_VARIABLE) {
                    // var
                    result_type = var_symbol->type;
                } else {
                    // cannot use function/setter as variable
                    *context->error_code = ERR_SEMANTIC_OTHER;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Invalid use of '%s' as variable\n", var_name);
                    }
                    return TYPE_UNKNOWN;
                }
              }
            break;
        }

        case NODE_CALL:
            // Call node, analyze the call
            analyze_call(node, context);
            result_type = node->data_type; // Call node's data type is set in analyze_call
            break;

        case NODE_BINOP: {
            // Binary operation node
            // Recursively analyze left and right operands
            ASTNode* left_node = node->children[0];
            ASTNode* right_node = node->children[1];
            DataType left_type = analyze_expression(left_node, context);
            DataType right_type = analyze_expression(right_node, context);
            const char* operator = node->value;

            // See if error occurred during operand analysis
            if (*context->error_code != ERR_OK) {
                return TYPE_UNKNOWN;
            }


            // Type checking for arithmetic operators (+, -, *, /)
            if (strcmp(operator, "+") == 0) {
                // Addition or string concatenation
                if (is_num_type(left_type) && is_num_type(right_type)) {
                    // addition
                    // Atleast one operand is float -> result is float
                    result_type = (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                } else if (left_type == TYPE_STRING && right_type == TYPE_STRING) {
                    // String concatenation
                    result_type = TYPE_STRING;
                } else if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
                    result_type = TYPE_UNKNOWN; // Don't know yet
                } else {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Type mismatch in binary operation '%s'\n", operator);
                    }
                }

            // Type checking for subtraction, multiplication, division
            } else if (strcmp(operator, "-") == 0 ||
                       strcmp(operator, "*") == 0 ||
                       strcmp(operator, "/") == 0) {
                // Subtraction, multiplication, division
                if (is_num_type(left_type) && is_num_type(right_type)) {
                    // Atleast one operand is float -> result is float
                    result_type = (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                } else if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
                    result_type = TYPE_UNKNOWN; // Don't know yet
                    // Division by zero check maybe
                    // Can add probable result type based on left operand
                } else {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Type mismatch in binary operation '%s'\n", operator);
                    }
                }

            // Type checking for comparison operators (<, <=, >, >=)
            } else if (strcmp(operator, "<") == 0 ||
                       strcmp(operator, "<=") == 0 ||
                       strcmp(operator, ">") == 0 ||
                       strcmp(operator, ">=") == 0) {
                // Comparison operators, result type is always BOOL
                if (is_num_type(left_type) && is_num_type(right_type)) {
                    // Has to be both num types
                    result_type = TYPE_BOOL;
                } else if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
                    result_type = TYPE_BOOL; // Has to be bool or runtime error
                } else {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Type mismatch in comparison operation '%s'\n", operator);
                    }
                }

            // Type checking for equality operators (==, !=)
            } else if (strcmp(operator, "==") == 0 ||
                       strcmp(operator, "!=") == 0) {
                // Equality operators, result type is always BOOL
                // Allow comparison of any types
                result_type = TYPE_BOOL;

            // Type checking for logical operators (&&, ||)
            } else if (strcmp(operator, "&&") == 0 ||
                       strcmp(operator, "||") == 0) {
                // Logical operators works with truuthy/falsy values of any type
                result_type = TYPE_BOOL;

            // Type checking for IS operator
            } else if (strcmp(operator, "is") == 0) {
                // parser ensures right operand is type
                result_type = TYPE_BOOL;
            }
            break;
        }

        case NODE_UNOP: {
            // Unary operation node
            ASTNode* operand_node = node->children[0];
            DataType operand_type = analyze_expression(operand_node, context);
            const char* operator = node->value;

            // Type checking for unary minus (-)
            if (strcmp(operator, "-") == 0) {
                if (is_num_type(operand_type)) {
                    result_type = operand_type; // Result type is same as operand
                } else if (operand_type == TYPE_UNKNOWN) {
                    result_type = TYPE_UNKNOWN; // Don't know yet
                } else {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Type mismatch in unary operation '%s'\n", operator);
                    }
                }

            // Type checking for logical NOT (!)
            } else if (strcmp(operator, "!") == 0) {
                // Works with truthy/falsy values of any type
                result_type = TYPE_BOOL;
            }
            break;
        }

        case NODE_TERNARY: {
            // Ternary operation node (cond ? expr1 : expr2)
            ASTNode* cond_node = node->children[0];
            ASTNode* expr1_node = node->children[1];
            ASTNode* expr2_node = node->children[2];

            // Analyze condition
            analyze_expression(cond_node, context);

            // Analyze both expressions
            DataType true_type = analyze_expression(expr1_node, context);
            DataType false_type = analyze_expression(expr2_node, context);
            if (*context->error_code != ERR_OK) {
                return TYPE_UNKNOWN;
            }

            // If both types are the same, result type is that type
            if (true_type == false_type) {
                result_type = true_type;
            } else {
                result_type = TYPE_UNKNOWN; // Types differ, cannot determine
            }
            break;
        }

        default:
            break;
  }
  // Set the node's final or probable data type
    node->data_type = result_type;
    return result_type;
}


int semantic_analysis(ASTNode* root, bool debug) {
    if (root == NULL) {
        return ERR_INTERNAL; // No AST to analyze
    }

    int error_code = ERR_OK;

    // Initialize global symbol table
    SymTable* global_table = malloc(sizeof(SymTable));
    if (global_table == NULL || symtable_init(global_table) != ERR_OK) {
        free(global_table);
        fprintf(stderr, "Semantic Error: Failed to initialize global symbol table\n");
        return ERR_INTERNAL;
    }

    // Malloc scope stack
    ScopeStack* scope_stack = malloc(sizeof(ScopeStack));
    if (scope_stack == NULL) {
        symtable_free(global_table);
        free(global_table);
        fprintf(stderr, "Semantic Error: Failed to initialize scope stack\n");
        return ERR_INTERNAL;
    }
    init_Scope_Stack(scope_stack);

    // push global scope
    if (!push_Scope(scope_stack, global_table)) {
        symtable_free(global_table);
        free(global_table);
        free(scope_stack);
        fprintf(stderr, "Semantic Error: Failed to push global scope\n");
        return ERR_INTERNAL;
    }

    // Insert built-in functions into the global symbol table
    fill_global_table(global_table, &error_code);
    if (error_code != ERR_OK) {
        symtable_free(global_table);
        free(global_table);
        free(scope_stack);
        return error_code;
    }

    // prepare analysis context
    AnalysisContext context;
    context.global_table = global_table;
    context.scope_stack = scope_stack;
    context.current_function = NULL;
    context.debug = debug;
    context.error_code = &error_code;

    // Start analyzing from the root node
    analyze_node(root, &context);

    // Cleanup
    // Stack should have only global scope
    SymTable* global_scope = pop_Scope(scope_stack);
    symtable_free(global_scope);
    free(global_scope);
    free(scope_stack);

    return error_code;
}








