#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <parser.h>
#include <scanner.h>
#include <ast.h>
#include <panic.h>
#include <compile.h>
#include <symbol.h>

static int64_t func_id = 0;
static struct Token last_tok;


static struct ASTNode* compound_statement(void);
static struct ASTNode* funccall(void);


static void passert(TOKEN_TYPE type, const char* what) {
    if (last_tok.type != type) {
        printf("%d\n", last_tok.type);
        printf("Syntax error: Expected '%s' on line %d\n", what, last_tok.line_number == 0 ? 1 : last_tok.line_number);
        panic();
    }
}


static SYMBOL_PTYPE tok2type(TOKEN_TYPE tok) {
    switch (tok) {
        case TT_NONE:
            return P_NONE;
        case TT_U8:
            return P_U8;
        case TT_U16:
            return P_U16;
        case TT_U32:
            return P_U32;
        case TT_U64:
            return P_U64;
        default:
            return P_INVALID;
    }
}


static SYMBOL_PTYPE value2ptype(int64_t value) {
    if (value < 0) {
        return P_INVALID;           // TODO.
    }

    if (value <= UCHAR_MAX) {
        return P_U8;
    } else if (value <= USHRT_MAX) {
        return P_U16;
    } else if (value <= UINT_MAX) {
        return P_U32;
    } else if (value <= ULLONG_MAX) {
        return P_U64;
    } else {
        printf("Error: Value overflows U64, line %d\n", last_tok.line_number);
        panic();
    }
}


static struct ASTNode* primary(void) {
    const struct ASTNode* n;
    int64_t id;

