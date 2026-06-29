#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "asm.h"
#include "token.h"

bool asm_compile(const char *input, const char *output) {
    FILE *f_in = fopen(input, "r");
    if (!f_in) {
        fprintf(stderr, "Error: cannot open '%s'\n", input);
        return false;
    }

    FILE *f_out = fopen(output, "wb");
    if (!f_out) {
        fprintf(stderr, "Error: cannot open '%s' for writing\n", output);
        fclose(f_in);
        return false;
    }

    fprintf(stderr, "Assembling '%s' -> '%s'\n", input, output);

    char *line = NULL;
    size_t len = 0;
    int line_num = 0;

    while (getline(&line, &len, f_in) != -1) {
        line_num++;

        // Strip trailing newline/carriage return
        size_t llen = strlen(line);
        while (llen > 0 && (line[llen - 1] == '\n' || line[llen - 1] == '\r')) {
            line[--llen] = '\0';
        }

        AsmToken *tokens = NULL;
        int count = asm_tokenize_line(line, line_num, &tokens);

        // DEBUG: print tokens to stderr
        for (int i = 0; i < count; i++) {
            fprintf(stderr, "  L%d C%d: [%-12s] \"%s\"\n",
                    line_num, tokens[i].col,
                    asm_token_type_name(tokens[i].type),
                    tokens[i].value);
        }

        asm_free_tokens(tokens);
    }
    free(line);

    fclose(f_in);
    fclose(f_out);
    return true;
}
