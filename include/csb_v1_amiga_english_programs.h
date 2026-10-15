#ifndef CSB_V1_AMIGA_ENGLISH_PROGRAMS_H
#define CSB_V1_AMIGA_ENGLISH_PROGRAMS_H

#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"
#include <string.h>

/* ReDMCSB COMPILE.H:199-213 and :274-282 assign English A31/A35
 * APPB.FTL to C03_GAME and BJELoad_R to C02_LAUNCHER. Both original programs
 * are required to enter that edition; SWSH/action-name rendering is not an
 * additional admission requirement. Resolve only beside selected graphics. */
static inline int csb_v1_amiga_english_programs_ready(
    const char *graphics_path, const char *version_id)
{
    static const char *const names[] = { "APPB.FTL", "BJELoad_R" };
    const char *hashes[2];
    const char *member = NULL, *cursor, *slash;
    char parent[FSP_PATH_MAX], path[FSP_PATH_MAX];
    size_t prefix = 0u, i;
    if (!graphics_path || !version_id) return 0;
    if (strcmp(version_id, "amiga31-en") == 0) {
        hashes[0] = "af50ff33c61c22e20784d74266d81d1e";
        hashes[1] = "2788ef63f246b4d3c74c64dff9dcdbde";
    } else if (strcmp(version_id, "amiga35-en") == 0) {
        hashes[0] = "11d8d059cd8f241d6f68ec09c5c8b66d";
        hashes[1] = "6aec320606f014a149548e664719cf5b";
    } else return 0;
    for (cursor = graphics_path; (cursor = strstr(cursor, "::")) != NULL;
         cursor += 2) member = cursor + 2;
    if (member) {
        slash = strrchr(member, '/');
        prefix = (size_t)((slash ? slash + 1 : member) - graphics_path);
    } else if (!FSP_ParentDir(parent, sizeof(parent), graphics_path)) return 0;
    for (i = 0u; i < 2u; ++i) {
        if (member) {
            if (prefix + strlen(names[i]) >= sizeof(path)) return 0;
            memcpy(path, graphics_path, prefix);
            strcpy(path + prefix, names[i]);
        } else if (FSP_FindUniqueFileIgnoreCase(parent, names[i], path,
                                                  sizeof(path)) != 1) return 0;
        if (!asset_file_matches_md5(path, hashes[i])) return 0;
    }
    return 1;
}

#endif
