
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
#include "compress.h"


static void parse_objects(struct repository *repo);

// }

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif



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
 * Attempt to resolve and set the provided 'gitdir' for repository 'repo'.
 * Return 0 upon success and a non-zero value upon failure.
 */




/*
 * Initialize 'repo' based on the provided 'gitdir'.
 * Return 0 upon success and a non-zero value upon failure.
 */
int repo_init(struct repository *repo,
              const char *gitdir,
              const char *worktree)
{
	memset(repo, 0, sizeof(*repo));

	if (repo_init_gitdir(repo, gitdir))
		goto error;

	repo->hash_algo = detect_repo_hash(gitdir); // set the appropriate hash_algo

	if (worktree)
		repo->worktree = strdup(worktree);

	repo -> all_objects = malloc(sizeof(struct object*) * 10000);
	if (!repo -> all_objects){
		DEBUG("failed to allocate memory for all_objects");
		return -1;
	}
	repo -> num_objects = 0;
	parse_objects(repo);
	free(repo -> all_objects);
	return 0;

error:
	repo_clear(repo);
	return -1;
}



void repo_clear(struct repository *repo)
{
	if (!repo)
		return;

	free(repo->gitdir);
	repo->gitdir = NULL;

	free(repo->worktree);
	repo->worktree = NULL;

	
	
	// free(repo -> all_objects);

	// free(repo);
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

static void process(struct repository *repo, const char *hash_value, const char *file_path) {
    size_t total_size;
    
    // 1. Decompress the file into a temporary buffer
    char *unzipped_buffer = decompress_file(file_path, &total_size);
    if (!unzipped_buffer) {
        ERROR("Decompression failed for %s", file_path);
        return; 
    }

    // 2. Identify the object type from the header string
    enum object_type type = get_type_from_header(unzipped_buffer);
    if (type == OBJ_NONE) {
        free(unzipped_buffer);
        return;
    }

    struct object *obj = malloc(sizeof(struct object));
    if (!obj) {
        free(unzipped_buffer);
        return;
    }

    // 4. Fill common metadata
    obj->type = type;
	
	size_t digest_len = (repo->hash_algo == HASH1_DIGEST_LENGTH) ? HASH1_DIGEST_LENGTH : HASH256_DIGEST_LENGTH;

	// 3. Convert Hex String to Binary Bytes
	// You need a helper function for this. strncpy will NOT work.
	for (size_t i = 0; i < digest_len; i++) {
		// This reads 2 hex characters and converts them to 1 byte
		sscanf(&hash_value[i * 2], "%02hhx", &obj->oid.hash[i]);
	}
	// 3. Delegate type-specific processing
	switch (type) {
		case OBJ_BLOB:
			obj->as.blob = process_blob(unzipped_buffer, total_size);
			if (!obj->as.blob) {
				free(obj);
				free(unzipped_buffer);
				return;
			}
			break;
		// case OBJ_COMMIT:
		// 	obj->as.commit = process_commit(unzipped_buffer, total_size, obj);
		// 	if (!obj->as.commit) {
		// 		free(obj->oid);
		// 		free(obj);
		// 		free(unzipped_buffer);
		// 		return;
		// 	}
		// 	break;
		// case OBJ_TREE:
		// 	obj->as.tree = process_tree(unzipped_buffer, total_size, obj);
		// 	if (!obj->as.tree) {
		// 		free(obj->oid);
		// 		free(obj);
		// 		free(unzipped_buffer);
		// 		return;
		// 	}
		// 	break;
		default:
			ERROR("Unsupported object type: %d", type);
			// free(obj->oid);
			free(obj);
			free(unzipped_buffer);
			return;
	}
	// DEBUG("OID registerd -> %s", obj->oid.hash);

	DEBUG("Object %s (Type: %d) processed successfully", hash_value, type);

	// exit(1);

    // 5. Delegate type-specific processing
    // if (type == OBJ_BLOB) {
    //     obj->as.blob = process_blob(unzipped_buffer, total_size);
    //     if (!obj->as.blob) {
    //         free(obj);
    //         free(unzipped_buffer);
    //         return;
    //     }
    // } 
    // (You can add other types like OBJ_TREE here later)

	repo -> all_objects[repo -> num_objects++] = obj;
    // 6. Final cleanup of the temporary decompression buffer
    free(unzipped_buffer);

    // DEBUG("Object %s (Type: %d) processed successfully", hash_value, type);x
    
    // TODO: Add 'obj' to your repo's object table/map
}





static void parse_objects(struct repository *repo)
{
    DEBUG("starting parse_objects");

    DIR *d = NULL;
    DIR *d2 = NULL;
    struct dirent *dir1;
    struct dirent *dir2;

    char *objects_path = utl_path_join(repo->gitdir, "objects", 0);
    if (!objects_path) {
        ERROR("could not create .git/objects path");
        return;
    }

    d = opendir(objects_path);
    if (!d) {
        ERROR("could not open directory %s", objects_path);
        free(objects_path);
        return;
    }

    DEBUG("opened objects directory %s", objects_path);

    /* iterate over object prefix directories (e.g. ab/) */
    while ((dir1 = readdir(d)) != NULL) {

        const char *prefix = dir1->d_name;

        /* skip . and .. */
        if (strcmp(prefix, ".") == 0 || strcmp(prefix, "..") == 0)
            continue;

        /* skip pack and info */
        if (strcmp(prefix, "pack") == 0 || strcmp(prefix, "info") == 0)
            continue;

        char *prefix_path = utl_path_join(objects_path, prefix, 0);
        if (!prefix_path) {
            ERROR("could not create .git/objects/%s path", prefix);
            continue;
        }

        if (!is_directory(prefix_path)) {
            free(prefix_path);
            continue;
        }

        DEBUG("processing directory %s", prefix_path);

        d2 = opendir(prefix_path);
        if (!d2) {
            ERROR("could not open directory %s", prefix_path);
            free(prefix_path);
            continue;
        }

        /* iterate over loose objects inside prefix */
        while ((dir2 = readdir(d2)) != NULL) {

            const char *suffix = dir2->d_name;

            /* skip . and .. */
            if (strcmp(suffix, ".") == 0 || strcmp(suffix, "..") == 0)
                continue;

            char *file_path = utl_path_join(prefix_path, suffix, 0);
            if (!file_path) {
                ERROR("could not create .git/objects/%s/%s path",
                      prefix, suffix);
                continue;
            }

            if (is_directory(file_path)) {
                free(file_path);
                continue;
            }

            char *hash_value = utl_path_join(prefix, suffix, 1);
            if (!hash_value) {
                ERROR("could not create hash value %s%s",
                      prefix, suffix);
                free(file_path);
                continue;
            }

            DEBUG("processing object %s", hash_value);

            process(repo, hash_value, file_path);

            free(hash_value);
            free(file_path);
        }

        closedir(d2);
        d2 = NULL;
        free(prefix_path);
    }

    closedir(d);
    free(objects_path);

    DEBUG("finished parse_objects");
}





void repo_parse_objects(struct repository *repo){
	DEBUG("starting repo_parse_objects");
	if (!repo && !repo -> gitdir){
		ERROR("uninitalized repo for %s %s", "repo_parse_objects", "repository.c");
		return;
	}

	parse_objects(repo);
	
}