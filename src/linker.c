
#include "linker.h"
#include "instruction.h"

/* Link the instruction list against the symbol table and build the instructions
    The parser would throw any invalid arg errors, so it is assured the arguments are valid
    Iterate through all the instructions and create the raw instruction using the arguments and the symbols
*/
Status link(Instruction *instr_list, uint8_t instr_count, HashMap *sym_map) {
    for (uint8_t idx = 0; idx < instr_count; idx++) {
        Instruction *instr = &instr_list[idx];
        InstructionDefinition *def = instr->instruction;

        if (!def) {
            return makeStatus(ERR_LINK_MISSING_INSTRUCTION, idx, 0, "Missing instruction, failed link");
        }
        switch (def->arg_type) {
        case NO_ARG: /* Means the instruction is hard-coded, so the content is already inside the mask*/
            instr->raw = def->mask;
            break;

        case REG:
            instr->raw = def->mask | (instr->arg1->tok.value << (def->arg1_start));
            break;

        case ADDR: { /* Means an address is expected, so search the symbol table for it */
            uint8_t *addr = getPointerInHashMap(sym_map, instr->arg1->tok.name);
            if (!addr) {
                return makeStatus(ERR_LINK_SYMBOL_UNDEFINED, instr->arg1->tok.line, instr->arg1->tok.col, "Undefined symbol '%s'. Not found inside the symbol table", instr->arg1->tok.name);
            }
            instr->raw = def->mask | (*addr << (def->arg1_start));
            break;
        }
        case REG_REG: /* INPUT and OUTPUT*/
        case REG_IMM: /* INPUTP and OUTPUTP*/
            instr->raw = def->mask | (instr->arg1->tok.value << (def->arg1_start));
            instr->raw |= (instr->arg2->tok.value << (def->arg2_start));
            break;
        case REG_ANY:
            if (instr->arg2->tok.type == TOK_REGISTER) {
                instr->raw = def->mask | (instr->arg1->tok.value << (def->arg1_start));
                instr->raw |= (instr->arg2->tok.value << (def->arg2_start));
            } else {
                instr->raw = ((uint16_t)def->mask) << 12;
                instr->raw ^= (instr->arg1->tok.value << (def->arg1_start));
                instr->raw ^= (instr->arg2->tok.value);
            }
            break;
        default:
            return makeStatus(ERR_LINK_UNKNOWN_ARG_TYPE, idx, 0, "Unknown arg type %u", def->arg_type);
        }
    }
    return (Status){.code = OK};
}