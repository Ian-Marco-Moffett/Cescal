#include <stdio.h>
#include <stdint.h>
#include <parser.h>
#include <scanner.h>
#include <ast.h>
#include <panic.h>
#include <compile.h>
#include <symbol.h>

static struct Token last_tok;


static void passert(TOKEN_TYPE type, const char* what) {
    if (last_tok.type != type) {
        printf("Syntax error: Expected '%s' on line %d\n", what, last_tok.line_number == 0 ? 1 : last_tok.line_number);
        panic();
    }
}


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


static struct ASTNode* binexpr(uint64_t line) {
    struct ASTNode* left;
    struct ASTNode* right;

    left = primary();

    if (last_tok.type == TT_SEMI) {
        scan(&last_tok);
        return left;
    } else if (last_tok.type == TT_RBRACE || last_tok.type == TT_EOF) {
        printf("Syntax error: Missing semicolon, line %d\n", line);
        panic();
    }

    switch (last_tok.type) {
        case TT_PLUS:
        case TT_MINUS:
        case TT_STAR:
        case TT_SLASH:
        case TT_LESSTHAN:
        case TT_LESSTHANEQ:
        case TT_GREATERTHAN:
        case TT_GREATERTHANEQ:
        case TT_EQEQ:
        case TT_NOTEQUAL:
            break;
        default:
            printf("Syntax error: Expected '<arithmetic operator>' on line %d\n", last_tok.line_number);
            panic();
    }

    // Get node type.
    AST_NODE_TYPE ntype = arithop(last_tok.type);
    scan(&last_tok);

    // Get right-hand tree.
    right = binexpr(last_tok.line_number);
    return mkastnode(ntype, left, right, 0);
}


static void ident(void) {
    passert(TT_ID, "identifier");
}

static struct ASTNode* print_statement(void) {
    struct ASTNode* tree;
    int64_t reg;

    passert(TT_LINUX_PUTS, "__linux_puts");
    scan(&last_tok);
    tree = binexpr(last_tok.line_number);
    tree = mkastunary(A_LINUX_PUTS, tree, 0);
    return tree;
}


static struct ASTNode* funccall(void) {
    struct ASTNode* tree;
    int64_t id;

    if ((id = findglob(last_tok.tokstring)) == -1) {
        printf("Error: Undeclared function '%s', line %d\n", last_tok.tokstring, last_tok.line_number);
        panic();
    }

    scan(&last_tok);
    passert(TT_LPAREN, "(");
    scan(&last_tok);

    // TODO: ARGUMENTS!!!
    passert(TT_RPAREN, ")");
    scan(&last_tok);

    tree = mkastunary(A_FUNCCALL, tree, id);
    return tree;
}


static struct ASTNode* compound_statement(void) {
    struct ASTNode* left = NULL;
    struct ASTNode* tree = NULL;

    passert(TT_LBRACE, "{");
    scan(&last_tok);

    while (1) {
        switch (last_tok.type) {
            case TT_INTLIT:
                tree = binexpr(last_tok.line_number);
                break;
            case TT_RBRACE:
                scan(&last_tok);
                return left;
            case TT_LINUX_PUTS:
                tree = print_statement();
                break;
            case TT_ID:
                // TODO: CHANGE THIS LATER.
                tree =funccall();
                scan(&last_tok);            // Skip semi.
                break;
            default:
                printf("%d\n", last_tok.type);
                printf("Syntax error: Expected statement on line %d\n", last_tok.line_number);
                panic();
                break;
        }

        if (tree) {
            if (left == NULL)
                left = tree;
            else
                left = mkastnode(A_GLUE, left, tree, 0);
        }
    }
}


static struct ASTNode* func_decl(void) {
    struct ASTNode* tree;
    passert(TT_FUNC, "func");
    scan(&last_tok);
    ident();
    uint64_t nameslot = addglob(last_tok.tokstring);
    
    scan(&last_tok);
    passert(TT_EQUALS, "=>");
    scan(&last_tok);
    passert(TT_GREATERTHAN, "=>");
    scan(&last_tok);
    passert(TT_NULL, "null");                   // TODO: Allow other return types.
    scan(&last_tok);

    tree = compound_statement();

    return mkastunary(A_FUNCTION, tree, nameslot);
}


void parse(void) {
    compile_start();

    scan(&last_tok);
    while (!(scanner_is_eof())) {
        struct ASTNode* root = func_decl();
        ast_gen(root, -1, 0);
    }

    compile_end();
}
