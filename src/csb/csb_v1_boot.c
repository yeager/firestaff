#include "csb_v1_boot.h"

#include "asset_find_by_hash.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ReDMCSB source-lock for this boot/profile boundary:
 * ENTRANCE.C F0806 lines 409-441 builds the entrance micro-dungeon and
 * selects C28_ENTRANCE_CSB for CSB media.
 * ENTRANCE.C F0806 lines 857-883 waits on the entrance state machine and
 * switches G0298_B_NewGame to C001_MODE_LOAD_DUNGEON.
 * LOADSAVE.C F0435 lines 1940-1944 loads the initial party location from
 * DUNGEON.DAT and sets G0309_i_PartyMapIndex to map 0 for new games.
 */

static const char *const g_csb_boot_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611",
    "ebf6a57af3f27782e358c0490bfd2f2e",
    "291e1bc6803e3dc4b974c60117ca5d68",
    "cefaddfdf5651df2c91f61b5611a8362",
    NULL
};

static const CSB_V1_VariantId g_csb_boot_graphics_variants[] = {
    CSB_V1_VARIANT_PC34_EN,
    CSB_V1_VARIANT_ST21_EN,
    CSB_V1_VARIANT_AMIGA35_EN,
    CSB_V1_VARIANT_AMIGA35_MULTI
};

static const char *const g_csb_boot_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",
    NULL
};

static void csb_v1_boot_copy(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0U) return;
    if (!src) src = "";
    snprintf(dst, dst_size, "%s", src);
}

static CSB_V1_AssetGfxArchiveType csb_v1_boot_graphics_kind(const char *path)
{
    const char *name;
    if (!path) return CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    name = strrchr(path, '/');
#if defined(_WIN32)
    {
        const char *slash = strrchr(path, '\\');
        if (slash && (!name || slash > name)) name = slash;
    }
#endif
    name = name ? name + 1 : path;
    if (strcmp(name, "CSB.DAT") == 0 || strcmp(name, "csb.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSB;
    }
    if (strcmp(name, "CSBGRAPH.DAT") == 0 || strcmp(name, "csbgraph.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    }
    return CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
}

void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));
    csb_v1_boot_copy(profile->game_id, sizeof(profile->game_id), CSB_V1_BOOT_GAME_ID);
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id), "unknown");
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label), "Unknown");
    profile->tick_ms = CSB_V1_TICK_MS_NOMINAL;
    profile->entrance_map_index = 255U;
    profile->start_map_index = 0U;
    profile->default_party_x = CSB_V1_START_PARTY_X;
    profile->default_party_y = CSB_V1_START_PARTY_Y;
    profile->default_party_dir = CSB_V1_START_PARTY_DIR;
    profile->imported_party_ready = 0;
    csb_v1_character_init_default(&profile->imported_party);
    csb_v1_runtime_init(&profile->runtime, NULL);
}

int csb_v1_boot_set_imported_party(CSB_V1_BootProfile *profile,
                                   const CSB_V1_PartyState *party)
{
    if (!profile || !party) return -1;
    if (party->ChampionCount <= 0 ||
        party->ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    profile->imported_party = *party;
    profile->imported_party_ready = 1;
    return 0;
}

void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir)
{
    if (!profile) return;
    if (save_dir && save_dir[0] != '\0') {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root), save_dir);
    } else if (profile->asset_root[0] != '\0') {
        snprintf(profile->save_root, sizeof(profile->save_root),
                 "%s/../%s", profile->asset_root, CSB_V1_BOOT_SAVE_SUBDIR);
    } else {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root),
                         csb_v1_runtime_save_dir());
    }
}

