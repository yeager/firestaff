#ifndef FIRESTAFF_ZIP_EXTRACT_H
#define FIRESTAFF_ZIP_EXTRACT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Extract a single entry from a ZIP archive by filename suffix match.
 * Scans the central directory for the first entry whose name ends with
 * the given suffix (case-insensitive).  Handles both stored (method 0)
 * and deflate (method 8) entries.
 *
 * On success, *out_data is malloc'd and *out_size is set.
 * Caller must free(*out_data).
 * Returns 0 on success, -1 on failure (no match, read error, inflate
 * error, or allocation failure). */
int firestaff_zip_extract_by_suffix(const char *zip_path,
                                    const char *suffix,
                                    uint8_t **out_data,
                                    size_t *out_size);

/* Extract a single entry from a ZIP archive by exact filename match
 * (case-insensitive, matches the basename after any directory components).
 * Same semantics as _by_suffix. */
int firestaff_zip_extract_by_name(const char *zip_path,
                                  const char *filename,
                                  uint8_t **out_data,
                                  size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ZIP_EXTRACT_H */
