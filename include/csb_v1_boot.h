#ifndef FIRESTAFF_CSB_V1_BOOT_H
#define FIRESTAFF_CSB_V1_BOOT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_BOOT_GAME_ID "csb"
#define CSB_V1_BOOT_SAVE_SUBDIR "saves/csb"

typedef enum {
    CSB_V1_BOOT_STATE_EMPTY = 0,
    CSB_V1_BOOT_STATE_PROFILE_READY,
    CSB_V1_BOOT_STATE_ASSETS_READY,
    CSB_V1_BOOT_STATE_RUNTIME_READY
} CSB_V1_BootState;

typedef struct {
    char game_id[8];
    CSB_V1_BootState state;
    CSB_V1_VariantId variant_id;
    char version_id[32];
    char variant_label[64];
    char media_ref[64];

    char asset_root[512];
    char graphics_path[512];
    char dungeon_path[512];
    char save_root[512];
    char graphics_md5[33];
    char dungeon_md5[33];

    int assets_verified;
    int graphics_verified;
    int dungeon_verified;
    CSB_V1_AssetGfxArchiveType graphics_kind;

    uint32_t tick_ms;
    uint32_t entrance_map_index;
    uint32_t start_map_index;
    uint32_t default_party_x;
    uint32_t default_party_y;
    uint32_t default_party_dir;

    CSB_V1_RuntimeProfile runtime;
} CSB_V1_BootProfile;

void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile);
int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir);
int csb_v1_boot_probe_available(const char *data_dir);
void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir);
int csb_v1_boot_enter_game(CSB_V1_BootProfile *profile);
void csb_v1_boot_cleanup(CSB_V1_BootProfile *profile);
size_t csb_v1_boot_diagnostic_report(const CSB_V1_BootProfile *profile,
                                     char *buf,
                                     size_t buf_size);
void csb_v1_boot_print_summary(const CSB_V1_BootProfile *profile);
const char *csb_v1_boot_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_BOOT_H */
