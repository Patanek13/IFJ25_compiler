/*
* @file semantic.c
* @author Patrik Lošťák (xlostap00)
* @brief Semantic Analysis implementation file
*/

#include "semantic.h"

// Enum for phase of semantic analysis
typedef enum {
    PHASE_DEFINITION, // Phase 1: Definition of functions, setters, getters
    PHASE_ANALYSIS // Phase 2: Full semantic analysis
} AnalysisPhase;

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

/*========= SEMANTIC ANALYSIS IMPLEMENTATION =========*/

/*========= Analysis Context Structure =========*/
// Context structure to hold analysis state
typedef struct {
    ScopeStack* scope_stack; // Stack of symbol tables for scopes
    bool debug;              // Debug flag
    int* error_code;        // Pointer to error code
    SymTable* global_table; // Global symbol table
    ASTNode* current_function; // Pointer to AST node of current function
    size_t block_depth;      // Current block depth
    const char* current_function_name; // Name of the current function
} AnalysisContext;

/*================================================================*/

/*======== Prototypes of Semantic Analysis Functions =============*/
static void analyze_node(ASTNode* node, AnalysisContext* context, AnalysisPhase phase);
static void fill_global_table(SymTable* global_table, int* error_code);

static void analyze_function(ASTNode* node, AnalysisContext* context, AnalysisPhase phase);
static void analyze_block(ASTNode* node, AnalysisContext* context, AnalysisPhase phase);
static void analyze_return(ASTNode* node, AnalysisContext* context);
static void analyze_assign(ASTNode* node, AnalysisContext* context);
static void analyze_call(ASTNode* node, AnalysisContext* context);
static void analyze_var_decl(ASTNode* node, AnalysisContext* context);
static void analyze_if(ASTNode* node, AnalysisContext* context, AnalysisPhase phase);
static void analyze_while(ASTNode* node, AnalysisContext* context, AnalysisPhase phase);
static DataType analyze_expression(ASTNode* node, AnalysisContext* context);
static void analyze_setter(ASTNode* node, AnalysisContext* context, AnalysisPhase phase);
static void analyze_getter(ASTNode* node, AnalysisContext* context, AnalysisPhase phase);
static bool func_exists(SymTable* table, const char* base_key);
/*===========================================================================*/

static bool is_numeric_ret_func(const char* func_name) {
    return (strcmp(func_name, "Ifj.read_num") == 0 ||
            strcmp(func_name, "Ifj.length") == 0 ||
            strcmp(func_name, "Ifj.floor") == 0) ||
            strcmp(func_name, "Ifj.strcmp") == 0 ||
            strcmp(func_name, "Ifj.ord") == 0;
}
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
static void analyze_node(ASTNode* node, AnalysisContext* context, AnalysisPhase phase) {
    if (node == NULL || *context->error_code != ERR_OK) {
        return;
    }

    if (context->debug) {
        fprintf(stderr, "Semantic Analysis: Analyzing node of type %d\n", node->type);
    }

    switch (node->type) {
        case NODE_PROGRAM:
            // Program just iterates its children
            for (size_t child_idx = 0; child_idx < node->child_count; child_idx++) {
                analyze_node(node->children[child_idx], context, phase);
                if (*context->error_code != ERR_OK) {
                    return; // Stop on first error
                }
            }
            return; // Stop to not analyze program again below
        // Phase 1: Definition
        case NODE_BLOCK:
            analyze_block(node, context, phase);
            return; // Stop to not analyze block again below

        case NODE_FUNCTION:
            analyze_function(node, context, phase);
            return; // Stop to not analyze function again below
        case NODE_SETTER:
            analyze_setter(node, context, phase);
            return; // Stop to not analyze setter again below
        case NODE_GETTER:
            analyze_getter(node, context, phase);
            return; // Stop to not analyze getter again below
        // Phase 2
        default:
            if (phase == PHASE_ANALYSIS) {
              switch (node->type) {
                  case NODE_RETURN:
                      analyze_return(node, context);
                      break;
                  case NODE_ASSIGN:
                      analyze_assign(node, context);
                      break;
                  case NODE_VAR_DECL:
                      analyze_var_decl(node, context);
                      break;
                  case NODE_CALL:
                      analyze_call(node, context);
                      break;
                  case NODE_IF:
                      analyze_if(node, context, phase);
                      break;
                  case NODE_WHILE:
                      analyze_while(node, context, phase);
                      break;
                  // Expressions are handled in analyze_expression
                  case NODE_ID:
                  case NODE_BINOP:
                  case NODE_LITERAL:
                  case NODE_UNOP:
                  case NODE_TERNARY:
                  case NODE_TYPE_ID:
                      analyze_expression(node, context);
                      break;
                  // These nodes are handled as part of function declarations/calls
                  case NODE_PARAM_LIST:
                  case NODE_ARG_LIST:
                      break; // Nothing to do here
                  default:
                      // SHOULD NOT REACH HERE
                      break;
              }
          }
          // In phase 1, other nodes are ignored
       break;
    }
}

