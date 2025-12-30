#ifndef UTL_H
#define UTL_H


/* Joins two path components. [base] and [name] are assumed to be
 * valid path components, and [no_slash] indicate there is no slash between them.
 * no_slash is non-zero if [base] does not end with a slash. 
 */
char *utl_path_join(const char *base, const char *name, int no_slash);



/*  Checks if the given path is a directory 
* Returns 1 if path is a directory, 0 otherwise
*/
int is_directory(const char *path);

#endif
