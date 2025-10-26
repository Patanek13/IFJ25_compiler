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

// Initialize symbol table
ErrorCode symtable_init(SymTable* table);

// Free symbol table
void symtable_free(SymTable* table);

// Hash function
size_t symtable_hash(const char* key, size_t table_size);

// Insert symbol into table
ErrorCode symtable_insert(SymTable* table, const char* key, SymbolData data);

// Lookup symbol in table
SymbolData *symtable_lookup(SymTable* table, const char* key);

// Delete symbol from table
ErrorCode symtable_delete(SymTable* table, const char* key);

// Helpers for SymbolData
SymbolData create_variable_symbol(DataType type, bool is_global);
SymbolData create_function_symbol(DataType return_type, size_t param_count);
SymbolData create_getter_symbol(DataType type);
SymbolData create_setter_symbol(DataType param_type);

// Function to make unique keys for functions
char *make_function_key(const char* name, size_t param_count);

#endif // IFJ_SYMTABLE_H
