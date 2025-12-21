
typedef enum {
    HASH_SHA1 = 0,
    HASH_SHA256 = 1
} hash_algo_t;


#define DEFAULT_HASH_ALGO HASH_SHA1


// if 40 characters it's Hash_Sha1 
// else it is sha256
hash_algo_t detect_repo_hash(const char *gitdir);

void generate_sha1();

void hash();

void generate_sha256();



// given a hash function return its hash 