/*
* @brief Analyzes the variable declaration AST node
* @param node Pointer to the variable declaration ASTNode.
* @param pointer to the analysis context
*/
static void analyze_var_decl(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
        fprintf(stdout, "Semantic Analysis: Analyzing variable declaration node\n");
    }

    // Check for var outside function
    if (context->current_function == NULL) {
        *context->error_code = SYNTAX_ERROR;
        if (context->debug) {
            fprintf(stderr, "Syntax Error: 'var' declaration outside function\n");
        }
        return;
    }

    // Get variable name from AST
    ASTNode* id_node = node->children[0];
    const char* var_name = id_node->value;

    if (strncmp(var_name, "__", 2) == 0) {
        *context->error_code = ERR_SEMANTIC_REDEFINITION;
        return;
    }

    SymTable* current_scope = top_Scope(context->scope_stack);

    // 1. Create Mangled Name: func_depth_var
    char mangled_name[256];
    const char* func_name = (context->current_function_name != NULL) ? context->current_function_name : "global";
    snprintf(mangled_name, sizeof(mangled_name), "%s_%zu_%s", func_name, context->block_depth, var_name);

    // 2. Store symbol with mangled name info
    SymbolData var_data = create_variable_symbol(TYPE_UNKNOWN);
    var_data.mangled_name = str_dup(mangled_name); // Store for later lookups

    ErrorCode insert_result = symtable_insert(current_scope, var_name, var_data);

    if (insert_result == ERR_SEMANTIC_REDEFINITION) {
        *context->error_code = ERR_SEMANTIC_REDEFINITION;
        // Cleanup the mangled name copy in local variable
        // create_variable_symbol creates a struct on stack. mangled_name pointer is copied.
        // If insert fails, we must free it because SymTable didn't take it.
        free(var_data.mangled_name);
        if (context->debug) fprintf(stderr, "Semantic Error: Redefinition of variable '%s'\n", var_name);

    } else if (insert_result == ERR_INTERNAL) {
        *context->error_code = ERR_INTERNAL;
        free(var_data.mangled_name);
        return;
    } else {
        // Success: Update AST
        free(id_node->value);
        id_node->value = str_dup(mangled_name); // AST has unique name
        id_node->frame = FRAME_LOCAL;
        id_node->data_type = TYPE_NULL;

        // In symtable_insert, the SymbolData is copied into the table.
        // pointer var_data.mangled_name is now owned by the table item. Do not free here.
    }
}

/*
* @brief Analyzes the block AST node
* @param node Pointer to the block ASTNode.
* @param pointer to the analysis context
*/
static void analyze_block(ASTNode* node, AnalysisContext* context, AnalysisPhase phase) {
    if (context->debug) {
      fprintf(stderr, "Semantic Analysis: Analyzing block node\n");
    }

    bool new_scope_created = false;

    // In analysis phase, create a new scope for the block
    if (phase == PHASE_ANALYSIS &&
        node->parent->type != NODE_FUNCTION &&
        node->parent->type != NODE_SETTER &&
        node->parent->type != NODE_GETTER) {


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

      context->block_depth++; // Increase block depth for new block
    }

    // Analyze all child nodes in the block
    for (size_t child_idx = 0; child_idx < node->child_count; child_idx++) {
        analyze_node(node->children[child_idx], context, phase);
        if (*context->error_code != ERR_OK) {
            break; // Stop on first error
        }
    }

    // Pop the local scope after analyzing the block
    if (new_scope_created) {
        SymTable* old_table = pop_Scope(context->scope_stack);
        symtable_free(old_table);
        free(old_table);
        context->block_depth--; // Decrease block depth after exiting block
    }
  }

