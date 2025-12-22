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

	struct object *all_objects;





};

/* Repository lifecycle */
int repo_init(struct repository *repo,
              const char *gitdir,
              const char *worktree);


void repo_clear(struct repository *repo);

void repo_parse_objects(struct repository *repo);
