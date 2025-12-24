#include <stdlib.h>
#include "object.h"

void blob_free(struct blob_object *b) {
    if (!b) return;
    free(b->data);
    free(b);
}

void tree_free(struct tree_object *t) {
    if (!t) return;
    for (size_t i = 0; i < t->entry_count; i++) {
        free(t->entries[i].name);
    }
    free(t->entries);
    free(t);
}

void commit_free(struct commit_object *c) {
    if (!c) return;
    free(c->parents);
    free(c->author);
    free(c->message);
    free(c);
}

void tag_free(struct tag_object *t) {
    if (!t) return;
    free(t->tag_name);
    free(t->tagger);
    free(t->message);
    free(t);
}

void object_free(struct object *obj)
{
    if (!obj) return;
    switch (obj->type) {
        case OBJ_BLOB:   blob_free(obj->as.blob); break;
        case OBJ_TREE:   tree_free(obj->as.tree); break;
        case OBJ_COMMIT: commit_free(obj->as.commit); break;
        case OBJ_TAG:    tag_free(obj->as.tag); break;
        default: break;
    }
    free(obj);
}
