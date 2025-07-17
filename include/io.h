
#ifndef IO_H
#define IO_H
#include <stddef.h>
#include <stdio.h>
#include "token_list.h"
#include "instruction.h"
#include "status.h"

typedef void (*FormatterFn)(char *buf, size_t buf_size, uint8_t line_number, const Instruction *instr);

const char *getProgramName(const char *path);

Status readTokensFromFile(TokenList *tl, const char *f_name);
Status writeInstructionsToFile(Instruction *instr_list, const char *f_name, FormatterFn formatter);

void VHDL_STYLE_HEX(char *buf, size_t buf_size, uint8_t line_number, const Instruction *instr);
void VHDL_STYLE_BIN(char *buf, size_t buf_size, uint8_t line_number, const Instruction *instr);
void DEBUG(char *buf, size_t buf_size, uint8_t line_number, const Instruction *instr);

#endif