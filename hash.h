#ifndef HASH_H
#define HASH_H

typedef enum {
    HASH_SHA1 = 0,
    HASH_SHA256 = 1
} hash_algo_t;

#define DEFAULT_HASH_ALGO HASH_SHA1

hash_algo_t detect_repo_hash(const char *gitdir);

#endif /* HASH_H */
