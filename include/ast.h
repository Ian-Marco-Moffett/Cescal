#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <token.h>


typedef enum {
    A_ADD,
    A_SUB,
    A_MUL,
    A_DIV,
    A_INTLIT
} AST_NODE_TYPE;


struct ASTNode {
    AST_NODE_TYPE op;
    struct ASTNode* left;
    struct ASTNode* right;
    int64_t val_int;
};


AST_NODE_TYPE arithop(TOKEN_TYPE tok);
struct ASTNode* mkastnode(AST_NODE_TYPE op, struct ASTNode* left, struct ASTNode* right, int64_t val_int);
struct ASTNode* mkastleaf(AST_NODE_TYPE op, int64_t int_val);


#endif
