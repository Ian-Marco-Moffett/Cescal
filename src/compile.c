#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <compile.h>
#include <regs.h>
#include <ast.h>
#include <symbol.h>
#include <panic.h>
#include <parser.h>

#define GCC_PATH "/bin/gcc"
#define NASM_PATH "/bin/nasm"
#define OUT_NAME "/tmp/cescal-out.asm"

#define ALIGN_UP(val, align)    (((val) + (align)-1) & ~((align)-1))


static FILE* g_out_file = NULL;
REG_T ast_gen(struct ASTNode* n, int reg, int parent_ast_top);

static uint64_t alloc_label(void) {
    static uint64_t i = 0;
    return i++;
}


static void gen_label(uint64_t label) {
    fprintf(g_out_file, "L%d:\n", label);
}


static REG_T cmpandset(int ast_top, REG_T r1, REG_T r2) {
    if (ast_top < A_EQ || ast_top > A_GE) {
        printf("__INTERNAL_ERROR__: Bad ast_top in %s()\n", __func__);
        panic();
    }

    static char* const CMPLIST[6] = {"sete", "setne", "setl", "setg", "setle", "setge"};

    fprintf(g_out_file, "\tcmp %s, %s\n", get_rreg_str(r1), get_rreg_str(r2));
    fprintf(g_out_file, "\t%s %s\n", CMPLIST[ast_top - A_EQ], get_breg_str(r2));
    fprintf(g_out_file, "\tmovzb %s, %s\n", get_rreg_str(r2), get_breg_str(r1));
    reg_free(r1);
    return r2;
}


static REG_T cmpandjmp(int ast_top, REG_T r1, REG_T r2, uint64_t label) {
    if (ast_top < A_EQ || ast_top > A_GE) {
        printf("__INTERNAL_ERROR__: Bad ast_top in %s()\n", __func__);
        panic();
    }

    static char* const INVERTED_CMPLIST[6] = {"jne", "je", "jge", "jle", "jg", "jl"};
    fprintf(g_out_file, "\tcmp %s, %s\n", get_rreg_str(r1), get_rreg_str(r2));
    fprintf(g_out_file, "\t%s L%d\n", INVERTED_CMPLIST[ast_top - A_EQ], label);
    regs_free();
    return -1;
}


static void jmp(uint64_t l) {
    fprintf(g_out_file, "\tjmp L%d\n", l);
}


static REG_T gen_if_ast(struct ASTNode* n) {
    int lfalse = alloc_label();

    ast_gen(n->left, lfalse, n->op);
    regs_free();

    ast_gen(n->right, -1, n->op);
    regs_free();

    jmp(lfalse);
    
    gen_label(lfalse);

    return -1;
}


static int gen_while_ast(struct ASTNode* n) {
    uint64_t lstart = alloc_label();
    uint64_t lend = alloc_label();

    gen_label(lstart);
    ast_gen(n->left, lend, n->op);

    regs_free();
    if (n->right) {
        ast_gen(n->right, -1, n->op);
    }
    regs_free();

    jmp(lstart);
    gen_label(lend);
}

static void prologue(void) {
  fputs(
          "extern printf\n\n"
          "section .data\n"
          "integer: db \"%d\", 0xA, 0\n\n"
          "string: db \"%s\", 0xA, 0\n\n"
          "section .text\n" 
          "printint:\n"
          "\tmov rsi, rdi\n"
          "\tmov rdi, integer\n"
          "\tpush rsp\n"
          "\tcall [rel printf wrt ..got]\n"
          "\tpop rsp\n"
          "\tret\n\n"
          "printstr:\n"
          "\tmov rsi, rdi\n"
          "\tmov rdi, string\n"
          "\tcall [rel printf wrt ..got]\n"
          "\tret\n"
          "\n",
  g_out_file);
}


static void kessy_kern_prologue(void) {
    fputs(
          "extern kputs\n\n"
          "section .text\n" 
          "printstr:\n"
          "\tcall kputs\n"
          "\tret\n"
          "\n",
    g_out_file);
}


