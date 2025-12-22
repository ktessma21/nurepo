#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <openssl/sha.h>

typedef enum {
    HASH_SHA1 = 0,
    HASH_SHA256 = 1
} hash_algo_t;

#define DEFAULT_HASH_ALGO HASH_SHA1

hash_algo_t detect_repo_hash(const char *gitdir);

void generate_sha1(const void *data, size_t len,
                   unsigned char out[SHA_DIGEST_LENGTH]);

void generate_sha256(const void *data, size_t len,
                     unsigned char out[SHA256_DIGEST_LENGTH]);

#endif /* HASH_H */
