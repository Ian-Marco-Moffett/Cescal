#include <stdio.h>
#include <parser.h>
#include <scanner.h>
#include <ast.h>
#include <panic.h>
#include <compile.h>

static struct Token last_tok;


static struct ASTNode* primary(void) {
    const struct ASTNode* n;

    switch (last_tok.type) {
        case TT_INTLIT:
            n = mkastleaf(A_INTLIT, last_tok.tokint);
            scan(&last_tok);
            return (void*)n;
        default:
            printf("Syntax error: Expected integer on line %d\n", last_tok.line_number);
            panic();
    }
}


static struct ASTNode* binexpr(void) {
    struct ASTNode* left;
    struct ASTNode* right;

    left = primary();

    if (last_tok.type == TT_EOF) {
        return left;
    }

    // Get node type.
    AST_NODE_TYPE ntype = arithop(last_tok.type);
    scan(&last_tok);

    // Get right-hand tree.
    right = binexpr();
    return mkastnode(ntype, left, right, 0);
}

void parse(void) {
    scan(&last_tok);
    struct ASTNode* root = binexpr();
    compile(root);
}
