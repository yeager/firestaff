#ifndef FIRESTAFF_CSB_V1_X68K_STARTUP_CATALOG_H
#define FIRESTAFF_CSB_V1_X68K_STARTUP_CATALOG_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_x68k_hdm.h"

/* ReDMCSB's X68000 branch opens these root files directly (MEMORY.C,
 * ENTRANCE.C and platform-specific startup code). This is an inventory of
 * source media, not an emulation of CHAOS_ST.X or a permission to boot it. */
typedef struct {
    CSB_V1_X68kHdmReceipt media;
    uint16_t root_file_count;
    uint32_t program_bytes;
    uint32_t graphics_bytes;
    uint32_t dungeon_bytes;
    uint32_t title_bytes;
    uint32_t animation_bytes;
    uint32_t entrance_music_bytes;
    uint32_t animation_script_bytes;
    uint32_t mini_dungeon_bytes;
    int core_startup_files_present;
    int x68000_identity_bound;
    int native_runtime_launch_permitted;
} CSB_V1_X68kStartupCatalog;

/* Inventory required root files and their exact media sizes. The caller never
 * receives original payload bytes. A cracked helper (CK.R) is not treated as
 * canonical proof and cannot enable native launch. */
int csb_v1_x68k_hdm_startup_catalog(const uint8_t *hdm, size_t hdm_size,
                                    CSB_V1_X68kStartupCatalog *out_catalog);

#endif
