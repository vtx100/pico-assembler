
#ifndef LEXER_H
#define LEXER_H
#include "token_list.h"
#include "status.h"
Status classifyToken(TokenList *tl, const char *tkn, const uint8_t line_number, const uint8_t col_number);
#endif