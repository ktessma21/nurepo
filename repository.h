#include "hash.h"

struct object_database;


/*
 * A minimal repository representation.
 *
 * Assumptions:
 * - single repository
 * - single worktree
 * - SHA-1 only (for now)
 * - no submodules
 * - no alternates
 * - no index (yet)
 */
struct repository {
	/* Path to .git directory */
	char *gitdir;

	/* Path to working directory (NULL for bare repo) */
	char *worktree;

	/* Object storage (.git/objects, packfiles) */
	struct object_database *objects;

	/* Hash algorithm (SHA-1) */
	const struct git_hash_algo *hash_algo;
};

/* Repository lifecycle */
int repo_init(struct repository *repo,
              const char *gitdir,
              const char *worktree);

void repo_clear(struct repository *repo);

/* Simple accessors */
const char *repo_gitdir(struct repository *repo);
const char *repo_worktree(struct repository *repo);
const char *repo_object_dir(struct repository *repo);
