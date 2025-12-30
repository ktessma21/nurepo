    // git_zlib_wrapper.h
#pragma once
#include <zlib.h>

typedef struct git_zstream {
    z_stream z;
    unsigned long avail_in;
    unsigned long avail_out;
    unsigned long total_in;
    unsigned long total_out;
    unsigned char *next_in;
    unsigned char *next_out;
} git_zstream;

void git_inflate_init(git_zstream *s);
void git_inflate_end(git_zstream *s);
int  git_inflate(git_zstream *s, int flush);

void git_deflate_init(git_zstream *s, int level);
void git_deflate_end(git_zstream *s);
int  git_deflate(git_zstream *s, int flush);