/*
* @brief Analyzes the function AST node
* @param node Pointer to the function ASTNode.
* @param pointer to the analysis context
*/
static void analyze_function(ASTNode* node, AnalysisContext* context, AnalysisPhase phase) {
    if (context->debug) {
        fprintf(stderr, "Semantic Analysis: Analyzing function node\n");
    }

    // Get information about the function from the AST node
    const char* func_name = node->value;
    ASTNode* param_list = node->children[0]; // First child is parameter list
    ASTNode* func_block = node->children[1]; // Second child is function body
    size_t param_count = param_list->child_count;

    if (phase == PHASE_DEFINITION) {
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
      // No further analysis in definition phase
    }

    // Phase 2: Analyze function body
    if (phase == PHASE_ANALYSIS) {
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

    // Set current function name in context
    context->current_function_name = func_name;
    context->current_function = node; // Set current function AST node
    context->block_depth = 1; // Block depth starts at 1 inside function

    // Insert parameters into the function's symbol table
    for (size_t param_idx = 0; param_idx < param_count; param_idx++) {
        ASTNode* param_node = param_list->children[param_idx];
        const char* param_name = param_node->value;

        // Create variable symbol for the parameter
        SymbolData param_data = create_variable_symbol(TYPE_UNKNOWN);
        // Insert parameter into function scope and check for redefinition
        ErrorCode insert_result = symtable_insert(func_table, param_name, param_data);
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

         param_node->frame = FRAME_LOCAL; // Parameters are local
    }
    // Analyze the function body block
    analyze_node(func_block, context, phase);

    context->current_function = NULL; // Clear current function context
    context->current_function_name = NULL; // Clear current function name
    context->block_depth = 0; // Reset block depth

    // Pop the function scope after analysis
    SymTable* old_table = pop_Scope(context->scope_stack);
    symtable_free(old_table);
    free(old_table);
    }
}

