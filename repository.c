
// Source - https://stackoverflow.com/a
// Posted by alk, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-21, License - CC BY-SA 4.0

#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "log.h"
#include "repository.h"
#include "hash.h"
#include "utl.h"
#include "compression/compress.h"
#include "ram.h"


static void dump_object_pretty(const char *hash,
                               const char *header,
                               const char *body,
                               size_t body_len)
{
    fprintf(stderr, "\n===== %s =====\n", hash);

    fprintf(stderr, "HEADER:\n%s\n\n", header);
    fprintf(stderr, "BODY:\n");

    size_t i = 0;

    /* print printable prefix */
    for (; i < body_len; i++) {
        unsigned char c = (unsigned char)body[i];

        /* printable chars, incl newline + tab */
        if (c == '\n' || c == '\r' || c == '\t' ||
            (c >= 0x20 && c <= 0x7E)) {

            fputc(c, stderr);
        } else {
            break; /* stop at first non-printable */
        }
    }

    fputc('\n', stderr);

    /* if entire body was printable, we're done */
    if (i == body_len) {
        fprintf(stderr, "(end; body fully printable)\n");
        fprintf(stderr, "=================\n");
        return;
    }

    /* otherwise report */
    fprintf(stderr,
            "<non-printable data begins at offset %zu>\n", i);

    /* hex dump remaining bytes */
    for (; i < body_len; i++) {
        fprintf(stderr, "%02x ", (unsigned char)body[i]);
        if (((i+1) % 16) == 0)
            fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n=================\n");
}



static void parse_objects(struct repository *repo, struct RAM* memory);

// }

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif



static void process_one_line(const char *line, char *key, char *buf){

}

static int is_porcelain_repo(const char *gitdir)
{
	char path[4096];

#define CHECK(p) do { \
	snprintf(path, sizeof(path), "%s/%s", gitdir, p); \
	if (access(path, F_OK) != 0) return 0; \
} while (0)

	CHECK("objects");
	CHECK("refs");
	CHECK("HEAD");
	CHECK("config");

#undef CHECK
	return 1;
}


static int is_git_directory(const char *path)
{	if (!path)
		return 1;

	DEBUG("checking if it is directory");
	if (is_directory(path)){
		DEBUG(" %s is directory", path);
		return is_porcelain_repo(path);
	}
	DEBUG(" %s is not directory", path);
	return 1;
	
}

int repo_init_gitdir(struct repository *repo, const char *gitdir)
{
	if (!gitdir)
		return -1;

	if (!is_git_directory(gitdir))
		return -1;

	repo->gitdir = strdup(gitdir);
	if (!repo->gitdir)
		return -1;

	return 0;
}






/*
 * Initialize 'repo' based on the provided 'gitdir'.
 * Return 0 upon success and a non-zero value upon failure.
 */
int repo_init(struct repository *repo,
              const char *gitdir,
              const char *worktree)
{
	memset(repo, 0, sizeof(*repo)); // zero out the structure for a fresh start 

	if (repo_init_gitdir(repo, gitdir))
		goto error;

	repo->hash_algo = detect_repo_hash(gitdir); // set the appropriate hash_algo

	if (worktree)
		repo->worktree = strdup(worktree);
	
    struct RAM* memory = ram_init();
    if (!memory){
        DEBUG("ram_init failed in repository.c/repo_init");
        return -1;
    }
    
	parse_objects(repo, memory);
    
    ram_destroy(memory);

	return 0;

error:
	repo_clear(repo);
	return -1;
}



void repo_clear(struct repository *repo)
{
    if (!repo) return;


    free(repo->gitdir);
    free(repo->worktree);
}


static enum object_type get_type_from_header(const char *unzipped_buffer) {
    if (strncmp(unzipped_buffer, "blob ", 5) == 0) {
        return OBJ_BLOB;
    } else if (strncmp(unzipped_buffer, "commit ", 7) == 0) {
        return OBJ_COMMIT;
    } else if (strncmp(unzipped_buffer, "tree ", 5) == 0) {
        return OBJ_TREE;
    } else if (strncmp(unzipped_buffer, "tag ", 4) == 0) {
        return OBJ_TAG;
    }
    return OBJ_NONE;
}


