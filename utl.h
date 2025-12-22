#ifndef UTL_H
#define UTL_H


/* Joins two path components.
 * Returns malloc-owned string or NULL on OOM.
 */
char *utl_path_join(const char *base, const char *name);



/*  Checks if the given path is a directory 
* Returns 1 if path is a directory, 0 otherwise
*/
int is_directory(const char *path);

#endif
