#ifndef TOKEN_LIST_H
#define TOKEN_LIST_H
#include "token.h"
#include "sll.h"
typedef struct {
    SllNode link;
    Token tok;
} TokenNode;

typedef struct {
    Sll list;
} TokenList;

void tokenListInit(TokenList *tl);
void tokenListPushBack(TokenList *tl, Token tok);
void printAllTokens(TokenList *tl);
void deallocTokenList(TokenList *tl);
#endif
