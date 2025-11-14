#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// checkout https://libgit2.org/docs/reference/main/commit/index.html



enum TREE_ENTRY_MODE {
    TREE_MODE_BLOB = 0100644,
    TREE_MODE_BLOB_EXE = 0100755,
    TREE_MODE_TREE = 0040000,
    TREE_MODE_SYMLINK = 0120000,
    TREE_MODE_SUBMODULE = 0160000
};


struct TreeEntry {
    char name[128];        // file or directory name
    char hash[40];         // hash of the blob or subtree
    bool is_blob;          // true if file, false if directory
    struct TreeEntry *next; // linked list of entries
    struct Tree *subtree;  // pointer if this is a directory
};


struct Tree {
    struct TreeEntry *entries;  // linked list of entries in this directory
};


struct Commit {
    char id[40];            // The SHA-1 hash (unique ID for the commit)
    char message[256];      // Commit message
    char author[64];        // Author name/email
    time_t timestamp;       // When the commit was made
    struct Tree *snapshot;  // Pointer to the tree (the file snapshot)
    struct Commit *parent;  // Pointer to the previous commit (parent)
};



int main(){

    printf("num %d \n", 10);
}