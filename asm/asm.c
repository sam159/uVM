#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "asm.h"

#include <errno.h>

#include "token.h"
#include "tree.h"

bool asm_compile(const char *input, const char *output) {
    FILE *f_in = fopen(input, "r");
    if (!f_in) {
        fprintf(stderr, "Error: cannot open '%s'\n", input);
        return false;
    }

    char * outputTemp = malloc(strlen(output) + 5);
    if (!outputTemp) {
        fclose(f_in);
        return false;
    }
    outputTemp[0] = '\0';
    strcat(outputTemp, output);
    strcat(outputTemp, ".tmp");

    FILE *f_out = fopen(outputTemp, "wb");
    if (!f_out) {
        free(outputTemp);
        fprintf(stderr, "Error: cannot open '%s' for writing\n", output);
        fclose(f_in);
        return false;
    }

    fprintf(stderr, "Assembling '%s' -> '%s'\n", input, output);

    bool success = true;
    
    char *line = NULL;
    size_t len = 0;
    int lineNum = 0;
    AsmTokenList *tokens = asm_token_list_create();
    if (!tokens) {
        fclose(f_in);
        free(outputTemp);
        fclose(f_out);
        return false;
    }

    while (getline(&line, &len, f_in) != -1) {
        lineNum++;

        // Convert \r\n into \n
        size_t llen = strlen(line);
        if (llen > 1 && line[llen - 1] == '\r' && line[llen - 2] == '\n') {
            line[llen - 2] = '\n';
            line[llen - 1] = '\0';
            llen--;
        }

        // Enforce maximum line length
        if (llen > 1024) {
            fprintf(stderr, "Error: line %d exceeds 1024 characters (%zu chars)\n", lineNum, llen);
            success = false;
            break;
        }

        AsmTokenList *lineTokens = asm_tokenize_line(line, lineNum);

        asm_token_list_concat(tokens, lineTokens);
        asm_token_list_free(lineTokens);
    }
    free(line);

    AsmToken eof = {ASM_TOKEN_EOF, strdup(""), 0, lineNum + 1, 1};
    asm_token_list_append(tokens, eof);

    ASMProgram *program = asm_parse(input, tokens);

    if (program && !asm_resolve(program)) {
        success = false;
    }

    if (success && program) {
        if (!asm_emit(program, f_out)) {
            success = false;
        }
    }

    asm_free_program(program);
    asm_token_list_free(tokens);

    fclose(f_in);
    fclose(f_out);

    if (success) {
        // copy outputTemp to output
        if (rename(outputTemp, output) == -1) {
            fprintf(
                stderr,
                "Error: failed to rename '%s' to '%s: %s'\n",
                outputTemp,
                output,
                strerror(errno)
                );
            unlink(outputTemp);
            success = false;
        }
    } else {
        unlink(outputTemp);
    }
    free(outputTemp);

    return success;
}