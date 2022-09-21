#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <compile.h>
#include <regs.h>
#include <ast.h>
#include <symbol.h>
#include <panic.h>

#define GCC_PATH "/bin/gcc"
#define NASM_PATH "/bin/nasm"
#define OUT_NAME "/tmp/cescal-out.asm"


static FILE* g_out_file = NULL;

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
          "\tpush rsp\n"
          "\tcall [rel printf wrt ..got]\n"
          "\tpop rsp\n"
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


static void gen_func_prologue(const char* name) {
    fprintf(g_out_file, 
            "global %s\n\n"
            "section .text\n"
            "%s:\n"
            "\tpush rbp\n"
            "\tmov rbp, rsp\n", name, name);
}


static void gen_func_epilogue(void) {
    fprintf(g_out_file,
            "\tmov rax, 0\n"
            "\tleave\n"
            "\tretq\n\n");
}


static void call(const char* func_name) {
    fprintf(g_out_file, "\tcall %s\n", func_name);
}


void genglobsym(int64_t nameslot) {
    switch (g_globsymTable[nameslot].ptype) {
        case P_U8:
            fprintf(g_out_file, "\nsection .data\n%s: db 0\n\n", g_globsymTable[nameslot]);
            break;
        case P_U16:
            fprintf(g_out_file, "\nsection .data\n%s: dw 0\n\n", g_globsymTable[nameslot]);
            break;
        case P_U32:
            fprintf(g_out_file, "\nsection .data\n%s: dd 0\n\n", g_globsymTable[nameslot]);
            break;
        case P_U64:
            fprintf(g_out_file, "\nsection .data\n%s: dq 0\n\n", g_globsymTable[nameslot]);
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



REG_T ast_gen(struct ASTNode* n, int reg, int parent_ast_top) {
    int leftreg, rightreg;

    switch (n->op) {
        case A_GLUE:
            ast_gen(n->left, -1, n->op);
            regs_free();
            ast_gen(n->right, -1, n->op);
            regs_free();
            return -1;
        case A_FUNCTION:
            gen_func_prologue(g_globsymTable[n->id].name);
            if (n->left != NULL) {
                ast_gen(n->left, -1, n->op);
            }
            gen_func_epilogue();
            return -1;
        case A_FUNCCALL:
            call(g_globsymTable[n->id].name);
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
        case A_EQ:
            return equal(leftreg, rightreg);
        case A_NE:
            return notequal(leftreg, rightreg);
        case A_LT:
            return lessthan(leftreg, rightreg);
        case A_GT:
            return greaterthan(leftreg, rightreg);
        case A_LE:
            return lessequal(leftreg, rightreg);
        case A_GE:
            return greaterequal(leftreg, rightreg);
        case A_INTLIT:
            return reg_load(n->val_int);
        case A_LVIDENT:
            return reg_store_glob(reg, n->id);
        case A_STRLIT:
            return load_strlit(n->id);
        case A_ID:
            return load_glob(n->id);
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
    fclose(g_out_file);

    if (get_flags() & (COMPILE_FLAG_ASMONLY)) {
        return;
    }

    pid_t child = fork();
    if (child == 0) {
        execl(NASM_PATH, NASM_PATH, "-felf64", "-o./ces.o", OUT_NAME, NULL);
    } else {
        waitpid(child, 0, 0);
        kill(child, SIGKILL);
    }

    if (get_flags() & (COMPILE_FLAG_OBJ)) {
        return;
    }

    child = fork();
    if (child == 0) {
        execl(GCC_PATH, GCC_PATH, "-o./a.out", "ces.o", "-no-pie", NULL);
    } else {
        waitpid(child, 0, 0);
        remove("ces.o");
        kill(child, SIGKILL);
    }
}
