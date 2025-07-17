#ifndef PARSER_H
#define PARSER_H
#define TOK_OR_EOF(node) ((node) ? CONTAINER_OF(node, TokenNode, link) : NULL)
#include "status.h"
#include "hashmap.h"
#include "sll.h"
#include "instruction.h"
Status consumeArgs(HashMap *instr_list, InstructionDefinition *def, SllNode *mnemonic_node, SllNode **next, Instruction *out);
Status parseTokenList(TokenList *tl, HashMap *inst_map, HashMap *sym_map, Instruction *instr_list, uint16_t *loc_count);
#endif