static struct object *process(struct repository *repo,
                              const char *hash_value,
                              const char *file_path)
{
    DEBUG("processing object file: %s", file_path);
    size_t total_size = 0;
    char *unzipped_buffer = decompress_file(file_path, &total_size);
    if (!unzipped_buffer) {
        ERROR("Decompression failed for %s", file_path);
        return NULL;
    }
    
    //
    // --- parse header ---
    //
    char *nul = memchr(unzipped_buffer, '\0', total_size);
    if (!nul) {
        ERROR("Invalid object: missing NUL header terminator");
        free(unzipped_buffer);
        return NULL;
    }

    char *body = nul + 1;
    size_t body_len = total_size - (body - unzipped_buffer);

    size_t header_len = (size_t)(nul - unzipped_buffer);
    char saved_header[256] = {0};
    snprintf(saved_header, sizeof(saved_header), "%.*s",
            (int)header_len, unzipped_buffer);

    dump_object_pretty(hash_value, saved_header, body, body_len);
    enum object_type type = get_type_from_header(unzipped_buffer);
    if (type == OBJ_NONE) {
        ERROR("Unknown object type in %s", file_path);
        free(unzipped_buffer);
        return NULL;
    }

    //
    // --- allocate object ---
    //
    struct object *obj = calloc(1, sizeof(struct object));
    if (!obj) {
        free(unzipped_buffer);
        return NULL;
    }

    obj->type = type;
    obj->flags = OBJ_FLAG_NONE;

    //
    // --- fill OID ---
    //
    size_t digest_len =
        (repo->hash_algo == HASH1_DIGEST_LENGTH)
        ? HASH1_DIGEST_LENGTH
        : HASH256_DIGEST_LENGTH;

    strcpy((char *)obj->oid.hash, hash_value);


    //
    // --- parse payload based on type ---
    //
    switch (type) {

    case OBJ_BLOB: {
        struct blob_object *blob = malloc(sizeof(*blob));
        if (!blob) goto fail;

        blob->size = body_len;
        blob->data = malloc(body_len);
        if (!blob->data) {
            free(blob);
            goto fail;
        }

        memcpy(blob->data, body, body_len);
        obj->as.blob = blob;
        break;
    }

    case OBJ_COMMIT:

        // get tree id 
        char *line_start = body;
        char *line_end = memchr(line_start, '\n', body_len);
        if (!line_end)
            goto fail;

        assert(strncmp(line_start, "tree ", 5) == 0);
        line_start += 5;

        char tree_hash[HASH256_DIGEST_LENGTH] = {0};
        size_t tree_hash_len = line_end - line_start;
        if (tree_hash_len >= sizeof(tree_hash))
            goto fail;
        strncpy(tree_hash, line_start, tree_hash_len);
        tree_hash[tree_hash_len] = '\0';

        // get parent id - ma
        line_start = line_end + 1;
        // line_end = memchr(line_start, '\n', body + body_len - line_start);
        



        struct commit_object *commit = malloc(sizeof(*commit));
        if (!commit) goto fail;
        commit->parents = NULL;

        obj->as.commit = commit;
        break;

    case OBJ_TREE:
        // TODO later
        break;

    case OBJ_TAG:
        // TODO later
        break;

    default:
        goto fail;
    }

    free(unzipped_buffer);
    DEBUG("Processed object %s of type %d", hash_value, type);
    return obj;

fail:
    if (obj)
        object_free(obj);  // frees nested stuff too
    free(unzipped_buffer);
    return NULL;
}





static void parse_objects(struct repository *repo, struct RAM* memory)
{
    DEBUG("starting parse_objects");

    DIR *d = NULL;
    DIR *d2 = NULL;
    struct dirent *dir1;
    struct dirent *dir2;

    char *objects_path = utl_path_join(repo->gitdir, "objects", 0);
    if (!objects_path)
        return;

    d = opendir(objects_path);
    if (!d) {
        free(objects_path);
        return;
    }
    DEBUG("opened objects directory: %s", objects_path);
    while ((dir1 = readdir(d)) != NULL) {

        const char *prefix = dir1->d_name;
        if (!strcmp(prefix,".") || !strcmp(prefix,"..") ||
            !strcmp(prefix,"pack") || !strcmp(prefix,"info"))
            continue;

        char *prefix_path = utl_path_join(objects_path, prefix, 0);
        if (!prefix_path)
            continue;

        if (!is_directory(prefix_path)) {
            free(prefix_path);
            continue;
        }

        d2 = opendir(prefix_path);
        if (!d2) {
            free(prefix_path);
            continue;
        }

        while ((dir2 = readdir(d2)) != NULL) {

            const char *suffix = dir2->d_name;
            if (!strcmp(suffix,".") || !strcmp(suffix,".."))
                continue;

            char *file_path = utl_path_join(prefix_path, suffix, 0);
            if (!file_path)
                continue;

            if (is_directory(file_path)) {
                free(file_path);
                continue;
            }

            char *hash_value = utl_path_join(prefix, suffix, 1);
            if (!hash_value) {
                free(file_path);
                continue;
            }

            struct object *obj =
                process(repo, hash_value, file_path);

            if (obj) {

                struct RAM_VALUE value;
                switch (obj->type) {
                    case OBJ_BLOB:   value.value_type = RAM_VALUE_BLOB; break;
                    case OBJ_TREE:   value.value_type = RAM_VALUE_TREE; break;
                    case OBJ_COMMIT: value.value_type = RAM_VALUE_COMMIT; break;
                    case OBJ_TAG:    value.value_type = RAM_VALUE_TAG; break;
                    default:         value.value_type = RAM_VALUE_NONE; break;
                }

                value.obj_value = obj;

                if (!ram_write_cell_by_name(memory, value, hash_value)) {
                    ERROR("ram_write_cell_by_name failed for %s", hash_value);
                }

                object_free(obj);   // RAM now owns its own clone
            }

            free(hash_value);
            free(file_path);
        }

        closedir(d2);
        free(prefix_path);
    }

    closedir(d);
    free(objects_path);

    DEBUG("finished parse_objects");
}
