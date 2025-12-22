#include "utl.h"


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>



char *utl_path_join(const char *base, const char *name)
{
    size_t base_len = strlen(base);
    int need_slash = (base_len > 0 && base[base_len - 1] != '/');

    size_t len = base_len + strlen(name) + need_slash + 1;
    char *out = malloc(len);
    if (!out)
        return NULL;

    snprintf(out, len, "%s%s%s",
             base,
             need_slash ? "/" : "",
             name);
    return out;
}


#include <sys/stat.h>

/* Returns 1 if path is a directory, 0 otherwise */
int is_directory(const char *path)
{
    struct stat st;

    if (stat(path, &st) < 0)
        return 0;

    return S_ISDIR(st.st_mode);
}