clear
make valgrind
clear
make valgrind
clear
mkae
make
clear
make valgrind
clear
make valgrind
clear
make
make valgrind
clear
make valgrind
cear
clear
make
make valgrind
clear
exit
clear
make
make valgrind 
make
make valgrind
clear
make valgrind
clear
exit
mkdir test_single_repo
nano ./.gitignore
cd test_single_repo/
ls
clear
git init
ls
ls -a
clear
touch sample.txt
ls
git add .
git status
git commit -m "inital commit"
clear
ls
cd .git
ls
cd objects
ls
cd ..
cd ..
cd .. ..
cd ..
clear
make 
make valgrind
make valgrind
clear
make valgrind
cd test_single_repo/
clear
git unpack-objects 
clear
cd ..
git unpack-objects < test_single_repo/*
git unpack-objects < test_single_repo/*.pack
cd test_single_repo/
ls
cd .got
cd .git
cd objects
ls
clear
git unpack < *
cd ..
git unpack < objects/*
cd ..
git unpack 
git --help 
clear
cd ..
make
make valgrind
clear
make valgrind
clear
make
make valgrind
clear
make
mkae
make
clear
make
clear
make valgrind 
clear
make valgrind
cd test_so
de test_single_repo/
cd test_single_repo/
clear
cd .git
ls
nano 
cd objects
ls
clear
cd ..
cd ..
cd ..
cd ..
clear
cd ~
cleas
clear
ls
clear
make valgrind 
clear
make valgrind 
clear
make valgrind
clear
make valgrind 
clear
make valgrind 
clear
make valgrind
clear
make valgrind 
clear
make valgrind
clear
make valgrind 
clear
make valgrind
cler
clear
make valgrind
clear
clear
make valgrind
clear
make valgrind 
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind 
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
#define MAX_OBJECT_ID_LENGTH SHA256_DIGEST_LENGTH
#define HASH256_DIGEST_LENGTH 64
#define HASH1_DIGEST_LENGTH 40
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

#endif /* OBJECT_H */


clear
clear

clear
make
make valgrind
clear
make valgrind 
clear
make valgrind
clear
make valgrind 
clear
exit
clear
git log
clear
clear
make valgrind 
clear
make valgrind 
clear
mv vector.h ram.h
mv vector.c ram.c
clear
make 
clear
make 
clear
make 
clear
make 
clear
make 
clear
make
clear
make
clear
make
clear
make
make valgrind
clear
make valgrind 
clear
make valgrind 
make
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
git add . 
git commit -m " Stable memory management unit built"
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind 
clear
make valgrind 
clear
make valgrind
clear
make 
clear
make valgrind
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind 
clear
make valgrind 
clear
make valgrind 
clear
git add . 
git commit -m "Cleared the process function. Add a safe_exit for safely existing during error."
git log
clear
git push 
clear
exit
clear
git log
clear
make valgrind
exit
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
rm -rf blobs
clear
make file
make
eclear
clear
make file
make
make valgrind
eclear
clear
make valgrind 
ls
clear
make valgrind
make valgrind
clear
make valgrind 
clear
make valgrind 
clear
make valgrind
clear
make valgrind 
clear
make valgrind
clear
cp ./ ./test_single_repo/.git/objects/48/5f3c9a6f6ef3b6246eafd4ee26f77069cf6c90
cp -r ./ ./test_single_repo/.git/objects/48/5f3c9a6f6ef3b6246eafd4ee26f77069cf6c90
cp -r ./new  ./test_single_repo/.git/objects/48/5f3c9a6f6ef3b6246eafd4ee26f77069cf6c90
git -C test_single_repo cat-file -t 485f3c9a6f6ef3b6246eafd4ee26f77069cf6c90
clear
make valgrind
clear
make valgrind 
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind
clar
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind
clear
make valgrind 
clear
make valgrind 
clear
make valgrind 
clear
make valgrind
clear
make valgrind
ckear
clear
make valgrind
clear
make valgrind
clear
make valgrind
rm unzipped*
ls
clear
make valgrind
git add . 
git commit -m "added object_clone function, no memory leak issues for now"
clear
git add . 
git commit -m "removed unzipped"
clear
git log
clear
ls
make valgrind
make valgrind
make valgrind
clear
rm unzipped*
make valgrind
make valgrind
clear 
make valgrind
rm unzipped*
make valgrind
clear
make valgrind 
cd test_single_repo/
git add . 
git status
clear
git commit -m "second commit"
cd ..
make valgrind
clear
make valgrind
clear
git add .
git commit -m "working code, no leak, clean and understandable. STABLE"
clear
make
make valgrind
clear
make valgrind 
clear
make valgrind
clear
make valgrind
clear
make valgrind
make valgrind 
clear
make valgrind
clear
git status 
git log
exit