int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir)
{
    char graphics_path[ASSET_PATH_MAX];
    char dungeon_path[ASSET_PATH_MAX];
    int graphics_match = -1;
    int dungeon_match = -1;
    const CSB_V1_VariantInfo *variant;
    const char *root;

    if (!profile) return -1;
    root = (data_dir && data_dir[0] != '\0') ? data_dir : ".";
    csb_v1_boot_copy(profile->asset_root, sizeof(profile->asset_root), root);
    /* A reused launcher profile must not carry stale CSB paths across scans.
     * ReDMCSB only enters the CSB load path after the current media probe has
     * selected CSB and found a dungeon to load.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    profile->assets_verified = 0;
    profile->graphics_verified = 0;
    profile->dungeon_verified = 0;
    profile->graphics_path[0] = '\0';
    profile->dungeon_path[0] = '\0';
    profile->graphics_md5[0] = '\0';
    profile->dungeon_md5[0] = '\0';
    profile->graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    profile->variant_id = CSB_V1_VARIANT_UNKNOWN;

    /* A successful csb_v1_boot_enter_game() hands the verified DUNGEON.DAT
     * off to the runtime as profile->runtime.dungeon_handle and to the global
     * singleton via csb_v1_dungeon_set_current().  A follow-up rescan
     * (different data_dir, removed asset, launcher refresh) must not leave
     * that handoff alive: the runtime-owned handle would still point at the
     * previous heap allocation, the global singleton would still expose the
     * previous dungeon through csb_v1_dungeon_get_current(), and the next
     * enter_game() would either fail to replace the handle (when verification
     * fails) or silently keep serving the previous dungeon through the new
     * profile paths.  Release the handle and reset the singleton here, before
     * the rescan-driven profile fields are populated.  The full runtime
     * re-init still happens in csb_v1_boot_enter_game() on the next launch.
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    if (profile->runtime.dungeon_handle != NULL ||
        csb_v1_dungeon_get_current() != NULL) {
        csb_v1_dungeon_unload();
        free(profile->runtime.dungeon_handle);
        profile->runtime.dungeon_handle = NULL;
    }

    profile->graphics_verified =
        asset_find_by_md5_list(root, g_csb_boot_graphics_hashes,
                               graphics_path, sizeof(graphics_path),
                               &graphics_match, 4);
    profile->dungeon_verified =
        asset_find_by_md5_list(root, g_csb_boot_dungeon_hashes,
                               dungeon_path, sizeof(dungeon_path),
                               &dungeon_match, 4);
    if (profile->graphics_verified) {
        csb_v1_boot_copy(profile->graphics_path, sizeof(profile->graphics_path),
                         graphics_path);
        csb_v1_boot_copy(profile->graphics_md5, sizeof(profile->graphics_md5),
                         g_csb_boot_graphics_hashes[graphics_match]);
        profile->graphics_kind = csb_v1_boot_graphics_kind(graphics_path);
        profile->variant_id = g_csb_boot_graphics_variants[graphics_match];
    }
    if (profile->dungeon_verified) {
        csb_v1_boot_copy(profile->dungeon_path, sizeof(profile->dungeon_path),
                         dungeon_path);
        csb_v1_boot_copy(profile->dungeon_md5, sizeof(profile->dungeon_md5),
                         g_csb_boot_dungeon_hashes[dungeon_match]);
    }

    profile->assets_verified = profile->graphics_verified && profile->dungeon_verified;
    if (profile->variant_id == CSB_V1_VARIANT_UNKNOWN && profile->dungeon_verified) {
        profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    }
    variant = csb_v1_runtime_get_variant_info(profile->variant_id);
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label),
                     variant->name);
    csb_v1_boot_copy(profile->media_ref, sizeof(profile->media_ref),
                     variant->media_ref);
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id),
                     profile->graphics_md5[0] ? profile->graphics_md5 : "unknown");
    if (profile->save_root[0] == '\0') {
        csb_v1_boot_set_save_root(profile, NULL);
    }
    if (profile->assets_verified) {
        profile->state = CSB_V1_BOOT_STATE_ASSETS_READY;
        return 0;
    }
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    return -1;
}

int csb_v1_boot_probe_available(const char *data_dir)
{
    CSB_V1_BootProfile profile;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, data_dir) == 0 ? 1 : 0;
}

int csb_v1_boot_enter_game(CSB_V1_BootProfile *profile)
{
    if (!profile || !profile->assets_verified) return -1;
    /* The launcher may carry several game profiles at once.  Do not let an
     * aggregate READY bit alone hand a non-CSB or partial profile to the CSB
     * runtime: ReDMCSB enters the CSB dungeon only after the CSB entrance/media
     * path has selected C28_ENTRANCE_CSB and the load path has a dungeon header
     * to consume.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0 ||
        !profile->graphics_verified ||
        !profile->dungeon_verified ||
        profile->graphics_path[0] == '\0' ||
        profile->dungeon_path[0] == '\0') {
        return -1;
    }
    /* Re-entering the CSB profile replaces the live dungeon context just as
     * ReDMCSB's global dungeon/map state is replaced when a new game is
     * loaded.  Clear the previous heap-owned runtime before csb_v1_runtime_init
     * overwrites its handle fields.
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 */
    csb_v1_runtime_cleanup(&profile->runtime);
    csb_v1_runtime_init(&profile->runtime, profile->asset_root);
    profile->runtime.variant_id = profile->variant_id;
    profile->runtime.difficulty = CSB_V1_DIFFICULTY_HARD;
    profile->runtime.save_dir = profile->save_root;
    profile->runtime.dungeon_path = profile->dungeon_path;
    profile->runtime.graphics_path = profile->graphics_path;
    profile->runtime.dungeon_asset.path = profile->dungeon_path;
    profile->runtime.dungeon_asset.kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    profile->runtime.graphics_asset.path = profile->graphics_path;
    profile->runtime.graphics_asset.kind = profile->graphics_kind;
    /* Copy entrance/start map indices from the boot profile so the runtime
     * honours the source-locked new-game map selection.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (C255_MAP_INDEX_ENTRANCE)
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 (new-game map 0) */
    profile->runtime.entrance_map_index = profile->entrance_map_index;
    profile->runtime.start_map_index = profile->start_map_index;
    profile->runtime.state = CSB_STATE_TITLE;
    profile->runtime.chaos_magic.magic_initialized = 1;
    profile->runtime.chaos_magic.spell_grid_version = 0U;
    profile->runtime.chaos_magic.chaos_level = 0U;
    if (profile->imported_party_ready) {
        (void)csb_v1_runtime_set_party_state(&profile->runtime,
                                             &profile->imported_party);
    }
    /* Load the verified DUNGEON.DAT into the runtime so that the
     * dungeon-layer accessors (csb_v1_dungeon_get_current_level,
     * csb_v1_dungeon_get_square_type, ...) become live immediately
     * after launch — without a second hash search or a follow-up
     * csb_v1_runtime_boot() call from the game-view.
     *
     * The dungeon is heap-allocated and owned by the runtime profile
     * (dungeon_handle).  csb_v1_runtime_cleanup() / csb_v1_boot_cleanup()
     * are responsible for releasing it.
     *
     * Failure is non-fatal: if the verified path cannot be opened
     * (e.g. archive-backed path not yet materialized by M12), the
     * runtime continues with dungeon_handle == NULL and the dungeon
     * accessors return ENDOF — matching csb_v1_runtime_boot()'s
     * pre-existing tolerant behaviour.
     *
     * Source: CSBWin/CSBCode.cpp:6800-6950 LoadDungeon
     * Source: ReDMCSB DUNGEON.C F0237 dungeon load entry
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 entrance micro-dungeon */
    {
        CSB_V1_DungeonData *dungeon = (CSB_V1_DungeonData *)calloc(1, sizeof(CSB_V1_DungeonData));
        if (dungeon) {
            if (csb_v1_dungeon_load_from_file(dungeon, profile->dungeon_path) == 0) {
                profile->runtime.dungeon_handle = dungeon;
                csb_v1_dungeon_set_current(dungeon);
                csb_v1_dungeon_set_current_level(0);
            } else {
                free(dungeon);
                profile->runtime.dungeon_handle = NULL;
            }
        }
    }
    profile->state = CSB_V1_BOOT_STATE_RUNTIME_READY;
    return 0;
}

