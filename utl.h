#ifndef UTL_H
#define UTL_H


/* Joins two path components.
 * Returns malloc-owned string or NULL on OOM.
 */
char *utl_path_join(const char *base, const char *name);


/* Print error message to stderr */
void utl_error(const char *fmt, ...);

/* Print error + strerror(errno) */
void utl_perror(const char *fmt, ...);

/* Fatal error: prints message and exits */
void utl_fatal(const char *fmt, ...);

/* Fatal error + strerror(errno) */
void utl_pfatal(const char *fmt, ...);

#endif
