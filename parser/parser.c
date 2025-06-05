#include "parser.h"
#include <string.h>

int parse_fields(const char *input, char sep, char **fields, size_t max_fields, size_t field_buf_size) {
    if (!input || !fields || max_fields == 0 || field_buf_size == 0)
        return 0;

    size_t input_len = strlen(input);
    if (input_len == 0) return 0;

    size_t field_count = 0;
    const char *start = input;
    const char *end = input;
    while (*end && field_count < max_fields) {
        if (*end == sep) {
            size_t len = end - start;
            if (len >= field_buf_size) len = field_buf_size - 1;
            strncpy(fields[field_count], start, len);
            fields[field_count][len] = '\0';
            field_count++;
            start = end + 1;
        }
        end++;
    }
    // Last field (or only field if no separator found)
    if (field_count < max_fields && start < input + input_len) {
        size_t len = input + input_len - start;
        if (len >= field_buf_size) len = field_buf_size - 1;
        strncpy(fields[field_count], start, len);
        fields[field_count][len] = '\0';
        field_count++;
    }
    return field_count;
}
