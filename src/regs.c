#include <regs.h>
#include <symbol.h>
#include <parser.h>
#include <panic.h>
#include <stddef.h>

static uint8_t reg_bmp = 0xFF;
static FILE* g_out_file;
static const char* const REGS[5] = {"r8", "r9", "r10", "r11", "rax"};
static const char* const BREGS[5] = {"r8b", "r9b", "r10b", "r11b", "rax"};
static const char* const WREGS[5] = {"r8w", "r9w", "r10w", "r11w", "rax"};
static const char* const DREGS[5] = {"r8d", "r9d", "r10d", "r11d", "rax"};

static REG_T reg_alloc(void) {
    for (REG_T reg = 0; reg < 4; ++reg) {
        if (reg_bmp & (1 << reg)) {
            reg_bmp &= ~(1 << reg);
            return reg;
        }
    }

    return -1;
}


void reg_free(REG_T reg) {
    if (reg < 4) {
        reg_bmp |= (1 << reg);
    }
}


const char* get_rreg_str(REG_T r) {
    return REGS[r];
}

const char* get_breg_str(REG_T r) {
    return BREGS[r];
}

const char* get_dreg_str(REG_T r) {
    return DREGS[r];
}

const char* get_wreg_str(REG_T r) {
    return WREGS[r];
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


REG_T reg_shl(REG_T r1, REG_T r2) {
    fprintf(g_out_file, "\tmov cl, %s\n", BREGS[r2]);
    fprintf(g_out_file, "\tshl %s, cl\n", REGS[r1]);
    reg_free(r2);
    return r1;
}


REG_T reg_shr(REG_T r1, REG_T r2) {
    fprintf(g_out_file, "\tmov cl, %s\n", BREGS[r2]);
    fprintf(g_out_file, "\tshr %s, cl\n", REGS[r1]);
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

void reg_printstr(REG_T r) {
    fprintf(g_out_file, "\tmov rdi, %s\n", REGS[r]);
    fprintf(g_out_file, "\tcall printstr\n");
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


REG_T reg_store_var(REG_T r, int64_t nameslot) {
    struct Symbol sym = g_globsymTable[nameslot];
    const char* glob_name = g_globsymTable[nameslot].name;

    if (get_func_id() != -1)
        sym = sym.local_symtbl[nameslot];

    switch (sym.ptype) {
        case P_U8:
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.rbp_off, BREGS[r]);
                break;
            }

            fprintf(g_out_file, "\tmov byte [%s], %s\n", glob_name, BREGS[r]);
            break;
        case P_U16:
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.rbp_off, WREGS[r]);
                break;
            }

            fprintf(g_out_file, "\tmov word [%s], %s\n", glob_name, WREGS[r]);
            break;
        case P_U32:
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.rbp_off, DREGS[r]);
                break;
            }

            fprintf(g_out_file, "\tmov dword [%s], %s\n", glob_name, DREGS[r]);
            break;
        case P_U64: 
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmov [rbp-%d], %s\n", sym.rbp_off, REGS[r]);
                break;
            }

            fprintf(g_out_file, "\tmov qword [%s], %s\n", glob_name, REGS[r]);
            break;
        default:
            printf("__INTERNAL_ERROR__: Invalid ptype in %s()\n", __func__);
            panic();
    }

    return r;
}


REG_T load_var(int64_t nameslot) {
    struct Symbol sym;
    const char* glob_name = NULL;

    if (get_func_id() != -1) {
        sym = g_globsymTable[get_func_id()].local_symtbl[nameslot];
        glob_name = sym.name;
    } else {
        sym = g_globsymTable[nameslot];
    }

    REG_T alloc = reg_alloc(); 
    
    switch (sym.ptype) {
        case P_U8:
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmovsx %s, byte [rbp-%d]\n", REGS[alloc], sym.rbp_off);
                break;
            }

            fprintf(g_out_file, "\tmovsx %s, byte [%s]\n", REGS[alloc], glob_name);
            break;
        case P_U16:
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmovsx %s, word [rbp-%d]\n", REGS[alloc], sym.rbp_off);
                break;
            }

            fprintf(g_out_file, "\tmovsx %s, word [%s]\n", REGS[alloc], glob_name);
            break;
        case P_U32:
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmovsxd %s, dword [rbp-%d]\n", REGS[alloc], sym.rbp_off);
                break;
            }

            fprintf(g_out_file, "\tmovsxd %s, dword [%s]\n", REGS[alloc], glob_name);
            break;
        case P_U64:
            if (get_func_id() != -1) {
                fprintf(g_out_file, "\tmov %s, [rbp-%d]\n", REGS[alloc], sym.rbp_off);
                break;
            }

            fprintf(g_out_file, "\tmov %s, qword [%s]\n", REGS[alloc], glob_name);
            break;
        default:
            printf("__INTERNAL_ERROR__: Invalid ptype in %s()\n", __func__);
            panic();
    }

    return alloc;
}


REG_T load_local(int64_t nameslot);

REG_T load_strlit(size_t str_num) {
    REG_T alloc = reg_alloc();
    fprintf(g_out_file, "\tmov %s, _STR_%d_\n", REGS[alloc], str_num);
    return alloc;
}
