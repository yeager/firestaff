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
    int imported_party_ready;
    CSB_V1_PartyState imported_party;

    CSB_V1_RuntimeProfile runtime;
} CSB_V1_BootProfile;

void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile);
int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir);
int csb_v1_boot_probe_available(const char *data_dir);
void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir);
int csb_v1_boot_set_imported_party(CSB_V1_BootProfile *profile,
                                   const CSB_V1_PartyState *party);
int csb_v1_boot_enter_game(CSB_V1_BootProfile *profile);
void csb_v1_boot_cleanup(CSB_V1_BootProfile *profile);
size_t csb_v1_boot_diagnostic_report(const CSB_V1_BootProfile *profile,
                                     char *buf,
                                     size_t buf_size);
void csb_v1_boot_print_summary(const CSB_V1_BootProfile *profile);
const char *csb_v1_boot_source_evidence(void);

/* ----------------------------------------------------------------
 * CSB V1 boot profile -> M11 entry guard.
 *
 * Deterministic go/no-go check used by both the M12 launch flow
 * and the M11_GameView_Start CSB branch before the launcher hands
 * a verified CSB boot profile off to the FS_GAME_CSB runtime.
 *
 * Two entry shapes are supported:
 *
 *   1) `csb_v1_boot_profile_m11_entry_gate(profile, reason, reason_size)`
 *      validates an already-scanned CSB_V1_BootProfile.
 *      Pass = (state >= CSB_V1_BOOT_STATE_ASSETS_READY) AND
 *             assets_verified AND graphics_verified AND dungeon_verified
 *             AND the matched MD5s come from the canonical CSB V1
 *             graphics + dungeon hash registry (PC 3.4 EN, Atari ST
 *             2.x, Amiga 3.x EN, Amiga 3.x ML).
 *
 *   2) `csb_v1_boot_graphics_dungeon_m11_entry_gate(graphics_md5,
 *      dungeon_md5, reason, reason_size)` validates a raw pair of
 *      matched MD5 hex strings (the same pair the launcher pushes
 *      into the M11_GameLaunchSpec via verifiedAssetPath/Md5 plus
 *      the dungeon MD5 it records after asset scan). Useful when
 *      the caller does not have a boot profile in scope.
 *
 * Both forms return 1 on pass and 0 on fail.  When 0 is returned,
 * `reason` (when non-NULL and `reason_size > 0`) is filled with a
 * short human-readable explanation suitable for an M12 launch
 * dialog or stderr log; it is always NUL-terminated.
 *
 * Source-lock boundary:
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance setup;
 *     the canonical CSB V1 entry path is gated by media-class
 *     detection, not by filesystem layout, so the gate hashes
 *     mirror the documented CSB V1 media hash registry).
 *   ReDMCSB LOADSAVE.C F0435 lines 1936-1944 (new-game dungeon
 *     load; the gate blocks launches without a verified dungeon
 *     because F0435 will not reach C001_MODE_LOAD_DUNGEON without
 *     a header it can read).
 *
 * The gate is intentionally narrow: it does NOT touch the live
 * dungeon handle, does NOT mutate the boot profile, and does NOT
 * load or decode asset bytes. It only validates the matched-MD5
 * evidence the launcher already collected via asset_find_by_md5().
 */
int csb_v1_boot_profile_m11_entry_gate(const CSB_V1_BootProfile *profile,
                                       char *reason,
                                       size_t reason_size);
int csb_v1_boot_graphics_dungeon_m11_entry_gate(const char *graphics_md5,
                                                const char *dungeon_md5,
                                                char *reason,
                                                size_t reason_size);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_BOOT_H */
