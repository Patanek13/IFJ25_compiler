#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/symtable.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { \
        printf("[PASS] %s\n", msg); \
        tests_passed++; \
    } else { \
        printf("[FAIL] %s\n", msg); \
        tests_failed++; \
    } \
} while (0)

int main(void) {
    SymTable table;
    ErrorCode rc = symtable_init(&table);
    ASSERT(rc == ERR_OK, "symtable_init returns ERR_OK");
    ASSERT(table.items != NULL, "table.items is allocated");
    ASSERT(table.count == 0, "table.count is zero after init");

    // Insert a variable
    SymbolData var = create_variable_symbol(TYPE_NUM);
    rc = symtable_insert(&table, "x", var);
    ASSERT(rc == ERR_OK, "Insert variable x returns ERR_OK");

    SymbolData *found = symtable_lookup(&table, "x");
    ASSERT(found != NULL, "Lookup x returns non-NULL");
    if (found) {
        ASSERT(found->kind == SYM_VARIABLE, "x kind is SYM_VARIABLE");
        ASSERT(found->type == TYPE_NUM, "x type is TYPE_NUM");
    }

    // Insert a function
    SymbolData fn = create_function_symbol(TYPE_NUM, 2);
    rc = symtable_insert(&table, "f#2", fn);
    ASSERT(rc == ERR_OK, "Insert function f#2 returns ERR_OK");

    SymbolData *ff = symtable_lookup(&table, "f#2");
    ASSERT(ff != NULL, "Lookup f#2 returns non-NULL");
    if (ff) {
        ASSERT(ff->kind == SYM_FUNCTION, "f#2 kind is SYM_FUNCTION");
        ASSERT(ff->info.function.param_count == 2, "f#2 param_count is 2");
    }

    // Redefinition should fail
    rc = symtable_insert(&table, "x", var);
    ASSERT(rc == ERR_SEMANTIC_REDEFINITION, "Reinserting x returns ERR_SEMANTIC_REDEFINITION");

    // Delete x
    rc = symtable_delete(&table, "x");
    ASSERT(rc == ERR_OK, "Delete x returns ERR_OK");
    SymbolData *after_del = symtable_lookup(&table, "x");
    ASSERT(after_del == NULL, "Lookup x after delete returns NULL");

    // Delete non-existing should return ERR_SEMANTIC_UNDEFINED
    rc = symtable_delete(&table, "nonexist");
    ASSERT(rc == ERR_SEMANTIC_UNDEFINED, "Deleting non-existing returns ERR_SEMANTIC_UNDEFINED");

    // Cleanup
    symtable_free(&table);

    /* Additional tests */

    // symtable_make_getter and symtable_make_setter
    SymbolData getter = create_getter_symbol(TYPE_STRING);
    ASSERT(getter.kind == SYM_GETTER, "make_getter sets kind SYM_GETTER");
    ASSERT(getter.type == TYPE_STRING, "make_getter sets correct return type");

    SymbolData setter = create_setter_symbol(TYPE_NUM);
    ASSERT(setter.kind == SYM_SETTER, "make_setter sets kind SYM_SETTER");
    ASSERT(setter.info.function.param_count == 1, "make_setter param_count == 1");
    if (setter.info.function.param_types) {
        ASSERT(setter.info.function.param_types[0] == TYPE_NUM, "make_setter param type correct");
        free(setter.info.function.param_types); // cleanup
    }

    // make_function_key
    char *k = make_function_key("foo", 3);
    ASSERT(k != NULL, "make_function_key returns non-NULL");
    if (k) {
        ASSERT(strcmp(k, "foo#3") == 0, "make_function_key produces expected key");
        free(k);
    }

    // Collision and reuse tests on a small table
    SymTable small = {0};
    small.size = 7; // small size to force collisions
    small.count = 0;
    small.items = calloc(small.size, sizeof(SymItem));
    ASSERT(small.items != NULL, "small table allocated");

    SymbolData v1 = create_variable_symbol(TYPE_NUM);
    SymbolData v2 = create_variable_symbol(TYPE_STRING);
    SymbolData v3 = create_variable_symbol(TYPE_BOOL);

    rc = symtable_insert(&small, "alpha", v1);
    ASSERT(rc == ERR_OK, "insert alpha into small table");
    rc = symtable_insert(&small, "bravo", v2);
    ASSERT(rc == ERR_OK, "insert bravo into small table");
    rc = symtable_insert(&small, "charlie", v3);
    ASSERT(rc == ERR_OK, "insert charlie into small table");

    // Delete middle entry and ensure reuse
    rc = symtable_delete(&small, "bravo");
    ASSERT(rc == ERR_OK, "delete bravo from small table");

    SymbolData v4 = create_variable_symbol(TYPE_NULL);
    rc = symtable_insert(&small, "delta", v4);
    ASSERT(rc == ERR_OK, "insert delta reuses deleted slot");

    // Lookup checks
    ASSERT(symtable_lookup(&small, "alpha") != NULL, "lookup alpha in small table");
    ASSERT(symtable_lookup(&small, "bravo") == NULL, "lookup bravo returns NULL after delete");
    ASSERT(symtable_lookup(&small, "delta") != NULL, "lookup delta in small table");

    // Cleanup small table
    symtable_free(&small);

    /* Test multiple functions with same name but different parameter counts */
    SymTable funcs = {0};
    rc = symtable_init(&funcs);
    ASSERT(rc == ERR_OK, "init funcs table");

    char *k1 = make_function_key("over", 1);
    char *k2 = make_function_key("over", 2);
    ASSERT(k1 != NULL && k2 != NULL, "make_function_key for overloaded names");

    SymbolData of1 = create_function_symbol(TYPE_NUM, 1);
    SymbolData of2 = create_function_symbol(TYPE_NUM, 2);

    rc = symtable_insert(&funcs, k1, of1);
    ASSERT(rc == ERR_OK, "insert overloaded function over#1");
    rc = symtable_insert(&funcs, k2, of2);
    ASSERT(rc == ERR_OK, "insert overloaded function over#2");

    SymbolData *got1 = symtable_lookup(&funcs, k1);
    SymbolData *got2 = symtable_lookup(&funcs, k2);
    ASSERT(got1 != NULL && got2 != NULL, "lookup both overloaded functions");
    if (got1) ASSERT(got1->info.function.param_count == 1, "over#1 has 1 param");
    if (got2) ASSERT(got2->info.function.param_count == 2, "over#2 has 2 params");

    free(k1); free(k2);
    symtable_free(&funcs);

    printf("\nTests passed: %d, failed: %d\n", tests_passed, tests_failed);

    return tests_failed == 0 ? 0 : 1;
}
