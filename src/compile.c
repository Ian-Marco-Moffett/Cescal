#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <compile.h>
#include <regs.h>
#include <ast.h>

#define GCC_PATH "/bin/gcc"
#define NASM_PATH "/bin/nasm"


static FILE* g_out_file = NULL;

static void prologue(void) {
  fputs(
          "global main\n"
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
          "\n"
          "main:\n"
          "\tpush rbp\n"
          "\tmov rbp, rsp\n",
  g_out_file);
}


static void epilogue(void) {
    fputs(
            "\n\tmov rax, 0\n"
            "\tleave\n"
            "\tret\n", g_out_file);
}


static REG_T ast_gen(struct ASTNode* n) {
    int leftreg, rightreg;

    if (n->left)
        leftreg = ast_gen(n->left);

    if (n->right)
        rightreg = ast_gen(n->right);

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
    }
}


void compile(struct ASTNode* n) {
    const char* const OUT_NAME = "/tmp/cescal-out.asm";
    g_out_file = fopen(OUT_NAME, "w");
    reg_init(g_out_file);

    prologue();
    reg_printint(ast_gen(n));
    epilogue();

    fclose(g_out_file);

    pid_t child = fork();
    if (child == 0) {
        execl(NASM_PATH, NASM_PATH, "-felf64", "-o./ces.o", OUT_NAME, NULL);
    } else {
        waitpid(child, 0, 0);
        remove(OUT_NAME);
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