static const char* const arg_regs_8[6] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char* const arg_regs_16[6] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char* const arg_regs_32[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char* const arg_regs_64[6] = {"rdi", "rsi", "rdx", "rcx", "r8d", "r9d"};

static void gen_func_prologue(int64_t func_id) {
    struct Symbol sym = g_globsymTable[func_id];

    if (sym.func_flags & FUNC_PUBLIC) {
        fprintf(g_out_file, "global %s\n", sym.name);
    }

    // Do not generate prologue if function has the 'naked' attribute.
    if (!(sym.func_flags & FUNC_NAKED)) {
        fprintf(g_out_file, 
                "section .text\n"
                "%s:\n"
                "\tpush rbp\n"
                "\tmov rbp, rsp\n", sym.name);

        struct Symbol sym = g_globsymTable[func_id];
        // Carve out n bytes of space.
        // The global symbol's rbp_off field
        // holds the max rbp off value (i.e rbp_off value for last local variable).
        if (sym.rbp_off != 0) {
            fprintf(g_out_file, "\tsub rsp, %d\n", ALIGN_UP(sym.rbp_off, 16));

            for (int i = 0; i < sym.n_local_symbols; ++i) {
                if (i < 6) {
                    switch (sym.local_symtbl[i].ptype) {
                        case P_U8:
                            fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.local_symtbl[i].rbp_off, arg_regs_8[i]);
                            break;
                        case P_U16:
                            fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.local_symtbl[i].rbp_off, arg_regs_16[i]);
                            break;
                        case P_U32:
                            fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.local_symtbl[i].rbp_off, arg_regs_32[i]);
                            break;
                        case P_U64:
                            fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.local_symtbl[i].rbp_off, arg_regs_32[i]);
                            break;
                    }
                } else {
                    // Start getting things off the stack now.
                    uint64_t rbp_off = 0;

                    switch (sym.local_symtbl[i].ptype) {
                        case P_U8:
                            rbp_off += 1;
                            fprintf(g_out_file, "\tmov r8b, [rbp+%d]\n\tmov [rbp-%d], r8b\n", rbp_off, sym.local_symtbl[i].rbp_off);
                            break;
                        case P_U16:
                            rbp_off += 2;
                            fprintf(g_out_file, "\tmov r8w, [rbp+%d]\n\tmov [rbp-%d], r8w\n", rbp_off, sym.local_symtbl[i].rbp_off);
                            break;
                        case P_U32:
                            rbp_off += 4;
                            fprintf(g_out_file, "\tmov r8d, [rbp+%d]\n\tmov [rbp-%d], r8d\n", rbp_off, sym.local_symtbl[i].rbp_off);
                            break;
                        case P_U64:
                            fprintf(g_out_file, "\tmov r8, [rbp+%d]\n\tmov [rbp-%d], r8\n", rbp_off, sym.local_symtbl[i].rbp_off);
                            rbp_off += 8;
                            break;
                    }
                }
            }
        }

    } else {
        fprintf(g_out_file, 
                "global %s\n\n"
                "section .text\n"
                "%s:\n", sym.name, sym.name);
    }
}


static void gen_func_epilogue(void) {
    fprintf(g_out_file,
            "\tleave\n"
            "\tretq\n\n");
}


