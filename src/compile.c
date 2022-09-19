#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <compile.h>
#include <regs.h>
#include <ast.h>
#include <symbol.h>

#define GCC_PATH "/bin/gcc"
#define NASM_PATH "/bin/nasm"
#define OUT_NAME "/tmp/cescal-out.asm"


static FILE* g_out_file = NULL;

static void prologue(void) {
  fputs(
          "extern printf\n\n"
          "section .data\n"
          "string: db \"%d\", 0xA, 0\n\n"
          "section .text\n" 
          "printint:\n"
          "\tmov rsi, rdi\n"
          "\tmov rdi, string\n"
          "\tpush rsp\n"
          "\tcall [rel printf wrt ..got]\n"
          "\tpop rsp\n"
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
            gen_func_prologue(g_globsymTable[n->id]);
            if (n->left != NULL) {
                ast_gen(n->left, -1, n->op);
            }
            gen_func_epilogue();
            return -1;
        case A_FUNCCALL:
            call(g_globsymTable[n->id]);
            return -1;
    }

    if (n->left)
        leftreg = ast_gen(n->left, -1, n->op);

    if (n->right)
        rightreg = ast_gen(n->right, -1, n->op);

    switch (n->op) {
        case A_ADD:
            return reg_add(leftreg, rightreg);
        case A_SUB:
            return reg_sub(leftreg, rightreg);
        case A_MUL:
            return reg_mul(leftreg, rightreg);
        case A_DIV:
            return reg_div(leftreg, rightreg);
        case A_INTLIT:
            return reg_load(n->val_int);
        case A_LINUX_PUTS:
            reg_printint(leftreg);
            regs_free();
            return -1; 
    }
}


void compile_start(void) {
    g_out_file = fopen(OUT_NAME, "w");
    reg_init(g_out_file);
    prologue();
}


void compile_end(void) {
    fclose(g_out_file);

    pid_t child = fork();
    if (child == 0) {
        execl(NASM_PATH, NASM_PATH, "-felf64", "-o./ces.o", OUT_NAME, NULL);
    } else {
        waitpid(child, 0, 0);
        // remove(OUT_NAME);
        kill(child, SIGKILL);
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
