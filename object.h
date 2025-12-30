#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>
#include <openssl/sha.h>


/*
 * ============================================================
 * Object ID (content address)
 * ============================================================
 */

/* Max hash size we support (SHA-256) */
#define HASH256_DIGEST_LENGTH 65 /* e.g., SHA-256 in hex + null terminator */
#define HASH1_DIGEST_LENGTH 41. /* e.g., SHA-1 in hex + null terminator */
#define MAX_OBJECT_ID_LENGTH HASH256_DIGEST_LENGTH


struct object_id {
    unsigned char hash[MAX_OBJECT_ID_LENGTH];
};


/*
 * ============================================================
 * Object types (Git-like)
 * ============================================================
 */

enum object_type {
    OBJ_NONE   = 0,
    OBJ_COMMIT = 1,
    OBJ_TREE   = 2,
    OBJ_BLOB   = 3,
    OBJ_TAG    = 4
};

/*
 * ============================================================
 * Object flags (in-memory traversal / state)
 * ============================================================
 */

enum object_flags {
    OBJ_FLAG_NONE   = 0,
    OBJ_FLAG_SEEN   = 1 << 0,
    OBJ_FLAG_MARKED = 1 << 1,
    OBJ_FLAG_BAD    = 1 << 2
};

/*
 * ============================================================
 * Object payloads (type-specific data)
 * ============================================================
 */

/* ---------- Blob ---------- */
struct blob_object {
    size_t size;
    void *data;
};

/* ---------- Tree ---------- */
struct tree_entry {
    char *name;
    enum object_type type;   /* blob or tree */
    struct object_id oid;
};

struct tree_object {
    size_t entry_count;
    struct tree_entry *entries;
};

/* ---------- Commit ---------- */
struct commit_object {
    struct object_id tree;
    struct object_id *parents;
    size_t parent_count;
    char *author;
    char *message;
};

/* ---------- Tag ---------- */
struct tag_object {
    struct object_id target;
    enum object_type target_type;
    char *tag_name;
    char *tagger;
    char *message;
};

/*
 * ============================================================
 * Unified object (tagged union)
 * ============================================================
 */

struct object {
    enum object_type type;      /* what kind of object */
    unsigned flags;             /* OBJ_FLAG_* */
    struct object_id oid;     /* content hash */

    union {
        struct blob_object   *blob;
        struct tree_object   *tree;
        struct commit_object *commit;
        struct tag_object    *tag;
    } as;
};




void object_free(struct object *obj);
void blob_free(struct blob_object *b);
void tree_free(struct tree_object *t);
void commit_free(struct commit_object *c);
void tag_free(struct tag_object *t);

struct object *object_clone(const struct object *src);


#endif