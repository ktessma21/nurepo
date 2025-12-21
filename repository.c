// #include "git-compat-util.h"
// #include "abspath.h"
#include "repository.h"
#include "hash.h"

// #include "odb.h"
// #include "config.h"
// #include "object.h"
// #include "lockfile.h"
// #include "path.h"
// #include "read-cache-ll.h"
// #include "remote.h"
// #include "setup.h"
// #include "loose.h"
// #include "submodule-config.h"
// #include "sparse-index.h"
// #include "trace2.h"
// #include "promisor-remote.h"
// #include "refs.h"

/*
 * We do not define `USE_THE_REPOSITORY_VARIABLE` in this file because we do
 * not want to rely on functions that implicitly use `the_repository`. This
 * means that the `extern` declaration of `the_repository` isn't visible here,
 * which makes sparse unhappy. We thus declare it here.
 */
// extern struct repository *the_repository;

// /* The main repository */
// static struct repository the_repo;
// struct repository *the_repository = &the_repo;

// /*
//  * An escape hatch: if we hit a bug in the production code that fails
//  * to set an appropriate hash algorithm (most likely to happen when
//  * running outside a repository), we can tell the user who reported
//  * the crash to set the environment variable to "sha1" (all lowercase)
//  * to revert to the historical behaviour of defaulting to SHA-1.
//  */
// static void set_default_hash_algo(struct repository *repo)
// {
// 	const char *hash_name;
// 	int algo;

// 	hash_name = getenv("GIT_TEST_DEFAULT_HASH_ALGO");
// 	if (!hash_name)
// 		return;
// 	algo = hash_algo_by_name(hash_name);
// 	if (algo == GIT_HASH_UNKNOWN)
// 		return;

// 	repo_set_hash_algo(repo, algo);
// }

// void initialize_repository(struct repository *repo)
// {
// 	repo->remote_state = remote_state_new();
// 	repo->parsed_objects = parsed_object_pool_new(repo);
// 	ALLOC_ARRAY(repo->index, 1);
// 	index_state_init(repo->index, repo);
// 	repo->check_deprecated_config = true;

// 	/*
// 	 * When a command runs inside a repository, it learns what
// 	 * hash algorithm is in use from the repository, but some
// 	 * commands are designed to work outside a repository, yet
// 	 * they want to access the_hash_algo, if only for the length
// 	 * of the hashed value to see if their input looks like a
// 	 * plausible hash value.
// 	 *
// 	 * We are in the process of identifying such code paths and
// 	 * giving them an appropriate default individually; any
// 	 * unconverted code paths that try to access the_hash_algo
// 	 * will thus fail.  The end-users however have an escape hatch
// 	 * to set GIT_TEST_DEFAULT_HASH_ALGO environment variable to
// 	 * "sha1" to get back the old behaviour of defaulting to SHA-1.
// 	 *
// 	 * This escape hatch is deliberately kept unadvertised, so
// 	 * that they see crashes and we can get a report before
// 	 * telling them about it.
// 	 */
// 	if (repo == the_repository)
// 		set_default_hash_algo(repo);
// }

// static void expand_base_dir(char **out, const char *in,
// 			    const char *base_dir, const char *def_in)
// {
// 	free(*out);
// 	if (in)
// 		*out = xstrdup(in);
// 	else
// 		*out = xstrfmt("%s/%s", base_dir, def_in);
// }

// const char *repo_get_git_dir(struct repository *repo)
// {
// 	if (!repo->gitdir)
// 		BUG("repository hasn't been set up");
// 	return repo->gitdir;
// }

// const char *repo_get_common_dir(struct repository *repo)
// {
// 	if (!repo->commondir)
// 		BUG("repository hasn't been set up");
// 	return repo->commondir;
// }

// const char *repo_get_object_directory(struct repository *repo)
// {
// 	if (!repo->objects->sources)
// 		BUG("repository hasn't been set up");
// 	return repo->objects->sources->path;
// }

// const char *repo_get_index_file(struct repository *repo)
// {
// 	if (!repo->index_file)
// 		BUG("repository hasn't been set up");
// 	return repo->index_file;
// }

// const char *repo_get_graft_file(struct repository *repo)
// {
// 	if (!repo->graft_file)
// 		BUG("repository hasn't been set up");
// 	return repo->graft_file;
// }

// const char *repo_get_work_tree(struct repository *repo)
// {
// 	return repo->worktree;
// }

// static void repo_set_commondir(struct repository *repo,
// 			       const char *commondir)
// {
// 	struct strbuf sb = STRBUF_INIT;

// 	free(repo->commondir);

// 	if (commondir) {
// 		repo->different_commondir = 1;
// 		repo->commondir = xstrdup(commondir);
// 		return;
// 	}

// 	repo->different_commondir = get_common_dir_noenv(&sb, repo->gitdir);
// 	repo->commondir = strbuf_detach(&sb, NULL);
// }

// void repo_set_gitdir(struct repository *repo,
// 		     const char *root,
// 		     const struct set_gitdir_args *o)
// {
// 	const char *gitfile = read_gitfile(root);
// 	/*
// 	 * repo->gitdir is saved because the caller could pass "root"
// 	 * that also points to repo->gitdir. We want to keep it alive
// 	 * until after xstrdup(root). Then we can free it.
// 	 */
// 	char *old_gitdir = repo->gitdir;

// 	repo->gitdir = xstrdup(gitfile ? gitfile : root);
// 	free(old_gitdir);

