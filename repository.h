#include "hash.h"

struct object_database;


/*
 * A minimal repository representation.
 *
 * Assumptions:
 * - single repository
 * - single worktree
 * - SHA-1 only (for now)
 * - submodules : we may add submodules to be tricky and to have some recursive calls. 
 * - staging area or index : we may also wanna add index 
 */
struct repository {
	/* Path to .git directory */
	char *gitdir;

	/* Path to working directory (NULL for bare repo) */
	char *worktree;

	/* Object storage (.git/objects, packfiles) */
	struct object_database *objects;    /// we will think about it 

	// we need a HEAD : if properly set will point to the correct 

	// we may also wanna add index 

	// we may add submodules to be tricky and to have some recursive calls. 

	/* Hash algorithm (SHA-1) */
	hash_algo_t hash_algo;

};

/* Repository lifecycle */
int repo_init(struct repository *repo,
              const char *gitdir,
              const char *worktree);

void repo_clear(struct repository *repo);

/* Simple accessors */
const char *repo_gitdir(struct repository *repo);
const char *repo_worktree(struct repository *repo);
// const char *repo_object_dir(struct repository *repo);
