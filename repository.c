
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

static int parse_one_line_of_commit_object(
    char **cursor,
    const char *end,
    const char *prefix,
    char *out,
    size_t out_size
){
    size_t prefix_len = prefix ? strlen(prefix) : 0;

    char *line_start = *cursor;
    if (line_start >= end)
        return -1;

    char *line_end = memchr(line_start, '\n', end - line_start);
    if (!line_end)
        return -1;

    /* OPTIONAL FIELD HANDLING */
    if (prefix_len > 0 &&
        strncmp(line_start, prefix, prefix_len) != 0)
    {
        return 1;   // <-- means "prefix not present"
    }

    line_start += prefix_len;

    size_t len = line_end - line_start;
    if (len >= out_size)
        return -1;

    strncpy(out, line_start, len);
    out[len] = '\0';

    *cursor = line_end + 1;
    return 0;
}



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



static void parse_objects(struct repository *repo, struct RAM *memory);
static enum RAM_VALUE_TYPES object_type_to_ram_value_type(enum object_type type);
static int should_skip_directory_entry(const char *name);

/* Constants */
#define HEADER_MAX_SIZE 256
#define COMMIT_FIELD_MAX_SIZE 256
#define MESSAGE_MAX_SIZE 1024
#define MODE_TREE_LEN 5
#define MODE_BLOB_LEN 6
#define TREE_MODE_STR "40000"
#define BLOB_MODE_STR "100644"

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
    const char *required_files[] = {"objects", "refs", "HEAD", "config"};
    const size_t num_files = sizeof(required_files) / sizeof(required_files[0]);

    for (size_t i = 0; i < num_files; i++) {
        snprintf(path, sizeof(path), "%s/%s", gitdir, required_files[i]);
        if (access(path, F_OK) != 0)
            return 0;
    }

    return 1;
}


