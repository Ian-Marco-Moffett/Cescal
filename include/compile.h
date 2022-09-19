#ifndef COMPILE_H
#define COMPILE_H

#include <ast.h>
#include <regs.h>

void compile_start(void);
void compile_end(void);
REG_T ast_gen(struct ASTNode* n, int reg, int parent_ast_top);

#endif
