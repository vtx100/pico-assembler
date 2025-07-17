#include "parser.h"
#include "status.h"
#include "token_list.h"
#include <stdio.h>

/* Try to consume the expected args for a given instruction and throw errors if the expected types do not match */
Status consumeArgs(HashMap *instr_list, InstructionDefinition *def, SllNode *mnemonic_node, SllNode **next, Instruction *out) {
    SllNode *curr = mnemonic_node->next;
    if (!def) {
        return (Status){
            .code = ERR_PARSE_INTERNAL,
            .message = "Missing instruction definition"};
    }

    TokenNode *arg1 = NULL;
    TokenNode *arg2 = NULL;

    switch (def->arg_type) {
    case NO_ARG:
        out->arg1 = out->arg2 = NULL;
        *next = curr;
        return (Status){.code = OK};

    case REG:
        arg1 = TOK_OR_EOF(curr);
        if (!arg1) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected 1 register, reached EOF");
        }
        if (arg1->tok.type != TOK_REGISTER) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg1->tok.col, "Expected register, recieved '%s'(%u)", arg1->tok.name, arg1->tok.type);
        }
        out->arg1 = arg1;
        *next = curr->next;
        return (Status){.code = OK};

    case ADDR:
        arg1 = TOK_OR_EOF(curr);
        if (!arg1) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected 1 address, reached EOF");
        }
        if (arg1->tok.type != TOK_MNEMONIC) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg1->tok.col, "Expected address, recieved '%s'(%u)", arg1->tok.name, arg1->tok.type);
        }
        if (getPointerInHashMap(instr_list, arg1->tok.name) != NULL) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg1->tok.col, "Expected address, recieved an instruction '%s'(%u)", arg1->tok.name, arg1->tok.type);
        }
        out->arg1 = arg1;
        *next = curr->next;
        return (Status){.code = OK};

    case REG_REG:
        arg1 = TOK_OR_EOF(curr);
        if (!arg1) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected register as arg1, reached EOF");
        }
        if (arg1->tok.type != TOK_REGISTER) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg1->tok.col, "Expected register as arg1, recieved '%s'(%u)", arg1->tok.name, arg1->tok.type);
        }
        out->arg1 = arg1;

        arg2 = TOK_OR_EOF(curr->next);
        if (!arg2) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected register as arg2, reached EOF");
        }
        if (arg2->tok.type != TOK_REGISTER) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg2->tok.col, "Expected register as arg2, recieved '%s'(%u)", arg2->tok.name, arg2->tok.type);
        }
        out->arg2 = arg2;
        *next = curr->next->next;
        return (Status){.code = OK};

    case REG_IMM:
        arg1 = TOK_OR_EOF(curr);
        if (!arg1) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected register as arg1, reached EOF");
        }
        if (arg1->tok.type != TOK_REGISTER) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg1->tok.col, "Expected register as arg1, recieved '%s'(%u)", arg1->tok.name, arg1->tok.type);
        }
        out->arg1 = arg1;

        arg2 = TOK_OR_EOF(curr->next);
        if (!arg2) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected immediate as arg2, reached EOF");
        }
        if (arg2->tok.type != TOK_NUMBER) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg2->tok.col, "Expected immediate as arg2, recieved '%s'(%u)", arg2->tok.name, arg2->tok.type);
        }
        out->arg2 = arg2;
        *next = curr->next->next;
        return (Status){.code = OK};

    case REG_ANY:
        arg1 = TOK_OR_EOF(curr);
        if (!arg1) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected register as arg1, reached EOF");
        }
        if (arg1->tok.type != TOK_REGISTER) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg1->tok.col, "Expected register as arg1, recieved '%s'(%u)", arg1->tok.name, arg1->tok.type);
        }
        out->arg1 = arg1;

        arg2 = TOK_OR_EOF(curr->next);
        if (!arg2) {
            return makeStatus(ERR_PARSE_ARG_COUNT, NO_POS, NO_POS, "Expected register or immediate as arg2, reached EOF");
        }
        if (arg2->tok.type != TOK_NUMBER && arg2->tok.type != TOK_REGISTER) {
            return makeStatus(ERR_PARSE_ARG_TYPE, NO_POS, arg1->tok.type != TOK_NUMBER ? arg1->tok.col : arg2->tok.col, "Expected register or immediate as arg2, recieved '%s'(%u) and '%s'(%u)", arg1->tok.name, arg1->tok.type, arg2->tok.name, arg2->tok.type);
        }
        out->arg2 = arg2;
        *next = curr->next->next;
        return (Status){.code = OK};
    }
    return makeStatus(ERR_PARSE_INTERNAL, NO_POS, NO_POS, "Internal error");
}

Status parseTokenList(TokenList *tl, HashMap *inst_map, HashMap *sym_map, Instruction *instr_list, uint16_t *loc_count) {
    if (!tl->list.head) {
        return makeStatus(ERR_PARSE_INTERNAL, 0, 0, "No tokens (source file empty)");
    }

    /* Start parsing each token one by one*/
    uint8_t loc_counter = 0;
    for (SllNode *n = tl->list.head; n;) {
        TokenNode *tn = CONTAINER_OF(n, TokenNode, link);
        if (loc_counter > UINT8_MAX) {
            return makeStatus(ERR_PARSE_INTERNAL, NO_POS, NO_POS, "Program contains more than 255 instructions");
        }
        switch (tn->tok.type) {
        case TOK_MNEMONIC: {
            InstructionDefinition *def = getPointerInHashMap(inst_map, tn->tok.name);
            if (!def) {
                /* Detected forward jump*/
                n = n->next;
                break;
            }
            /* It is an instruction, so now handle the building of a new instruction and and update loc_counter */
            instr_list[loc_counter].instruction = def;
            SllNode *next = NULL;
            Status res = consumeArgs(inst_map, def, n, &next, &instr_list[loc_counter]);
            if (res.code != OK) {
                *loc_count = loc_counter;
                /* Populate the line and col fields according to the instruction that called for arguments*/
                if (res.line == res.col) { /* If an argument is missing just throw the token where the expected value was ommited*/
                    return makeStatus(res.code, tn->tok.line, tn->tok.col, "At '%s': %s", tn->tok.name, res.message);
                } else { /* Else, print the actual column where the arg is missmatched */
                    return makeStatus(res.code, tn->tok.line, res.col, "At '%s': %s", tn->tok.name, res.message);
                }
            }
            loc_counter++;
            n = next;
            break;
        }
        case TOK_LABEL: {
            bool insertion_status = insertHashMap(sym_map, tn->tok.name, &loc_counter, sizeof(uint8_t));
            if (!insertion_status) {
                /* Do not allow duplicate entries as this would not make sense*/
                return makeStatus(ERR_PARSE_DUP_SYMBOL, tn->tok.line, tn->tok.col, "Failed insertion of symbol %s into symbol table, symbol already exists", tn->tok.name);
            }
            n = n->next;
            break;
        }
        default: {
            return makeStatus(ERR_PARSE_INTERNAL, tn->tok.line, tn->tok.col, "Internal error, Unrecognized symbol: %s", CONTAINER_OF(n, TokenNode, link)->tok.name);
        }
        }
    }
    *loc_count = loc_counter;
    return (Status){.code = OK};
}