void csb_v1_boot_cleanup(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    /* The boot profile owns the runtime handoff dungeon.  Release it through
     * the same runtime cleanup path used by csb_v1_runtime_boot() so the
     * singleton dungeon/map accessors do not retain a stale CSB context after
     * leaving the profile.
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 */
    csb_v1_runtime_cleanup(&profile->runtime);
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    memset(&profile->runtime, 0, sizeof(profile->runtime));
}

size_t csb_v1_boot_diagnostic_report(const CSB_V1_BootProfile *profile,
                                     char *buf,
                                     size_t buf_size)
{
    int n;
    if (!profile || !buf || buf_size == 0U) return 0U;
    n = snprintf(buf, buf_size,
                 "=== CSB V1 Boot Profile ===\n"
                 "state=%d verified=%s variant=%s media=%s\n"
                 "asset_root=%s\n"
                 "graphics=%s md5=%s\n"
                 "dungeon=%s md5=%s\n"
                 "save_root=%s tick_ms=%u entrance_map=%u start_map=%u\n",
                 (int)profile->state,
                 profile->assets_verified ? "YES" : "NO",
                 profile->variant_label,
                 profile->media_ref,
                 profile->asset_root[0] ? profile->asset_root : "(unset)",
                 profile->graphics_path[0] ? profile->graphics_path : "(missing)",
                 profile->graphics_md5[0] ? profile->graphics_md5 : "(missing)",
                 profile->dungeon_path[0] ? profile->dungeon_path : "(missing)",
                 profile->dungeon_md5[0] ? profile->dungeon_md5 : "(missing)",
                 profile->save_root[0] ? profile->save_root : "(unset)",
                 (unsigned)profile->tick_ms,
                 (unsigned)profile->entrance_map_index,
                 (unsigned)profile->start_map_index);
    if (n < 0) return 0U;
    return (size_t)n < buf_size ? (size_t)n : buf_size - 1U;
}

