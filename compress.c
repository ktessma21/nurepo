#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK 16384

/**
 * Decompresses a git object file by path and returns the data in a buffer.
 * @param path: The full path to the git object file.
 * @param out_size: Pointer to store the total length of decompressed data.
 * @return: A heap-allocated buffer (must be freed by caller), or NULL on error.
 */
char *decompress_file(const char *path, size_t *out_size) {
    // 1. Open the file
    FILE *source = fopen(path, "rb");
    if (!source) {
        perror("Failed to open object file");
        return NULL;
    }

    // 2. Setup zlib and memory
    int ret;
    z_stream strm = {0}; // Initialize all to zero
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    size_t capacity = CHUNK;
    size_t total_out = 0;
    char *result = malloc(capacity);
    if (!result) {
        fclose(source);
        return NULL;
    }

    if (inflateInit(&strm) != Z_OK) {
        free(result);
        fclose(source);
        return NULL;
    }

    // 3. Decompression Loop
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            inflateEnd(&strm);
            free(result);
            fclose(source);
            return NULL;
        }
        if (strm.avail_in == 0) break;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = (unsigned char *)out;
            ret = inflate(&strm, Z_NO_FLUSH);
            
            if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                free(result);
                fclose(source);
                return NULL;
            }

            size_t have = CHUNK - strm.avail_out;
            
            // Grow buffer if needed
            if (total_out + have >= capacity) {
                capacity *= 2;
                char *temp = realloc(result, capacity);
                if (!temp) {
                    inflateEnd(&strm);
                    free(result);
                    fclose(source);
                    return NULL;
                }
                result = temp;
            }

            memcpy(result + total_out, out, have);
            total_out += have;

        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    // 4. Cleanup
    inflateEnd(&strm);
    fclose(source);

    // Ensure space for a null terminator just in case we treat it as a string
    char *final_ptr = realloc(result, total_out + 1);
    if (final_ptr) {
        result = final_ptr;
        result[total_out] = '\0';
    }

    if (out_size) *out_size = total_out;
    return result;
}