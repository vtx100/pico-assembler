#include "status.h"
#include <stdarg.h>
#include <stdio.h>

/* Used to return status from functions where execution might fail*/
Status makeStatus(StatusCode code, uint8_t line, uint8_t col, const char *fmt, ...) {
    Status s = {.code = code, .line = line, .col = col};
    va_list args;
    va_start(args, fmt);
    vsnprintf(s.message, sizeof(s.message), fmt, args);
    va_end(args);
    return s;
}

/* Format print a returned status message */
void printStatus(const Status *s, const char *tag) {
    if (s->code == OK) {
        fprintf(stdout, "[OK]: %s\n", tag);
    } else {
        if (s->line == NO_POS && s->col == NO_POS) { /* Means that they are not relevant */
            fprintf(stderr, "[ERROR -> %s]: %s", tag, s->message);
        } else {
            fprintf(stderr, "[ERROR -> %s]: %s : (%u:%u)\n", tag, s->message, s->line, s->col);
        }
    }
}