#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "utils.h"

const char *get_temp_dir(void) {
    const char *tmp = getenv("TMPDIR");
    if (tmp != NULL && *tmp != '\0') return tmp;

    tmp = getenv("TMP");
    if (tmp != NULL && *tmp != '\0') return tmp;

    return "/tmp";
}

bool copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return false;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return false;
    }

    char buf[4096];
    size_t n;
    bool ok = true;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            ok = false;
            break;
        }
    }
    if (ferror(in)) ok = false;

    fclose(in);
    fclose(out);
    return ok;
}
