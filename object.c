#include <stdlib.h>
#include "object.h"
#include <string.h>

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


struct object *object_clone(const struct object *src)
{
    if (!src)
        return NULL;

    struct object *dst = calloc(1, sizeof(*dst));
    if (!dst)
        return NULL;

    dst->type  = src->type;
    dst->flags = src->flags;
    memcpy(&dst->oid, &src->oid, sizeof(struct object_id));

    switch (src->type) {

    case OBJ_BLOB: {
        dst->as.blob = malloc(sizeof(struct blob_object));
        if (!dst->as.blob)
            goto fail;

        dst->as.blob->size = src->as.blob->size;
        dst->as.blob->data = malloc(dst->as.blob->size);
        if (!dst->as.blob->data)
            goto fail;

        memcpy(dst->as.blob->data,
               src->as.blob->data,
               dst->as.blob->size);
        break;
    }

    case OBJ_COMMIT:
        /* TODO: deep copy commit fields */
        break;

    case OBJ_TREE:
        /* TODO */
        break;

    case OBJ_TAG:
        /* TODO */
        break;

    default:
        break;
    }

    return dst;

fail:
    object_free(dst);
    return NULL;
}

static int validate_blob(const struct blob_object *blob)
{
    return blob != NULL && blob->data != NULL;
}

static int validate_tree(const struct tree_object *tree)
{
    if (tree == NULL || tree->entries == NULL || tree->entry_count == 0)
        return 0;

    for (size_t i = 0; i < tree->entry_count; i++) {
        if (tree->entries[i].name == NULL)
            return 0;
    }

    return 1;
}

static int validate_commit(const struct commit_object *commit)
{
    if (commit == NULL || commit->author == NULL || commit->message == NULL)
        return 0;

    /* Parents array must match parent_count */
    if ((commit->parent_count > 0 && commit->parents == NULL) ||
        (commit->parent_count == 0 && commit->parents != NULL))
        return 0;

    return 1;
}

static int validate_tag(const struct tag_object *tag)
{
    return tag != NULL &&
           tag->tag_name != NULL &&
           tag->tagger != NULL &&
           tag->message != NULL;
}

int confirm_object_is_valid(const struct object *obj)
{
    if (obj == NULL)
        return 0;

    switch (obj->type) {
    case OBJ_BLOB:
        return validate_blob(obj->as.blob);

    case OBJ_TREE:
        return validate_tree(obj->as.tree);

    case OBJ_COMMIT:
        return validate_commit(obj->as.commit);

    case OBJ_TAG:
        return validate_tag(obj->as.tag);

    default:
        return 0;
    }
}
