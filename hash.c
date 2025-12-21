#include "hash.h"
#include <dirent.h>
#include "utl.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int ieq(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) !=
            tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static hash_algo_t detect_repo_hash_from_config(FILE *f)
{
    char line[512];
    int in_extensions = 0;

    while (fgets(line, sizeof(line), f)) {
        char *p = line;

        /* skip leading whitespace */
        while (isspace((unsigned char)*p))
            p++;

        /* comments / empty lines */
        if (*p == '#' || *p == ';' || *p == '\0')
            continue;

        /* section */
        if (*p == '[') {
            in_extensions = strcasecmp(p, "[extensions]", 12) == 0;
            continue;
        }

        if (!in_extensions)
            continue;

        char *eq = strchr(p, '=');
        if (!eq)
            continue;

        *eq++ = '\0';

        /* trim key */
        char *end = p + strlen(p);
        while (end > p && isspace((unsigned char)end[-1]))
            *--end = '\0';

        /* trim value */
        while (isspace((unsigned char)*eq))
            eq++;

        if (strcasecmp(p, "objectformat") == 0) {
            if (strcasecmp(eq, "sha256") == 0)
                return HASH_SHA256;
            return HASH_SHA1;
        }
    }

    return HASH_SHA1;
}




hash_algo_t detect_repo_hash(const char *gitdir)
{
    char* fname = utl_path_join(gitdir, "config"); // dynamically allocated 
    if (!fname){
        utl_perror("couldn't find config file");
        return DEFAULT_HASH_ALGO;
    }
        
    // read the config file inside the gitdir
    FILE *f = fopen(fname, "r");
    if (!f) {
        utl_perror("failed to open %s", fname);
        free(fname);
        return DEFAULT_HASH_ALGO;
    }

    // read the line [extenstions]
    hash_algo_t hashf = detect_repo_hash_from_config(f);

    fclose(f);
    free(fname);
    return hashf;
}