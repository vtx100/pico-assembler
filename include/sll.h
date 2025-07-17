#include <stdlib.h>
#include <string.h>
#ifndef SLL_H
#define SLL_H

/* CONTAINER_OF: Recover outer struct pointer from a member pointer */
#define CONTAINER_OF(addr, type, member) ( \
    (type *)((char *)(addr)) - (size_t)(&((type *)0)->member))

/* Simple single linked list node */
typedef struct _SllNode {
    struct _SllNode *next;
} SllNode;

/* Single linked list header */
typedef struct {
    SllNode *head;
    SllNode *tail;
} Sll;

/* Initialize the head of the Single Linked List */
static inline void initHead(Sll *sll) {
    sll->head = NULL;
    sll->tail = NULL;
}

/* Push back */
static inline void sllPushBack(Sll *sll, SllNode *n) {
    n->next = NULL;
    if (sll->tail) {
        sll->tail->next = n;
    } else {
        sll->head = n;
    }
    sll->tail = n;
}
#endif