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


// error handling utl functions 
static void vreport(const char *prefix,
                    int show_errno,
                    const char *fmt,
                    va_list ap)
{
    fprintf(stderr, "%s: ", prefix);
    vfprintf(stderr, fmt, ap);

    if (show_errno)
        fprintf(stderr, ": %s", strerror(errno));

    fputc('\n', stderr);
}

void utl_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport("error", 0, fmt, ap);
    va_end(ap);
}

void utl_perror(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport("error", 1, fmt, ap);
    va_end(ap);
}

void utl_fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport("fatal", 0, fmt, ap);
    va_end(ap);
    exit(1);
}

void utl_pfatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport("fatal", 1, fmt, ap);
    va_end(ap);
    exit(1);
}