void csb_v1_boot_print_summary(const CSB_V1_BootProfile *profile)
{
    if (!profile) {
        printf("CSB: no boot profile\n");
        return;
    }
    printf("CSB: %s assets=%s dungeon=%s graphics=%s\n",
           profile->variant_label,
           profile->assets_verified ? "READY" : "missing",
           profile->dungeon_verified ? "ok" : "missing",
           profile->graphics_verified ? "ok" : "missing");
}

/* ── CSB V1 boot profile -> M11 entry guard ─────────────────────
 *
 * The canonical CSB V1 media hash registry below mirrors the four
 * graphics md5s and the one dungeon md5 the launcher scans for.
 * The gate hashes here MUST stay in sync with g_csb_boot_graphics_hashes
 * and g_csb_boot_dungeon_hashes above; the registry is the source of
 * truth that the M11 dispatch consults before activating a CSB launch.
 *
 * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB media class
 *   detection by hash, not by filename/path).
 * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 (new-game dungeon
 *   load is gated on a hash-known dungeon header). */
static const char *const g_csb_m11_entry_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611", /* PC DOS 3.4 English         MEDIA278 */
    "ebf6a57af3f27782e358c0490bfd2f2e", /* Atari ST 2.0/2.1 English   MEDIA332 */
    "291e1bc6803e3dc4b974c60117ca5d68", /* Amiga 3.5 English          MEDIA529 */
    "cefaddfdf5651df2c91f61b5611a8362", /* Amiga 3.5 Multilanguage    MEDIA529 */
    NULL
};

static const char *const g_csb_m11_entry_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de", /* shared CSB V1 DUNGEON.DAT  MEDIA278/332/529 */
    NULL
};

