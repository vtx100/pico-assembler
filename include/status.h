#ifndef STATUS_H
#define STATUS_H
#include <stdint.h>

#define NO_POS 0xFF

typedef enum {
    OK = 0,

    ERR_LEX_LABEL_DEFINITION,
    ERR_LEX_REG_INDEX,
    ERR_LEX_REG_BOUNDS,
    ERR_LEX_IMM_BOUNDS,
    ERR_LEX_INVALID_IMM_FORMAT,

    ERR_PARSE_ARG_COUNT,
    ERR_PARSE_ARG_TYPE,
    ERR_PARSE_INTERNAL,
    ERR_PARSE_DUP_SYMBOL,

    ERR_IO_INVALID_FILE,
    ERR_IO_FAIL_OPEN_FILE,
    ERR_IO_EMPTY_INSTRUCTION_LIST,

    ERR_LINK_SYMBOL_UNDEFINED,
    ERR_LINK_UNKNOWN_ARG_TYPE,
    ERR_LINK_MISSING_INSTRUCTION,

} StatusCode;

typedef struct {
    StatusCode code;
    uint8_t line;
    uint8_t col;
    char message[128];
} Status;

Status makeStatus(StatusCode code, uint8_t line, uint8_t col, const char *fmt, ...);
void printStatus(const Status *s, const char *tag);
#endif