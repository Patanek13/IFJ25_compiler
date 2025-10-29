#include "../src/ast.h"
#include "../src/strutils.h"
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

/*
* static add(a, b) {
*     return a + b;
* }
*/

void test_ast_creation_and_free() {
   // static add(a, b) { return a + b }
    ASTNode *program = ast_create_node(NODE_PROGRAM, NULL);
    ASSERT(program != NULL, "AST program node created");
    ASTNode *func = ast_create_node(NODE_FUNCTION, "add");
    ASSERT(func != NULL, "AST function node created");
    ASTNode *paramA = ast_create_node(NODE_ID, "a");
    ASTNode *paramB = ast_create_node(NODE_ID, "b");
    ASSERT(paramA != NULL, "AST parameter A node created");
    ASSERT(paramB != NULL, "AST parameter B node created");
    ASTNode *block = ast_create_node(NODE_BLOCK, NULL);
    ASSERT(block != NULL, "AST block node created");
    ASTNode *ret = ast_create_node(NODE_RETURN, NULL);
    ASSERT(ret != NULL, "AST return node created");
    ASTNode *binop = ast_create_node(NODE_BINOP, "+");
    ASSERT(binop != NULL, "AST binary operation node created");
    ASTNode *left = ast_create_node(NODE_ID, "a");
    ASTNode *right = ast_create_node(NODE_ID, "b");
    ASSERT(left != NULL, "AST left operand node created");
    ASSERT(right != NULL, "AST right operand node created");

    ast_add_child(binop, left);
    ASSERT(binop->child_count == 1, "AST binary operation has 1 child after adding left");
    ast_add_child(binop, right);
    ASSERT(binop->child_count == 2, "AST binary operation has 2 children after adding right");
    ast_add_child(ret, binop);
    ASSERT(ret->child_count == 1, "AST return has 1 child after adding binop");
    ast_add_child(block, ret);
    ASSERT(block->child_count == 1, "AST block has 1 child after adding return");
    ast_add_child(func, paramA);
    ASSERT(func->child_count == 1, "AST function has 1 child after adding paramA");
    ast_add_child(func, paramB);
    ASSERT(func->child_count == 2, "AST function has 2 children after adding paramB");
    ast_add_child(func, block);
    ASSERT(func->child_count == 3, "AST function has 3 children after adding block");
    ast_add_child(program, func);
    ASSERT(program->child_count == 1, "AST program has 1 child after adding function");

    ast_print_debug(program, 0);
    ast_free(program);
    ASSERT(true, "AST freed without crashes");
}

int main(void) {
    test_ast_creation_and_free();

    printf("\nTests passed: %d, failed: %d\n", tests_passed, tests_failed);

    return tests_failed == 0 ? 0 : 1;
}
