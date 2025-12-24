#include "hash.h"
#include <dirent.h>
#include "utl.h"
#include "log.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/sha.h>


static hash_algo_t detect_repo_hash_from_config(FILE *f)
{
    char line[512];
    int in_extensions = 0;
    bool interesting_line = false;

    while (fgets(line, sizeof(line), f)) {
        // printf("%s", line);

        /* indented line */
        if (*line == '\t') {
            // printf("check again");

            if (!interesting_line) {
                continue;
            }

            /* remove trailing newline */
            // line[strcspn(line, "\n")] = 0;

            /* skip leading spaces */
            char *p = line;
            while (*p == ' ')
                p++;

            /* find '=' */
            // printf("key %s", p);
            char *eq = strchr(p, '=');
            if (!eq)
                continue;

            /* split key and value */
            *eq = '\0';

            /* trim trailing spaces from key */
            char *key_end = eq - 1;
            while (key_end > p && *key_end == ' ')
                *key_end-- = '\0';

            /* value starts after '=' */
            char *value = eq + 1;

            /* skip leading spaces in value */
            while (*value == ' ')
                value++;

            /* remove trailing newline from value */
            value[strcspn(value, "\n")] = '\0';

            /* now p is key, value is value */
            if (strcmp(p, "\tobjectformat") != 0) // biggest formatting problem is they use \t for \tab
                continue;

            if (strcmp(value, "sha256") == 0)
                return HASH_SHA256;

            return HASH_SHA1;

        }

        /* non-indented line → section header */

        interesting_line = false;

        /* find closing bracket */
        char *c = line;
        while (*c && *c != ']')
            c++;

        if (*c != ']')
            continue;

        *c = 0;  /* terminate at ']' */

        char *str = line + 1;  /* skip '[' */

        if (strcmp(str, "extensions") == 0){
            interesting_line = true;
            
        }
            
    }

    return DEFAULT_HASH_ALGO;
}




hash_algo_t detect_repo_hash(const char *gitdir)
{
    char* fname = utl_path_join(gitdir, "config", 0); // dynamically allocated 
    if (!fname){
        ERROR("couldn't find config file");
        return DEFAULT_HASH_ALGO;
    }
        
    // read the config file inside the gitdir
    FILE *f = fopen(fname, "r");
    if (!f) {
        ERROR("failed to open %s", fname);
        free(fname);
        return DEFAULT_HASH_ALGO;
    }

    // read the line [extenstions]
    hash_algo_t hashf = detect_repo_hash_from_config(f);

    fclose(f);
    free(fname);
    return hashf;
}


void generate_sha256(const void *data, size_t len,
                     unsigned char out[SHA256_DIGEST_LENGTH])
{
    SHA256((const unsigned char *)data, len, out);
}

void generate_sha1(const void *data, size_t len,
                     unsigned char out[SHA_DIGEST_LENGTH])
{
    SHA1((const unsigned char *)data, len, out);
}