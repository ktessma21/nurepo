
#include <stdio.h>
#include "repository.h"

int main(void)
{
    struct repository repo;
    const char *gitdir = "./testrepo/.git";

    if (repo_init(&repo, gitdir, NULL) != 0) {
        printf("repo_init failed\n");
        return 1;
    }

    printf("repo_init succeeded\n");
    printf("gitdir    : %s\n", repo.gitdir);

    if (repo.hash_algo == HASH_SHA1)
        printf("hash algo : SHA1\n");
    else if (repo.hash_algo == HASH_SHA256)
        printf("hash algo : SHA256\n");
    else
        printf("hash algo : UNKNOWN\n");

    repo_clear(&repo);
    return 0;
}

