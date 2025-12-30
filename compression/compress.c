#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zlib.h>

#define CHUNK 16384


static void dump_raw(const char *hash, const char *data, size_t len)
{
    char filename[256];
    snprintf(filename, sizeof(filename), "unzipped_%s.bin", hash);

    FILE *f = fopen(filename, "wb");
    if (!f) return;

    fwrite(data, 1, len, f);
    fclose(f);
}
/**
 * Decompresses a git object file by path and returns the data in a buffer.
 * @param path: The full path to the git object file.
 * @param out_size: Pointer to store the total length of decompressed data.
 * @return: A heap-allocated buffer (must be freed by caller), or NULL on error.
 */
char *decompress_file(const char *path, size_t *out_size) {

    FILE *source = fopen(path, "rb");
    if (!source) return NULL;

    int ret;
    z_stream strm = {0};
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    size_t capacity = CHUNK;
    size_t total_out = 0;
    char *result = malloc(capacity);

    if (!result) { fclose(source); return NULL; }

    if (inflateInit(&strm) != Z_OK) {
        free(result);
        fclose(source);
        return NULL;
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) goto fail;

        if (strm.avail_in == 0) break;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
                goto fail;

            size_t have = CHUNK - strm.avail_out;

            if (total_out + have >= capacity) {
                capacity *= 2;
                char *tmp = realloc(result, capacity);
                if (!tmp) goto fail;
                result = tmp;
            }

            memcpy(result + total_out, out, have);
            total_out += have;

        } while (!strm.avail_out);

    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    fclose(source);

    // Null-terminate for convenience
    char *tmp = realloc(result, total_out + 1);
    if (tmp) result = tmp;
    result[total_out] = '\0';

    if (out_size) *out_size = total_out;

    
    return result;

fail:
    inflateEnd(&strm);
    fclose(source);
    free(result);
    return NULL;
}
