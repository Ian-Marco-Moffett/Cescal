#include <ast.h>
#include <stdio.h>
#include <stddef.h>


static struct ASTNode** allocated_nodes = NULL;
static size_t nodes_alloc_idx = 0;


AST_NODE_TYPE arithop(TOKEN_TYPE tok) {
    switch (tok) {
        case TT_PLUS:
            return A_ADD;
        case TT_MINUS:
            return A_SUB;
        case TT_STAR:
            return A_MUL;
        case TT_SLASH:
            return A_DIV;
        default:
            printf("__INTERNAL_ERROR__: Invalid token in arithop()\n");
            return 0;
    }
}

struct ASTNode* mkastnode(AST_NODE_TYPE op, struct ASTNode* left, struct ASTNode* right, int64_t val_int) {
    struct ASTNode* n = malloc(sizeof(struct ASTNode));

    if (n == NULL) {
        printf("__INTERNAL_ERROR__: mkastnode() malloc() call failure! (out of memory)\n");
        return NULL;
    }

    if (allocated_nodes == NULL) {
        allocated_nodes = malloc(sizeof(struct ASTNode*));
    }

    allocated_nodes[nodes_alloc_idx++] = n;
    allocated_nodes = realloc(allocated_nodes, sizeof(struct ASTNode) * (nodes_alloc_idx + 1));

    if (allocated_nodes == NULL) {
        printf("__INTERNAL_ERROR__: mkastnode() realloc() call failure! (out of memory)\n");
        return NULL;
    }

    n->op = op;
    n->left = left;
    n->right = right;
    n->val_int = val_int;
    return n;
}

struct ASTNode* mkastleaf(AST_NODE_TYPE op, int64_t int_val) {
    return mkastnode(op, NULL, NULL, int_val);
}
