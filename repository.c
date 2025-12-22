
// Source - https://stackoverflow.com/a
// Posted by alk, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-21, License - CC BY-SA 4.0

#define _POSIX_C_SOURCE 200809L
#define LOG_ENABLE_DEBUG      // enable DEBUG()
#define LOG_LEVEL LOG_DEBUG   // show DEBUG and above

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
		return is_porcelain_repo(path);
	}

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

	parse_objects(repo);

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

static void process(struct repository *repo, const char *hash_value, const char *file_path) {
	// Implementation for SHA1
	// Example: repo->objects[hash_value] = ...;


	// unzip the file 

	FILE* dest;
	FILE* source = fopen(file_path, "rb");
	if (!source){
		ERROR("file_path was error or can't open file %s", "process");
		return;
	}

	DEBUG("trying to decompress the file");
	decompress_file(source, dest);

	if (!dest)
    	DIE("cannot open destination file %s", "process");

	DEBUG("surviving decompressing the file");

	
// properly read from dest and store in repo->objects
	// ...
	//print from dest
	int ch;

	while ((ch = fgetc(dest)) != EOF) {
		putchar(ch);
	}



	fclose(source);
	fclose(dest);

}

static void parse_objects(struct repository *repo) {
    DEBUG("starting parse_objects");
    // implementation for SHA1

    DIR *d;                     // Directory stream pointer
    struct dirent *dir;

    DEBUG("creating the .git/objects path and checking directory exist");

	char *objects_path = utl_path_join(repo->gitdir, "objects");
    d = opendir(objects_path);
    if (!d) {
        ERROR("could not open directory %s",
              utl_path_join(repo->gitdir, "objects"));
        return;
    }

    DEBUG("survived creating the .git/objects path");

    while ((dir = readdir(d)) != NULL) {

		char *file_path = utl_path_join(objects_path, dir->d_name);
		
        if (!is_directory(file_path)) { // check if it is directory
            continue;
        }

        /* skip . and .. */
        if (strcmp(dir->d_name, ".") == 0 ||
            strcmp(dir->d_name, "..") == 0) {
            continue;
        }

        DEBUG("survived creating the .git/objects/%s path", dir->d_name);

        DIR *d2 = NULL;  // avoid unused-variable warning
        
		
		char *hash_value = dir->d_name;
        d2 = opendir(file_path);

        if (!d2) {
            ERROR("could not open directory %s",
                  file_path);
            continue;
        }

		DEBUG("survived creating the %s path", file_path);
		while ((dir = readdir(d2)) != NULL){
			if (is_directory(file_path)) { // check if it is directory
				continue;
			}

			/* skip . and .. */
			if (strcmp(dir->d_name, ".") == 0 ||
				strcmp(dir->d_name, "..") == 0) {
				continue;	
			}

			DEBUG("survived creating the %s/%s path", file_path, dir->d_name);

			// process for SHA1
			char *hash_value = utl_path_join(hash_value, dir->d_name);
			file_path = utl_path_join(file_path, dir->d_name);
			process(repo, hash_value, file_path);
		}


		closedir(d2);
    }

    closedir(d);
}





void repo_parse_objects(struct repository *repo){
	DEBUG("starting repo_parse_objects");
	if (!repo && !repo -> gitdir){
		ERROR("uninitalized repo for %s %s", "repo_parse_objects", "repository.c");
		return;
	}

	parse_objects(repo);
	
}