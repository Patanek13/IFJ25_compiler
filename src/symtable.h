/**
 * @file symtable.h
 * @brief Symbol table definitions
 * @author Patrik Lošťák (xlostap00)
 * @details This file contains the definitions and declarations for the symbol table in the IFJ project.
 */

#ifndef IFJ_SYMTABLE_H
#define IFJ_SYMTABLE_H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include "error.h"

#define SYMTABLE_DEFAULT_SIZE 27457 // A prime number for better hash distribution

// ==== Data types =====================
typedef enum {
    TYPE_UNDEFINED,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_NULL,
    TYPE_BOOL
} DataType;

// ==== Type of symbol ====================
typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_GETTER,
    SYM_SETTER
} SymbolKind;

// ==== Symbol data structure for one entry in the symbol table =================
typedef struct SymbolData {
    SymbolKind kind; // variable, function, getter, setter
    DataType type; // return type for functions, data type for variables
    bool defined; // whether the symbol is defined
    bool is_global; // whether the symbol is global

    union {
        // For functions
        struct {
            DataType* param_types; // array of parameter types
            size_t param_count; // number of parameters
        } function;

        // For variables
        struct {
            // Additional variable-specific data maybe
        } variable;
    } info;
} SymbolData;

// ==== Symbol table entry =====================
typedef enum {
    SLOT_EMPTY,
    SLOT_OCCUPIED,
    SLOT_DELETED
} SlotStatus;

typedef struct SymItem {
    char* key; // symbol name
    SymbolData data; // symbol data
    SlotStatus state; // status of the slot
} SymItem;

// ==== Symbol table structure =====================
typedef struct SymTable {
    SymItem* items; // array of symbol table entries
    size_t size; // size of the table
    size_t count; // number of occupied entries
} SymTable;

// ==== Function prototypes =====

/*
 * @brief Initialize symbol table
 * @param table Pointer to the symbol table
 * @return ErrorCode ERR_OK on success, ERR_INTERNAL on failure
 */
ErrorCode symtable_init(SymTable* table);

/*
 * @brief Free symbol table resources
 * @param table Pointer to the symbol table
 */
void symtable_free(SymTable* table);

/*
 * @brief Hash function for symbol table
 * @param key Pointer to the symbol name
 * @return Hash value
 */
size_t symtable_hash(const char* key);

/*
 * @brief Insert symbol into table
 * @param table Pointer to the symbol table
 * @param key Pointer to the symbol name
 * @param data Symbol data to insert
 * @return ErrorCode ERR_OK on success, ERR_SEMANTIC_REDEFINITION if symbol already exists
 */
ErrorCode symtable_insert(SymTable* table, const char* key, SymbolData data);

/*
 * @brief Lookup symbol in table
 * @param table Pointer to the symbol table
 * @param key Pointer to the symbol name
 * @return Pointer to the symbol data if found, NULL otherwise
 */
SymbolData *symtable_lookup(SymTable* table, const char* key);

/*
 * @brief Delete symbol from table
 * @param table Pointer to the symbol table
 * @param key Pointer to the symbol name
 * @return ErrorCode ERR_OK on success, ERR_SEMANTIC_UNDEFINED if symbol does not exist
 */
ErrorCode symtable_delete(SymTable* table, const char* key);

/*
 * @brief Create a variable symbol
 * @param type Data type of the variable
 * @param is_global Whether the variable is global
 * @return SymbolData for the variable
 */
SymbolData create_variable_symbol(DataType type, bool is_global);

/*
 * @brief Create a function symbol
 * @param return_type Return type of the function
 * @param param_count Number of parameters
 * @return SymbolData for the function
 */
SymbolData create_function_symbol(DataType return_type, size_t param_count);

/*
 * @brief Create a getter symbol
 * @param type Data type of the getter
 * @return SymbolData for the getter
 */
SymbolData create_getter_symbol(DataType type);

/*
 * @brief Create a setter symbol
 * @param param_type Data type of the setter's parameter
 * @return SymbolData for the setter
 */
SymbolData create_setter_symbol(DataType param_type);

/*
 * @brief Generate a unique key for function symbols based on name and parameter count
 * @param name Function name
 * @param param_count Number of parameters
 * @return Dynamically allocated string representing the unique key
 */
char *make_function_key(const char* name, size_t param_count);

#endif // IFJ_SYMTABLE_H
