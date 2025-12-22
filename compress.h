

#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdio.h>

/* Compress from file source to file dest until EOF on source.
 * Returns 0 on success, negative on error.
 */
int compress_file(FILE *source, FILE *dest);

/* Decompress from file source to file dest until EOF on source.
 * Returns 0 on success, negative on error.
 */
int decompress_file(FILE *source, FILE *dest);

#endif /* COMPRESS_H */
