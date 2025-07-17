#include "token_list.h"
#include <stdio.h>

void tokenListInit(TokenList *tl) {
    initHead(&tl->list);
}

void tokenListPushBack(TokenList *tl, Token tok) {
    TokenNode *n = (TokenNode *)malloc(sizeof(*n));
    n->tok = tok;
    sllPushBack(&tl->list, &n->link);
}

/* Print all the tokens in the list, used for debugging */
void printAllTokens(TokenList *tl) {
    for (SllNode *n = tl->list.head; n; n = n->next) {
        TokenNode *tn = CONTAINER_OF(n, TokenNode, link);
        printf("( %s  %u %u) \n", tn->tok.name, tn->tok.type, tn->tok.value);
    }
}
/* Dealloc the token list (Also clears up .name, which is populated using strdup)*/
void deallocTokenList(TokenList *tl) {
    SllNode *curr = tl->list.head;
    while (curr) {
        SllNode *next = curr->next;
        TokenNode *node = CONTAINER_OF(curr, TokenNode, link);
        if (node->tok.name) {
            /*  Safely free the name field, which contains a pointer returned by strdup */
            free((void *)node->tok.name);
        }
        free(node);
        curr = next;
    }
    initHead(&tl->list);
}