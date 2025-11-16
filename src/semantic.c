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
  * @param key Symbol name to search for
  * @return Pointer to SymbolData if found, NULL otherwise
*/
static SymbolData* top_Scope(ScopeStack* stack, const char* key) {
    for (int i = stack->topIndex; i >= 0; i--) {
        SymbolData* data = symtable_get(stack->tables[i], key);
        if (data != NULL) {
            return data; // Found in this scope
        }
    }
    return NULL; // Not found in any scope
}

/* @brief Search symbol only in the current (top) scope
  * @param stack Pointer to the scope stack
  * @param key Symbol name to search for
  * @return Pointer to SymbolData if found, NULL otherwise
*/
static SymbolData* top_Scope_Current(ScopeStack* stack, const char* key) {
    if (stack->topIndex < 0) {
        return NULL;
    }
    return symtable_get(stack->tables[stack->topIndex], key);
}

/*========= Semantic Analysis Implementation =========*/

typedef struct {
    ScopeStack* scope_stack; // Stack of symbol tables for scopes
    bool debug;              // Debug flag
    int* error_code;        // Pointer to error code
    SymTable* global_table; // Global symbol table
    ASTNode* current_function; // Pointer to AST node of current function
} AnalysisContext;


/* @brief Fills up the global table with built-in functions
  * @param global_table Pointer to the global symbol table
*/
static void fill_global_table(SymTable* global_table) {
    char* key = NULL;
    SymbolData data;

    // Built-in function: Ifj.read_str() -> String | Null
    key = make_function_key("Ifj.read_str", 0);
    data = create_function_symbol(TYPE_UNKNOWN, 0);
    data.defined = true;
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.read_num() -> Num | Null
    key = make_function_key("Ifj.read_num", 0);
    data = create_function_symbol(TYPE_UNKNOWN, 0);
    data.defined = true;
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.write(value: Any) -> Null
    key = make_function_key("Ifj.write", 1);
    data = create_function_symbol(TYPE_NULL, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_UNKNOWN; // Any type
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.length(str: String) -> Num
    key = make_function_key("Ifj.length", 1);
    data = create_function_symbol(TYPE_INT, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.floor(num: Num) -> Num
    key = make_function_key("Ifj.floor", 1);
    data = create_function_symbol(TYPE_INT, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_FLOAT;
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.substring(str: String, start: Num, length: Num) -> String | Null
    key = make_function_key("Ifj.substring", 3);
    data = create_function_symbol(TYPE_UNKNOWN, 3);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    data.info.function.param_types[1] = TYPE_INT;
    data.info.function.param_types[2] = TYPE_INT;
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.str(value: Any) -> String
    key = make_function_key("Ifj.str", 1);
    data = create_function_symbol(TYPE_STRING, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_UNKNOWN; // Any type
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.strcmp(str1: String, str2: String) -> Num
    key = make_function_key("Ifj.strcmp", 2);
    data = create_function_symbol(TYPE_INT, 2);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    data.info.function.param_types[1] = TYPE_STRING;
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.ord(str: String, index: Num) -> Num
    key = make_function_key("Ifj.ord", 2);
    data = create_function_symbol(TYPE_INT, 2);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_STRING;
    data.info.function.param_types[1] = TYPE_INT;
    symtable_insert(global_table, key, data);
    free(key);

    // Built-in function: Ifj.chr(ASCI code: Num) -> String
    key = make_function_key("Ifj.chr", 1);
    data = create_function_symbol(TYPE_STRING, 1);
    data.defined = true;
    data.info.function.param_types[0] = TYPE_INT;
    symtable_insert(global_table, key, data);
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
            analyze_setter(node, context);
            break;
        case NODE_GETTER:
            analyze_getter(node, context);
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
    for (size_t i = 0; i < node->child_count; i++) {
        analyze_node(node->children[i], context);
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

    // Check for redefinition in the global scope
    if (symtable_lookup(context->global_table, key) != NULL) {
        *context->error_code = ERR_SEMANTIC_REDEFINITION;
        if (context->debug) {
            fprintf(stderr, "Semantic Error: Redefinition of function '%s'\n", func_name);
        }
        free(key);
        return;
    }

    // Create and insert function symbol into the global table
    SymbolData func_data = create_function_symbol(TYPE_UNKNOWN, param_count);
    func_data.defined = true; // Function is now defined

    if (symtable_insert(context->global_table, key, func_data) != ERR_OK) {
        *context->error_code = ERR_INTERNAL;
        free(key);
        return;
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
    for (size_t i = 0; i < param_count; i++) {
        ASTNode* param_node = param_list->children[i];
        const char* param_name = param_node->value;

        // Check for redefinition inside parameters
        if (top_Scope_Current(context->scope_stack, param_name) != NULL) {
            *context->error_code = ERR_SEMANTIC_REDEFINITION;
            if (context->debug) {
                fprintf(stderr, "Semantic Error: Redefinition of parameter '%s' in function '%s'\n", param_name, func_name);
            }
            continue; // Continue checking other parameters
        }

        // Create variable symbol for the parameter
        SymbolData param_data = create_variable_symbol(TYPE_UNKNOWN);
        symtable_insert(func_table, param_name, param_data);

        //


}
