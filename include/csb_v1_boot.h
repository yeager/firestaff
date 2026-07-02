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
    int cmp_import_attempted;
    int cmp_import_succeeded;
    int cmp_imported_slot;
    int cmp_imported_champion_count;
    int engine_version_displayed;
    CSB_V1_PartyState imported_party;

    CSB_V1_RuntimeProfile runtime;
} CSB_V1_BootProfile;

void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile);
int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir);
int csb_v1_boot_probe_available(const char *data_dir);
void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir);
int csb_v1_boot_set_imported_party(CSB_V1_BootProfile *profile,
                                   const CSB_V1_PartyState *party);
int csb_v1_boot_set_imported_party_from_cmp(CSB_V1_BootProfile *profile,
                                            const uint8_t *cmp_buf,
                                            size_t cmp_size);
int csb_v1_boot_mark_imported_party_ready(CSB_V1_BootProfile *profile);
void csb_v1_boot_reset_engine_version_to_dm1(void);

/* ── Launch→runtime assumption gate ─────────────────────────────────────
 *
 * csb_v1_boot_assume_no_dm1_runtime() rejects boot profiles that look
 * like they were constructed from DM1 defaults.  It is the explicit
 * launch-to-runtime boundary gate: every CSB handoff must clear these
 * assertions before csb_v1_boot_enter_game() rebuilds the runtime.
 *
 * What it rejects (return -1):
 *   - profile->game_id != "csb"                  (defensive: defends against
 *                                                 misrouted DM1/DM2 profiles)
 *   - profile->variant_id outside CSB_V1_VARIANT_* range
 *                                               (catches raw enum leakage)
 *   - profile->default_party_x/y == DM1 HoC     (catches (11,29) leakage)
 *   - profile->tick_ms != CSB_V1_TICK_MS_NOMINAL (catches non-CSB tick quantum)
 *   - profile->entrance_map_index != 255U       (C255_MAP_INDEX_ENTRANCE only)
 *   - profile->start_map_index != 0U            (LOADSAVE.C F0435 new-game map)
 *
 * What it does NOT reject (returns 0):
 *   - graphics_path/dungeon_path fields       (verified in csb_v1_boot_enter_game)
 *   - assets_verified bit                    (verified in csb_v1_boot_enter_game)
 *   - DM1 dungeon hash on the wire           (verified in csb_v1_boot_scan_assets)
 *
 * The scan + enter_game path calls this gate automatically so callers
 * don't have to thread it manually.  The probe + tests call it directly
 * to surface which assertion a DM1-shape profile trips.
 *
 * Returns 0 if the profile is a clean CSB shape, -1 otherwise.  The
 * failure reason is reported through csb_v1_boot_last_assumption_reason()
 * so a regression can pinpoint which CSB-only invariant leaked.
 *
 * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance/C28_ENTRANCE_CSB)
 * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 (new-game map 0)
 * Source: ReDMCSB BASE.C line 36-39 (G0298_B_NewGame mode storage)
 * Source: csb_v1_runtime_pc34_compat.h CSB_V1_TICK_MS_NOMINAL (55ms) */
int csb_v1_boot_assume_no_dm1_runtime(const CSB_V1_BootProfile *profile);

/* Returns a short human-readable reason string for the last failed
 * csb_v1_boot_assume_no_dm1_runtime() call.  The pointer is owned by
 * the boot module and remains valid for the lifetime of the process
 * (it is overwritten by every call).  Useful for diagnostics and CI
 * logs that need to attribute the failure to a specific CSB invariant. */
const char *csb_v1_boot_last_assumption_reason(void);

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
