/**
 * @file
 * Path-related function prototypes.
 */

#ifndef PATH_H
#define PATH_H

/**
 * Combine two paths and return the combined path. Returns path.
 */
char *path_combine(const char *src, const char *dst, char *path, size_t size);

/**
 * Normalize a path; the passed in path is modified in-place.
 */
void path_normalize(char *path);

/**
 * Combine two paths and normalize the result. The result is a static buffer;
 * it is valid until the next call to either this function or to
 * combine_path().
 */
char *path_combine_and_normalize(const char *src, const char *dst, char *path, size_t size);

#endif /* PATH_H */