/*
* @brief Analyzes the assign AST node
*/
static void analyze_assign(ASTNode *node, AnalysisContext* context) {
    ASTNode* id_node = node->children[0]; // First child is the ID
    ASTNode* expr_node = node->children[1]; // Second child is the expression
    const char* var_name = id_node->value; // Variable name

    if (context->debug) {
        fprintf(stderr, "Semantic Analysis: Analyzing assign node for variable '%s'\n", var_name);
    }

    DataType expr_type = analyze_expression(expr_node, context);
    if (*context->error_code != ERR_OK) return;

    // 1. Try find variable in scopes
    SymbolData* var_symbol = scope_lookup(context->scope_stack, var_name);

    // 2. If not found locally, check for Setter or Global
    if (var_symbol == NULL || var_symbol->kind != SYM_VARIABLE) {

        // SETTER CHECK
        char* setter_key = malloc(strlen(var_name) + 2);
        if (!setter_key) { *context->error_code = ERR_INTERNAL; return; }
        sprintf(setter_key, "%s=", var_name);

        SymbolData* setter_data = symtable_lookup(context->global_table, setter_key);

        if (setter_data != NULL && setter_data->kind == SYM_SETTER) {
            // Found Setter
            var_symbol = setter_data;

            // AST Update for Setter: Use "name=" and FRAME_GLOBAL
            free(id_node->value);
            id_node->value = setter_key; // Reuse allocated key

            id_node->frame = FRAME_GLOBAL;
            id_node->data_type = TYPE_NULL;
        }
        else {
            free(setter_key); // Setter not found, free key

            // Check if function exists with the name
        if (func_exists(context->global_table, var_name)) {
            *context->error_code = ERR_SEMANTIC_OTHER; // Function name used as variable
            if (context->debug) fprintf(stderr, "Semantic Error: Assignment to function name '%s'\n", var_name);
            return;
        }

        // Check for Getter used as variable
        SymbolData* getter_data = symtable_lookup(context->global_table, var_name);
        if (getter_data != NULL && getter_data->kind == SYM_GETTER) {
            *context->error_code = ERR_SEMANTIC_OTHER; // Getter name used as variable
            if (context->debug) fprintf(stderr, "Semantic Error: Assignment to getter name '%s'\n", var_name);
            return;
        }

            // GLOBAL VARIABLE CHECK
            if (strncmp(var_name, "__", 2) == 0) {
                var_symbol = symtable_lookup(context->global_table, var_name);
                if (var_symbol == NULL) {
                    // Auto-declare global
                    SymbolData global_var_data = create_variable_symbol(expr_type);
                    symtable_insert(context->global_table, var_name, global_var_data);
                    id_node->data_type = expr_type;
                    id_node->frame = FRAME_GLOBAL;
                    return;
                }
            } else {
                *context->error_code = ERR_SEMANTIC_UNDEFINED;
                if (context->debug) fprintf(stderr, "Semantic Error: Assignment to undefined variable '%s'\n", var_name);
                return;
            }
        }
    }

    // 3. Final AST Updates
    if (var_symbol->kind == SYM_SETTER) {
        // Type check for setter
        if (var_symbol->info.setter.param_type != expr_type &&
            var_symbol->info.setter.param_type != TYPE_UNKNOWN &&
            expr_type != TYPE_UNKNOWN) {
            *context->error_code = ERR_SEMANTIC_TYPE;
        }
        // Frame and Value already set above
    }
    else if (var_symbol->kind == SYM_VARIABLE) {
        // Variable Found
        var_symbol->type = expr_type;
        id_node->data_type = expr_type;

        if (strncmp(var_name, "__", 2) == 0) {
            id_node->frame = FRAME_GLOBAL;
        } else {
            id_node->frame = FRAME_LOCAL;
            // Apply Mangled Name from SymbolData
            if (var_symbol->mangled_name != NULL) {
                free(id_node->value);
                id_node->value = str_dup(var_symbol->mangled_name);
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
        fprintf(stderr, "Semantic Analysis: Analyzing call node for function '%s' return type: %s\n", func_name, data_type_to_string(node->data_type));
    }

    // Analyze all argument expressions to get their types (recursive)
    for (size_t arg_idx = 0; arg_idx < arg_count; arg_idx++) {
        analyze_expression(arg_list->children[arg_idx], context);
        if (*context->error_code != ERR_OK) {
            return;
        }
    }

    // Shawdowing check: function name used as variable
    SymbolData* shadow_var = scope_lookup(context->scope_stack, func_name);
    if (shadow_var != NULL && shadow_var->kind == SYM_VARIABLE) {
        // Trying to call a variable as function
        if (context->debug) {
            fprintf(stderr, "Semantic Error: Function name '%s' is shadowed by a variable\n", func_name);
        }
        *context->error_code = ERR_SEMANTIC_OTHER; // Function name shadowed
        return;
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
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Argument type mismatch in call to 'Ifj.length'\n");
                }

            }

            if (arg->type == NODE_CALL && arg->children[0]->type == NODE_ID) {
                if (is_numeric_ret_func(arg->children[0]->value)) {
                    *context->error_code = ERR_SEMANTIC_FUNCTION;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Argument is not String in call to 'Ifj.length'\n");
                    }
                    return;
                }
              }

        } else if (strcmp(func_name, "Ifj.substring") == 0) {
            // Ifj.substring(str: String, start: Num, length: Num) -> String | Null
            ASTNode* arg1 = arg_list->children[0];
            ASTNode* arg2 = arg_list->children[1];
            ASTNode* arg3 = arg_list->children[2];
            // First argument must be String
            if (arg1->type == NODE_LITERAL && arg1->data_type != TYPE_STRING) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: First argument type mismatch in call to 'Ifj.substring'\n");
                }
            }

            if (arg1->type == NODE_CALL && arg1->children[0]->type == NODE_ID) {
                if (is_numeric_ret_func(arg1->children[0]->value)) {
                    *context->error_code = ERR_SEMANTIC_FUNCTION;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: First argument is not String in call to 'Ifj.substring'\n");
                    }
                    return;
                }
              }
            // Second and third arguments must be Num (Int)
            if (arg2->type == NODE_LITERAL && !is_num_type(arg2->data_type)) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Second argument type mismatch in call to 'Ifj.substring'\n");
                }
            } else {
                if (arg2->data_type != TYPE_INT && arg2->data_type != TYPE_UNKNOWN) {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Second argument is not Int in call to 'Ifj.substring'\n");
                    }
                }
            }

            if (arg3->type == NODE_LITERAL && !is_num_type(arg3->data_type)) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Third argument type mismatch in call to 'Ifj.substring'\n");
                }
            } else {
                if (arg3->data_type != TYPE_INT && arg3->data_type != TYPE_UNKNOWN) {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Third argument is not Int in call to 'Ifj.substring'\n");
                    }
                }
            }
        } else if (strcmp(func_name, "Ifj.ord") == 0) {
            // Ifj.ord(str: String, index: Num) -> Num
            ASTNode* arg1 = arg_list->children[0];
            ASTNode* arg2 = arg_list->children[1];
            // First argument must be String
            if (arg1->type == NODE_LITERAL && arg1->data_type != TYPE_STRING) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: First argument type mismatch in call to 'Ifj.ord'\n");
                }
            }

            if (arg1->type == NODE_CALL && arg1->children[0]->type == NODE_ID) {
                if (is_numeric_ret_func(arg1->children[0]->value)) {
                    *context->error_code = ERR_SEMANTIC_FUNCTION;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: First argument is not String in call to 'Ifj.ord'\n");
                    }
                    return;
                }
              }

            // Second argument must be Num (Int)
            if (arg2->type == NODE_LITERAL && !is_num_type(arg2->data_type)) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Second argument type mismatch in call to 'Ifj.ord'\n");
                }
            } else {
                if (arg2->data_type != TYPE_INT && arg2->data_type != TYPE_UNKNOWN) {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Second argument is not Int in call to 'Ifj.ord'\n");
                    }
                }
            }
        } else if (strcmp(func_name, "Ifj.chr") == 0) {
            // Ifj.chr(ASCI code: Num) -> String
            ASTNode* arg1 = arg_list->children[0];
            // Argument must be Int
            if (arg1->type == NODE_LITERAL && !is_num_type(arg1->data_type)) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Argument type mismatch in call to 'Ifj.chr'\n");
                }
            } else {
                if (arg1->data_type != TYPE_INT && arg1->data_type != TYPE_UNKNOWN) {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Argument is not Int in call to 'Ifj.chr'\n");
                    }
                }
            }
        } else if (strcmp(func_name, "Ifj.floor") == 0) {
            // Ifj.floor(num: Num) -> Num
            ASTNode* arg1 = arg_list->children[0];
            // Argument must be Num
            if (arg1->type == NODE_LITERAL && !is_num_type(arg1->data_type)) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Argument type mismatch in call to 'Ifj.floor'\n");
                }
            }
        } else if (strcmp(func_name, "Ifj.strcmp") == 0) {
            // Ifj.strcmp(str1: String, str2: String) -> Num
            ASTNode* arg1 = arg_list->children[0];
            ASTNode* arg2 = arg_list->children[1];

            if (arg1->type == NODE_CALL && arg1->children[0]->type == NODE_ID) {
                if (is_numeric_ret_func(arg1->children[0]->value)) {
                    *context->error_code = ERR_SEMANTIC_FUNCTION;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: First argument is not String in call to 'Ifj.strcmp'\n");
                    }
                    return;
                }
              }

            if (arg2->type == NODE_CALL && arg2->children[0]->type == NODE_ID) {
                if (is_numeric_ret_func(arg2->children[0]->value)) {
                    *context->error_code = ERR_SEMANTIC_FUNCTION;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Second argument is not String in call to 'Ifj.strcmp'\n");
                    }
                    return;
                }
              }

            // Both arguments must be String
            if (arg1->type == NODE_LITERAL && arg1->data_type != TYPE_STRING) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: First argument type mismatch in call to 'Ifj.strcmp'\n");
                }
            }
            if (arg2->type == NODE_LITERAL && arg2->data_type != TYPE_STRING) {
                *context->error_code = ERR_SEMANTIC_FUNCTION;
                if (context->debug) {
                    fprintf(stderr, "Semantic Error: Second argument type mismatch in call to 'Ifj.strcmp'\n");
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
        fprintf(stderr, "Semantic Analysis: Analyzing return node\n");
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
    char* func_key = NULL;

    // Handle the type of func for right key
    if (context->current_function->type == NODE_GETTER) {
        // Getter has same key as name
        func_key = str_dup(func_name);
    } else if (context->current_function->type == NODE_SETTER) {
        // Setter key is name with '='
        func_key = str_dup(func_name);
    } else {
        // Regular function with "name#paramcount" key
        size_t param_count = context->current_function->children[0]->child_count; // First child is param list
        func_key = make_function_key(func_name, param_count); // Create function key
    }


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
    // If there is an expression, analyze it
    if (expr_node) {
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
    } else {
        // No return expression, expected type must be Null
        if (expected_return_type != TYPE_NULL && expected_return_type != TYPE_UNKNOWN) {
            *context->error_code = ERR_SEMANTIC_TYPE;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Function '%s' returns NULL but non-NULL value expected\n", func_name);
            }
        }
    }
}

