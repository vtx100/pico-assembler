#ifndef LINKER_H
#define LINKER_H
#include "status.h"
#include "instruction.h"
#include "hashmap.h"
Status link(Instruction *instr_list, uint8_t instr_count, HashMap *sym_map);
#endif