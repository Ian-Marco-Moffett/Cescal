#ifndef PARSER_H
#define PARSER_H


void parse(void);

/*
 *  Parses and doesn't reset compiler stuff.
 *
 */
void parse_noreset(void);

int64_t get_func_id(void);


#endif
