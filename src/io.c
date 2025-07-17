#include "io.h"
#include "lexer.h"
#include "status.h"

/* Used to retrieve the executable's name from the full path */
const char *getProgramName(const char *path) {
    const char *p = strrchr(path, '\\');
    if (!p) {
        p = strrchr(path, '/');
    }
    return p ? p + 1 : path;
}
/* Process individual line by sterilizing it and classifying the tokens to the token list*/
Status processLine(TokenList *tl, char *line, uint8_t line_number) {
    uint8_t col_number = 1;
    char *tkn;
    char *rest = line;
    while ((tkn = strtok_r(rest, " ,\t\n", &rest))) {
        if (tkn[0] == ';') {
            /* Handles both: ;comm and ; comm */
            break;
        }
        Status token_ok = classifyToken(tl, tkn, line_number, col_number);
        if (token_ok.code != OK) {
            return token_ok;
        }
        col_number++;
    }
    return (Status){.code = OK};
}
/* Read the given file and populate the token list with the contents */
Status readTokensFromFile(TokenList *tl, const char *f_name) {
    FILE *fp = fopen(f_name, "r");
    if (!fp) {
        return makeStatus(ERR_IO_INVALID_FILE, NO_POS, NO_POS, "Could not open file: %s", f_name);
    }
    uint8_t line_number = 1;

    char line[255];
    while (fgets(line, 255, fp)) {
        Status line_ok = processLine(tl, line, line_number);
        if (line_ok.code != OK) {
            fclose(fp);
            return line_ok;
        }
        line_number++;
    }
    fclose(fp);
    return (Status){.code = OK};
}
/* Takes the list of instructions inside instr_list and writes it to the given file using the specified formatter */
Status writeInstructionsToFile(Instruction *instr_list, const char *f_name, FormatterFn formatter) {
    if (!instr_list || !formatter) {
        return makeStatus(ERR_IO_EMPTY_INSTRUCTION_LIST, NO_POS, NO_POS, "Missing instructions or formatter");
    }
    FILE *fp = fopen(f_name, "w");
    if (!fp) {
        return makeStatus(ERR_IO_FAIL_OPEN_FILE, NO_POS, NO_POS, "Failed openning the file %s in write mode.", f_name);
    }
    char line_buf[64];
    for (uint8_t i = 0; instr_list[i].instruction != NULL; ++i) {
        formatter(line_buf, sizeof(line_buf), i, &instr_list[i]);
        fprintf(fp, "%s", line_buf);
    }
    fclose(fp);
    return (Status){.code = OK};
}

/* Custom Formatter functions, new ones can be implemented easily by following the schema */

void VHDL_STYLE_HEX(char *buf, size_t buf_size, uint8_t line_number, const Instruction *instr) {
    snprintf(buf, buf_size, " \"%u\" => x\"%04X\",\n", line_number, instr->raw);
}

void VHDL_STYLE_BIN(char *buf, size_t buf_size, uint8_t line_number, const Instruction *instr) {
    char binary[17];
    for (int i = 15; i >= 0; i--) {
        binary[15 - i] = (instr->raw & (1 << i)) ? '1' : '0';
    }
    binary[16] = '\0';
    snprintf(buf, buf_size, " \"%u\" => b\"%s\",\n", line_number, binary);
}

void DEBUG(char *buf, size_t buf_size, uint8_t line_number, const Instruction *instr) {
    char head[64] = "";
    char binary[64];
    if (line_number % 10 == 0) {
        snprintf(head, sizeof(head), "      FEDC BA98 7654 3210\n");
    }
    int pos = 0;
    for (int i = 15; i >= 0; i--) {
        binary[pos++] = (instr->raw & (1 << i)) ? '1' : '0';
        if (i % 4 == 0 && i != 0) {
            binary[pos++] = ' ';
        }
    }
    binary[pos] = '\0';
    if (line_number % 10 == 0) {
        snprintf(buf, buf_size, " %s%.3u => %s\n", head, line_number, binary);
    } else {
        snprintf(buf, buf_size, "%.3u => %s\n", line_number, binary);
    }
}