static REG_T call(struct ASTNode* n) {
    uint32_t arg_i = 0;

    struct ASTNode* cur = n->left;
    while (1) {
        if (cur == NULL) {
            break;
        }

        REG_T r = ast_gen(cur->left, -1, 0);

        if (arg_i < 6) {
            switch (cur->val_int) {
                case P_U8:
                    fprintf(g_out_file, "\tmov %s, %s\n", arg_regs_8[arg_i], get_breg_str(r));
                    break;
                case P_U16:
                    fprintf(g_out_file, "\tmov %s, %s\n", arg_regs_16[arg_i], get_wreg_str(r));
                    break;
                case P_U32:
                    fprintf(g_out_file, "\tmovs %s, %s\n", arg_regs_32[arg_i], get_dreg_str(r));
                    break;
                case P_U64:
                    fprintf(g_out_file, "\tmov %s, %s\n", arg_regs_64[arg_i], get_rreg_str(r));
                    break;
            }
        } else {
            switch (cur->val_int) {
                case P_U8:
                    fprintf(g_out_file, "\tmovsx r8, %s\n\tpush r8", get_rreg_str(r));
                    break;
                case P_U16:
                    fprintf(g_out_file, "\tmovsx r8, %s\n\tpush r8", get_rreg_str(r));
                    break;
                case P_U32:
                    fprintf(g_out_file, "\tmovsxd r8, %s\n\tpush r8", get_rreg_str(r));
                    break;
                case P_U64:
                    fprintf(g_out_file, "\tmovsx r8, %s\n\tpush r8", get_rreg_str(r));
                    break;
            }
        }

        cur = cur->right;
    }


    fprintf(g_out_file, "\tcall %s\n", g_globsymTable[n->id].name);
    return RREG_RET;
}


static void ret(REG_T r, int64_t func_id) {
    switch (g_globsymTable[func_id].ptype) {
        case P_U8:
            fprintf(g_out_file, "\tmovsx rax, %s\n", get_breg_str(r));
            break;
        case P_U16:
            fprintf(g_out_file, "\tmovsx rax, %s\n", get_wreg_str(r));
            break;
        case P_U32:
            fprintf(g_out_file, "\tmovsxd rax, %s\n", get_dreg_str(r));
            break;
        case P_U64:
            fprintf(g_out_file, "\tmov rax, %s\n", get_rreg_str(r));
            break;
    }
}


void genglobsym(int64_t nameslot) {
    switch (g_globsymTable[nameslot].ptype) {
        case P_U8:
            fprintf(g_out_file, "\nsection .data\n%s: db 0\n\n", g_globsymTable[nameslot].name);
            break;
        case P_U16:
            fprintf(g_out_file, "\nsection .data\n%s: dw 0\n\n", g_globsymTable[nameslot].name);
            break;
        case P_U32:
            fprintf(g_out_file, "\nsection .data\n%s: dd 0\n\n", g_globsymTable[nameslot].name);
            break;
        case P_U64:
            fprintf(g_out_file, "\nsection .data\n%s: dq 0\n\n", g_globsymTable[nameslot].name);
            break;
        default:
            printf("__INTERNAL_ERROR__: Invalid ptype in %s()\n", __func__);
            panic();
    }
}


static size_t str_count = 0;
void genglobsym_str(const char* str) {
    fprintf(g_out_file, "\nsection .data\n_STR_%d_: db \"%s\", 0\n\n", str_count++, str);
}


size_t globsym_get_strcnt(void) {
    return str_count;
}


/*
 *  @param no_tab is 0 if inline assembly should be tabbed.
 *
 *
 */


static void insert_asm(struct ASTNode* asm_node, uint8_t no_tab) {
    if (asm_node != NULL) {
        if (no_tab == 0)
            fprintf(g_out_file, "\t%s\n", asm_node->_asm);
        else
            fprintf(g_out_file, "%s\n", asm_node->_asm);

        insert_asm(asm_node->right, no_tab);
    }
}


void gen_global_extern(const char* name) {
    fprintf(g_out_file, "extern %s\n", name);
}


