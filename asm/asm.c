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

    // TODO: Implement assembly logic here

    fclose(f_in);
    fclose(f_out);
    return true;
}
