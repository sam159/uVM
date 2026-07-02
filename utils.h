#ifndef UVM_UTILS_H
#define UVM_UTILS_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *get_temp_dir(void);
bool copy_file(const char *src, const char *dst);

#ifdef __cplusplus
}
#endif

#endif //UVM_UTILS_H
