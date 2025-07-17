#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <stdint.h>
#include "token_list.h"
typedef enum { NO_ARG,
               REG,
               ADDR,
               REG_REG,
               REG_IMM,
               REG_ANY
} ArgumentType;

typedef struct {
    uint16_t mask;
    ArgumentType arg_type;
    uint8_t arg1_start;
    uint8_t arg2_start;
} InstructionDefinition;

typedef struct {
    InstructionDefinition *instruction;
    TokenNode *arg1;
    TokenNode *arg2;
    uint16_t raw;
} Instruction;
#endif