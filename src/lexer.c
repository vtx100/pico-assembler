#include <stdbool.h>
#include <string.h>
#include "status.h"
#include "token_list.h"
#include "lexer.h"

/* Check if a given string is a valid binary representation and returns its value */
bool isBinary(const char *p, uint16_t *binary_out) {
    if (*p == '\0') {
        return false;
    }
    uint16_t value = 0;
    while (*p != '\0') {
        if (*p != '0' && *p != '1') {
            return false;
        }
        value = (value << 1) | (*p - '0');
        p++;
    }
    *binary_out = value;
    return true;
}

/* Check if a given string is a valid decimal representation and returns its value */
bool isDecimal(const char *p, uint16_t *binary_out) {
    if (*p == '\0') {
        return false;
    }
    uint16_t value = 0;
    while (*p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
        /* No overflow protection, however good enough to tell it is larger than the expected 8 bits representation */
        value = value * 10 + (*p - '0');
        p++;
    }
    *binary_out = value;
    return true;
}

/* Classify a given token and add it to the token list
    Also pefrom basic checks on values for bounds/max values
 */
Status classifyToken(TokenList *tl, const char *tkn, const uint8_t line_number, const uint8_t col_number) {
    if (tkn[0] == '#') { /* Classify as label */
        if (strlen(tkn) > 1) {
            tokenListPushBack(tl, (Token){.name = strdup(tkn + 1), .type = TOK_LABEL, .value = 0, .line = line_number, .col = col_number});
            return (Status){.code = OK};
        }
        return makeStatus(ERR_LEX_LABEL_DEFINITION, line_number, col_number, "Bad label definition: '%s'. Label(#) must be immediately followed by a name", tkn);
    }

    uint16_t value = 0;
    if (tkn[0] == '%') { /* Classify as register, allows format: %[Decimal: from 0 to 15] */
        if (isDecimal(tkn + 1, &value) == false) {
            return makeStatus(ERR_LEX_REG_INDEX, line_number, col_number, "Bad register index: '%s'. Register index must be a decimal number", tkn);
        }
        /* Don't allow indexes larger than 16 */
        if (value > 15) {
            return makeStatus(ERR_LEX_REG_BOUNDS, line_number, col_number, "Bad register index: '%s'. Register indexing out of bounds, maximum index 15. Use decimal representation [0 - 15]", tkn);
        }
        tokenListPushBack(tl, (Token){.name = strdup(tkn), .type = TOK_REGISTER, .value = (uint8_t)value, .line = line_number, .col = col_number});
        return (Status){.code = OK};
    }

    if (tkn[0] == '!') { /* Classify as immediate max 255 unsigned format: ![d/b]*/
        if (tkn[1] == 'b' && isBinary(tkn + 2, &value)) {
            if (value > UINT8_MAX) {
                return makeStatus(ERR_LEX_IMM_BOUNDS, line_number, col_number, "Bad binary immediate: '%s' (%u). Maximum representable binary immediate is %u", tkn, value, UINT8_MAX);
            }
            tokenListPushBack(tl, (Token){.name = strdup(tkn + 2), .type = TOK_NUMBER, .value = (uint8_t)value, .line = line_number, .col = col_number});
            return (Status){.code = OK};
        } else if (tkn[1] == 'd' && isDecimal(tkn + 2, &value)) {
            if (value > UINT8_MAX) {
                return makeStatus(ERR_LEX_IMM_BOUNDS, line_number, col_number, "Bad decimal immediate: '%s' (%u). Maximum representable decimal immediate is %u", tkn, value, UINT8_MAX);
            }
            tokenListPushBack(tl, (Token){.name = strdup(tkn + 2), .type = TOK_NUMBER, .value = (uint8_t)value, .line = line_number, .col = col_number});
            return (Status){.code = OK};
        } else {
            return makeStatus(ERR_LEX_INVALID_IMM_FORMAT, line_number, col_number, "Invalid immediate type '%c'. Use b or d", tkn[1]);
        }
    }
    // TODO: Implement Hex Immediate type
    /* Allow for 0 for easier writing */
    if (!strcmp(tkn, "0")) {
        tokenListPushBack(tl, (Token){.name = strdup("0"), .type = TOK_NUMBER, .value = 0, .line = line_number, .col = col_number});
        return (Status){.code = OK};
    }
    /* Reaching here means it is probably the use of a label */
    tokenListPushBack(tl, (Token){.name = strdup(tkn), .type = TOK_MNEMONIC, .value = 0, .line = line_number, .col = col_number});
    return (Status){.code = OK};
}
