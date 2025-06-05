#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

// Splits 'input' into fields separated by 'sep'.
// 'fields' is an array of char* buffers, each of size 'field_buf_size'.
// 'max_fields' is the maximum number of fields to extract.
// Returns the number of fields found.
int parse_fields(const char *input, char sep, char **fields, size_t max_fields, size_t field_buf_size);

#endif // PARSER_H
