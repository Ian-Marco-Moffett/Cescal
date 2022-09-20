#include <preprocessor.h>
#include <scanner.h>
#include <panic.h>
#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INCLUDE_DIRECTIVE "@include"

void strremove(char* str, const char* sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char* p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
}


void preprocessor_include(const char* file, char** current_file_data, size_t cur_line) {
    /*
     *  We will append the included file to the top of the current
     *  file.
     *
     */

    FILE* fp = fopen(file, "r");

    if (!(fp)) {
        printf("ERROR: File '%s' included at line %d not found!\n", file, cur_line);
        panic();
    }

    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate memory for file buffer.
    char* buf = calloc(fsize + 1, sizeof(char));

    // Copy file contents to buffer.
    fread(buf, sizeof(char), fsize, fp);

    const char* const TMP = *current_file_data;
    scanner_change_buf(buf);
    set_current_filename(strdup(file));

    parse_noreset();

    set_current_filename(NULL);
    scanner_restore_buf();

    free(buf);
    fclose(fp);
}
