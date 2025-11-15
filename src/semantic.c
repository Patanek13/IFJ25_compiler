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

/* @brief Initializes the scope stack */
static void init_Scope_Stack(ScopeStack* stack) {
    stack->topIndex = -1;
}

/* @brief Pushes a symbol table onto the scope stack */
static bool push_Scope(ScopeStack* stack, SymTable* table) {
    if (stack->topIndex >= SCOPE_STACK_MAX - 1) {
        fprintf(stderr, "Semantic Error: Scope stack overflow\n");
        return false;
    }
    stack->tables[++stack->topIndex] = table;
    return true;
}

/* @brief Pops a symbol table from the scope stack */
static SymTable* pop_Scope(ScopeStack* stack) {
    if (stack->topIndex < 0) {
        return NULL;
    }
    return stack->tables[stack->topIndex--];
}

/* @brief Returns the top symbol table on the scope stack */
static SymbolData* top_Scope(ScopeStack* stack, const char* key) {
    for (int i = stack->topIndex; i >= 0; i--) {
        SymbolData* data = symtable_get(stack->tables[i], key);
        if (data != NULL) {
            return data; // Found in this scope
        }
    }
    return NULL; // Not found in any scope
}

/* @brief Search symbol only in the current (top) scope */
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


/* @brief Fills up the global table with built-in functions */
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
