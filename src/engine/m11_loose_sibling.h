#ifndef M11_LOOSE_SIBLING_H
#define M11_LOOSE_SIBLING_H

#include "fs_portable_compat.h"

/* Keep engine call sites on the shared exact-directory ownership policy. */
static inline int m11_find_unique_loose_sibling(
    const char* parent, const char* wanted, char* out, size_t outBytes) {
    return FSP_FindUniqueFileIgnoreCase(parent, wanted, out, outBytes);
}

#endif