static int is_git_directory(const char *path)
{
    if (!path)
        return 1;

    DEBUG("checking if it is directory");
    if (is_directory(path)) {
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
    memset(repo, 0, sizeof(*repo)); /* zero out the structure for a fresh start */

    if (repo_init_gitdir(repo, gitdir))
        goto error;

    repo->hash_algo = detect_repo_hash(gitdir); /* set the appropriate hash_algo */

    if (worktree)
        repo->worktree = strdup(worktree);

    struct RAM *memory = ram_init();
    if (!memory) {
        DEBUG("ram_init failed in repository.c/repo_init");
        goto error;
    }

    parse_objects(repo, memory);

    /* Validate objects in RAM */
    for (int i = 0; i < memory->size; i++) {
        struct RAM_VALUE value = memory->cells[i];
        if (!confirm_object_is_valid(value.obj_value)) {
            ERROR("Invalid object detected in RAM for variable %s",
                  memory->map[i].varname);
        }
    }

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


static enum object_type get_type_from_header(const char *header)
{
    if (strncmp(header, "blob ", 5) == 0)
        return OBJ_BLOB;
    if (strncmp(header, "commit ", 7) == 0)
        return OBJ_COMMIT;
    if (strncmp(header, "tree ", 5) == 0)
        return OBJ_TREE;
    if (strncmp(header, "tag ", 4) == 0)
        return OBJ_TAG;

    return OBJ_NONE;
}


static int parse_blob(struct object *obj, const char *body, size_t body_len)
{
    struct blob_object *blob = malloc(sizeof(*blob));
    if (!blob)
        return -1;

    blob->size = body_len;
    blob->data = malloc(body_len);
    if (!blob->data) {
        free(blob);
        return -1;
    }

    memcpy(blob->data, body, body_len);
    obj->as.blob = blob;
    return 0;
}

static int parse_commit(struct object *obj, const char *body, size_t body_len,
                       size_t digest_len)
{
    char *cursor = (char *)body;
    char *end = (char *)(body + body_len);

    char tree_hash[HASH256_DIGEST_LENGTH] = {0};
    char parent_hash[HASH256_DIGEST_LENGTH] = {0};
    char author[COMMIT_FIELD_MAX_SIZE] = {0};
    char committer[COMMIT_FIELD_MAX_SIZE] = {0};
    char message[MESSAGE_MAX_SIZE] = {0};

    DEBUG("parsing commit object fields");

    if (parse_one_line_of_commit_object(&cursor, end, "tree ", tree_hash,
                                       sizeof(tree_hash)) < 0)
        return -1;
    DEBUG("parsed tree: %s", tree_hash);

    if (parse_one_line_of_commit_object(&cursor, end, "parent ", parent_hash,
                                       sizeof(parent_hash)) < 0)
        return -1;
    DEBUG("parsed parent: %s", parent_hash);

    if (parse_one_line_of_commit_object(&cursor, end, "author ", author,
                                       sizeof(author)) < 0)
        return -1;
    DEBUG("parsed author: %s", author);

    if (parse_one_line_of_commit_object(&cursor, end, "committer ", committer,
                                       sizeof(committer)) < 0)
        return -1;
    DEBUG("parsed committer: %s", committer);

    if (parse_one_line_of_commit_object(&cursor, end, NULL, message,
                                       sizeof(message)) < 0)
        return -1;
    DEBUG("parsed message: %s", message);

    struct commit_object *commit = malloc(sizeof(*commit));
    if (!commit)
        return -1;

    commit->parents = malloc(sizeof(struct object_id));
    if (!commit->parents) {
        free(commit);
        return -1;
    }

    commit->parent_count = 1;
    commit->tree = (struct object_id){0};
    strncpy((char *)commit->tree.hash, tree_hash, digest_len);
    commit->parents[0] = (struct object_id){0};
    strncpy((char *)commit->parents[0].hash, parent_hash, digest_len);
    commit->author = strdup(author);
    commit->message = strdup(message);

    obj->as.commit = commit;
    return 0;
}

static enum object_type parse_tree_mode(const char *mode_start, size_t mode_len)
{
    if (mode_len == MODE_TREE_LEN && strncmp(mode_start, TREE_MODE_STR, MODE_TREE_LEN) == 0)
        return OBJ_TREE;
    if (mode_len == MODE_BLOB_LEN && strncmp(mode_start, BLOB_MODE_STR, MODE_BLOB_LEN) == 0)
        return OBJ_BLOB;

    return OBJ_NONE;
}

static int parse_tree_entry(struct tree_object *tree, char **cursor,
                           const char *end, size_t digest_len)
{
    char *tree_cursor = *cursor;

    /* Parse mode */
    char *mode_start = tree_cursor;
    char *sp = memchr(mode_start, ' ', end - mode_start);
    if (!sp)
        return -1;

    size_t mode_len = sp - mode_start;
    enum object_type entry_type = parse_tree_mode(mode_start, mode_len);
    if (entry_type == OBJ_NONE)
        return -1;

    tree_cursor = sp + 1; /* skip past space after mode */

    /* Parse name */
    char *name_start = tree_cursor;
    char *name_end = memchr(name_start, '\0', end - name_start);
    if (!name_end)
        return -1;

    size_t name_len = name_end - name_start;
    char name[name_len + 1];
    memcpy(name, name_start, name_len);
    name[name_len] = '\0';

    tree_cursor = name_end + 1; /* skip null terminator */
    DEBUG("parsed tree entry name: %s", name);

    /* Parse hash */
    size_t hash_len = digest_len;
    if (tree_cursor + hash_len > end)
        return -1;

    /* Store entry */
    tree->entry_count++;
    tree->entries = realloc(tree->entries,
                           tree->entry_count * sizeof(struct tree_entry));
    if (!tree->entries)
        return -1;

    struct tree_entry *entry = &tree->entries[tree->entry_count - 1];
    entry->name = strdup(name);
    entry->type = entry_type;
    memset(&entry->oid, 0, sizeof(entry->oid));
    memcpy(entry->oid.hash, tree_cursor, hash_len);

    tree_cursor += hash_len;

    DEBUG("parsed tree entry hash (first 4 bytes): %02x%02x%02x%02x",
          entry->oid.hash[0], entry->oid.hash[1],
          entry->oid.hash[2], entry->oid.hash[3]);

    *cursor = tree_cursor;
    return 0;
}

static int parse_tree(struct object *obj, const char *body, size_t body_len,
                     size_t digest_len)
{
    obj->as.tree = malloc(sizeof(struct tree_object));
    if (!obj->as.tree)
        return -1;

    struct tree_object *tree = obj->as.tree;
    tree->entry_count = 0;
    tree->entries = NULL;

    char *tree_cursor = (char *)body;
    char *tree_end = (char *)(body + body_len);

    while (tree_cursor < tree_end) {
        DEBUG("parsing tree object fields");
        if (parse_tree_entry(tree, &tree_cursor, tree_end, digest_len) < 0)
            return -1;
    }

    return 0;
}

static int parse_object_payload(struct object *obj, enum object_type type,
                               const char *body, size_t body_len,
                               size_t digest_len)
{
    switch (type) {
    case OBJ_BLOB:
        return parse_blob(obj, body, body_len);

    case OBJ_COMMIT:
        return parse_commit(obj, body, body_len, digest_len);

    case OBJ_TREE:
        return parse_tree(obj, body, body_len, digest_len);

    case OBJ_TAG:
        /* TODO: implement tag parsing */
        return -1;

    default:
        return -1;
    }
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

    /* Parse header */
    char *nul = memchr(unzipped_buffer, '\0', total_size);
    if (!nul) {
        ERROR("Invalid object: missing NUL header terminator");
        free(unzipped_buffer);
        return NULL;
    }

    char *body = nul + 1;
    size_t body_len = total_size - (body - unzipped_buffer);
    size_t header_len = (size_t)(nul - unzipped_buffer);

    char saved_header[HEADER_MAX_SIZE] = {0};
    snprintf(saved_header, sizeof(saved_header), "%.*s",
            (int)header_len, unzipped_buffer);

    dump_object_pretty(hash_value, saved_header, body, body_len);

    enum object_type type = get_type_from_header(unzipped_buffer);
    if (type == OBJ_NONE) {
        ERROR("Unknown object type in %s", file_path);
        free(unzipped_buffer);
        return NULL;
    }

    /* Allocate object */
    struct object *obj = calloc(1, sizeof(struct object));
    if (!obj) {
        free(unzipped_buffer);
        return NULL;
    }

    obj->type = type;
    obj->flags = OBJ_FLAG_NONE;

    /* Fill OID */
    size_t digest_len = (repo->hash_algo == HASH1_DIGEST_LENGTH)
                       ? HASH1_DIGEST_LENGTH
                       : HASH256_DIGEST_LENGTH;
    strcpy((char *)obj->oid.hash, hash_value);

    /* Parse payload based on type */
    if (parse_object_payload(obj, type, body, body_len, digest_len) < 0) {
        object_free(obj);
        free(unzipped_buffer);
        return NULL;
    }

    free(unzipped_buffer);
    DEBUG("Processed object %s of type %d", hash_value, type);
    return obj;
}





static enum RAM_VALUE_TYPES object_type_to_ram_value_type(enum object_type type)
{
    switch (type) {
    case OBJ_BLOB:   return RAM_VALUE_BLOB;
    case OBJ_TREE:   return RAM_VALUE_TREE;
    case OBJ_COMMIT: return RAM_VALUE_COMMIT;
    case OBJ_TAG:    return RAM_VALUE_TAG;
    default:         return RAM_VALUE_NONE;
    }
}

static int should_skip_directory_entry(const char *name)
{
    return !strcmp(name, ".") || !strcmp(name, "..") ||
           !strcmp(name, "pack") || !strcmp(name, "info");
}

static void parse_objects(struct repository *repo, struct RAM *memory)
{
    DEBUG("starting parse_objects");

    char *objects_path = utl_path_join(repo->gitdir, "objects", 0);
    if (!objects_path)
        return;

    DIR *d = opendir(objects_path);
    if (!d) {
        free(objects_path);
        return;
    }

    DEBUG("opened objects directory: %s", objects_path);

    struct dirent *dir1;
    while ((dir1 = readdir(d)) != NULL) {
        const char *prefix = dir1->d_name;
        if (should_skip_directory_entry(prefix))
            continue;

        char *prefix_path = utl_path_join(objects_path, prefix, 0);
        if (!prefix_path)
            continue;

        if (!is_directory(prefix_path)) {
            free(prefix_path);
            continue;
        }

        DIR *d2 = opendir(prefix_path);
        if (!d2) {
            free(prefix_path);
            continue;
        }

        struct dirent *dir2;
        while ((dir2 = readdir(d2)) != NULL) {
            const char *suffix = dir2->d_name;
            if (!strcmp(suffix, ".") || !strcmp(suffix, ".."))
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

            struct object *obj = process(repo, hash_value, file_path);
            if (obj) {
                struct RAM_VALUE value;
                value.value_type = object_type_to_ram_value_type(obj->type);
                value.obj_value = obj;

                if (!ram_write_cell_by_name(memory, value, hash_value)) {
                    ERROR("ram_write_cell_by_name failed for %s", hash_value);
                }

                object_free(obj); /* RAM now owns its own clone */
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
