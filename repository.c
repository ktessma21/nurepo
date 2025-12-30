
// Source - https://stackoverflow.com/a
// Posted by alk, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-21, License - CC BY-SA 4.0

#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "log.h"
#include "repository.h"
#include "hash.h"
#include "utl.h"
#include "compression/compress.h"
#include "ram.h"


static void parse_objects(struct repository *repo, struct RAM* memory);

// }

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif



static unsigned char hex_to_byte(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static void parse_hex_oid(const char *hex, unsigned char *out, size_t len)
{
    for (size_t i = 0; i < len; i++)
        out[i] = (hex_to_byte(hex[i*2]) << 4) |
                  hex_to_byte(hex[i*2+1]);
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

static struct blob_object *process_blob(char *unzipped_buffer, size_t total_size) {
    // 1. Allocate the blob detail structure
    struct blob_object *blob = malloc(sizeof(struct blob_object));
    if (!blob) return NULL;

    // 2. Find the end of the header (the first null byte)
    char *null_byte = memchr(unzipped_buffer, '\0', total_size);
    if (!null_byte) {
        free(blob);
        return NULL;
    }

	// here the total size written on the blob file may be the size of the actual content instead. 
    char *data_start = null_byte + 1;
    size_t header_len = (size_t)(data_start - unzipped_buffer);
    blob->size = total_size - header_len;

    // 4. Allocate memory for the content and copy it
    // We use malloc + memcpy because blobs can be binary data
    blob->data = malloc(blob->size); 
    if (!blob->data) {
        free(blob);
        return NULL;
    }
    
    memcpy(blob->data, data_start, blob->size);

    return blob;
}

static struct object *process_commit(char *unzipped_buffer, size_t total_size, struct object *obj){
	return NULL;
}

static struct object *process_tree(char *unzipped_buffer, size_t total_size, struct object *obj){
	return NULL;
}


static struct object *process(struct repository *repo,
                              const char *hash_value,
                              const char *file_path)
{
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

    size_t header_len = (size_t)(nul - unzipped_buffer);
    char *body = nul + 1;
    size_t body_len = total_size - header_len - 1;

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

    parse_hex_oid(hash_value, obj->oid.hash, digest_len);

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
        // TODO later
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
