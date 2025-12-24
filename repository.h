#include "hash.h"
#include "object.h"



/*
 * A minimal repository representation.
 *
 * Assumptions:
 * - single repository
 * - single worktree
 * - SHA-1 only (for now)
 * - submodules : we may add submodules to be tricky and to have some recursive calls. 
 * - staging area or index : we may also wanna add index 
 * - dangling cached objects - temporarily stored 
 */
struct repository {
    char *gitdir;
    char *worktree;

    hash_algo_t hash_algo;

    struct object **all_objects;
    size_t num_objects;
    size_t objects_cap;

    struct object_table ot;   // OID -> object*

    struct object *head;
};



/* Repository lifecycle */
int repo_init(struct repository *repo,
              const char *gitdir,
              const char *worktree);


void repo_clear(struct repository *repo);

void repo_parse_objects(struct repository *repo);