// 	repo_set_commondir(repo, o->commondir);

// 	if (!repo->objects)
// 		repo->objects = odb_new(repo, o->object_dir, o->alternate_db);
// 	else if (!o->skip_initializing_odb)
// 		BUG("cannot reinitialize an already-initialized object directory");

// 	repo->disable_ref_updates = o->disable_ref_updates;

// 	expand_base_dir(&repo->graft_file, o->graft_file,
// 			repo->commondir, "info/grafts");
// 	expand_base_dir(&repo->index_file, o->index_file,
// 			repo->gitdir, "index");
// }

// void repo_set_hash_algo(struct repository *repo, int hash_algo)
// {
// 	repo->hash_algo = &hash_algos[hash_algo];
// }

// void repo_set_compat_hash_algo(struct repository *repo, int algo)
// {
// 	if (hash_algo_by_ptr(repo->hash_algo) == algo)
// 		BUG("hash_algo and compat_hash_algo match");
// 	repo->compat_hash_algo = algo ? &hash_algos[algo] : NULL;
// 	if (repo->compat_hash_algo)
// 		repo_read_loose_object_map(repo);
// }

// void repo_set_ref_storage_format(struct repository *repo,
// 				 enum ref_storage_format format)
// {
// 	repo->ref_storage_format = format;
// }
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "hash.c"

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


void repo_set_worktree(struct repository *repo, const char *path)
{
	repo->worktree = real_pathdup(path, 1);

	trace2_def_repo(repo);
}



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

	repo_set_hash_algo(repo, detect_repo_hash(gitdir));  // working so far 

	if (worktree)
		repo_set_worktree(repo, worktree);

	return 0;

error:
	repo_clear(repo);
	return -1;
}




// static void repo_clear_path_cache(struct repo_path_cache *cache)
// {
// 	FREE_AND_NULL(cache->squash_msg);
// 	FREE_AND_NULL(cache->squash_msg);
// 	FREE_AND_NULL(cache->merge_msg);
// 	FREE_AND_NULL(cache->merge_rr);
// 	FREE_AND_NULL(cache->merge_mode);
// 	FREE_AND_NULL(cache->merge_head);
// 	FREE_AND_NULL(cache->fetch_head);
// 	FREE_AND_NULL(cache->shallow);
// }

// void repo_clear(struct repository *repo)
// {
// 	struct hashmap_iter iter;
// 	struct strmap_entry *e;

// 	FREE_AND_NULL(repo->gitdir);
// 	FREE_AND_NULL(repo->commondir);
// 	FREE_AND_NULL(repo->graft_file);
// 	FREE_AND_NULL(repo->index_file);
// 	FREE_AND_NULL(repo->worktree);
// 	FREE_AND_NULL(repo->submodule_prefix);

// 	odb_free(repo->objects);
// 	repo->objects = NULL;

// 	parsed_object_pool_clear(repo->parsed_objects);
// 	FREE_AND_NULL(repo->parsed_objects);

// 	repo_settings_clear(repo);

// 	if (repo->config) {
// 		git_configset_clear(repo->config);
// 		FREE_AND_NULL(repo->config);
// 	}

// 	if (repo->submodule_cache) {
// 		submodule_cache_free(repo->submodule_cache);
// 		repo->submodule_cache = NULL;
// 	}

// 	if (repo->index) {
// 		discard_index(repo->index);
// 		FREE_AND_NULL(repo->index);
// 	}

// 	if (repo->promisor_remote_config) {
// 		promisor_remote_clear(repo->promisor_remote_config);
// 		FREE_AND_NULL(repo->promisor_remote_config);
// 	}

// 	if (repo->remote_state) {
// 		remote_state_clear(repo->remote_state);
// 		FREE_AND_NULL(repo->remote_state);
// 	}

// 	strmap_for_each_entry(&repo->submodule_ref_stores, &iter, e)
// 		ref_store_release(e->value);
// 	strmap_clear(&repo->submodule_ref_stores, 1);

// 	strmap_for_each_entry(&repo->worktree_ref_stores, &iter, e)
// 		ref_store_release(e->value);
// 	strmap_clear(&repo->worktree_ref_stores, 1);

// 	repo_clear_path_cache(&repo->cached_paths);
// }

// int repo_read_index(struct repository *repo)
// {
// 	int res;

// 	/* Complete the double-reference */
// 	if (!repo->index) {
// 		ALLOC_ARRAY(repo->index, 1);
// 		index_state_init(repo->index, repo);
// 	} else if (repo->index->repo != repo) {
// 		BUG("repo's index should point back at itself");
// 	}

// 	res = read_index_from(repo->index, repo->index_file, repo->gitdir);

// 	prepare_repo_settings(repo);
// 	if (repo->settings.command_requires_full_index)
// 		ensure_full_index(repo->index);

// 	/*
// 	 * If sparse checkouts are in use, check whether paths with the
// 	 * SKIP_WORKTREE attribute are missing from the worktree; if not,
// 	 * clear that attribute for that path.
// 	 */
// 	clear_skip_worktree_from_present_files(repo->index);

// 	return res;
// }

// int repo_hold_locked_index(struct repository *repo,
// 			   struct lock_file *lf,
// 			   int flags)
// {
// 	if (!repo->index_file)
// 		BUG("the repo hasn't been setup");
// 	return hold_lock_file_for_update(lf, repo->index_file, flags);
// }