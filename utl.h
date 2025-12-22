#ifndef UTL_H
#define UTL_H


/* Joins two path components.
 * Returns malloc-owned string or NULL on OOM.
 */
char *utl_path_join(const char *base, const char *name);



#endif
