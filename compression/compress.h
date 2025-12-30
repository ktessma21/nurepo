

#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdio.h>

/* Compress from file source to file dest until EOF on source.
 * Returns 0 on success, negative on error.
 */
char *compress_file(const char *path, size_t *out_size);

/**
 * Decompresses a git object file by path and returns the data in a buffer.
 * @param path: The full path to the git object file.
 * @param out_size: Pointer to store the total length of decompressed data.
 * @return: A heap-allocated buffer (must be freed by caller), or NULL on error.
 */
char *decompress_file(const char *path, size_t *out_size);

#endif /* COMPRESS_H */
