#include <stdio.h>
#include <stdbool.h>
#include "asm.h"

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

    /* TODO:
     * - read entire file and split into lines on new lines (\n or \r\n)
     * - parse each line into an array of ASM_Line object
     * - for debug write each line to stdout as parsed
     */

    fclose(f_in);
    fclose(f_out);
    return true;
}
