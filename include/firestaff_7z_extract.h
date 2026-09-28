#ifndef FIRESTAFF_7Z_EXTRACT_H
#define FIRESTAFF_7Z_EXTRACT_H

#include <stddef.h>
#include <stdint.h>

/* Strict in-memory reader for the ordinary one-file, one-folder LZMA2 7z
 * transport used by supplied CSB preservation media. It deliberately rejects
 * encoded headers, encryption, solid/multi-file archives and every method
 * other than LZMA2. The caller owns *out_bytes on success. */
int firestaff_7z_extract_single_lzma2_file(const char *path,
                                           uint8_t **out_bytes,
                                           size_t *out_size,
                                           char *out_name,
                                           size_t out_name_size);

/* Visits regular files in an ordinary 7z archive using the bundled 7-Zip
 * SDK's in-memory reader. The visitor's byte pointer is borrowed and remains
 * valid only until the next visitor call or function return. Return 0 to
 * continue, >0 to stop successfully, or <0 to abort with failure. Archive,
 * solid-folder output and SDK allocations are bounded by the implementation. */
typedef int (*Firestaff7zFileVisitor)(const char *name,
                                      const uint8_t *bytes,
                                      size_t byte_count,
                                      void *user_data);

int firestaff_7z_visit_files(const char *path,
                             Firestaff7zFileVisitor visitor,
                             void *user_data);

/* Copies one regular member into caller-owned memory. Member matching is
 * ASCII case-insensitive; callers must use virtual archive paths and must not
 * materialize this data on disk. */
int firestaff_7z_read_member(const char *path,
                             const char *member_name,
                             uint8_t **out_bytes,
                             size_t *out_size);

#endif
