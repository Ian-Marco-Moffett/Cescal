#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <scanner.h>
#include <stddef.h>
#include <parser.h>
#include <symbol.h>

static char* g_buf = NULL;
static FILE* g_fp = NULL;

void panic(void) {
    if (g_buf != NULL) free(g_buf);
    if (g_fp != NULL) fclose(g_fp);
    scanner_destroy();
    destroy_symtbl();
    exit(1);
}

static void compile(FILE* fp) {
    // Get file size.
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate memory for file buffer.
    char* buf = calloc(fsize + 1, sizeof(char));
    fread(buf, sizeof(char), fsize, fp);
    
    sym_tbl_init();
    scanner_init(buf);
    parse();
    free(buf);
    destroy_symtbl();
}


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("sesc: Too few arguments!\n");
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        FILE* fp = fopen(argv[i], "r");

        if (!(fp)) {
            perror(argv[i]);
            return 1;
        }

        compile(fp);
    }

    return 0;
}
