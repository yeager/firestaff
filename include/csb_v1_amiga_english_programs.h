#ifndef CSB_V1_AMIGA_ENGLISH_PROGRAMS_H
#define CSB_V1_AMIGA_ENGLISH_PROGRAMS_H

#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"
#include <string.h>

/* Preserve the final disk/member directory; never search another package. */
static inline int csb_v1_amiga_selected_companion(const char *graphics_path,
    const char *name, char *out, size_t out_size)
{
    const char *member = NULL, *cursor, *slash;
    char parent[FSP_PATH_MAX];
    size_t prefix;
    if (!graphics_path || !name || !out || !out_size) return 0;
    for (cursor = graphics_path; (cursor = strstr(cursor, "::")) != NULL;
         cursor += 2) member = cursor + 2;
    if (!member) return FSP_ParentDir(parent, sizeof(parent), graphics_path) &&
        FSP_FindUniqueFileIgnoreCase(parent, name, out, out_size) == 1;
    slash = strrchr(member, '/');
    prefix = (size_t)((slash ? slash + 1 : member) - graphics_path);
    if (prefix + strlen(name) >= out_size) return 0;
    memcpy(out, graphics_path, prefix);
    strcpy(out + prefix, name);
    return 1;
}

/* ReDMCSB COMPILE.H:207-213 and :274-282 assign English A31/A35
 * APPB.FTL to C03_GAME and BJELoad_R to C02_LAUNCHER. Both original programs
 * are required to enter that edition; SWSH/action-name rendering is not an
 * additional admission requirement. */
static inline int csb_v1_amiga_english_programs_ready(
    const char *graphics_path, const char *version_id)
{
    const char *hashes[2];
    static const char *const names[] = { "APPB.FTL", "BJELoad_R" };
    char path[FSP_PATH_MAX];
    size_t i;
    if (!graphics_path || !version_id) return 0;
    if (strcmp(version_id, "amiga31-en") == 0) {
        hashes[0] = "af50ff33c61c22e20784d74266d81d1e";
        hashes[1] = "2788ef63f246b4d3c74c64dff9dcdbde";
    } else if (strcmp(version_id, "amiga35-en") == 0) {
        hashes[0] = "11d8d059cd8f241d6f68ec09c5c8b66d";
        hashes[1] = "6aec320606f014a149548e664719cf5b";
    } else return 0;
    for (i = 0u; i < 2u; ++i) {
        if (!csb_v1_amiga_selected_companion(graphics_path, names[i], path,
                sizeof(path)) || !asset_file_matches_md5(path, hashes[i])) return 0;
    }
    return 1;
}

/* LOADSAVE.C F0435:1940-1944 loads this edition's new-game dungeon. */
static inline int csb_v1_amiga_english_package_ready(
    const char *graphics_path, const char *version_id)
{
    char dungeon[FSP_PATH_MAX];
    return csb_v1_amiga_english_programs_ready(graphics_path, version_id) &&
        csb_v1_amiga_selected_companion(graphics_path, "DUNGEON.DAT", dungeon,
                                       sizeof(dungeon)) &&
        asset_file_matches_md5(dungeon, "6695d2acebce49f95db1d8f3a5c733de");
}

#endif
