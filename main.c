
#include <stdio.h>
#include "repository.h"
#include "ram.h"
#include "compression/compress.h"

static void print_hash_algo(hash_algo_t algo)
{
    if (algo == HASH_SHA1)
        printf("hash algo : SHA1\n");
    else if (algo == HASH_SHA256)
        printf("hash algo : SHA256\n");
    else
        printf("hash algo : UNKNOWN\n");
}

static int test_repository_init(const char *test_name, const char *gitdir)
{
    printf("%s\n", test_name);
    struct repository repo;

    if (repo_init(&repo, gitdir, NULL) != 0) {
        printf("repo_init failed\n");
        return 1;
    }

    printf("repo_init succeeded\n");
    printf("gitdir    : %s\n", repo.gitdir);
    print_hash_algo(repo.hash_algo);

    repo_clear(&repo);
    return 0;
}

int unit_test_empty(void)
{
    return test_repository_init("unit_test_empty", "./test_empty_repo/.git");
}

int unit_test_single_branch(void)
{
    return test_repository_init("unit_test_single_branch", "./test_single_repo/.git");
}

int test_ram(void)
{
    struct RAM *memory = ram_init();
    if (!memory) {
        printf("ram_init failed\n");
        return 1;
    }
    printf("ram_init succeeded\n");

    printf("Initial RAM size: %d\n", ram_size(memory));
    printf("Initial RAM capacity: %d\n", ram_capacity(memory));

    struct RAM_VALUE val;
    val.value_type = RAM_VALUE_NONE;
    val.obj_value = NULL; /* For simplicity, not setting an actual object */

    if (!ram_write_cell_by_addr(memory, val, 0)) {
        printf("ram_write_cell_by_addr failed\n");
        ram_destroy(memory);
        return 1;
    }
    printf("ram_write_cell_by_addr succeeded\n");

    struct RAM_VALUE *read_val = ram_read_cell_by_addr(memory, 0);
    if (!read_val) {
        printf("ram_read_cell_by_addr failed\n");
        ram_destroy(memory);
        return 1;
    }

    printf("ram_read_cell_by_addr succeeded, value_type: %d\n", read_val->value_type);

    ram_free_value(read_val);
    ram_destroy(memory);
    return 0;
}

int unit_test_compression(void)
{
    return 0;
}

int main(void)
{
    if (unit_test_single_branch() != 0) {
        printf("unit_test_single_branch failed\n");
        return 1;
    }

    printf("All tests passed\n");
    return 0;
}

