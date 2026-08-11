#include "csb_v1_x68k_startup_catalog.h"

#include <string.h>

static uint32_t find_size(const uint8_t *hdm, size_t hdm_size,
                          uint16_t count, const char *name) {
    uint16_t i;
    for (i = 0u; i < count; ++i) {
        CSB_V1_X68kHdmRootEntry entry;
        if (!csb_v1_x68k_hdm_root_entry(hdm, hdm_size, i, &entry)) return 0u;
        if (strcmp(entry.name, name) == 0) return entry.byte_count;
    }
    return 0u;
}

int csb_v1_x68k_hdm_startup_catalog(const uint8_t *hdm, size_t hdm_size,
                                    CSB_V1_X68kStartupCatalog *out_catalog) {
    CSB_V1_X68kStartupCatalog catalog;
    if (!out_catalog) return 0;
    memset(out_catalog, 0, sizeof(*out_catalog));
    memset(&catalog, 0, sizeof(catalog));
    if (!csb_v1_x68k_hdm_probe(hdm, hdm_size, &catalog.media)) return 0;
    catalog.root_file_count = catalog.media.root_file_count;
    catalog.program_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "CHAOS_ST.X");
    catalog.graphics_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "GRAPHICS.DAT");
    catalog.dungeon_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "DUNGEON.DAT");
    catalog.title_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "TITL.DAT");
    catalog.animation_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "ANIM.DAT");
    catalog.entrance_music_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "ENTER.SNG");
    catalog.animation_script_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "ANIM.FTL");
    catalog.mini_dungeon_bytes = find_size(hdm, hdm_size, catalog.root_file_count, "MINI.DAT");
    catalog.core_startup_files_present = catalog.program_bytes && catalog.graphics_bytes &&
        catalog.dungeon_bytes && catalog.title_bytes && catalog.animation_bytes &&
        catalog.entrance_music_bytes && catalog.animation_script_bytes &&
        catalog.mini_dungeon_bytes;
    catalog.x68000_identity_bound = catalog.core_startup_files_present;
    catalog.native_runtime_launch_permitted = 0;
    *out_catalog = catalog;
    return catalog.core_startup_files_present;
}