    switch (last_tok.type) {
        case TT_INTLIT:
            n = mkastleaf_type(A_INTLIT, last_tok.tokint, value2ptype(last_tok.tokint));
            scan(&last_tok);
            return (void*)n;
        case TT_ID:
            id = findglob(last_tok.tokstring);

            struct Token peek;
            scanner_peek(&peek);

            // If the token ahead's type is a '(' then 
            // this must be a function call.
            if (peek.type == TT_LPAREN) {
                return funccall();
            }

            if (id == -1) {
                printf("ERROR: Undeclared variable '%s' used on line %d\n", last_tok.tokstring, last_tok.line_number);
                panic();
            }
            n = mkastleaf(A_ID, id);
            scan(&last_tok);
            return (void*)n;
        case TT_STRINGLIT:
            free((void*)last_tok.tokstring);
            printf("ERROR: String variables are coming soon! (error on line %d)\n", last_tok.line_number);
            panic();
        default:
            printf("Syntax error: Expected expression on line %d\n", last_tok.line_number);
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
    } else if (last_tok.type == TT_RPAREN) {
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
            printf("%d\n", last_tok.type);
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

    passert(TT_PUTS, "puts");
    scan(&last_tok);
    
    if (last_tok.type == TT_STRINGLIT) {
        tree = mkastleaf(A_STRLIT, globsym_get_strcnt());
        genglobsym_str(last_tok.tokstring);
        free((void*)last_tok.tokstring);
        scanner_clear_cache();  

        tree = mkastunary(A_LINUX_PUTS, tree, 0);
        scan(&last_tok);
        passert(TT_SEMI, ";");
        scan(&last_tok);
        return tree;
    }

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


static struct ASTNode* var_def(SYMBOL_PTYPE ptype) {
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* tree;
    int64_t id;

    passert(TT_ID, "identifier");

    if (findglob(last_tok.tokstring) != -1) {
        printf("ERROR: Redeclaring variable on line %d\n", last_tok.line_number);
        panic();
    }

    size_t line = last_tok.line_number;
    uint64_t nameslot = addglob(last_tok.tokstring, S_VAR, ptype);
    scan(&last_tok);

    right = mkastleaf(A_LVIDENT, nameslot);
    
    // If variable is unassigned like so:
    // u8 var;
    if (last_tok.type == TT_SEMI) {
        genglobsym(nameslot);
        scan(&last_tok);
        return NULL;
    } else if (last_tok.type != TT_EQUALS) {
        printf("Syntax Error: Expected '=' or ';' on line %d\n", line);
        panic();
    }

    scan(&last_tok);
    
    left = binexpr(last_tok.line_number);

    if (left->type > ptype) {
        printf("Warning: Value assigned overflows variable type on line %d\n", line);
    }

    tree = mkastnode(A_ASSIGN, left, right, 0);
    genglobsym(nameslot);
    return tree;
}


static struct ASTNode* id(void) {
    struct Token peek;
    scanner_peek(&peek);

    if (peek.type == TT_LPAREN) {
        struct ASTNode* tree = funccall();
        scan(&last_tok);
        return tree;
    }

    int64_t id = findglob(last_tok.tokstring);

    if (id == -1) {
        printf("ERROR: Attempting to assign to non-existing variable '%s' on line %d\n", last_tok.tokstring, last_tok.line_number);
        panic();
    }

    if (g_globsymTable[id].stype != S_VAR) {
        printf("ERROR: Bro, you tried to assign a value to a function on line %d\n", last_tok.line_number);
        panic();
    }
    
    scan(&last_tok);
    struct ASTNode* right = mkastleaf(A_LVIDENT, id);
    
    // Must be reassignment for now.
    passert(TT_EQUALS, "=");
    scan(&last_tok);

    struct ASTNode* left = binexpr(last_tok.line_number);
    struct ASTNode* tree = mkastnode(A_ASSIGN, left, right, 0);
    return tree;
}


static struct ASTNode* if_statement(void) {
    struct ASTNode* conditionAST;
    struct ASTNode* trueAST;
    // struct ASTNode* falseAST; TODO

    scan(&last_tok);

    passert(TT_LPAREN, "(");
    scan(&last_tok);

    conditionAST = binexpr(last_tok.line_number);
    if (conditionAST->op < A_EQ || conditionAST->op > A_GE) {
        printf("Syntax error: Bad comparison operator on line %d\n", last_tok.line_number);
        panic();
    }

    passert(TT_RPAREN, ")");
    scan(&last_tok);

    trueAST = compound_statement();
    return mkastnode(A_IF, conditionAST, trueAST, 0);
}


static struct ASTNode* while_statement(void) {
    struct ASTNode* condition_ast;
    struct ASTNode* body_ast;

    scan(&last_tok);
    passert(TT_LPAREN, "(");
    scan(&last_tok);

    condition_ast = binexpr(last_tok.line_number);
    if ((condition_ast->op < A_EQ || condition_ast->op > A_GE) && condition_ast->op != A_INTLIT) {
        printf("Syntax error: Bad comparison operator on line %d\n", last_tok.line_number);
        panic();
    } else if (condition_ast->op == A_INTLIT && condition_ast->val_int == 0) {
        printf("Syntax error: Useless statment on line %d\n", last_tok.line_number);
        panic();
    }

    passert(TT_RPAREN, ")");
    scan(&last_tok);
    if (last_tok.type == TT_RBRACE) {
        return mkastnode(A_WHILE, condition_ast, NULL, 0);
    }
    body_ast = compound_statement();
    return mkastnode(A_WHILE, condition_ast, body_ast, 0);
}


static struct ASTNode* return_statement(void) {
    struct ASTNode* tree;
    SYMBOL_PTYPE return_type;

    if (g_globsymTable[func_id].ptype == P_NONE) {
        printf("Syntax Error: Cannot return a value from a function of type none, line %d\n", last_tok.line_number);
        panic();
    }

    scan(&last_tok);
    tree = binexpr(last_tok.line_number);
    return_type = tree->type;
    return mkastunary(A_RETURN, tree, 0);
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
            case TT_PUTS:
                tree = print_statement();
                break;
            case TT_ID:
                tree = id();
                break;
            case TT_U8:
                scan(&last_tok);
                tree = var_def(P_U8);
                break;
            case TT_U16:
                scan(&last_tok);
                tree = var_def(P_U16);
                break;
            case TT_U32:
                scan(&last_tok);
                tree = var_def(P_U32);
                break;
            case TT_U64:
                scan(&last_tok);
                tree = var_def(P_U64);
                break;
            case TT_IF:
                tree = if_statement();
                break;
            case TT_WHILE:
                tree = while_statement();
                break;
            case TT_RETURN:
                tree = return_statement();
                break;
            default:
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
    uint64_t nameslot = addglob(last_tok.tokstring, S_FUNC, 0);
    size_t line = last_tok.line_number;
    func_id = nameslot;
    
    scan(&last_tok);
    passert(TT_EQUALS, "=>");
    scan(&last_tok);
    passert(TT_GREATERTHAN, "=>");
    scan(&last_tok);

    SYMBOL_PTYPE ret_type = tok2type(last_tok.type);
    g_globsymTable[func_id].ptype = ret_type;

    // Ensure return type is valid.
    if (ret_type == P_INVALID) {
        printf("Syntax Error: Expected valid return type after '=>', line %d\n", line);
        panic();
    }

    scan(&last_tok);
    tree = compound_statement();

    return mkastunary(A_FUNCTION, tree, nameslot);
}


int64_t get_func_id(void) {
    return func_id;
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


void parse_noreset(void) {
    struct Token tmp = last_tok;

    scan(&last_tok);
    while (!(scanner_is_eof())) {
        struct ASTNode* root = func_decl();
        ast_gen(root, -1, 0);
    }

    last_tok = tmp;
}
