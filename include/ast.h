#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <token.h>
#include <symbol.h>


typedef enum {
    A_EOF,          // Unused.
    A_ADD,
    A_SUB,
    A_MUL,
    A_DIV,
    A_EQ,
    A_NE,
    A_LT,
    A_GT,
    A_LE,
    A_GE,
    A_SHR,
    A_SHL,
    A_INTLIT,
    A_GLUE,
    A_FUNCTION,
    A_LINUX_PUTS,
    A_FUNCCALL,
    A_LVIDENT,
    A_ASSIGN,
    A_ID,
    A_STRLIT,
    A_IF,
    A_WHILE,
    A_RETURN,
    A_INLINE_ASM,
} AST_NODE_TYPE;


struct ASTNode {
    AST_NODE_TYPE op;
    SYMBOL_PTYPE type;
    struct ASTNode* left;
    struct ASTNode* right;
    const char* _asm;               // Assembly attachment for inline assembly.

    union {
        int64_t val_int;
        int64_t id;
    };
};


AST_NODE_TYPE arithop(TOKEN_TYPE tok);
struct ASTNode* mkastnode(AST_NODE_TYPE op, struct ASTNode* left, struct ASTNode* right, int64_t val_int);
struct ASTNode* mkastleaf(AST_NODE_TYPE op, int64_t int_val);
struct ASTNode* mkastleaf_type(AST_NODE_TYPE op, int64_t int_val, SYMBOL_PTYPE type);
struct ASTNode* mkastunary(AST_NODE_TYPE op, struct ASTNode* left, int64_t int_val);

#endif