static void analyze_if(ASTNode* node, AnalysisContext* context, AnalysisPhase phase) {
    if (context->debug) {
        fprintf(stderr, "Semantic Analysis: Analyzing if node\n");
    }

    // First child is the cond expression (only analyzed in analysis phase)
    if (phase == PHASE_ANALYSIS) {
        analyze_expression(node->children[0], context);
        if (*context->error_code != ERR_OK) {
            return;
        }
    }

    // Second child is the if block
    analyze_node(node->children[1], context, phase);
    if (*context->error_code != ERR_OK) {
        return;
      }

      // Optional third child is the else block (empty if not present)
      if (node->child_count > 2) {
          analyze_node(node->children[2], context, phase);
      }
}

/* @brief Analyzes the while AST node
  * @param node Pointer to the while ASTNode.
  * @param pointer to the analysis context
*/
static void analyze_while(ASTNode* node, AnalysisContext* context, AnalysisPhase phase) {
    if (context->debug) {
        fprintf(stderr, "Semantic Analysis: Analyzing while node\n");
    }

    // First child is the cond expression (only analyzed in analysis phase)
    if (phase == PHASE_ANALYSIS) {
        analyze_expression(node->children[0], context);
        if (*context->error_code != ERR_OK) {
            return;
        }
    }

    // Second child is the while block
    analyze_node(node->children[1], context, phase);
}