static int csb_v1_md5_is_canonical_graphics(const char *md5)
{
    size_t i;
    if (!md5 || md5[0] == '\0') return 0;
    for (i = 0U; g_csb_m11_entry_graphics_hashes[i] != NULL; ++i) {
        if (strcmp(md5, g_csb_m11_entry_graphics_hashes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int csb_v1_md5_is_canonical_dungeon(const char *md5)
{
    size_t i;
    if (!md5 || md5[0] == '\0') return 0;
    for (i = 0U; g_csb_m11_entry_dungeon_hashes[i] != NULL; ++i) {
        if (strcmp(md5, g_csb_m11_entry_dungeon_hashes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void csb_v1_boot_gate_set_reason(char *reason,
                                        size_t reason_size,
                                        const char *fmt,
                                        ...)
{
    if (!reason || reason_size == 0U) return;
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(reason, reason_size, fmt, args);
        va_end(args);
        reason[reason_size - 1U] = '\0';
    }
}

int csb_v1_boot_graphics_dungeon_m11_entry_gate(const char *graphics_md5,
                                                const char *dungeon_md5,
                                                char *reason,
                                                size_t reason_size)
{
    if (!graphics_md5 || graphics_md5[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: GRAPHICS md5 is empty "
            "(scanner did not record a matched graphics hash)");
        return 0;
    }
    if (!dungeon_md5 || dungeon_md5[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: DUNGEON md5 is empty "
            "(scanner did not record a matched dungeon hash)");
        return 0;
    }
    if (!csb_v1_md5_is_canonical_graphics(graphics_md5)) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: GRAPHICS md5 %s is not in the canonical "
            "CSB V1 media registry (PC3.4EN / Atari ST 2.x / Amiga 3.x)",
            graphics_md5);
        return 0;
    }
    if (!csb_v1_md5_is_canonical_dungeon(dungeon_md5)) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: DUNGEON md5 %s is not in the canonical "
            "CSB V1 media registry",
            dungeon_md5);
        return 0;
    }
    csb_v1_boot_gate_set_reason(reason, reason_size,
        "CSB M11 entry guard: GRAPHICS=%s DUNGEON=%s accepted",
        graphics_md5, dungeon_md5);
    return 1;
}

int csb_v1_boot_profile_m11_entry_gate(const CSB_V1_BootProfile *profile,
                                       char *reason,
                                       size_t reason_size)
{
    if (!profile) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: NULL boot profile "
            "(launcher did not initialize a CSB profile before dispatch)");
        return 0;
    }
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: profile game_id=%s != %s "
            "(foreign game reached the CSB entry path)",
            profile->game_id[0] ? profile->game_id : "(empty)",
            CSB_V1_BOOT_GAME_ID);
        return 0;
    }
    if (profile->state < CSB_V1_BOOT_STATE_ASSETS_READY) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: boot state=%d below ASSETS_READY "
            "(scanner did not record both required assets)",
            (int)profile->state);
        return 0;
    }
    if (!profile->assets_verified ||
        !profile->graphics_verified ||
        !profile->dungeon_verified) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: assets_verified=%d graphics_verified=%d "
            "dungeon_verified=%d (one or more required files missing)",
            profile->assets_verified,
            profile->graphics_verified,
            profile->dungeon_verified);
        return 0;
    }
    if (profile->graphics_path[0] == '\0' ||
        profile->dungeon_path[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: empty asset path "
            "(graphics_path=%s dungeon_path=%s)",
            profile->graphics_path[0] ? profile->graphics_path : "(empty)",
            profile->dungeon_path[0] ? profile->dungeon_path : "(empty)");
        return 0;
    }
    return csb_v1_boot_graphics_dungeon_m11_entry_gate(
        profile->graphics_md5, profile->dungeon_md5, reason, reason_size);
}

const char *csb_v1_boot_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C F0806 lines 409-441: CSB entrance setup and C28_ENTRANCE_CSB palette\n"
        "ReDMCSB ENTRANCE.C F0806 lines 857-883: entrance waits then switches G0298_B_NewGame\n"
        "ReDMCSB LOADSAVE.C F0435 lines 1940-1944: new-game party location and map 0\n"
        "ReDMCSB BASE.C lines 36-39: G0298_B_NewGame boot/load mode storage\n";
}
