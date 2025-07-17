#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "status.h"
#include "token_list.h"
#include "hashmap.h"
#include "io.h"
#include "instruction.h"
#include "linker.h"
#include "parser.h"

#define DEFAULT_INPUT_FILE "in.txt"
#define DEFAULT_OUTPUT_FILE "out.txt"

int main(int argc, char *argv[]) {
    const char *in_path = DEFAULT_INPUT_FILE;
    const char *out_path = DEFAULT_OUTPUT_FILE;
    const char *program_name = getProgramName(argv[0]);
    FormatterFn formatter = DEBUG;

    int opt;
    while ((opt = getopt(argc, argv, "i:o:f:h")) != -1) {
        switch (opt) {
        case 'i':
            in_path = optarg;
            break;
        case 'o':
            out_path = optarg;
            break;
        case 'f':
            if (!strcmp(optarg, "debug")) {
                formatter = DEBUG;
            } else if (!strcmp(optarg, "vhdlbin")) {
                formatter = VHDL_STYLE_BIN;
            } else if (!strcmp(optarg, "vhdlhex")) {
                formatter = VHDL_STYLE_HEX;
            } else {
                fprintf(stderr, "[pico-assembler] Invalid format: %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            printf("[pico-assembler] Usage: %s [-i input_file] [-o output_file] [-f format]\n", program_name);
            printf("Options: \n");
            printf("    -i <file>   Input file (default: %s) \n", DEFAULT_INPUT_FILE);
            printf("    -o <file>   Output file (default: %s) \n", DEFAULT_OUTPUT_FILE);
            printf("    -f <format> Output format: debug, vhdlbin, vhdlhex \n");
            printf("    -h          Show Help message");
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "[pico-assembler] Usage: %s [-i input_file] [-o output_file] [-f format]\nUse: '%s -h' for help", program_name, program_name);
            exit(EXIT_FAILURE);
        }
    }

    Instruction instruction_list[256];
    memset(instruction_list, 0, sizeof(instruction_list));

    TokenList tl;
    tokenListInit(&tl);

    HashMap *instruction_set = NULL;
    bool instr_set_ok = allocHashMap(&instruction_set, HASH_MAP_BUCKETS);
    if (!instr_set_ok) {
        printf("Error allocating instruction set hash map");
        goto cleanup;
    }

    HashMap *symbol_set = NULL;
    bool symbol_set_ok = allocHashMap(&symbol_set, HASH_MAP_BUCKETS);
    if (!symbol_set_ok) {
        printf("Error allocating symbol set hash map");
        goto cleanup;
    }

    /* --- Program Control Group --- */
    /* Jump */
    insertHashMap(instruction_set, "JMP", &(InstructionDefinition){.mask = 0b1000000100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "JZ", &(InstructionDefinition){.mask = 0b1001000100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "JNZ", &(InstructionDefinition){.mask = 0b1001010100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "JC", &(InstructionDefinition){.mask = 0b1001100100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "JNC", &(InstructionDefinition){.mask = 0b1001110100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));

    /* Call */
    insertHashMap(instruction_set, "CALL", &(InstructionDefinition){.mask = 0b1000001100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "CALLZ", &(InstructionDefinition){.mask = 0b1001001100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "CALLNZ", &(InstructionDefinition){.mask = 0b1001011100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "CALLC", &(InstructionDefinition){.mask = 0b1001101100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "CALLNC", &(InstructionDefinition){.mask = 0b1001111100000000, .arg_type = ADDR, .arg1_start = 0}, sizeof(InstructionDefinition));

    /* Return */
    insertHashMap(instruction_set, "RET", &(InstructionDefinition){.mask = 0b1000000010000000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "RETZ", &(InstructionDefinition){.mask = 0b1001000010000000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "RETNZ", &(InstructionDefinition){.mask = 0b1001010010000000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "RETC", &(InstructionDefinition){.mask = 0b1001100010000000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "RETNC", &(InstructionDefinition){.mask = 0b1001110010000000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));

    /* --- Logical Group --- */
    insertHashMap(instruction_set, "LOAD", &(InstructionDefinition){.mask = 0b1100000000000000, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "AND", &(InstructionDefinition){.mask = 0b1100000000000001, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "OR", &(InstructionDefinition){.mask = 0b1100000000000010, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "XOR", &(InstructionDefinition){.mask = 0b1100000000000011, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));

    /* --- Arithmetic Group ---*/
    insertHashMap(instruction_set, "ADD", &(InstructionDefinition){.mask = 0b1100000000000100, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "ADDCY", &(InstructionDefinition){.mask = 0b1100000000000101, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "SUB", &(InstructionDefinition){.mask = 0b1100000000000110, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "SUBCY", &(InstructionDefinition){.mask = 0b1100000000000111, .arg_type = REG_ANY, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));

    /* --- Shift and Rotate Group ---*/
    /* Right */
    insertHashMap(instruction_set, "SR0", &(InstructionDefinition){.mask = 0b1101000000001110, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "SR1", &(InstructionDefinition){.mask = 0b1101000000001111, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "SRX", &(InstructionDefinition){.mask = 0b1101000000001010, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "SRA", &(InstructionDefinition){.mask = 0b1101000000001000, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "RR", &(InstructionDefinition){.mask = 0b1101000000001100, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    /* Left */
    insertHashMap(instruction_set, "SL0", &(InstructionDefinition){.mask = 0b1101000000000110, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "SL1", &(InstructionDefinition){.mask = 0b1101000000000111, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "SLX", &(InstructionDefinition){.mask = 0b1101000000000010, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition)); // Different !
    insertHashMap(instruction_set, "SLA", &(InstructionDefinition){.mask = 0b1101000000000000, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "RL", &(InstructionDefinition){.mask = 0b1101000000000100, .arg_type = REG, .arg1_start = 8}, sizeof(InstructionDefinition)); // Different !

    /* --- I/O Group ---*/
    /* Input */
    insertHashMap(instruction_set, "INPUT", &(InstructionDefinition){.mask = 0b1011000000000000, .arg_type = REG_REG, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "INPUTP", &(InstructionDefinition){.mask = 0b1010000000000000, .arg_type = REG_IMM, .arg1_start = 8, .arg2_start = 0}, sizeof(InstructionDefinition));
    /* Output */
    insertHashMap(instruction_set, "OUTPUT", &(InstructionDefinition){.mask = 0b1111000000000000, .arg_type = REG_REG, .arg1_start = 8, .arg2_start = 4}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "OUTPUTP", &(InstructionDefinition){.mask = 0b1110000000000000, .arg_type = REG_IMM, .arg1_start = 8, .arg2_start = 0}, sizeof(InstructionDefinition));

    /* --- Interrupt group ---*/
    /* Return Enable / Disable */
    insertHashMap(instruction_set, "RETE", &(InstructionDefinition){.mask = 0b1000000011111000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "RETD", &(InstructionDefinition){.mask = 0b1000000011011000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));
    /* Interrupt Enable / Disable */
    insertHashMap(instruction_set, "INTE", &(InstructionDefinition){.mask = 0b1000000011110000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));
    insertHashMap(instruction_set, "INTD", &(InstructionDefinition){.mask = 0b1000000011010000, .arg_type = NO_ARG}, sizeof(InstructionDefinition));

    /* Perform lexing */
    Status read_ok = readTokensFromFile(&tl, in_path);
    printStatus(&read_ok, "I/O + TOKEN");
    if (read_ok.code != OK) {
        goto cleanup;
    }

    /* Perform parsing */
    uint16_t loc = 0;
    Status parse_ok = parseTokenList(&tl, instruction_set, symbol_set, instruction_list, &loc);
    printStatus(&parse_ok, "PARSE");
    if (parse_ok.code != OK) {
        goto cleanup;
    }

    /* Perform linking*/
    Status link_ok = link(instruction_list, loc, symbol_set);
    printStatus(&link_ok, "LINKING");
    if (link_ok.code == OK) {
        Status write_ok = writeInstructionsToFile(instruction_list, out_path, formatter);
        printStatus(&write_ok, "WRITE TO FILE");
        if (write_ok.code != OK) {
            goto cleanup;
        }
    } else {
        goto cleanup;
    }
    printf("[pico-assembler] Successfully assembled '%s'. Wrote to '%s'.", in_path, out_path);

cleanup:
    deallocHashMap(instruction_set);
    deallocHashMap(symbol_set);
    deallocTokenList(&tl);
    exit(EXIT_SUCCESS);
}