#ifndef DM2_V1_MEDIA_PATH_H
#define DM2_V1_MEDIA_PATH_H

#include <stddef.h>
#include <string.h>

/* Match the selected transport suffix only. Directory names containing
 * ".zip" do not make loose media an archive; archive spelling is not an
 * identity gate and follows the scanner's case-insensitive suffix policy. */
static inline int dm2_v1_media_path_is_zip(const char *path)
{
    size_t size;
    if (!path || (size = strlen(path)) < 4u) return 0;
    path += size - 4u;
    return path[0] == '.' &&
        (path[1] == 'z' || path[1] == 'Z') &&
        (path[2] == 'i' || path[2] == 'I') &&
        (path[3] == 'p' || path[3] == 'P');
}

#endif