REG_T ast_gen(struct ASTNode* n, int reg, int parent_ast_top) {
    int leftreg, rightreg;

    switch (n->op) {
        case A_IF:
            return gen_if_ast(n);
        case A_WHILE:
            return gen_while_ast(n);
        case A_GLUE:
            ast_gen(n->left, -1, n->op);
            regs_free();
            ast_gen(n->right, -1, n->op);
            regs_free();
            return -1;
        case A_FUNCTION:
            gen_func_prologue(n->id);

            if (n->left != NULL) {
                ast_gen(n->left, -1, n->op);
            }
    
            // Do not generate epilogue if function has the 'naked' attribute.
            if (!(g_globsymTable[n->id].func_flags & FUNC_NAKED))
                gen_func_epilogue();
            else
                fprintf(g_out_file, "\tud2\n\n");

            return -1;
        case A_FUNCCALL:
            return call(n);
        case A_INLINE_ASM:
            fprintf(g_out_file, ";; -- USER-GENERATED ASSEMBLY BEGINS HERE --\n\n");
            insert_asm(n, n->val_int);
            fprintf(g_out_file, ";; -- USER-GENERATED ASSEMBLY ENDS HERE --\n\n");
            return -1;
    }

    if (n->left)
        leftreg = ast_gen(n->left, -1, n->op);

    if (n->right)
        rightreg = ast_gen(n->right, leftreg, n->op);

    switch (n->op) {
        case A_ADD:
            return reg_add(leftreg, rightreg);
        case A_SUB:
            return reg_sub(leftreg, rightreg);
        case A_MUL:
            return reg_mul(leftreg, rightreg);
        case A_DIV:
            return reg_div(leftreg, rightreg);
        case A_SHR:
            return reg_shr(leftreg, rightreg);
        case A_SHL:
            return reg_shl(leftreg, rightreg);
        case A_EQ:
        case A_NE:
        case A_LT:
        case A_GT:
        case A_LE:
        case A_GE:
            if (parent_ast_top == A_IF || (parent_ast_top == A_WHILE))
                return cmpandjmp(n->op, leftreg, rightreg, reg);
            else
                return cmpandset(n->op, leftreg, rightreg);
        case A_INTLIT:
            return reg_load(n->val_int);
        case A_LVIDENT:
            return reg_store_var(reg, n->id, n->left->val_int);
        case A_STRLIT:
            return load_strlit(n->id);
        case A_ID:
            return load_var(n->id, n->val_int);
        case A_ASSIGN:
            return rightreg;
        case A_LINUX_PUTS:
            if (n->left->op == A_STRLIT) {
                reg_printstr(leftreg);
                regs_free();
                return -1;
            }

            reg_printint(leftreg);
            regs_free();
            return -1;
        case A_RETURN:
            // 1 means returning the result of a function call.
            if (n->val_int != 1)
                ret(leftreg, get_func_id());

            return -1;
        default:
            printf("__INTERNAL_ERROR__: Unknown AST operator [%d]!\n", n->op);
            panic();
    }
}


void compile_start(void) {
    if (!(get_flags() & COMPILE_FLAG_ASMONLY))
        g_out_file = fopen(OUT_NAME, "w");
    else
        g_out_file = fopen("cescal-out.asm", "w");

    reg_init(g_out_file);

    if (!(get_flags() & COMPILE_FLAG_FREESTANDING) && !(get_flags() & COMPILE_FLAG_KESSYKERNEL)) {
        prologue();
    } else if (get_flags() & COMPILE_FLAG_KESSYKERNEL) {
        kessy_kern_prologue();
    }
}


void compile_end(void) {
    extern const char* target_fname;
    fclose(g_out_file);

    char* obj_file = calloc(strlen(target_fname) + 8, sizeof(char));
    sprintf(obj_file, "-o./%s.o", target_fname);

    if (get_flags() & (COMPILE_FLAG_ASMONLY)) {
        return;
    }

    pid_t child = fork();
    if (child == 0) {
        execl(NASM_PATH, NASM_PATH, "-felf64", obj_file, OUT_NAME, NULL);
    } else {
        waitpid(child, 0, 0);
        kill(child, SIGKILL);
    }

    if (get_flags() & (COMPILE_FLAG_OBJ)) {
        free(obj_file);
        return;
    }

    child = fork();
    if (child == 0) {
        sprintf(obj_file, "%s.o", target_fname);
        execl(GCC_PATH, GCC_PATH, "-o./a.out", obj_file, "-no-pie", NULL);
    } else {
        sprintf(obj_file, "%s.o", target_fname);
        waitpid(child, 0, 0);
        remove(obj_file);
        free(obj_file);
        kill(child, SIGKILL);
    }
}