static DataType analyze_expression(ASTNode* node, AnalysisContext* context) {
    if (context->debug) {
            fprintf(stderr, "DEBUG: Analyzing expression node type: %d\n", node->type);
            if (node->type == NODE_BINOP) fprintf(stderr, "DEBUG: Binop operator: %s\n", node->value);
            if (node->type == NODE_TYPE_ID) fprintf(stderr, "DEBUG: Type ID: %s\n", node->value);
          }

    // Base case: null node or error already occurred
    if (node == NULL || *context->error_code != ERR_OK) {
        return TYPE_UNKNOWN;
    }

    DataType result_type = TYPE_UNKNOWN;

    switch (node->type) {
        case NODE_TYPE_ID:
            // Type is not value --> cannot be used in expressions
            // Maybe ERR_SEMANTIC_OTHER idk
            *context->error_code = ERR_SEMANTIC_TYPE;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Invalid use of type identifier '%s' in expression\n", node->value);
            }
            return TYPE_UNKNOWN;
        case NODE_LITERAL:
            // Literal node: type is directly available
            result_type = node->data_type;
            break;
        case NODE_ID: {
            const char* var_name = node->value;

            // 1. Lookup symbol in all available scopes (including global for getters)
            SymbolData* var_symbol = scope_lookup(context->scope_stack, var_name);

            // If not found, check if it is an implicit global variable (__)
            if (var_symbol == NULL) {
                if (strncmp(var_name, "__", 2) == 0) {
                    var_symbol = symtable_lookup(context->global_table, var_name);

                    if (var_symbol == NULL) {
                        // If still not found -> Auto-declare global variable
                        return TYPE_UNKNOWN;
                    }
                } else {
                    // It is not global (__) and was not found -> Error 3
                    *context->error_code = ERR_SEMANTIC_UNDEFINED;
                    if (context->debug) fprintf(stderr, "Semantic Error: Undefined variable '%s'\n", var_name);
                    return TYPE_UNKNOWN;
                }
            }

            // Symbol was found

            // --- A. It is a GETTER -> TRANSFORM TO NODE_CALL ---
            if (var_symbol->kind == SYM_GETTER) {

                // Save original name
                char* func_name_str = str_dup(var_name);
                if (!func_name_str) { *context->error_code = ERR_INTERNAL; return TYPE_UNKNOWN; }

                // Free old value of ID node (because NODE_CALL does not have value)
                free(node->value);
                node->value = NULL;

                // Rewrite node: Change it to a function call
                node->type = NODE_CALL;
                node->frame = FRAME_GLOBAL;       // Call to getter is a global jump
                node->data_type = var_symbol->type; // Getter return type

                // create children for CALL node
                // Name node
                ASTNode* func_id = ast_create_node(NODE_ID, func_name_str, TYPE_UNKNOWN);
                // empty argument list
                ASTNode* args = ast_create_node(NODE_ARG_LIST, NULL, TYPE_UNKNOWN);

                free(func_name_str); // Free the helper string (ast_create_node made a copy)

                if (!func_id || !args) {
                    *context->error_code = ERR_INTERNAL;
                    return TYPE_UNKNOWN;
                }

                // Clear old children of the node)
                if (node->children) {
                    free(node->children);
                }
                node->children = NULL;
                node->child_count = 0;

                // Attach new children to the node (which is now a CALL)
                ast_add_child(node, func_id);
                ast_add_child(node, args);

                return var_symbol->type;
            }

            // It is VARIABLE
            else if (var_symbol->kind == SYM_VARIABLE) {
                result_type = var_symbol->type;

                if (strncmp(var_name, "__", 2) == 0) {
                    // Global variable
                    node->frame = FRAME_GLOBAL;
                } else {
                    // Local variable
                    node->frame = FRAME_LOCAL;

                    // If we have a stored unique name (from definition), use it
                    // This ensures that 'x' inside a block translates to 'main_2_x'.
                    if (var_symbol->mangled_name != NULL) {
                        free(node->value);
                        node->value = str_dup(var_symbol->mangled_name);
                    }
                }
            }

            // Other kinds (FUNC, SETTER) cannot be used as variables
            else {
                // Func or setter cannot be used as a variable in an expression -> Error 10
                *context->error_code = ERR_SEMANTIC_OTHER;
                if (context->debug) fprintf(stderr, "Semantic Error: Invalid use of function/setter '%s' as variable\n", var_name);
                return TYPE_UNKNOWN;
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
            const char* operator = node->value;

            if (strcmp(operator, "is") == 0) {
                analyze_expression(left_node, context);
                if (*context->error_code != ERR_OK) {
                    return TYPE_UNKNOWN;
                }

                // Just analyze right node (type)
                if (right_node->type != NODE_TYPE_ID) {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Right operand of 'is' must be a type identifier\n");
                    }
                    result_type = TYPE_BOOL;
            }
          } else {
                DataType left_type = analyze_expression(left_node, context);
                DataType right_type = analyze_expression(right_node, context);

                if (*context->error_code != ERR_OK) {
                    return TYPE_UNKNOWN;
                }

            // Type checking for arithmetic operators (+, -, *, /)
            if (strcmp(operator, "+") == 0) {
                // Addition or string concatenation
                //DEBUG:
                if (context->debug) {
                    fprintf(stderr, "DEBUG ADD: Left type: %s, Right type: %s\n", data_type_to_string(left_type), data_type_to_string(right_type));
                }

                if (is_num_type(left_type) && is_num_type(right_type)) {
                    // addition
                    // Atleast one operand is float -> result is float
                    result_type = (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                } else if ((left_type == TYPE_STRING || left_type == TYPE_UNKNOWN) &&
                           (right_type == TYPE_STRING || right_type == TYPE_UNKNOWN || right_type == TYPE_NULL)) {
                    // String concatenation
                    if (left_type == TYPE_NULL || (right_node->type == NODE_LITERAL && right_type == TYPE_NULL)) {
                        // If TYPE_NULL literal, static error
                        *context->error_code = ERR_SEMANTIC_TYPE;
                        if (context->debug) {
                            fprintf(stderr, "Semantic Error: Cannot concatenate NULL to String\n");
                        }
                    } else {
                        // Allow String + Unknown or Var with TYPE_NULL type (could be string at runtime)
                        result_type = TYPE_STRING;
                    }
                } else if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
                    result_type = TYPE_UNKNOWN; // Don't know yet
                } else {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Type mismatch in binary operation '%s'\n", operator);
                    }
                }

            // Type checking for multiplication (*)
            } else if (strcmp(operator, "*") == 0) {
                if (is_num_type(left_type) && is_num_type(right_type)) {
                    // Multiplication
                    // Atleast one operand is float -> result is float
                    result_type = (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                } else if(left_type == TYPE_STRING) {
                    // String iteration --> string multiplied by Num (type int)
                    if (right_type == TYPE_INT) {
                        result_type = TYPE_STRING;
                    } else if (right_type == TYPE_UNKNOWN) {
                        result_type = TYPE_STRING; // Don't know yet
                    } else {
                        *context->error_code = ERR_SEMANTIC_TYPE;
                        if (context->debug) {
                            fprintf(stderr, "Semantic Error: Type mismatch in binary operation '%s'\n", operator);
                        }
                    }
                } else if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
                    result_type = TYPE_UNKNOWN; // Don't know yet
                } else {
                    *context->error_code = ERR_SEMANTIC_TYPE;
                    if (context->debug) {
                        fprintf(stderr, "Semantic Error: Type mismatch in binary operation '%s'\n", operator);
                    }
                }

            // Type checking for subtraction (-) and division (/)
            } else if (strcmp(operator, "-") == 0 ||
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
                // (5 < 10) && 5  ---> return 5 (Int)
                 // false && 5     ---> return false (Bool)
                // Just determine return type in AST, runtime decides
                if (left_type == right_type && left_type != TYPE_UNKNOWN) {
                    result_type = left_type; // Both same type
                } else {
                    // Different types, but still valid
                    result_type = TYPE_UNKNOWN; // Cannot determine statically
            }
          }
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

/* @brief Analyzes the setter AST node
* @param node Pointer to the setter ASTNode.
* @param pointer to the analysis context
*/
static void analyze_getter(ASTNode* node, AnalysisContext* context, AnalysisPhase phase) {
    if (context->debug) {
        fprintf(stderr, "Semantic Analysis: Analyzing getter node\n");
    }

    const char* getter_name = node->value;
    ASTNode* getter_body = node->children[1]; // Second child is body



    // Phase 1: definition
    if (phase == PHASE_DEFINITION) {
      // Have to make function key to read it as var
      // Save name without function key because getter is unique
      char* key = str_dup(getter_name);
      if (key == NULL) {
        *context->error_code = ERR_INTERNAL;
        free(key);
        return;
    }
        // Create getter symbol with return type
        SymbolData getter_data = create_getter_symbol(TYPE_UNKNOWN);

        // Insert getter into global table and check for redefinition
        ErrorCode insert_result = symtable_insert(context->global_table, key, getter_data);
        if (insert_result == ERR_SEMANTIC_REDEFINITION) {
            *context->error_code = ERR_SEMANTIC_REDEFINITION;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Redefinition of getter '%s'\n", getter_name);
            }
            free(key);
            return; // Stop on redefinition error
    } else if (insert_result == ERR_INTERNAL) {
        *context->error_code = ERR_INTERNAL;
        free(key);
        return; // Stop on internal error
      }
    free(key);
    }

    // Phase 2: analysis
    if (phase == PHASE_ANALYSIS) {
        // Frame is always global
        node->frame = FRAME_GLOBAL;
        // Analyze the getter body and create new local scope
        SymTable* local_table = malloc(sizeof(SymTable));
        if (local_table == NULL || symtable_init(local_table) != ERR_OK) {
            *context->error_code = ERR_INTERNAL;
            free(local_table);
            return;
        }

       if(!push_Scope(context->scope_stack, local_table)) {
            *context->error_code = ERR_INTERNAL;
            symtable_free(local_table);
            free(local_table);
            return;
        }

        ASTNode* old_func = context->current_function;
        context->current_function = node; // Set current function context for return type checks
        // Recursive analyze getter body
        analyze_node(getter_body, context, phase);

        context->current_function = old_func; // Restore previous function context

         // Pop the local scope after analyzing the getter
        SymTable* old_table = pop_Scope(context->scope_stack);
        symtable_free(old_table);
        free(old_table);
    }
}

static void analyze_setter(ASTNode* node, AnalysisContext* context, AnalysisPhase phase) {
    if (context->debug) {
        fprintf(stderr, "Semantic Analysis: Analyzing setter node\n");
    }

    const char* setter_name = node->value;
    ASTNode* param_node = node->children[0]; // First child is parameter list
    ASTNode* setter_body = node->children[1]; // Second child is body

    // Have to make diff key to not conflict with getter
    // setter key --> "name="
    char* key = malloc(strlen(setter_name) + 2); // +1 for '=' and +1 for '\0'
    if (key == NULL) {
        *context->error_code = ERR_INTERNAL;
        return;
    }
    sprintf(key, "%s=", setter_name);
    // Phase 1: definition
    if (phase == PHASE_DEFINITION) {
        // Create setter symbol with param type
        SymbolData setter_data = create_setter_symbol(TYPE_UNKNOWN);

        // Insert setter into global table and check for redefinition
        ErrorCode insert_result = symtable_insert(context->global_table, key, setter_data);
        if (insert_result == ERR_SEMANTIC_REDEFINITION) {
            *context->error_code = ERR_SEMANTIC_REDEFINITION;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Redefinition of setter '%s'\n", setter_name);
            }
            free(key);
            return; // Stop on redefinition error
    } else if (insert_result == ERR_INTERNAL) {
            *context->error_code = ERR_INTERNAL;
            free(key);
            return; // Stop on internal error
    }
  }

    // Phase 2: analysis
    if (phase == PHASE_ANALYSIS) {
        // Change ast value to setter key
        free(node->value); // Free old name
        node->value = str_dup(key); // Set setter key name
        node->frame = FRAME_GLOBAL; // Setter is always global frame

        // Analyze the setter body and create new local scope
        SymTable* local_table = malloc(sizeof(SymTable));
        if (local_table == NULL || symtable_init(local_table) != ERR_OK) {
            *context->error_code = ERR_INTERNAL;
            free(local_table);
            return;
        }

        if (!push_Scope(context->scope_stack, local_table)) {
            *context->error_code = ERR_INTERNAL;
            symtable_free(local_table);
            free(local_table);
            return;
        }

        // Insert parameter into setter's local scope (only one param, parser enforces it)
        if (param_node->child_count > 0) {
            const char* param_name = param_node->children[0]->value;
            SymbolData param_data = create_variable_symbol(TYPE_UNKNOWN);
            symtable_insert(local_table, param_name, param_data);
        }

        ASTNode* old_func = context->current_function;
        context->current_function = node; // Set current function context for return type checks
        // Recursive analyze setter body
        analyze_node(setter_body, context, phase);

        context->current_function = old_func; // Restore previous function context

        // Pop the local scope after analyzing the setter
        SymTable* old_table = pop_Scope(context->scope_stack);
        symtable_free(old_table);
        free(old_table);
    }
    free(key); // Free key after use
}

