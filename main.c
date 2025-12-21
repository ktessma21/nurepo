




#include <stdio.h>
#include "hash.h"



int main() {


    const char *dir = ".git";

 
    if (detect_repo_hash(dir) == HASH_SHA1){
        printf("hash_sha1");
        return 0;
    }
    printf("success \n");


    return 0;
}
