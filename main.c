
#include <stdio.h>
#include "repository.h"


int unit_test_empty(void)
{
    struct repository repo;
    const char *gitdir = "./test_empty_repo/.git";

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

int main(void)
{
   if (unit_test_empty() != 0) {
        printf("unit_test_empty failed\n");
        return 1;
    }

    printf("All tests passed\n");
    return 0;
}

