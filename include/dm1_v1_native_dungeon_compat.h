#ifndef DM1_V1_NATIVE_DUNGEON_COMPAT_H
#define DM1_V1_NATIVE_DUNGEON_COMPAT_H

#include "asset_find_by_hash.h"
#include <string.h>

/* Original LOADSAVE.C F0434:1859-1937 reads native Motorola-order dungeon
 * headers, optionally after the 0x8104 wrapper. F0435:2336-2343 selects the
 * Atari/Amiga world; :2363-2375 selects distinct Towns/PC worlds. These three
 * authenticated little-endian originals cannot be automatic native companions.
 * This identifies known incompatible originals, not arbitrary custom formats. */
static const char* const dm1_v1_pc_towns_dungeon_md5s[] = {
    "766450c940651fc021c92fe5d0d0b3a6",
    "3dc0a932d0e0adfe59878f07c51700c5",
    "fe098f70ce83cfe3f2333565093daf35"
};

static inline int dm1_v1_dungeon_is_pc_towns_original_md5(const char* md5) {
    size_t i;
    if (!md5) return 0;
    for (i=0U; i<sizeof(dm1_v1_pc_towns_dungeon_md5s)/sizeof(dm1_v1_pc_towns_dungeon_md5s[0]); ++i)
        if (strcmp(md5,dm1_v1_pc_towns_dungeon_md5s[i])==0) return 1;
    return 0;
}

static inline int dm1_v1_dungeon_is_pc_towns_original_bytes(
    const unsigned char* bytes, size_t size) {
    size_t i;
    for (i=0U; i<sizeof(dm1_v1_pc_towns_dungeon_md5s)/sizeof(dm1_v1_pc_towns_dungeon_md5s[0]); ++i)
        if (asset_memory_matches_md5(bytes,size,dm1_v1_pc_towns_dungeon_md5s[i])) return 1;
    return 0;
}
#endif
