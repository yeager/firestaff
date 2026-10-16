#ifndef FIRESTAFF_PRODOS_H
#define FIRESTAFF_PRODOS_H

#include <stddef.h>
#include <stdint.h>

/* Read-only native ProDOS logical-block images, without game policy. Paths
 * are volume-relative, slash-separated original ProDOS names. Bytes and path
 * remain valid only during the callback; zero continues, nonzero stops. */
typedef int (*FirestaffProDosFileVisitor)(const char *path,
                                        const uint8_t *bytes,
                                        size_t byte_count, void *user_data);
#ifndef FIRESTAFF_PRODOS_API
#define FIRESTAFF_PRODOS_API
#endif

/* Validate the complete supported directory tree before delivering files.
 * Returns delivered count (including a callback that stops), or -1 on invalid
 * media, unsupported storage, allocation failure, or resource limits.
 * Supports seedling/sapling/tree standard files and subdirectories, including
 * sparse holes. No DOS-sector reordering, container or extended-fork support.
 * Limits: 65535 blocks, 4096 entries, 32 directory levels, 511-byte paths. */
FIRESTAFF_PRODOS_API int firestaff_prodos_visit_files(
    const uint8_t *image, size_t image_size,
    FirestaffProDosFileVisitor visitor, void *user_data);

#endif
