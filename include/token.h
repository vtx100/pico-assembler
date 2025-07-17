#ifndef TOKEN_H
#define TOKEN_H
#include <stdint.h>
typedef enum { TOK_LABEL,
               TOK_MNEMONIC,
               TOK_REGISTER,
               TOK_NUMBER } TokenType;

typedef struct {
    const char *name;
    TokenType type;
    uint8_t value;
    uint8_t line;
    uint8_t col;
} Token;

#endif