#ifndef FIRESTAFF_ZIP_EXTRACT_H
#define FIRESTAFF_ZIP_EXTRACT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*FirestaffZipExtractProgressFn)(uint64_t completedBytes,
                                              uint64_t totalBytes,
                                              void* userData);

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

/* Extract one member to a temporary memory mapping. This is for large
 * original disc images whose bytes are consumed in place and then released;
 * it does not write game data to disk. Release the returned buffer with
 * firestaff_zip_free_mapped(). */
int firestaff_zip_extract_by_name_mapped(const char *zip_path,
                                         const char *filename,
                                         uint8_t **out_data,
                                         size_t *out_size);
/* Mapped variant of _with_progress. A callback return of zero stops the
 * extraction and returns -2; the output mapping is released on cancellation. */
int firestaff_zip_extract_by_name_mapped_with_progress(
    const char *zip_path, const char *filename,
    uint8_t **out_data, size_t *out_size,
    FirestaffZipExtractProgressFn progressFn, void *userData);
void firestaff_zip_free_mapped(uint8_t *data, size_t size);

/* Same extraction as firestaff_zip_extract_by_name, with bounded progress
 * callbacks while stored/deflated member bytes are copied. A callback return
 * of zero stops extraction and returns -2; ordinary errors still return -1. */
int firestaff_zip_extract_by_name_with_progress(
    const char *zip_path, const char *filename,
    uint8_t **out_data, size_t *out_size,
    FirestaffZipExtractProgressFn progressFn, void *userData);

/* Same bounded stored/deflate ZIP extraction, but from an already resident
 * byte buffer.  This is for nested game media and never creates a file. */
int firestaff_zip_extract_memory_by_suffix(const uint8_t *zip_data,
                                           size_t zip_size,
                                           const char *suffix,
                                           uint8_t **out_data,
                                           size_t *out_size);

/* Return the first non-directory member name ending in suffix from an
 * already-resident ZIP.  This is intentionally metadata-only: callers use
 * it to retain a nested media identity before reading the selected member. */
int firestaff_zip_find_memory_member_by_suffix(const uint8_t *zip_data,
                                               size_t zip_size,
                                               const char *suffix,
                                               char *out_name,
                                               size_t out_name_size);
/* Stream a suffix-matched entry directly to an ordinary file.  This is for
 * original disc images that are too large to materialize in one allocation. */
int firestaff_zip_extract_by_suffix_to_path(const char *zip_path,
                                            const char *suffix,
                                            const char *out_path);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ZIP_EXTRACT_H */
