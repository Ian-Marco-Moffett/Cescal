#include <regs.h>

static uint8_t reg_bmp = 0xFF;
static FILE* g_out_file;
static const char* const REGS[4] = {"r8", "r9", "r10", "r11"};
static const char* const BREGS[4] = { "r8b", "r9b", "r10b", "r11b" };

static REG_T reg_alloc(void) {
    for (REG_T reg = 0; reg < 4; ++reg) {
        if (reg_bmp & (1 << reg)) {
            reg_bmp &= ~(1 << reg);
            return reg;
        }
    }

    return -1;
}


static void reg_free(REG_T reg) {
    if (reg < 4) {
        reg_bmp |= (1 << reg);
    }
}


static REG_T regs_cmp(REG_T r1, REG_T r2, const char* how) {
    REG_T tmp = reg_alloc();

    fprintf(g_out_file, "\tcmp %s, %s\n", REGS[r1], REGS[r2]);
    fprintf(g_out_file, "\t%s %s\n", how, BREGS[r2]);
    fprintf(g_out_file, "\tand %s, 0xFF\n", REGS[r2]);
    reg_free(r1);
    return r2;
}

REG_T equal(REG_T r1, REG_T r2) { return(regs_cmp(r1, r2, "sete")); }
REG_T notequal(REG_T r1, REG_T r2) { return(regs_cmp(r1, r2, "setne")); }
REG_T lessthan(REG_T r1, REG_T r2) { return(regs_cmp(r1, r2, "setl")); }
REG_T greaterthan(REG_T r1, REG_T r2) { return(regs_cmp(r1, r2, "setg")); }
REG_T lessequal(REG_T r1, REG_T r2) { return(regs_cmp(r1, r2, "setle")); }
REG_T greaterequal(REG_T r1, REG_T r2) { return(regs_cmp(r1, r2, "setge")); }



REG_T reg_load(int64_t value) {
    REG_T r = reg_alloc();
    fprintf(g_out_file, "\tmov %s, %d\n", REGS[r], value);
    return r;
}


REG_T reg_add(REG_T r1, REG_T r2) {
    fprintf(g_out_file, "\tadd %s, %s\n", REGS[r1], REGS[r2]);
    reg_free(r2);
    return r1;
}


REG_T reg_sub(REG_T r1, REG_T r2) {
    fprintf(g_out_file, "\tsub %s, %s\n", REGS[r1], REGS[r2]);
    reg_free(r2);
    return r1;
}


REG_T reg_div(REG_T r1, REG_T r2) {
    fprintf(g_out_file, "\tmov rax, %s\n", REGS[r1]);
    fprintf(g_out_file, "\tcqo\n");
    fprintf(g_out_file, "\tidiv %s\n", REGS[r2]);
    fprintf(g_out_file, "\tmov %s, rax\n", REGS[r1]);
    reg_free(r2);
    return r1;
}


void reg_printint(REG_T r) {
    fprintf(g_out_file, "\tmov rdi, %s\n", REGS[r]);
    fprintf(g_out_file, "\tcall printint\n");
    reg_free(r);
}


REG_T reg_mul(REG_T r1, REG_T r2) {
    fprintf(g_out_file, "\timul %s, %s\n", REGS[r1], REGS[r2]);
    reg_free(r2);
    return r1;
}


void reg_init(FILE* out) {
    g_out_file = out;
}

void regs_free(void) {
    reg_bmp = 0xFF;
}