/*
* @brief Performs semantic analysis on the AST
* @param root Pointer to the root ASTNode of the program.
* @param debug Boolean flag to enable/disable debug output.
*/
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

    // Phase 1
    if (debug) {
        fprintf(stderr, "Semantic Analysis: Starting Phase 1\n");
    }

    analyze_node(root, &context, PHASE_DEFINITION);
    if (error_code != ERR_OK) {
        // Cleanup
        SymTable* global_scope = pop_Scope(scope_stack);
        symtable_free(global_scope);
        free(global_scope);
        free(scope_stack);
        return error_code;
    }

    // Phase 2
    if (debug) {
        fprintf(stderr, "Semantic Analysis: Starting Phase 2\n");
    }
    analyze_node(root, &context, PHASE_ANALYSIS);

    // Check for main function existence
    if (error_code == ERR_OK) {
        char* main_key = make_function_key("main", 0);
        if (main_key == NULL) {
            error_code = ERR_INTERNAL;
        } else {
            SymbolData* main_func = symtable_lookup(global_table, main_key);

            if (main_func == NULL || !main_func->defined) {
                error_code = ERR_SEMANTIC_UNDEFINED;
                if (debug) {
                    fprintf(stderr, "Semantic Error: Undefined 'main' function\n");
                }
            }
            free(main_key);
        }
    }

    // Cleanup
    // Stack should have only global scope
    SymTable* global_scope = pop_Scope(scope_stack);
    symtable_free(global_scope);
    free(global_scope);
    free(scope_stack);

    return error_code;
}
