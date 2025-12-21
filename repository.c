// #include "git-compat-util.h"
// #include "abspath.h"
#include "repository.h"
#include "hash.h"

// }
#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>


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
{
	struct stat st;
	if (stat(path, &st) < 0)
		return 0;
	if (!S_ISDIR(st.st_mode))
		return 0;

	return is_porcelain_repo(path);
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

	// free(repo);
}
