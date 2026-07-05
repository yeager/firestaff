/*
 * csb_v1_runtime_pc34_compat.c — CSB V1 Runtime Profile Implementation
 *
 * Source-lock anchors:
 *   ENTRANCE.C: F0806_F0806_ENTRANCE_int       (game boot sequence)
 *   ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)
 *   ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics init)
 *   SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful   (header verify)
 *   SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful (header write)
 *   LOADSAVE.C: F0435_STARTEND_LoadGame              (save load)
 *   LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame (save)
 *   DUNGEON.C:  F0237_DUNGEON_DungeonLoad            (hash-verified load)
 *   CASTER.C:   F0211_CASTER_ClearSpellEffects       (spell grid reset at boot)
 *   BugsAndChanges.htm: CHANGE7_29   (new header format: CSBGAME.DAT)
 *   MEDIA529_F20E_F20J: F20E/F21E  (ST save path)
 *   MEDIA332_F20E_F21E_A31E_F31E: CSB C29 key index
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_movement_command_step_runtime_pc34_compat.h"
#include "csb_v1_teleporter_rotation_runtime_pc34_compat.h"
#include "asset_find_by_hash.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "dm1_v1_creature_render_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"
#include "memory_runtime_dynamics_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <sys/stat.h>

/* ── Known CSB hashes ──────────────────────────────────────────────────── */

/*
 * PC 3.4 English Atari ST + Amiga: same dungeon hash.
 * All CSB platforms share the same dungeon.dat content — only
 * graphics/assets vary by platform.
 */
static const char *const g_csb_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",
    NULL
};

static int csb_v1_runtime_locate_appended_expool_record_internal(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size);
static void csb_v1_runtime_schedule_explosion_advance_event(
    CSB_V1_RuntimeProfile *profile,
    const struct TimelineEvent_Compat *event);
static int csb_v1_runtime_stat_or_default(
    const CSB_V1_Champion *champion,
    int stat_index,
    int stat_kind);
static uint32_t csb_v1_runtime_creature_attack_seed(
    const CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int creature_type,
    int creature_index,
    int champion_index);
static int csb_v1_runtime_fill_defender_combat_snapshot(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    struct CombatantChampionSnapshot_Compat *out);
static void csb_v1_runtime_mark_champion_dead(
    CSB_V1_RuntimeProfile *profile,
    int champion_index);
static int csb_v1_runtime_location_after_level_change(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int level_delta,
    int *inout_map_x,
    int *inout_map_y,
    int *out_map_index);
static int csb_v1_runtime_stairs_exit_direction(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y);
static int csb_v1_runtime_object_type_from_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing);
static void csb_v1_runtime_trigger_remote_sensor_event(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell);
static void csb_v1_runtime_process_object_floor_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t placed_thing,
    int level,
    int map_x,
    int map_y);
static uint16_t csb_v1_runtime_csbwin_item16_group_thing(
    uint16_t monster_index);

typedef struct {
    const CSB_V1_RuntimeProfile *profile;
    const CSB_V1_DungeonData *dungeon;
} CSB_V1_RuntimeSkinCacheLookupCtx;

static int csb_v1_runtime_skin_cache_record_lookup(
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size,
    void *user)
{
    const CSB_V1_RuntimeSkinCacheLookupCtx *ctx =
        (const CSB_V1_RuntimeSkinCacheLookupCtx *)user;

    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!ctx) return 0;

    /* CSBWin DSA.cpp SETSKIN writes through Expool. A resumed CSBWin save
     * can therefore carry skin records that supersede the static dungeon
     * DB11 records; use the runtime save tail first, then fall back to the
     * loaded dungeon's original Expool. */
    if (csb_v1_runtime_locate_appended_expool_record_internal(
            ctx->profile, record_id, out_bytes, out_size)) {
        return 1;
    }
    return csb_v1_dungeon_skin_cache_record_lookup(
        record_id,
        out_bytes,
        out_size,
        (void *)ctx->dungeon);
}

/* GRAPHICS.DAT (or CSB.DAT / CSBGRAPH.DAT) MD5 hashes for all known
 * CSB variants — mirrors g_csb_boot_graphics_hashes in csb_v1_boot.c
 * so the runtime path-finder matches the same set of files that the
 * scanner accepts. 2026-06-20: extended the runtime search to be
 * hash-based so files in arbitrary subdirs (e.g. Meynaf FR hard-disk
 * layouts, CSB expansion sets) are found. */
static const char *const g_csb_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611", /* CSB PC 3.4 English GRAPHICS.DAT */
    "ebf6a57af3f27782e358c0490bfd2f2e", /* CSB Atari ST 2.0/2.1 English */
    "291e1bc6803e3dc4b974c60117ca5d68", /* CSB Amiga 3.5 English */
    "cefaddfdf5651df2c91f61b5611a8362", /* CSB Amiga 3.5 Multilanguage */
    NULL
};

/* ── Variant info table ─────────────────────────────────────────────── */

/*
 * CSB variant info.  Platform-specific; game logic is identical.
 * md5_gfx  = GRAPHICS.DAT hash for this variant
 * md5_graf = CSBGRAPH.DAT / CSB.DAT hash (same hash as GRAPHICS.DAT for
 *            the "graphics-only" variants — they lack the overlay archive)
 * md5_dungeon = DUNGEON.DAT hash (shared by all CSB platforms)
 *
 * ReDMCSB COMPILE.H MEDIA tags + CSBWin AssetCache variant mapping.
 */
static const CSB_V1_VariantInfo g_csb_variants[CSB_V1_VARIANT_COUNT] = {
    [CSB_V1_VARIANT_UNKNOWN] = {
        CSB_V1_VARIANT_UNKNOWN,
        "Unknown",
        "",
        "",
        "",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_PC34_EN] = {
        CSB_V1_VARIANT_PC34_EN,
        "PC DOS 3.4 English",
        "MEDIA278:P20JA_P20JB",
        "61fbfd56887c94adc26888a9491c6611",
        "61fbfd56887c94adc26888a9491c6611",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_PC34_MULTI] = {
        CSB_V1_VARIANT_PC34_MULTI,
        "PC DOS 3.4 Multilanguage",
        "MEDIA278:I34E_I34M",
        "cefaddfdf5651df2c91f61b5611a8362",
        "cefaddfdf5651df2c91f61b5611a8362",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST20_EN] = {
        CSB_V1_VARIANT_ST20_EN,
        "Atari ST 2.0 English",
        "MEDIA332:S20E_S21E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST21_EN] = {
        CSB_V1_VARIANT_ST21_EN,
        "Atari ST 2.1 English",
        "MEDIA332:S20E_S21E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_AMIGA35_EN] = {
        CSB_V1_VARIANT_AMIGA35_EN,
        "Amiga 3.5 English",
        "MEDIA529:A35E",
        "291e1bc6803e3dc4b974c60117ca5d68",
        "291e1bc6803e3dc4b974c60117ca5d68",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_AMIGA35_MULTI] = {
        CSB_V1_VARIANT_AMIGA35_MULTI,
        "Amiga 3.5 Multilanguage",
        "MEDIA529:A35M",
        "cefaddfdf5651df2c91f61b5611a8362",
        "cefaddfdf5651df2c91f61b5611a8362",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST_F20J] = {
        CSB_V1_VARIANT_ST_F20J,
        "Atari ST TT (F20J)",
        "MEDIA529:F20J",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST_F20E] = {
        CSB_V1_VARIANT_ST_F20E,
        "Atari ST (F20E)",
        "MEDIA529:F20E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    }
};

_Static_assert(CSB_V1_VARIANT_ST_F20E == CSB_V1_VARIANT_COUNT - 1,
               "CSB_V1_VARIANT_COUNT must match last enum value");

/* ── Platform-specific save dir ────────────────────────────────────── */

#if defined(_WIN32)
#define CSB_PATH_SEP '\\'
#else
#define CSB_PATH_SEP '/'
#endif

static char g_csb_save_dir_buf[512];
static char g_csb_save_path_buf[512];
static int  g_save_dir_init = 0;

static int csb_v1_runtime_first_living_champion(
    const CSB_V1_PartyState *party);

#define CSB_V1_RUNTIME_SAVE_MAGIC   0x46534352u /* FSCR */
#define CSB_V1_RUNTIME_SAVE_VERSION 10u

typedef struct {
    int valid;
    uint16_t group_thing;
    int map_index;
    int map_x;
    int map_y;
    uint8_t delay_fleeing_from_target;
} CSB_V1_RuntimeActiveGroupStateV7;

typedef struct {
    int valid;
    uint16_t group_thing;
    int map_index;
    int map_x;
    int map_y;
    uint8_t cells;
    uint16_t directions;
    int prior_map_x;
    int prior_map_y;
    int home_map_x;
    int home_map_y;
    uint32_t last_move_time;
    uint8_t delay_fleeing_from_target;
} CSB_V1_RuntimeActiveGroupStateV8;

typedef struct {
    int valid;
    uint16_t group_thing;
    int map_index;
    int map_x;
    int map_y;
    uint8_t cells;
    uint16_t directions;
    int prior_map_x;
    int prior_map_y;
    int home_map_x;
    int home_map_y;
    uint32_t last_move_time;
    uint8_t aspect[4];
    uint8_t delay_fleeing_from_target;
} CSB_V1_RuntimeActiveGroupStateV9;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t byte_size;
    int32_t variant_id;
    int32_t difficulty;
    uint32_t dungeon_seed;
    uint16_t dungeon_game_id;
    uint16_t reserved0;
    int32_t current_level;
    int32_t current_world;
    int32_t level_count;
    int32_t world_count;
    int32_t party_x;
    int32_t party_y;
    int32_t party_z;
    int32_t party_dir;
    int32_t champion_count;
    int32_t leader_index;
    int32_t magic_caster_index;
    int32_t party_state_valid;
    int32_t state;
    int32_t paused;
    int32_t victory;
    int32_t game_over;
    uint32_t entrance_map_index;
    uint32_t start_map_index;
    uint64_t game_ticks;
    uint32_t game_time;
    uint64_t total_play_ms;
    uint32_t tick_count;
    struct DM1_EventQueue_V1 timeline_queue;
    struct DM1_TickDispatchResult_V1 last_timeline_dispatch;
    uint32_t timeline_dispatch_count;
    struct Dm1V1InputCommandQueuePc34Compat input_command_queue;
    struct Dm1V1InputQueueProcessResultPc34Compat last_input_dispatch;
    uint32_t input_dispatch_count;
    CSB_V1_ChaosMagicState chaos_magic;
    CSB_V1_PartyState party_state;
    struct ProjectileList_Compat projectiles;
    struct ExplosionList_Compat explosions;
    int32_t csbwin_header_tail_valid;
    uint8_t csbwin_header_byte22808[132];
    int32_t csbwin_appended_tail_valid;
    uint32_t csbwin_appended_tail_size;
    uint32_t csbwin_appended_tail_preserved_size;
    uint32_t csbwin_appended_tail_fnv1a;
    int32_t csbwin_appended_tail_truncated;
    uint8_t csbwin_appended_tail[CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES];
    uint16_t active_group_state_count;
    uint16_t active_group_state_reserved0;
    CSB_V1_RuntimeActiveGroupState
        active_group_state[CSB_V1_RUNTIME_ACTIVE_GROUP_CAP];
} CSB_V1_RuntimeSaveImageV1;

#define CSB_V1_RUNTIME_SAVE_V1_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, projectiles))
#define CSB_V1_RUNTIME_SAVE_V4_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_header_tail_valid))
#define CSB_V1_RUNTIME_SAVE_V5_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_appended_tail_valid))
#define CSB_V1_RUNTIME_SAVE_V6_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, active_group_state_count))
/* Version 7 carried the first active-group side-state table before Cells,
 * Directions, Prior/Home, and LastMoveTime were added.  Keep it loadable as
 * an older image; version 8 preserves the wider table, version 9 adds
 * Aspect[4], and version 10 adds ReDMCSB ActiveGroup.TargetMapX/Y. */
#define CSB_V1_RUNTIME_SAVE_V7_SIZE \
    (CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u + \
     (CSB_V1_RUNTIME_ACTIVE_GROUP_CAP * \
      (uint32_t)sizeof(CSB_V1_RuntimeActiveGroupStateV7)))
#define CSB_V1_RUNTIME_SAVE_V8_SIZE \
    (CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u + \
     (CSB_V1_RUNTIME_ACTIVE_GROUP_CAP * \
      (uint32_t)sizeof(CSB_V1_RuntimeActiveGroupStateV8)))
#define CSB_V1_RUNTIME_SAVE_V9_SIZE \
    (CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u + \
     (CSB_V1_RUNTIME_ACTIVE_GROUP_CAP * \
      (uint32_t)sizeof(CSB_V1_RuntimeActiveGroupStateV9)))

_Static_assert(sizeof(CSB_V1_RuntimeActiveGroupStateV7) == 24u,
               "CSB native save v7 active-group entry size drifted");
_Static_assert(sizeof(CSB_V1_RuntimeActiveGroupStateV8) == 48u,
               "CSB native save v8 active-group entry size drifted");
_Static_assert(sizeof(CSB_V1_RuntimeActiveGroupStateV9) == 52u,
               "CSB native save v9 active-group entry size drifted");

static int csb_v1_runtime_first_living_champion(
    const CSB_V1_PartyState *party);
static uint16_t csb_v1_runtime_normalize_leader_hand_thing(uint16_t thing);
static uint16_t csb_v1_runtime_export_leader_hand_thing(
    const CSB_V1_RuntimeProfile *profile);
static int csb_v1_runtime_target_champion_for_adjacent_attack(
    const CSB_V1_RuntimeProfile *profile,
    int attacker_x,
    int attacker_y,
    int creature_cell);
static CSB_V1_RuntimeActiveGroupState *
csb_v1_runtime_active_group_state_for_thing(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing);
static void csb_v1_runtime_set_active_group_target(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int target_x,
    int target_y);

static void csb_v1_init_save_dir(void)
{
    if (g_save_dir_init) return;
    g_save_dir_init = 1;

    if (0) {}
#if defined(_WIN32)
    {
        const char *appdata = getenv("APPDATA");
        const char *base = appdata ? appdata : "C:\\";
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s\\Firestaff\\csb\\saves\\", base);
    }
#elif defined(__APPLE__)
    {
        const char *home = getenv("HOME");
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s/Library/Application Support/Firestaff/csb/saves/",
                home ? home : "");
    }
#else
    {
        const char *home = getenv("HOME");
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s/.local/share/firestaff/csb/saves/",
                home ? home : "");
    }
#endif
}

/* ── Internal MD5 helper ─────────────────────────────────────────────── */

/*
 * Compute MD5 hex of a file.  Returns 0 on success, -1 on error.
 * outHex must be at least 33 bytes.
 * Uses the same MD5 backend as asset_find_by_hash.c.
 */
static int __attribute__((unused)) csb_v1_file_md5_hex (const char *path, char *outHex, size_t hexSize)
{
    /* Use asset_find_by_md5_list internally for file existence + MD5.
     * We only expose the hash computation through asset_status_m12.
     * For runtime use, the hash comes from M12_AssetStatus scan results.
     * This function stubs to the simplest cross-platform approach. */
    (void)path;
    (void)outHex;
    (void)hexSize;
    if (outHex && hexSize >= 33) {
        outHex[0] = '\0';
    }
    return -1;
}

/* ── Difficulty helpers ─────────────────────────────────────────────── */

int csb_v1_runtime_calc_difficulty(int champion_count)
{
    int x100;
    if (champion_count < 1) champion_count = 1;
    if (champion_count > 4) champion_count = 4;
    x100 = CSB_V1_DIFFICULTY_BASE + (champion_count - 1) * CSB_V1_DIFFICULTY_PER_CHAMP;
    return x100;
}

const char *csb_v1_runtime_difficulty_str(int difficulty_x100)
{
    switch (difficulty_x100) {
        case 100: return "Easy (1 champion)";
        case 125: return "Normal (2 champions)";
        case 150: return "Hard (3 champions)";
        case 200: return "Extreme (4 champions)";
        default:  return "Unknown";
    }
}

/* ── Variant diagnostics ────────────────────────────────────────────── */

const char *csb_v1_runtime_variant_name(CSB_V1_VariantId id)
{
    if (id >= 0 && id < CSB_V1_VARIANT_COUNT) {
        return g_csb_variants[id].name;
    }
    return "Unknown";
}

const CSB_V1_VariantInfo *csb_v1_runtime_get_variant_info(CSB_V1_VariantId id)
{
    if (id >= 0 && id < CSB_V1_VARIANT_COUNT) {
        return &g_csb_variants[id];
    }
    return &g_csb_variants[CSB_V1_VARIANT_UNKNOWN];
}

/*
 * Detect variant by matching gfx + dungeon MD5 hashes to known variants.
 * Falls back to UNKNOWN if no hash matches (assets not yet verified).
 * The dungeon hash is the primary filter (all CSB shares same dungeon hash).
 */
int csb_v1_runtime_detect_variant(const char *gfx_path,
                                    const char *dungeon_path,
                                    const char *md5_gfx,
                                    const char *md5_dungeon)
{
    int i;
    (void)gfx_path;  /* gfx_path used only for diagnostics, md5_gfx is the key */
    (void)dungeon_path;  /* same — md5_dungeon is the key */

    if (!md5_dungeon) return CSB_V1_VARIANT_UNKNOWN;
    if (strcmp(md5_dungeon, "6695d2acebce49f95db1d8f3a5c733de") != 0) {
        return CSB_V1_VARIANT_UNKNOWN;
    }

    if (md5_gfx) {
        for (i = 1; i < CSB_V1_VARIANT_COUNT; i++) {
            if (g_csb_variants[i].md5_gfx[0] != '\0' &&
                strcmp(md5_gfx, g_csb_variants[i].md5_gfx) == 0) {
                return g_csb_variants[i].id;
            }
        }
    }

    return CSB_V1_VARIANT_UNKNOWN;
}

/* ── Asset discovery ────────────────────────────────────────────────── */

/*
 * Search for CSB DUNGEON.DAT by hash.
 * ReDMCSB: DUNGEON.C F0237_DUNGEON_DungeonLoad (hash-gated open).
 *
 * Search order:
 *   data_dir/csb/       (canonical per-game subdirectory)
 *   data_dir/           (shared DM1/CSB/DM2 fallback)
 *   data_dir/csb/csb/   (nested double-drop, rare)
 */
const char *csb_v1_runtime_find_dungeon(const char *data_dir,
                                         CSB_V1_AssetResult *out_result)
{
    static char found_path[ASSET_PATH_MAX];

    if (!data_dir || !out_result) return NULL;
    memset(out_result, 0, sizeof(*out_result));

    if (!asset_find_by_md5_list(data_dir, g_csb_dungeon_hashes,
                                 found_path, sizeof(found_path), NULL, 4)) {
        return NULL;
    }

    out_result->path = found_path;
    out_result->kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    return found_path;
}

/*
 * Search for CSB graphics archive.
 *
 * ReDMCSB asset search (DISK.C + CSBWin AssetCache):
 *   Floppy platforms: CSB.DAT replaces GRAPHICS.DAT entirely
 *   Data/CD platforms: CSBGRAPH.DAT overlays GRAPHICS.DAT
 *   Hybrid platforms:  CSBGRAPH.DAT takes precedence over GRAPHICS.DAT
 *
 * File search order; we try all known archive names and trust the
 * caller (asset_status_m12) to verify the hash.
 */
static const char *const g_csb_gfx_search[] = {
    "csb.dat",
    "CSB.DAT",
    "csbgraph.dat",
    "CSBGRAPH.DAT",
    "graphics.dat",
    "GRAPHICS.DAT",
    NULL
};

const char *csb_v1_runtime_find_graphics(const char *data_dir,
                                             const char *version_hint,
                                             CSB_V1_AssetResult *out_result)
{
    static char found_path[ASSET_PATH_MAX];
    const char *const *names;
    (void)version_hint; /* TODO: narrow search by version hint */

    if (!data_dir || !out_result) return NULL;
    memset(out_result, 0, sizeof(*out_result));

    /* 2026-06-20: prefer MD5-hash search so files in arbitrary
     * subdirs (Meynaf FR hard-disk layouts, CSB expansion sets) are
     * discovered. Falls back to filname search if no hash match. */
    int matchIndex = -1;
    if (asset_find_by_md5_list(data_dir, g_csb_graphics_hashes,
                                 found_path, sizeof(found_path),
                                 &matchIndex, 4)) {
        /* Determine archive kind from the matched hash + extension */
        CSB_V1_AssetGfxArchiveType kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
        const char *base = strrchr(found_path, '/');
        base = base ? base + 1 : found_path;
        if (strcasecmp(base, "CSB.DAT") == 0 ||
            strcasecmp(base, "csb.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSB;
        } else if (strcasecmp(base, "CSBGRAPH.DAT") == 0 ||
                   strcasecmp(base, "csbgraph.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
        }
        out_result->path = found_path;
        out_result->kind = kind;
        return found_path;
    }

    for (names = g_csb_gfx_search; *names != NULL; names++) {
        char tmp[ASSET_PATH_MAX];
        struct stat st;
        CSB_V1_AssetGfxArchiveType kind;

        snprintf(tmp, sizeof(tmp), "%s/%s", data_dir, *names);
        if (stat(tmp, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        strncpy(found_path, tmp, sizeof(found_path) - 1);
        found_path[sizeof(found_path) - 1] = '\0';
        out_result->path = found_path;

        if (strcasecmp(*names, "CSB.DAT") == 0 ||
            strcasecmp(*names, "csb.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSB;
        } else if (strcasecmp(*names, "CSBGRAPH.DAT") == 0 ||
                   strcasecmp(*names, "csbgraph.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
        } else {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
        }
        out_result->kind = kind;
        return found_path;
    }
    return NULL;
}

/* ── Save namespace paths ───────────────────────────────────────────── */

const char *csb_v1_runtime_save_dir(void)
{
    csb_v1_init_save_dir();
    return g_csb_save_dir_buf;
}

const char *csb_v1_runtime_save_path(int slot)
{
    csb_v1_init_save_dir();
    if (slot < 0 || slot > 9) slot = 0;
    snprintf(g_csb_save_path_buf, sizeof(g_csb_save_path_buf),
             "%s%ccsb_save_%d.fsav",
             g_csb_save_dir_buf, CSB_PATH_SEP, slot);
    return g_csb_save_path_buf;
}

static uint16_t csb_v1_runtime_effective_game_id(
    const CSB_V1_RuntimeProfile *profile)
{
    if (profile && profile->dungeon_game_id) {
        return profile->dungeon_game_id;
    }
    return 0x1234u;
}

static void csb_v1_runtime_capture_save_image(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimeSaveImageV1 *image)
{
    memset(image, 0, sizeof(*image));
    image->magic = CSB_V1_RUNTIME_SAVE_MAGIC;
    image->version = CSB_V1_RUNTIME_SAVE_VERSION;
    image->byte_size = (uint32_t)sizeof(*image);
    image->variant_id = (int32_t)profile->variant_id;
    image->difficulty = (int32_t)profile->difficulty;
    image->dungeon_seed = profile->dungeon_seed;
    image->dungeon_game_id = csb_v1_runtime_effective_game_id(profile);
    image->current_level = profile->current_level;
    image->current_world = profile->current_world;
    image->level_count = profile->level_count;
    image->world_count = profile->world_count;
    image->party_x = profile->party_x;
    image->party_y = profile->party_y;
    image->party_z = profile->party_z;
    image->party_dir = profile->party_dir;
    image->champion_count = profile->champion_count;
    image->leader_index = profile->leader_index;
    image->magic_caster_index = profile->magic_caster_index;
    image->party_state_valid = profile->party_state_valid;
    image->state = profile->state;
    image->paused = profile->paused;
    image->victory = profile->victory;
    image->game_over = profile->game_over;
    image->entrance_map_index = profile->entrance_map_index;
    image->start_map_index = profile->start_map_index;
    image->game_ticks = profile->game_ticks;
    image->game_time = profile->game_time;
    image->total_play_ms = profile->total_play_ms;
    image->tick_count = profile->tick_count;
    image->timeline_queue = profile->timeline_queue;
    image->last_timeline_dispatch = profile->last_timeline_dispatch;
    image->timeline_dispatch_count = profile->timeline_dispatch_count;
    image->input_command_queue = profile->input_command_queue;
    image->last_input_dispatch = profile->last_input_dispatch;
    image->input_dispatch_count = profile->input_dispatch_count;
    image->chaos_magic = profile->chaos_magic;
    image->party_state = profile->party_state;
    image->projectiles = profile->projectiles;
    image->explosions = profile->explosions;
    image->csbwin_header_tail_valid =
        profile->csbwin_header_tail_valid ? 1 : 0;
    memcpy(image->csbwin_header_byte22808,
           profile->csbwin_header_byte22808,
           sizeof(image->csbwin_header_byte22808));
    image->csbwin_appended_tail_valid =
        profile->csbwin_appended_tail_valid ? 1 : 0;
    image->csbwin_appended_tail_size =
        (uint32_t)profile->csbwin_appended_tail_size;
    image->csbwin_appended_tail_preserved_size =
        (uint32_t)profile->csbwin_appended_tail_preserved_size;
    image->csbwin_appended_tail_fnv1a =
        profile->csbwin_appended_tail_fnv1a;
    image->csbwin_appended_tail_truncated =
        profile->csbwin_appended_tail_truncated ? 1 : 0;
    memcpy(image->csbwin_appended_tail,
           profile->csbwin_appended_tail,
           sizeof(image->csbwin_appended_tail));
    image->active_group_state_count = profile->active_group_state_count;
    memcpy(image->active_group_state,
           profile->active_group_state,
           sizeof(image->active_group_state));
}

static int csb_v1_runtime_validate_projectile_list(
    const struct ProjectileList_Compat *list)
{
    int i;
    int active_count = 0;

    if (!list) return 0;
    if (list->count < 0 || list->count > PROJECTILE_LIST_CAPACITY) return 0;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat *projectile =
            &list->entries[i];
        if (projectile->reserved3 == 0) {
            continue;
        }
        if (projectile->slotIndex != i ||
            projectile->mapIndex < 0 ||
            projectile->cell < 0 ||
            projectile->cell > 3 ||
            projectile->direction < 0 ||
            projectile->direction > 3) {
            return 0;
        }
        active_count++;
    }
    return active_count == list->count;
}

static int csb_v1_runtime_validate_explosion_list(
    const struct ExplosionList_Compat *list)
{
    int i;
    int active_count = 0;

    if (!list) return 0;
    if (list->count < 0 || list->count > EXPLOSION_LIST_CAPACITY) return 0;
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
        const struct ExplosionInstance_Compat *explosion =
            &list->entries[i];
        if (explosion->reserved0 == 0) {
            continue;
        }
        if (explosion->slotIndex != i ||
            explosion->mapIndex < 0 ||
            explosion->cell < 0 ||
            explosion->cell > EXPLOSION_CELL_CENTERED) {
            return 0;
        }
        active_count++;
    }
    return active_count == list->count;
}

static int csb_v1_runtime_validate_active_group_state(
    const CSB_V1_RuntimeSaveImageV1 *image)
{
    uint16_t i;
    uint16_t active = 0u;

    if (!image) return 0;
    if (image->active_group_state_count >
        CSB_V1_RUNTIME_ACTIVE_GROUP_CAP) {
        return 0;
    }
    for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
        const CSB_V1_RuntimeActiveGroupState *state =
            &image->active_group_state[i];
        if (!state->valid) continue;
        if (state->map_index < 0 ||
            state->map_x < 0 ||
            state->map_y < 0 ||
            state->target_map_x < 0 ||
            state->target_map_y < 0 ||
            ((state->group_thing >> 10) & 0x0Fu) != 4u) {
            return 0;
        }
        ++active;
    }
    return active == image->active_group_state_count;
}

static int csb_v1_runtime_active_group_state_entry_valid(
    const CSB_V1_RuntimeActiveGroupState *state)
{
    if (!state) return 0;
    if (!state->valid) return 1;
    if (state->map_index < 0 ||
        state->map_x < 0 ||
        state->map_y < 0 ||
        state->target_map_x < 0 ||
        state->target_map_y < 0 ||
        ((state->group_thing >> 10) & 0x0Fu) != 4u) {
        return 0;
    }
    return 1;
}

static int csb_v1_runtime_apply_active_group_state_from_save_image(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeSaveImageV1 *image)
{
    uint16_t i;
    uint16_t active = 0u;
    const uint8_t *base;

    if (!profile || !image) return -1;
    profile->active_group_state_count = 0u;
    memset(profile->active_group_state, 0,
           sizeof(profile->active_group_state));

    if (image->byte_size < CSB_V1_RUNTIME_SAVE_V6_SIZE + 4u) {
        return 0;
    }
    if (image->active_group_state_count >
        CSB_V1_RUNTIME_ACTIVE_GROUP_CAP) {
        return -1;
    }

    base = ((const uint8_t *)image) +
           offsetof(CSB_V1_RuntimeSaveImageV1, active_group_state);
    if (image->version == 7u &&
        image->byte_size == CSB_V1_RUNTIME_SAVE_V7_SIZE) {
        const CSB_V1_RuntimeActiveGroupStateV7 *legacy =
            (const CSB_V1_RuntimeActiveGroupStateV7 *)base;
        for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
            CSB_V1_RuntimeActiveGroupState *state =
                &profile->active_group_state[i];
            if (!legacy[i].valid) continue;
            state->valid = legacy[i].valid;
            state->group_thing = legacy[i].group_thing;
            state->map_index = legacy[i].map_index;
            state->map_x = legacy[i].map_x;
            state->map_y = legacy[i].map_y;
            state->prior_map_x = legacy[i].map_x;
            state->prior_map_y = legacy[i].map_y;
            state->home_map_x = legacy[i].map_x;
            state->home_map_y = legacy[i].map_y;
            state->target_map_x = legacy[i].map_x;
            state->target_map_y = legacy[i].map_y;
            state->delay_fleeing_from_target =
                legacy[i].delay_fleeing_from_target;
            if (!csb_v1_runtime_active_group_state_entry_valid(state)) {
                return -1;
            }
            ++active;
        }
    } else if (image->version == 8u &&
               image->byte_size == CSB_V1_RUNTIME_SAVE_V8_SIZE) {
        const CSB_V1_RuntimeActiveGroupStateV8 *legacy =
            (const CSB_V1_RuntimeActiveGroupStateV8 *)base;
        for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
            CSB_V1_RuntimeActiveGroupState *state =
                &profile->active_group_state[i];
            if (!legacy[i].valid) continue;
            state->valid = legacy[i].valid;
            state->group_thing = legacy[i].group_thing;
            state->map_index = legacy[i].map_index;
            state->map_x = legacy[i].map_x;
            state->map_y = legacy[i].map_y;
            state->cells = legacy[i].cells;
            state->directions = legacy[i].directions;
            state->prior_map_x = legacy[i].prior_map_x;
            state->prior_map_y = legacy[i].prior_map_y;
            state->home_map_x = legacy[i].home_map_x;
            state->home_map_y = legacy[i].home_map_y;
            state->last_move_time = legacy[i].last_move_time;
            state->target_map_x = legacy[i].map_x;
            state->target_map_y = legacy[i].map_y;
            state->delay_fleeing_from_target =
                legacy[i].delay_fleeing_from_target;
            if (!csb_v1_runtime_active_group_state_entry_valid(state)) {
                return -1;
            }
            ++active;
        }
    } else if (image->version == 9u &&
               image->byte_size == CSB_V1_RUNTIME_SAVE_V9_SIZE) {
        const CSB_V1_RuntimeActiveGroupStateV9 *legacy =
            (const CSB_V1_RuntimeActiveGroupStateV9 *)base;
        for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
            CSB_V1_RuntimeActiveGroupState *state =
                &profile->active_group_state[i];
            if (!legacy[i].valid) continue;
            state->valid = legacy[i].valid;
            state->group_thing = legacy[i].group_thing;
            state->map_index = legacy[i].map_index;
            state->map_x = legacy[i].map_x;
            state->map_y = legacy[i].map_y;
            state->cells = legacy[i].cells;
            state->directions = legacy[i].directions;
            state->prior_map_x = legacy[i].prior_map_x;
            state->prior_map_y = legacy[i].prior_map_y;
            state->home_map_x = legacy[i].home_map_x;
            state->home_map_y = legacy[i].home_map_y;
            state->last_move_time = legacy[i].last_move_time;
            state->target_map_x = legacy[i].map_x;
            state->target_map_y = legacy[i].map_y;
            memcpy(state->aspect, legacy[i].aspect, sizeof(state->aspect));
            state->delay_fleeing_from_target =
                legacy[i].delay_fleeing_from_target;
            if (!csb_v1_runtime_active_group_state_entry_valid(state)) {
                return -1;
            }
            ++active;
        }
    } else if (image->byte_size >=
               offsetof(CSB_V1_RuntimeSaveImageV1, active_group_state) +
                   sizeof(image->active_group_state)) {
        if (!csb_v1_runtime_validate_active_group_state(image)) {
            return -1;
        }
        profile->active_group_state_count =
            image->active_group_state_count;
        memcpy(profile->active_group_state,
               image->active_group_state,
               sizeof(profile->active_group_state));
        return 0;
    }

    if (active != image->active_group_state_count) {
        memset(profile->active_group_state, 0,
               sizeof(profile->active_group_state));
        return -1;
    }
    profile->active_group_state_count = active;
    return 0;
}

static int csb_v1_runtime_apply_save_image(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeSaveImageV1 *image,
    const CSB_V1_SaveHeader *header)
{
    int leader;
    if (!profile || !image || !header) return -1;
    if (image->magic != CSB_V1_RUNTIME_SAVE_MAGIC) {
        return -1;
    }
    if (!((image->version == 1u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V1_SIZE) ||
          (image->version == 4u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V4_SIZE) ||
          (image->version == 5u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V5_SIZE) ||
          (image->version == 6u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V6_SIZE) ||
          (image->version == 7u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V7_SIZE) ||
          (image->version == 8u &&
           image->byte_size == CSB_V1_RUNTIME_SAVE_V8_SIZE) ||
          (image->version == CSB_V1_RUNTIME_SAVE_VERSION &&
           image->byte_size == sizeof(*image)))) {
        return -1;
    }
    if (header->Magic != CSB_V1_SAVE_MAGIC_CSB ||
        header->GameID != image->dungeon_game_id) {
        return -1;
    }
    if (image->champion_count < 0 ||
        image->champion_count > CSB_V1_MAX_CHAMPIONS ||
        image->party_state.ChampionCount < 0 ||
        image->party_state.ChampionCount > CSB_V1_MAX_CHAMPIONS ||
        image->party_dir < 0 || image->party_dir > 3) {
        return -1;
    }

    /* ReDMCSB LOADSAVE.C F0435 lines 2721-2800 restores GLOBAL_DATA,
     * PARTY, EVENTS, and TIMELINE into live globals before play resumes.
     * Firestaff's CSB profile currently owns those boundaries directly in
     * this POD runtime image; asset paths and the loaded dungeon handle stay
     * with the caller's already booted profile. */
    profile->variant_id = (CSB_V1_VariantId)image->variant_id;
    profile->difficulty = (CSB_V1_Difficulty)image->difficulty;
    profile->dungeon_seed = image->dungeon_seed;
    profile->dungeon_game_id = image->dungeon_game_id;
    profile->current_level = image->current_level;
    csb_v1_dungeon_set_current_level(profile->current_level);
    profile->current_world = image->current_world;
    profile->level_count = image->level_count;
    profile->world_count = image->world_count;
    profile->party_x = image->party_x;
    profile->party_y = image->party_y;
    profile->party_z = image->party_z;
    profile->party_dir = image->party_dir & 3;
    profile->champion_count = image->champion_count;
    profile->leader_index = image->leader_index;
    profile->magic_caster_index = image->magic_caster_index;
    profile->party_state_valid = image->party_state_valid ? 1 : 0;
    profile->state = image->state;
    profile->paused = image->paused;
    profile->victory = image->victory;
    profile->game_over = image->game_over;
    profile->entrance_map_index = image->entrance_map_index;
    profile->start_map_index = image->start_map_index;
    profile->game_ticks = image->game_ticks;
    profile->game_time = image->game_time;
    profile->total_play_ms = image->total_play_ms;
    profile->tick_count = image->tick_count;
    profile->timeline_queue = image->timeline_queue;
    profile->last_timeline_dispatch = image->last_timeline_dispatch;
    profile->timeline_dispatch_count = image->timeline_dispatch_count;
    if (image->byte_size >=
        offsetof(CSB_V1_RuntimeSaveImageV1, explosions) +
            sizeof(image->explosions)) {
        if (!csb_v1_runtime_validate_projectile_list(&image->projectiles) ||
            !csb_v1_runtime_validate_explosion_list(&image->explosions)) {
            return -1;
        }
        profile->projectiles = image->projectiles;
        profile->explosions = image->explosions;
    } else {
        memset(&profile->projectiles, 0, sizeof(profile->projectiles));
        memset(&profile->explosions, 0, sizeof(profile->explosions));
    }
    profile->input_command_queue = image->input_command_queue;
    profile->last_input_dispatch = image->last_input_dispatch;
    profile->input_dispatch_count = image->input_dispatch_count;
    profile->chaos_magic = image->chaos_magic;
    profile->party_state = image->party_state;
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(
            profile->party_state.LeaderHandThing);
    if (image->byte_size >=
        offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_header_byte22808) +
            sizeof(image->csbwin_header_byte22808)) {
        profile->csbwin_header_tail_valid =
            image->csbwin_header_tail_valid ? 1 : 0;
        memcpy(profile->csbwin_header_byte22808,
               image->csbwin_header_byte22808,
               sizeof(profile->csbwin_header_byte22808));
    } else {
        profile->csbwin_header_tail_valid = 0;
        memset(profile->csbwin_header_byte22808, 0,
               sizeof(profile->csbwin_header_byte22808));
    }
    if (image->byte_size >=
        offsetof(CSB_V1_RuntimeSaveImageV1, csbwin_appended_tail) +
            sizeof(image->csbwin_appended_tail)) {
        if (image->csbwin_appended_tail_preserved_size >
                CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
            image->csbwin_appended_tail_size <
                image->csbwin_appended_tail_preserved_size) {
            return -1;
        }
        profile->csbwin_appended_tail_valid =
            image->csbwin_appended_tail_valid ? 1 : 0;
        profile->csbwin_appended_tail_size =
            image->csbwin_appended_tail_size;
        profile->csbwin_appended_tail_preserved_size =
            image->csbwin_appended_tail_preserved_size;
        profile->csbwin_appended_tail_fnv1a =
            image->csbwin_appended_tail_fnv1a;
        profile->csbwin_appended_tail_truncated =
            image->csbwin_appended_tail_truncated ? 1 : 0;
        memcpy(profile->csbwin_appended_tail,
               image->csbwin_appended_tail,
               sizeof(profile->csbwin_appended_tail));
    } else {
        profile->csbwin_appended_tail_valid = 0;
        profile->csbwin_appended_tail_size = 0u;
        profile->csbwin_appended_tail_preserved_size = 0u;
        profile->csbwin_appended_tail_fnv1a = 0u;
        profile->csbwin_appended_tail_truncated = 0;
        memset(profile->csbwin_appended_tail, 0,
               sizeof(profile->csbwin_appended_tail));
    }
    if (csb_v1_runtime_apply_active_group_state_from_save_image(
            profile,
            image) != 0) {
        return -1;
    }

    profile->party_state.PartyMapX = profile->party_x;
    profile->party_state.PartyMapY = profile->party_y;
    profile->party_state.PartyDirection = (uint8_t)(profile->party_dir & 3);
    profile->party_state.MagicCasterIndex = profile->magic_caster_index;
    leader = profile->leader_index;
    if (leader < -1 || leader >= profile->party_state.ChampionCount) {
        leader = csb_v1_runtime_first_living_champion(&profile->party_state);
    }
    profile->leader_index = leader;
    profile->party_state.LeaderIndex = leader;
    profile->timeline_queue.gameTick = profile->game_time;
    return 0;
}

int csb_v1_runtime_save_game_to_path(const CSB_V1_RuntimeProfile *profile,
                                     const char *path)
{
    CSB_V1_RuntimeSaveImageV1 image;
    CSB_V1_SaveHeader header;
    uint16_t game_id;

    if (!profile || !path) return -1;
    game_id = csb_v1_runtime_effective_game_id(profile);
    csb_v1_runtime_capture_save_image(profile, &image);
    memset(&header, 0, sizeof(header));
    if (csb_v1_save_header_build(&header,
                                  CSB_V1_SAVE_MAGIC_CSB,
                                  game_id,
                                  profile->dungeon_seed,
                                  profile->party_x,
                                  profile->party_y,
                                  profile->current_level,
                                  profile->party_dir,
                                  profile->champion_count,
                                  profile->game_time,
                                  (uint32_t)profile->total_play_ms) != 0) {
        return -1;
    }
    return csb_v1_save_game(path, &image, (int)sizeof(image), &header);
}

int csb_v1_runtime_import_csbgame_roster_from_path(
    CSB_V1_RuntimeProfile *profile,
    const char *path)
{
    CSB_V1_PartyState party;
    int imported;
    int pose_x;
    int pose_y;
    int pose_z;
    int pose_dir;
    int pose_level;

    if (!profile || !path) return CSB_SAVE_IMPORT_ERR_NULL;

    pose_x = profile->party_x;
    pose_y = profile->party_y;
    pose_z = profile->party_z;
    pose_dir = profile->party_dir & 3;
    pose_level = profile->current_level;

    memset(&party, 0, sizeof(party));
    imported = csb_v1_import_csb_save_file(&party, path);
    if (imported <= 0) {
        return imported;
    }

    /* ReDMCSB LOADSAVE.C F0435 restores a full running game, while
     * CHARACTER.C ReadingChampion()/CEDTINC8.C import only the roster
     * payload from CSBGAME.DAT.  Until the CSBGAME dungeon/global-data
     * body is source-locked, keep the already booted runtime pose and
     * promote only the champion roster into live CSB state. */
    party.PartyMapX = pose_x;
    party.PartyMapY = pose_y;
    party.PartyDirection = (uint8_t)pose_dir;
    party.MagicCasterIndex = party.LeaderIndex;

    if (csb_v1_runtime_set_party_state(profile, &party) != 0) {
        return -1;
    }

    profile->party_x = pose_x;
    profile->party_y = pose_y;
    profile->party_z = pose_z;
    profile->party_dir = pose_dir;
    profile->current_level = pose_level;
    csb_v1_dungeon_set_current_level(profile->current_level);
    profile->difficulty =
        (CSB_V1_Difficulty)csb_v1_runtime_calc_difficulty(imported);
    profile->party_state.PartyMapX = pose_x;
    profile->party_state.PartyMapY = pose_y;
    profile->party_state.PartyDirection = (uint8_t)pose_dir;
    profile->party_state.MagicCasterIndex = profile->magic_caster_index;
    profile->timeline_queue.gameTick = profile->game_time;
    return CSB_V1_LOAD_OK;
}

int csb_v1_runtime_load_game_from_path(CSB_V1_RuntimeProfile *profile,
                                       const char *path)
{
    CSB_V1_RuntimeSaveImageV1 image;
    CSB_V1_SaveHeader header;
    int result;

    if (!profile || !path) return -1;
    memset(&image, 0, sizeof(image));
    memset(&header, 0, sizeof(header));
    result = csb_v1_load_game(path, &image, (int)sizeof(image), &header);
    if (result != CSB_V1_LOAD_OK) {
        int import_result =
            csb_v1_runtime_import_csbgame_roster_from_path(profile, path);
        return (import_result == CSB_V1_LOAD_OK) ? CSB_V1_LOAD_OK : result;
    }
    return csb_v1_runtime_apply_save_image(profile, &image, &header);
}

static void csb_v1_runtime_apply_timeline_dispatch_side_effects(
    CSB_V1_RuntimeProfile *profile);

/* ── Internal tick helper ─────────────────────────────────────────────── */

static void csb_v1_fire_tick(CSB_V1_RuntimeProfile *profile)
{
    int dispatched;

    /* Source: ReDMCSB GAMELOOP.C F0002 lines 69-124 calls
     * F0065_SOUND_ProcessPendingSound before F0261_TIMELINE_Process_CPSEF()
     * and then increments G0313_ul_GameTime.  TIMELINE.C F0240 lines
     * 702-708 expires the first heap event when event_time <= G0313_ul_GameTime. */
    (void)csb_v1_audio_runtime_flush_pending(&profile->audio_runtime);
    profile->timeline_queue.gameTick = profile->game_time;
    memset(&profile->last_timeline_dispatch, 0,
           sizeof(profile->last_timeline_dispatch));
    dispatched = dm1v1_event_process_tick(&profile->timeline_queue,
                                          &profile->last_timeline_dispatch);
    if (dispatched > 0) {
        profile->timeline_dispatch_count += (uint32_t)dispatched;
        csb_v1_runtime_apply_timeline_dispatch_side_effects(profile);
    }

    profile->game_time++;
    profile->tick_count++;
    profile->game_ticks += CSB_V1_TICK_MS_NOMINAL;

    /* Chaos Magic spell grid is versioned on each tick.
     * F0211_CASTER_ClearSpellEffects increments spell_grid_version at world load.
     * We advance chaos_level here for ambient oscillation.
     * Source: CSBWin Magic.cpp ambient loop (no direct ReDMCSB ref). */
    if (profile->chaos_magic.magic_initialized) {
        uint32_t beat = profile->tick_count % 20U;
        profile->chaos_magic.chaos_level = (uint8_t)((beat < 10U) ? 0U : 1U);
        profile->chaos_magic.spell_grid_version++;
    }
}

static int csb_v1_runtime_default_wall_probe(
    const CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    void *context)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;
    int door_state;

    (void)context;
    dungeon = (profile && profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) {
        return 0;
    }
    level = csb_v1_dungeon_get_current_level();
    if (level < 0 || level >= dungeon->level_count) {
        level = 0;
    }
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0) return 1;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    if (dungeon->square_bytes == 1) {
        if (square_type == 0) return 1;
        if (square_type == 4) {
            /* ReDMCSB: CLIKMENU.C F0366 lines 282-286 blocks doors
             * unless M036_DOOR_STATE is open, one-fourth closed, or
             * destroyed.  DEFS.H lines 1039-1046 define states 0,1,5. */
            door_state = raw_square & 0x07;
            return door_state != 0 && door_state != 1 && door_state != 5;
        }
        if (square_type == 6) {
            /* ReDMCSB: CLIKMENU.C F0366 lines 287-290 blocks fake walls
             * unless MASK0x0004_FAKEWALL_OPEN or
             * MASK0x0001_FAKEWALL_IMAGINARY is set. */
            return !(raw_square & 0x04) && !(raw_square & 0x01);
        }
        return 0;
    }
    if (square_type == 4) {
        door_state = raw_square & 0x07;
        return door_state != 0 && door_state != 1 && door_state != 5;
    }
    if (square_type == 6) {
        return !(raw_square & 0x04) && !(raw_square & 0x01);
    }
    return square_type == 1;
}

static int csb_v1_runtime_sample_destination_square(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;

    if (!profile || !result || !result->movement_step_attempted) return -1;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return -1;

    level = profile->current_level;
    if (level < 0 || level >= dungeon->level_count) {
        level = csb_v1_dungeon_get_current_level();
    }
    if (level < 0 || level >= dungeon->level_count) {
        level = 0;
    }

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        result->movement_destination_x,
        result->movement_destination_y);
    if (raw_square < 0) return -1;

    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    result->movement_destination_raw_square = raw_square;
    result->movement_destination_square_type = square_type;
    if (square_type == 4) {
        result->movement_destination_door_state = raw_square & 0x07;
    }
    return level;
}

static int csb_v1_runtime_current_square_is_stairs(
    CSB_V1_RuntimeProfile *profile,
    int *out_raw_square,
    int *out_level)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;

    if (out_raw_square) *out_raw_square = -1;
    if (out_level) *out_level = -1;
    if (!profile) return 0;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return 0;
    level = profile->current_level;
    if (level < 0 || level >= dungeon->level_count) {
        level = csb_v1_dungeon_get_current_level();
    }
    if (level < 0 || level >= dungeon->level_count) return 0;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        profile->party_x,
        profile->party_y);
    if (raw_square < 0) return 0;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    if (square_type != 3) return 0;
    if (out_raw_square) *out_raw_square = raw_square;
    if (out_level) *out_level = level;
    return 1;
}

static int csb_v1_runtime_decode_destination_teleporter(
    const CSB_V1_DungeonData *dungeon,
    int level,
    const CSB_V1_InputCommandRuntimeResult *result,
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *out_teleporter,
    int *out_scope)
{
    int first_thing;
    int thing_type;
    int thing_size;
    uint16_t word;
    uint16_t target_word;
    const uint8_t *record;

    if (out_scope) *out_scope = 0;
    if (!dungeon || !result || !out_teleporter) return -1;
    if (result->movement_destination_square_type != 5) return -1;
    if (!(result->movement_destination_raw_square & 0x08)) return 0;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        result->movement_destination_x,
        result->movement_destination_y);
    if (first_thing < 0) return 0;
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        (uint16_t)first_thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_type != 1 || thing_size < 6) return 0;

    /* ReDMCSB: DEFS.H TELEPORTER for PC/I34E stores Next, then a packed
     * word with TargetMapX/Y, Rotation, AbsoluteRotation, Scope, Audible,
     * followed by Unreferenced and TargetMapIndex bytes. */
    word = (uint16_t)record[2] | (uint16_t)((uint16_t)record[3] << 8);
    target_word = (uint16_t)record[4] | (uint16_t)((uint16_t)record[5] << 8);
    memset(out_teleporter, 0, sizeof(*out_teleporter));
    out_teleporter->target_map_x = (int)(word & 0x1Fu);
    out_teleporter->target_map_y = (int)((word >> 5) & 0x1Fu);
    out_teleporter->rotation = (int)((word >> 10) & 0x03u);
    out_teleporter->absolute_rotation = (word & 0x1000u) ? 1 : 0;
    if (out_scope) *out_scope = (int)((word >> 13) & 0x03u);
    out_teleporter->audible = (word & 0x8000u) ? 1 : 0;
    out_teleporter->target_map_index = (int)((target_word >> 8) & 0xFFu);
    return 1;
}

static int csb_v1_runtime_decode_teleporter_at_square(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    int raw_square,
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *out_teleporter,
    int *out_scope)
{
    int first_thing;
    int thing_type;
    int thing_size;
    uint16_t word;
    uint16_t target_word;
    const uint8_t *record;

    if (out_scope) *out_scope = 0;
    if (!dungeon || !out_teleporter) return -1;
    if (((raw_square >> 5) & 0x07) != 5) return -1;
    if (!(raw_square & 0x08)) return 0;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (first_thing < 0) return 0;
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        (uint16_t)first_thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_type != 1 || thing_size < 6) return 0;

    word = (uint16_t)record[2] | (uint16_t)((uint16_t)record[3] << 8);
    target_word = (uint16_t)record[4] | (uint16_t)((uint16_t)record[5] << 8);
    memset(out_teleporter, 0, sizeof(*out_teleporter));
    out_teleporter->target_map_x = (int)(word & 0x1Fu);
    out_teleporter->target_map_y = (int)((word >> 5) & 0x1Fu);
    out_teleporter->rotation = (int)((word >> 10) & 0x03u);
    out_teleporter->absolute_rotation = (word & 0x1000u) ? 1 : 0;
    if (out_scope) *out_scope = (int)((word >> 13) & 0x03u);
    out_teleporter->audible = (word & 0x8000u) ? 1 : 0;
    out_teleporter->target_map_index = (int)((target_word >> 8) & 0xFFu);
    return 1;
}

static void csb_v1_runtime_apply_destination_teleporter(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 teleporter_result;
    int level;
    int scope = 0;
    int decoded;

    if (!profile || !result || !result->movement_step_applied) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;

    level = csb_v1_runtime_sample_destination_square(profile, result);
    if (level < 0) return;
    if (result->movement_destination_square_type != 5) return;
    result->teleporter_open =
        (result->movement_destination_raw_square & 0x08) ? 1 : 0;
    if (!result->teleporter_open) return;

    decoded = csb_v1_runtime_decode_destination_teleporter(
        dungeon,
        level,
        result,
        &teleporter,
        &scope);
    if (decoded <= 0) return;
    result->teleporter_scope = scope;
    result->teleporter_rotation = teleporter.rotation;
    result->teleporter_absolute_rotation = teleporter.absolute_rotation;
    result->teleporter_audible = teleporter.audible;
    result->teleporter_target_x = teleporter.target_map_x;
    result->teleporter_target_y = teleporter.target_map_y;
    result->teleporter_target_level = teleporter.target_map_index;
    if (!(scope & 0x02)) return;
    if (teleporter.target_map_index < 0 ||
        teleporter.target_map_index >= dungeon->level_count) {
        return;
    }

    /* ReDMCSB: MOVESENS.C F0267 lines 475-518 enters an open
     * C05_ELEMENT_TELEPORTER, requires object/party scope for the party,
     * moves to TargetMapX/Y/TargetMapIndex, and applies teleporter rotation
     * through CHAMPION.C F0284.  This bounded runtime handoff applies one
     * party teleporter only; chained teleporters, sounds, redraw timing, and
     * object/group/projectile teleportation remain separate work. */
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (csb_v1_teleporter_rotation_apply_party_pc34_compat(
            profile,
            &teleporter,
            &teleporter_result) != 0) {
        return;
    }
    csb_v1_dungeon_set_current_level(profile->current_level);
    result->new_party_level = profile->current_level;
    result->teleporter_transition_applied = 1;
}

static void csb_v1_runtime_apply_destination_stairs(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int square_type;
    int stair_up;
    int target_level;
    int target_x;
    int target_y;
    int exit_direction;

    if (!profile || !result || !result->movement_step_applied ||
        result->teleporter_transition_applied ||
        result->pit_fall_applied ||
        result->stair_transition_applied) {
        return;
    }
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;

    level = csb_v1_runtime_sample_destination_square(profile, result);
    if (level < 0) return;
    raw_square = result->movement_destination_raw_square;
    square_type = result->movement_destination_square_type;
    if (square_type != 3) return;

    /* ReDMCSB: CLIKMENU.C F0366 lines 264-276 reaches C03_ELEMENT_STAIRS
     * and calls F0364_COMMAND_TakeStairs.  F0364 lines 136-138 resolves
     * the destination through DUNGEON.C F0154, then applies F0155 exit
     * direction through CHAMPION.C F0284. */
    stair_up = (raw_square & 0x04) ? 1 : 0;
    target_x = result->movement_destination_x;
    target_y = result->movement_destination_y;
    result->stair_up = stair_up;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (!csb_v1_runtime_location_after_level_change(
            dungeon,
            level,
            stair_up ? -1 : 1,
            &target_x,
            &target_y,
            &target_level)) {
        return;
    }

    profile->current_level = target_level;
    profile->party_x = target_x;
    profile->party_y = target_y;
    profile->party_state.PartyMapX = target_x;
    profile->party_state.PartyMapY = target_y;
    csb_v1_dungeon_set_current_level(target_level);
    exit_direction = csb_v1_runtime_stairs_exit_direction(
        dungeon,
        target_level,
        target_x,
        target_y);
    (void)csb_v1_runtime_rotate_party(profile, exit_direction);
    result->new_party_level = target_level;
    result->stair_transition_applied = 1;
}

static void csb_v1_runtime_take_current_stairs(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int stair_up;
    int target_level;
    int target_x;
    int target_y;
    int exit_direction;

    if (!profile || !result) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;
    if (!csb_v1_runtime_current_square_is_stairs(
            profile,
            &raw_square,
            &level)) {
        return;
    }

    /* ReDMCSB: CLIKMENU.C F0365 lines 164-168 and F0366 lines 264-266
     * route turning on stairs and moving backward on stairs to
     * F0364_COMMAND_TakeStairs instead of applying the requested turn or
     * one-square backward step.  F0364 lines 135-138 processes party removal
     * from the stairs square, resolves DUNGEON.C F0154, then applies
     * F0155/F0284 exit direction. */
    stair_up = (raw_square & 0x04) ? 1 : 0;
    target_x = profile->party_x;
    target_y = profile->party_y;
    result->movement_destination_x = profile->party_x;
    result->movement_destination_y = profile->party_y;
    result->movement_destination_raw_square = raw_square;
    result->movement_destination_square_type = 3;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    result->stair_up = stair_up;
    if (!csb_v1_runtime_location_after_level_change(
            dungeon,
            level,
            stair_up ? -1 : 1,
            &target_x,
            &target_y,
            &target_level)) {
        return;
    }

    profile->current_level = target_level;
    profile->party_x = target_x;
    profile->party_y = target_y;
    profile->party_state.PartyMapX = target_x;
    profile->party_state.PartyMapY = target_y;
    csb_v1_dungeon_set_current_level(target_level);
    exit_direction = csb_v1_runtime_stairs_exit_direction(
        dungeon,
        target_level,
        target_x,
        target_y);
    (void)csb_v1_runtime_rotate_party(profile, exit_direction);
    result->new_party_level = target_level;
    result->stair_transition_applied = 1;
}

static int csb_v1_runtime_apply_party_fall_damage(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result,
    int map_index,
    int map_x,
    int map_y)
{
    struct RngState_Compat rng;
    int random_window;
    int base_attack;
    int damaged_count = 0;
    int total_damage = 0;
    int wound_mask = 0;
    int i;

    if (!profile || !profile->party_state_valid ||
        profile->party_state.ChampionCount <= 0) {
        return 0;
    }

    random_window = (20 >> 3) + 1;
    base_attack = 20 - random_window;
    random_window <<= 1;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)(map_index & 0xff) << 4) ^
            ((uint32_t)(map_x & 0xff) << 12) ^
            ((uint32_t)(map_y & 0xff) << 20) ^
            0xF0324u);

    /* ReDMCSB MOVESENS.C F0267 lines 590-603 applies
     * CHAMPION.C F0324 after a party pit fall with attack 20,
     * MASK0x0010_WOUND_LEGS | MASK0x0020_WOUND_FEET, and C2_ATTACK_SELF.
     * CHAMPION.C F0324 lines 1991-2022 randomizes attack by +/- 1/8 for
     * each champion before F0321 scaling/wound selection.  This bounded
     * CSB bridge uses deterministic local RNG until the runtime owns the
     * original global RNG stream. */
    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; ++i) {
        CSB_V1_Champion *champion = &profile->party_state.Champions[i];
        struct CombatantChampionSnapshot_Compat defender;
        int randomized_attack;
        int scaled_attack = 0;
        int selected_wounds = 0;

        if (champion->CurrentHealth <= 0 ||
            (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
            continue;
        }

        randomized_attack = base_attack +
            F0732_COMBAT_RngRandom_Compat(&rng, random_window);
        if (randomized_attack < 1) randomized_attack = 1;

        if (!csb_v1_runtime_fill_defender_combat_snapshot(
                profile,
                i,
                &defender) ||
            !F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
                COMBAT_ATTACK_SELF,
                randomized_attack,
                COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET,
                &defender,
                &rng,
                &scaled_attack,
                NULL) ||
            scaled_attack <= 0) {
            continue;
        }

        if (!F0739c_COMBAT_SelectChampionWoundsF0321Rng_Compat(
                scaled_attack,
                COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET,
                &defender,
                &rng,
                &selected_wounds,
                NULL)) {
            selected_wounds = 0;
        }

        champion->Wounds = (uint16_t)(champion->Wounds |
                                      (uint16_t)selected_wounds);
        wound_mask |= selected_wounds;
        if (scaled_attack >= champion->CurrentHealth) {
            total_damage += champion->CurrentHealth;
            champion->CurrentHealth = 0;
            csb_v1_runtime_mark_champion_dead(profile, i);
        } else {
            champion->CurrentHealth =
                (int16_t)(champion->CurrentHealth - scaled_attack);
            total_damage += scaled_attack;
        }
        damaged_count++;
    }

    if (result) {
        result->pit_fall_damaged_champion_count += damaged_count;
        result->pit_fall_total_damage += total_damage;
        result->pit_fall_wound_mask |= wound_mask;
    }
    return damaged_count;
}

static void csb_v1_runtime_apply_destination_pit(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int target_level;
    int target_x;
    int target_y;

    if (!profile || !result || !result->movement_step_applied ||
        result->teleporter_transition_applied ||
        result->stair_transition_applied) {
        return;
    }
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) return;

    level = csb_v1_runtime_sample_destination_square(profile, result);
    if (level < 0) return;
    raw_square = result->movement_destination_raw_square;
    if (result->movement_destination_square_type != 2) return;
    result->pit_open = (raw_square & 0x08) ? 1 : 0;
    if (!result->pit_open || (raw_square & 0x01)) return;

    /* ReDMCSB: MOVESENS.C F0267 lines 538-603 handles an open,
     * non-imaginary C02_ELEMENT_PIT by calling DUNGEON.C F0154 for a
     * downward map transition, then applies F0324 party fall damage.
     * Rope, sounds, and view redraw timing stay separate. */
    target_x = result->movement_destination_x;
    target_y = result->movement_destination_y;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (!csb_v1_runtime_location_after_level_change(
            dungeon,
            level,
            1,
            &target_x,
            &target_y,
            &target_level)) {
        return;
    }

    profile->current_level = target_level;
    profile->party_x = target_x;
    profile->party_y = target_y;
    profile->party_state.PartyMapX = target_x;
    profile->party_state.PartyMapY = target_y;
    csb_v1_dungeon_set_current_level(target_level);
    result->new_party_level = target_level;
    result->pit_fall_applied = 1;
    (void)csb_v1_runtime_apply_party_fall_damage(
        profile,
        result,
        target_level,
        target_x,
        target_y);
}

static void csb_v1_runtime_copy_first_teleporter_result(
    CSB_V1_InputCommandRuntimeResult *result,
    const CSB_V1_InputCommandRuntimeResult *step)
{
    if (!result || !step) return;
    if (!result->teleporter_transition_applied) {
        result->teleporter_open = step->teleporter_open;
        result->teleporter_scope = step->teleporter_scope;
        result->teleporter_absolute_rotation = step->teleporter_absolute_rotation;
        result->teleporter_rotation = step->teleporter_rotation;
        result->teleporter_audible = step->teleporter_audible;
        result->teleporter_target_x = step->teleporter_target_x;
        result->teleporter_target_y = step->teleporter_target_y;
        result->teleporter_target_level = step->teleporter_target_level;
    }
    result->teleporter_transition_applied = 1;
    result->teleporter_chain_count++;
}

static void csb_v1_runtime_copy_first_pit_result(
    CSB_V1_InputCommandRuntimeResult *result,
    const CSB_V1_InputCommandRuntimeResult *step)
{
    if (!result || !step) return;
    if (!result->pit_fall_applied) {
        result->pit_open = step->pit_open;
    }
    result->pit_fall_applied = 1;
    result->pit_chain_count++;
    result->pit_fall_damaged_champion_count +=
        step->pit_fall_damaged_champion_count;
    result->pit_fall_total_damage += step->pit_fall_total_damage;
    result->pit_fall_wound_mask |= step->pit_fall_wound_mask;
}

static void csb_v1_runtime_apply_destination_chain(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    int i;

    if (!profile || !result || !result->movement_step_applied) return;

    /* ReDMCSB: MOVESENS.C F0267 lines 468-574 repeats teleporter/pit
     * consequences in one move result.  PC34/I34E MEDIA529 caps chained
     * moves at 100, covering teleporter-to-teleporter and pit-series
     * routes while avoiding infinite self-feeding dungeon setups.  This
     * party-only runtime bridge keeps object/group/projectile movement,
     * audio, fall damage, rope, and redraw timing as separate work. */
    for (i = 0; i < 100; ++i) {
        CSB_V1_InputCommandRuntimeResult step;
        int before_x;
        int before_y;
        int before_level;
        int self_target_teleporter;

        memset(&step, 0, sizeof(step));
        step.movement_step_attempted = 1;
        step.movement_step_applied = 1;
        step.movement_destination_x = profile->party_x;
        step.movement_destination_y = profile->party_y;
        step.old_party_level = profile->current_level;
        step.new_party_level = profile->current_level;
        if (csb_v1_runtime_sample_destination_square(profile, &step) < 0) {
            break;
        }

        before_x = profile->party_x;
        before_y = profile->party_y;
        before_level = profile->current_level;
        if (step.movement_destination_square_type == 5) {
            csb_v1_runtime_apply_destination_teleporter(profile, &step);
            if (!step.teleporter_transition_applied) {
                if (!result->teleporter_open) {
                    result->teleporter_open = step.teleporter_open;
                }
                break;
            }
            self_target_teleporter =
                step.teleporter_target_x == before_x &&
                step.teleporter_target_y == before_y &&
                step.teleporter_target_level == before_level;
            csb_v1_runtime_copy_first_teleporter_result(result, &step);
            result->chained_move_count++;
            result->new_party_level = profile->current_level;
            if (self_target_teleporter) {
                break;
            }
            continue;
        }

        if (step.movement_destination_square_type == 2) {
            csb_v1_runtime_apply_destination_pit(profile, &step);
            if (!step.pit_fall_applied) {
                if (!result->pit_open) {
                    result->pit_open = step.pit_open;
                }
                break;
            }
            csb_v1_runtime_copy_first_pit_result(result, &step);
            result->chained_move_count++;
            result->new_party_level = profile->current_level;
            continue;
        }

        break;
    }
    if (i >= 100) {
        result->chained_move_limit_hit = 1;
    }
}

static int csb_v1_runtime_square_event_type_for_sensor_target(int square_type)
{
    static const int event_type_by_square_type[7] = {
        DM1_EVENT_WALL,
        DM1_EVENT_CORRIDOR,
        DM1_EVENT_PIT,
        DM1_EVENT_NONE,
        DM1_EVENT_DOOR,
        DM1_EVENT_TELEPORTER,
        DM1_EVENT_FAKEWALL
    };
    if (square_type < 0 || square_type >= 7) return DM1_EVENT_NONE;
    return event_type_by_square_type[square_type];
}

static int csb_v1_runtime_sensor_next_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing)
{
    const uint8_t *record;
    int thing_type;
    int thing_size;

    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_size < 2) return 0xFFFE;
    return (int)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
}

static uint16_t csb_v1_runtime_read_u16(const uint8_t *p)
{
    if (!p) return 0;
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void csb_v1_runtime_write_u16(uint8_t *p, uint16_t value)
{
    if (!p) return;
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static void csb_v1_runtime_decode_sensor_words(
    uint16_t next_word,
    uint16_t type_data,
    uint16_t flags_word,
    uint16_t target_word,
    struct DungeonSensor_Compat *out_sensor)
{
    if (!out_sensor) return;
    memset(out_sensor, 0, sizeof(*out_sensor));
    out_sensor->next = next_word;
    out_sensor->sensorType = (unsigned char)(type_data & 0x007Fu);
    out_sensor->sensorData = (unsigned short)(type_data >> 7);
    out_sensor->onceOnly = (unsigned char)((flags_word >> 2) & 0x01u);
    out_sensor->effect = (unsigned char)((flags_word >> 3) & 0x03u);
    out_sensor->revertEffect = (unsigned char)((flags_word >> 5) & 0x01u);
    out_sensor->audible = (unsigned char)((flags_word >> 6) & 0x01u);
    out_sensor->value = (unsigned char)((flags_word >> 7) & 0x0Fu);
    out_sensor->localEffect = (unsigned char)((flags_word >> 11) & 0x01u);
    out_sensor->ornamentOrdinal = (unsigned char)((flags_word >> 12) & 0x0Fu);
    out_sensor->targetCell = (unsigned char)((target_word >> 4) & 0x03u);
    out_sensor->targetMapX = (unsigned char)((target_word >> 6) & 0x1Fu);
    out_sensor->targetMapY = (unsigned char)((target_word >> 11) & 0x1Fu);
    out_sensor->localMultiple = (unsigned short)(target_word & 0x0FFFu);
}

static int csb_v1_runtime_projectile_subtype_from_explosion_thing(
    uint16_t associated_thing)
{
    unsigned int explosion_type;
    if (associated_thing < DM1_THING_FIRST_EXPLOSION) {
        return PROJECTILE_SUBTYPE_FIREBALL;
    }
    explosion_type = (unsigned int)(associated_thing - DM1_THING_FIRST_EXPLOSION);
    switch (explosion_type) {
    case C000_EXPLOSION_FIREBALL:
        return PROJECTILE_SUBTYPE_FIREBALL;
    case C001_EXPLOSION_SLIME:
        return PROJECTILE_SUBTYPE_SLIME;
    case C002_EXPLOSION_LIGHTNING_BOLT:
        return PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    case C003_EXPLOSION_HARM_NON_MATERIAL:
        return PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    case C004_EXPLOSION_OPEN_DOOR:
        return PROJECTILE_SUBTYPE_OPEN_DOOR;
    case C007_EXPLOSION_POISON_CLOUD:
        return PROJECTILE_SUBTYPE_POISON_CLOUD;
    default:
        return PROJECTILE_SUBTYPE_FIREBALL;
    }
}

static int csb_v1_runtime_projectile_attack_type_from_subtype(int subtype)
{
    switch (subtype) {
    case PROJECTILE_SUBTYPE_FIREBALL:
        return COMBAT_ATTACK_FIRE;
    case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:
        return COMBAT_ATTACK_LIGHTNING;
    case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
    case PROJECTILE_SUBTYPE_OPEN_DOOR:
        return COMBAT_ATTACK_MAGIC;
    default:
        return COMBAT_ATTACK_NORMAL;
    }
}

static int csb_v1_runtime_sensor_type_is_explosion_launcher(int sensor_type)
{
    return sensor_type == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION ||
           sensor_type == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION;
}

static int csb_v1_runtime_sensor_type_is_square_object_launcher(int sensor_type)
{
    return sensor_type == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ ||
           sensor_type == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ;
}

static int csb_v1_runtime_sensor_type_is_new_object_launcher(int sensor_type)
{
    return sensor_type == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ ||
           sensor_type == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ;
}

static uint8_t *csb_v1_runtime_mutable_thing_record(
    CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int *out_type,
    int *out_size)
{
    const uint8_t *record;

    if (out_type) *out_type = -1;
    if (out_size) *out_size = 0;
    if (!dungeon || !dungeon->raw_data) return NULL;
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        out_type,
        NULL,
        out_size);
    if (!record) return NULL;
    return dungeon->raw_data + (record - (const uint8_t *)dungeon->raw_data);
}

static uint8_t *csb_v1_runtime_square_first_thing_ptr(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int i;
    int column_index = 0;
    int column_counts_base;
    int thing_index;
    int thing_offset;
    int square_offset;

    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1) return NULL;
    if (level < 0 || level >= dungeon->level_count) return NULL;
    if (map_x < 0 || map_x >= dungeon->level_widths[level] ||
        map_y < 0 || map_y >= dungeon->level_heights[level]) {
        return NULL;
    }
    square_offset = dungeon->level_offsets[level] +
                    map_x * dungeon->level_heights[level] +
                    map_y;
    if (square_offset < 0 || square_offset >= dungeon->raw_size) return NULL;
    if ((dungeon->raw_data[square_offset] & 0x10u) == 0u) return NULL;

    column_counts_base = 44 + dungeon->level_count * 16;
    for (i = 0; i < level; ++i) {
        column_index += dungeon->level_widths[i];
    }
    column_counts_base += (column_index + map_x) * 2;
    if (column_counts_base + 2 > dungeon->raw_size) return NULL;
    thing_index = (int)csb_v1_runtime_read_u16(
        dungeon->raw_data + column_counts_base);
    for (i = 0; i < map_y; ++i) {
        if (dungeon->raw_data[dungeon->level_offsets[level] +
                              map_x * dungeon->level_heights[level] + i] &
            0x10u) {
            thing_index++;
        }
    }
    if (thing_index < 0 || thing_index >= dungeon->square_first_thing_count) {
        return NULL;
    }
    thing_offset = dungeon->square_first_thing_base + thing_index * 2;
    if (thing_offset + 2 > dungeon->raw_size) return NULL;
    return dungeon->raw_data + thing_offset;
}

static uint8_t *csb_v1_runtime_create_square_first_thing_ptr(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    uint16_t first_thing)
{
    int i;
    int global_column_index = 0;
    int total_columns = 0;
    int column_counts_base;
    int insertion_index;
    int square_offset;
    int insert_offset;
    int last_offset;
    int move_bytes;

    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1) return NULL;
    if (level < 0 || level >= dungeon->level_count) return NULL;
    if (map_x < 0 || map_x >= dungeon->level_widths[level] ||
        map_y < 0 || map_y >= dungeon->level_heights[level]) {
        return NULL;
    }
    if (dungeon->square_first_thing_count <= 0) return NULL;
    if (dungeon->square_first_thing_base < 0 ||
        dungeon->square_first_thing_base +
            dungeon->square_first_thing_count * 2 > dungeon->raw_size) {
        return NULL;
    }

    square_offset = dungeon->level_offsets[level] +
                    map_x * dungeon->level_heights[level] +
                    map_y;
    if (square_offset < 0 || square_offset >= dungeon->raw_size) return NULL;
    if ((dungeon->raw_data[square_offset] & 0x10u) != 0u) {
        return csb_v1_runtime_square_first_thing_ptr(
            dungeon,
            level,
            map_x,
            map_y);
    }

    last_offset = dungeon->square_first_thing_base +
                  (dungeon->square_first_thing_count - 1) * 2;
    if (csb_v1_runtime_read_u16(dungeon->raw_data + last_offset) != 0xFFFFu) {
        return NULL;
    }

    for (i = 0; i < dungeon->level_count; ++i) {
        if (i < level) global_column_index += dungeon->level_widths[i];
        total_columns += dungeon->level_widths[i];
    }
    global_column_index += map_x;
    column_counts_base = 44 + dungeon->level_count * 16;
    if (column_counts_base + total_columns * 2 > dungeon->raw_size) {
        return NULL;
    }

    insertion_index = (int)csb_v1_runtime_read_u16(
        dungeon->raw_data + column_counts_base + global_column_index * 2);
    for (i = 0; i < map_y; ++i) {
        if (dungeon->raw_data[dungeon->level_offsets[level] +
                              map_x * dungeon->level_heights[level] + i] &
            0x10u) {
            insertion_index++;
        }
    }
    if (insertion_index < 0 ||
        insertion_index >= dungeon->square_first_thing_count) {
        return NULL;
    }

    insert_offset = dungeon->square_first_thing_base + insertion_index * 2;
    move_bytes = (dungeon->square_first_thing_count -
                  insertion_index - 1) * 2;
    if (move_bytes > 0) {
        memmove(dungeon->raw_data + insert_offset + 2,
                dungeon->raw_data + insert_offset,
                (size_t)move_bytes);
    }
    csb_v1_runtime_write_u16(dungeon->raw_data + insert_offset, first_thing);
    dungeon->raw_data[square_offset] |= 0x10u;
    for (i = global_column_index + 1; i < total_columns; ++i) {
        uint8_t *count_ptr = dungeon->raw_data + column_counts_base + i * 2;
        uint16_t count = csb_v1_runtime_read_u16(count_ptr);
        csb_v1_runtime_write_u16(count_ptr, (uint16_t)(count + 1u));
    }
    /* ReDMCSB DUNGEON.C F0163 lines 1790-1829 creates a new square
     * first-thing slot by setting MASK0x0010_THING_LIST_PRESENT, inserting
     * into G0283_pT_SquareFirstThings, and incrementing later cumulative
     * column counters.  The original relies on preallocated free slots and
     * BUG0_08 if none remain; this bridge refuses insertion unless the last
     * slot is THING_NONE. */
    return dungeon->raw_data + insert_offset;
}

static int csb_v1_runtime_find_unused_group_record(
    CSB_V1_DungeonData *dungeon,
    uint8_t **out_record,
    int *out_index)
{
    int i;

    if (out_record) *out_record = NULL;
    if (out_index) *out_index = -1;
    if (!dungeon || !dungeon->raw_data) return 0;
    for (i = 0; i < dungeon->thing_type_counts[4]; ++i) {
        int offset = dungeon->thing_data_bases[4] + i * 16;
        if (offset < 0 || offset + 16 > dungeon->raw_size) return 0;
        if (csb_v1_runtime_read_u16(dungeon->raw_data + offset) == 0xFFFFu) {
            if (out_record) *out_record = dungeon->raw_data + offset;
            if (out_index) *out_index = i;
            return 1;
        }
    }
    return 0;
}

static int csb_v1_runtime_creature_movement_ticks(int creature_type)
{
    static const unsigned char movement_ticks[27] = {
        8, 15, 3, 10, 9, 20, 120, 185, 11,
        21, 17, 255, 7, 5, 10, 18, 13, 1,
        6, 10, 255, 17, 15, 10, 60, 10, 10
    };
    if (creature_type < 0 || creature_type >= 27) return 255;
    return (int)movement_ticks[creature_type];
}

static int csb_v1_runtime_creature_attack_ticks(int creature_type)
{
    static const unsigned char attack_ticks[27] = {
        20, 32, 5, 21, 8, 18, 10, 15, 16,
        14, 12, 8, 7, 10, 20, 19, 8, 16,
        6, 18, 25, 15, 14, 22, 28, 22, 22
    };
    if (creature_type < 0 || creature_type >= 27) return 1;
    return (int)attack_ticks[creature_type];
}

static int csb_v1_runtime_creature_attack_sound_index(int creature_type)
{
    static const signed char attack_sound_ordinal[27] = {
        4, 0, 6, 0, 1, 0, 3, 7, 2,
        10, 2, 0, 11, 9, 0, 5, 10, 0,
        11, 0, 8, 3, 0, 0, 1, 0, 0
    };
    static const signed char creature_sounds_attack[18] = {
        23, 25, 19, 20, 21, 22, 24, 26, 27,
        CSB_V1_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM,
        CSB_V1_SOUND_COMBAT, CSB_V1_SOUND_COMBAT, 25, -1, -1, -1, -1, 23
    };
    int ordinal;

    if (creature_type < 0 || creature_type >= 27) return CSB_V1_SOUND_NONE;
    ordinal = (int)attack_sound_ordinal[creature_type];
    if (ordinal <= 0 || ordinal > 18) return CSB_V1_SOUND_NONE;
    return (int)creature_sounds_attack[ordinal - 1];
}

static int csb_v1_runtime_creature_movement_sound_index(int creature_type)
{
    static const signed char attack_sound_ordinal[27] = {
        4, 0, 6, 0, 1, 0, 3, 7, 2,
        10, 2, 0, 11, 9, 0, 5, 10, 0,
        11, 0, 8, 3, 0, 0, 1, 0, 0
    };
    static const signed char creature_sounds_movement[18] = {
        CSB_V1_SOUND_MOVE_RED_DRAGON, -1,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SWAMP_SLIME_WATER_ELEMENTAL,
        CSB_V1_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SKELETON,
        CSB_V1_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SWAMP_SLIME_WATER_ELEMENTAL,
        CSB_V1_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER,
        CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU,
        CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU
    };
    int ordinal;

    if (creature_type < 0 || creature_type >= 27) return CSB_V1_SOUND_NONE;
    ordinal = (int)attack_sound_ordinal[creature_type];
    if (ordinal <= 0 || ordinal > 18) return CSB_V1_SOUND_NONE;
    return (int)creature_sounds_movement[ordinal - 1];
}

static void csb_v1_runtime_request_creature_movement_sound(
    CSB_V1_RuntimeProfile *profile,
    int creature_type,
    int map_x,
    int map_y)
{
    CsbV1AudioRequest request;
    int sound_index;

    if (!profile) return;
    sound_index = csb_v1_runtime_creature_movement_sound_index(creature_type);
    if (sound_index == CSB_V1_SOUND_NONE) return;

    memset(&request, 0, sizeof(request));
    request.soundIndex = (int16_t)sound_index;
    request.mapX = (int16_t)map_x;
    request.mapY = (int16_t)map_y;
    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
    request.volume = 64;
    request.priority = 4u;
    /* ReDMCSB MOVESENS.C F0267 lines 847-853 calls F0514, which maps
     * CreatureInfo.AttackSoundOrdinal through DUNGEON.C
     * G2003_aauc_CreatureSounds[][C1_MOVEMENT_SOUND] and requests
     * SOUND.C F0064 with C01_MODE_PLAY_IF_PRIORITIZED after a group move. */
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
}

static void csb_v1_runtime_request_creature_attack_sound(
    CSB_V1_RuntimeProfile *profile,
    int creature_type,
    int map_x,
    int map_y)
{
    CsbV1AudioRequest request;
    int sound_index;

    if (!profile) return;
    sound_index = csb_v1_runtime_creature_attack_sound_index(creature_type);
    if (sound_index == CSB_V1_SOUND_NONE) return;

    memset(&request, 0, sizeof(request));
    request.soundIndex = (int16_t)sound_index;
    request.mapX = (int16_t)map_x;
    request.mapY = (int16_t)map_y;
    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
    request.volume = 64;
    request.priority = 6u;
    /* ReDMCSB GROUP.C F0207 lines 1807-1808 maps
     * CreatureInfo.AttackSoundOrdinal through DUNGEON.C
     * G2003_aauc_CreatureSounds[][C0_ATTACK_SOUND], then requests
     * SOUND.C F0064 with C01_MODE_PLAY_IF_PRIORITIZED. */
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
}

static void csb_v1_runtime_request_buzz_sound(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y)
{
    CsbV1AudioRequest request;

    if (!profile) return;
    memset(&request, 0, sizeof(request));
    request.soundIndex = CSB_V1_SOUND_BUZZ;
    request.mapX = (int16_t)map_x;
    request.mapY = (int16_t)map_y;
    request.mode = CSB_V1_MODE_PLAY_IF_PRIORITIZED;
    request.volume = 64;
    request.priority = 4u;
    /* ReDMCSB GROUP.C F0209 line 2283 requests M560_SOUND_BUZZ at the
     * archenemy double-move destination with C01_MODE_PLAY_IF_PRIORITIZED. */
    (void)csb_v1_audio_runtime_request(&profile->audio_runtime, &request);
}

static int csb_v1_runtime_direction_from_source_to_destination(
    int source_x,
    int source_y,
    int dest_x,
    int dest_y);
static void csb_v1_runtime_set_active_group_aspect_attacking(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    int attacking);
static void csb_v1_runtime_compact_active_group_state_after_kill(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_index,
    int creature_count);
static void csb_v1_runtime_set_active_group_direction_all(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction);
static void csb_v1_runtime_set_active_group_direction_creature(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_index,
    int creature_count,
    int two_half_square_creatures);
static void csb_v1_runtime_set_active_group_direction_group(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size);
static void csb_v1_runtime_turn_active_group_toward_attack(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size);

static void csb_v1_runtime_schedule_c37_group_event(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    uint32_t delay_ticks)
{
    struct DM1_Event_V1 event;
    int movement_ticks;

    if (!profile || delay_ticks == 0u) return;
    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        map_index,
        profile->game_time + delay_ticks);
    event.type = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    event.priority = (uint8_t)(255 - movement_ticks);
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_schedule_c38_attack_event(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    uint32_t delay_ticks)
{
    struct DM1_Event_V1 event;
    int movement_ticks;

    if (!profile || delay_ticks == 0u || creature_index < 0 ||
        creature_index > 3) {
        return;
    }
    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        map_index,
        profile->game_time + delay_ticks);
    event.type = (uint8_t)(DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 +
                           creature_index);
    event.priority = (uint8_t)(255 - movement_ticks);
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_schedule_c38_followup_event(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    uint32_t attack_delay_ticks)
{
    struct DM1_Event_V1 event;
    uint32_t attack_time;
    uint32_t aspect_time;
    int movement_ticks;

    if (!profile || creature_index < 0 || creature_index > 3 ||
        attack_delay_ticks == 0u) {
        return;
    }
    attack_time = profile->game_time + attack_delay_ticks;
    aspect_time = profile->game_time + 1u;
    if (aspect_time >= attack_time) {
        csb_v1_runtime_schedule_c38_attack_event(
            profile,
            map_index,
            map_x,
            map_y,
            creature_type,
            creature_index,
            attack_delay_ticks);
        return;
    }

    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(map_index, aspect_time);
    event.type = (uint8_t)(DM1_EVENT_UPDATE_ASPECT_CREATURE_0 +
                           creature_index);
    event.priority = (uint8_t)(255 - movement_ticks);
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    event.c_effect = (uint8_t)((attack_time - aspect_time) & 0xffu);
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_apply_creature_aspect_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int creature_index;
    int remaining_ticks;
    int thing;
    int guard;

    if (!profile || !record || !profile->dungeon_handle) return;
    creature_index = record->eventType - DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    if (creature_index < 0 || creature_index > 3) return;
    remaining_ticks = record->effect & 0xff;
    if (remaining_ticks <= 0) remaining_ticks = 1;

    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *thing_record;
        uint16_t flags;
        int thing_type;
        int thing_size;

        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 4 && thing_size >= 16) {
            flags = csb_v1_runtime_read_u16(thing_record + 14);
            if ((flags & 0x000Fu) != 6u) return;
            if (creature_index > (int)((flags >> 5) & 0x03u)) return;

            /* ReDMCSB GROUP.C F0209 lines 2075-2148 handles C33..C36
             * aspect events by preparing the matching C38..C41 behavior
             * event.  F0208 lines 1820-1834 stores the remaining behavior
             * delay in C.Ticks; Firestaff's V1 queue carries it in
             * c_effect/record->effect.  Mirror the native ActiveGroup
             * attack-bit transition here; broader flip/offset RNG remains a
             * later sprite-exact slice. */
            csb_v1_runtime_set_active_group_aspect_attacking(
                profile,
                (uint16_t)thing,
                record->mapIndex,
                record->mapX,
                record->mapY,
                (int)thing_record[4],
                creature_index,
                0);
            csb_v1_runtime_schedule_c38_attack_event(
                profile,
                record->mapIndex,
                record->mapX,
                record->mapY,
                (int)thing_record[4],
                creature_index,
                (uint32_t)remaining_ticks);
            return;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_schedule_c38_attack_events(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_type,
    uint16_t group_flags)
{
    int count_index;
    int movement_ticks;

    if (!profile) return;
    movement_ticks = csb_v1_runtime_creature_movement_ticks(creature_type);
    count_index = (int)((group_flags >> 5) & 0x03u);
    for (; count_index >= 0; --count_index) {
        struct DM1_Event_V1 event;

        /* ReDMCSB GROUP.C F0209 lines 2108-2127 switches to C6 attack
         * and queues C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + creature index
         * for each creature in the group, reusing the group event priority
         * initialized as 255 - MovementTicks.  This bounded CSB bridge only
         * schedules those per-creature attack events; the C38 damage/evasion
         * body remains a later runtime slice. */
        memset(&event, 0, sizeof(event));
        event.map_time = DM1_MAP_TIME_MAKE(map_index, profile->game_time + 1u);
        event.type = (uint8_t)(DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 +
                               count_index);
        event.priority = (uint8_t)(255 - movement_ticks);
        event.b_mapX = (uint8_t)map_x;
        event.b_mapY = (uint8_t)map_y;
        (void)dm1v1_event_add(&profile->timeline_queue, &event);
    }
}

static int csb_v1_runtime_group_destination_is_blocked(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int raw_square;
    int square_type;
    int door_state;

    if (!dungeon || !dungeon->raw_data || level < 0 ||
        level >= dungeon->level_count) {
        return 1;
    }
    raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, map_x, map_y);
    if (raw_square < 0) return 1;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    if (square_type == 0) return 1;
    if (square_type == 4) {
        door_state = raw_square & 0x07;
        return door_state != 0 && door_state != 1 && door_state != 5;
    }
    if (square_type == 6) {
        return !(raw_square & 0x04) && !(raw_square & 0x01);
    }
    return 0;
}

static int csb_v1_runtime_find_group_thing_location(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int *out_level,
    int *out_x,
    int *out_y)
{
    int level;
    int x;
    int y;

    if (out_level) *out_level = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (!dungeon || !dungeon->raw_data) return 0;
    for (level = 0; level < dungeon->level_count; ++level) {
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int thing = csb_v1_dungeon_get_first_thing(
                    dungeon,
                    level,
                    x,
                    y);
                int guard;

                for (guard = 0;
                     guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
                     ++guard) {
                    const uint8_t *record;
                    int thing_type = -1;
                    int thing_size = 0;

                    if ((uint16_t)thing == group_thing) {
                        if (out_level) *out_level = level;
                        if (out_x) *out_x = x;
                        if (out_y) *out_y = y;
                        return 1;
                    }
                    record = csb_v1_dungeon_get_thing_record(
                        dungeon,
                        (uint16_t)thing,
                        &thing_type,
                        NULL,
                        &thing_size);
                    if (!record || thing_size < 2) break;
                    thing = (int)csb_v1_runtime_read_u16(record);
                }
            }
        }
    }
    return 0;
}

static int csb_v1_runtime_square_has_group(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    int thing;
    int guard;

    if (!dungeon || !dungeon->raw_data) return 0;
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        int thing_type = -1;
        int thing_size = 0;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 4) return 1;
        thing = (int)csb_v1_runtime_read_u16(record);
    }
    return 0;
}

static int csb_v1_runtime_group_destination_has_party_or_group(
    const CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y)
{
    if (!profile || !profile->dungeon_handle) return 1;
    if (profile->current_level == level &&
        profile->party_x == map_x &&
        profile->party_y == map_y &&
        profile->champion_count > 0) {
        return 1;
    }
    return csb_v1_runtime_square_has_group(
        profile->dungeon_handle,
        level,
        map_x,
        map_y);
}

static void csb_v1_runtime_schedule_move_group_event(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int target_level,
    int target_x,
    int target_y,
    int audible)
{
    struct DM1_Event_V1 event;

    if (!profile) return;
    if (target_level < 0 || target_level > 255 ||
        target_x < 0 || target_x > 255 ||
        target_y < 0 || target_y > 255) {
        return;
    }
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        target_level,
        profile->game_time + 5u);
    event.type = (uint8_t)(audible
        ? DM1_EVENT_MOVE_GROUP_AUDIBLE
        : DM1_EVENT_MOVE_GROUP_SILENT);
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)(group_thing & 0xFFu);
    event.c_effect = (uint8_t)((group_thing >> 8) & 0xFFu);
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static int csb_v1_runtime_move_group_thing_to_square(
    CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int old_level,
    int old_x,
    int old_y,
    int new_level,
    int new_x,
    int new_y)
{
    uint8_t *source_first_ptr;
    uint8_t *dest_first_ptr;
    uint8_t *group_record;
    uint8_t *previous_record;
    uint16_t thing;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon ||
        (old_level == new_level && old_x == new_x && old_y == new_y)) {
        return 0;
    }
    source_first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        old_level,
        old_x,
        old_y);
    if (!source_first_ptr) return 0;

    previous_record = NULL;
    thing = csb_v1_runtime_read_u16(source_first_ptr);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(group_record);
        if (thing == group_thing && thing_type == 4 && thing_size >= 16) {
            if (previous_record) {
                csb_v1_runtime_write_u16(previous_record, next_thing);
            } else {
                csb_v1_runtime_write_u16(source_first_ptr, next_thing);
            }
            dest_first_ptr = csb_v1_runtime_square_first_thing_ptr(
                dungeon,
                new_level,
                new_x,
                new_y);
            if (dest_first_ptr) {
                csb_v1_runtime_write_u16(
                    group_record,
                    csb_v1_runtime_read_u16(dest_first_ptr));
                csb_v1_runtime_write_u16(dest_first_ptr, group_thing);
                return 1;
            }
            dest_first_ptr = csb_v1_runtime_create_square_first_thing_ptr(
                dungeon,
                new_level,
                new_x,
                new_y,
                group_thing);
            if (!dest_first_ptr) {
                if (previous_record) {
                    csb_v1_runtime_write_u16(previous_record, group_thing);
                } else {
                    csb_v1_runtime_write_u16(source_first_ptr, group_thing);
                }
                csb_v1_runtime_write_u16(group_record, next_thing);
                return 0;
            }
            /* ReDMCSB: MOVESENS.C F0267 lines 858-867 moves C04 groups by
             * relinking with DUNGEON.C F0163. F0163 lines 1804-1829 creates
             * a square-first entry when the destination square has no thing
             * list. Firestaff keeps the original preallocated-slot contract
             * bounded by refusing insertion when no trailing THING_NONE slot
             * exists. */
            csb_v1_runtime_write_u16(group_record, 0xFFFEu);
            return 1;
        }
        previous_record = group_record;
        thing = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_apply_group_fall_damage(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y);
static void csb_v1_runtime_sync_active_group_state_from_record(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    const uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int preserve_home,
    int moved);

static uint16_t csb_v1_runtime_repeated_group_direction_pack(int direction)
{
    int d = direction & 3;
    return (uint16_t)(d | (d << 2) | (d << 4) | (d << 6));
}

static int csb_v1_runtime_decode_group_teleporter_at_square(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    int raw_square,
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 *out_teleporter,
    int *out_scope)
{
    int thing;
    int guard;

    if (out_scope) *out_scope = 0;
    if (!dungeon || !out_teleporter) return -1;
    if (((raw_square >> 5) & 0x07) != 5) return -1;
    if ((raw_square & 0x08) == 0) return 0;

    thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    if (thing < 0) return 0;

    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        const uint8_t *record;
        int thing_type = -1;
        int thing_size = 0;
        uint16_t word;
        uint16_t target_word;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 1 && thing_size >= 6) {
            word = csb_v1_runtime_read_u16(record + 2);
            target_word = csb_v1_runtime_read_u16(record + 4);
            memset(out_teleporter, 0, sizeof(*out_teleporter));
            out_teleporter->target_map_x = (int)(word & 0x1Fu);
            out_teleporter->target_map_y = (int)((word >> 5) & 0x1Fu);
            out_teleporter->rotation = (int)((word >> 10) & 0x03u);
            out_teleporter->absolute_rotation = (word & 0x1000u) ? 1 : 0;
            if (out_scope) *out_scope = (int)((word >> 13) & 0x03u);
            out_teleporter->audible = (word & 0x8000u) ? 1 : 0;
            out_teleporter->target_map_index =
                (int)((target_word >> 8) & 0xFFu);
            return 1;
        }
        thing = (int)csb_v1_runtime_read_u16(record);
    }
    return 0;
}

static int csb_v1_runtime_apply_group_consequences_at_square(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int *inout_map_index,
    int *inout_map_x,
    int *inout_map_y,
    int *out_group_alive)
{
    CSB_V1_DungeonData *dungeon;
    int moved_count = 0;
    int chain_guard;

    if (!profile || !profile->dungeon_handle || !inout_map_index ||
        !inout_map_x || !inout_map_y) {
        return 0;
    }
    if (out_group_alive) *out_group_alive = 1;
    dungeon = profile->dungeon_handle;
    for (chain_guard = 0; chain_guard < 100; ++chain_guard) {
        CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
        CSB_V1_TeleporterRotationRuntimeGroupPc34 group;
        CSB_V1_TeleporterRotationRuntimeGroupResultPc34 result;
        uint8_t *group_record;
        const struct CreatureBehaviorProfile_Compat *creature_profile;
        int raw_square;
        int square_type;
        int scope = 0;
        int thing_type = -1;
        int thing_size = 0;
        int creature_type;
        int current_map_index;
        uint16_t flags;
        int direction;

        current_map_index = *inout_map_index;
        if (current_map_index < 0 ||
            current_map_index >= dungeon->level_count) {
            break;
        }
        raw_square = csb_v1_dungeon_get_raw_square(
            dungeon,
            current_map_index,
            *inout_map_x,
            *inout_map_y);
        if (raw_square < 0) {
            break;
        }
        square_type = (raw_square >> 5) & 0x07;
        if (square_type == 2) {
            int target_map_index;
            int target_x = *inout_map_x;
            int target_y = *inout_map_y;

            if ((raw_square & 0x08) == 0 ||
                (raw_square & 0x01) != 0 ||
                !csb_v1_runtime_location_after_level_change(
                    dungeon,
                    current_map_index,
                    1,
                    &target_x,
                    &target_y,
                    &target_map_index)) {
                break;
            }
            if (csb_v1_runtime_group_destination_has_party_or_group(
                    profile,
                    target_map_index,
                    target_x,
                    target_y)) {
                csb_v1_runtime_schedule_move_group_event(
                    profile,
                    group_thing,
                    target_map_index,
                    target_x,
                    target_y,
                    0);
                break;
            }
            if (!csb_v1_runtime_move_group_thing_to_square(
                    dungeon,
                    group_thing,
                    current_map_index,
                    *inout_map_x,
                    *inout_map_y,
                    target_map_index,
                    target_x,
                    target_y)) {
                break;
            }
            *inout_map_index = target_map_index;
            *inout_map_x = target_x;
            *inout_map_y = target_y;
            moved_count++;
            if (csb_v1_runtime_apply_group_fall_damage(
                    profile,
                    group_thing,
                    target_map_index,
                    target_x,
                    target_y) == 2) {
                if (out_group_alive) *out_group_alive = 0;
                break;
            }
            continue;
        }
        if (square_type != 5 || (raw_square & 0x08) == 0) break;
        if (csb_v1_runtime_decode_group_teleporter_at_square(
                dungeon,
                current_map_index,
                *inout_map_x,
                *inout_map_y,
                raw_square,
                &teleporter,
                &scope) <= 0 ||
            (scope & 0x01) == 0) {
            break;
        }
        if (teleporter.target_map_index == current_map_index &&
            teleporter.target_map_x == *inout_map_x &&
            teleporter.target_map_y == *inout_map_y) {
            break;
        }
        if (teleporter.target_map_index < 0 ||
            teleporter.target_map_index >= dungeon->level_count) {
            break;
        }
        if (csb_v1_runtime_group_destination_is_blocked(
                dungeon,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y)) {
            break;
        }
        if (csb_v1_runtime_group_destination_has_party_or_group(
                profile,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y)) {
            csb_v1_runtime_schedule_move_group_event(
                profile,
                group_thing,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y,
                teleporter.audible);
            break;
        }
        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_type != 4 || thing_size < 16) break;

        creature_type = (int)group_record[4];
        creature_profile = CREATURE_GetProfile_Compat(creature_type);
        flags = csb_v1_runtime_read_u16(group_record + 14);
        direction = (int)((flags >> 8) & 0x03u);

        memset(&group, 0, sizeof(group));
        group.count = (int)((flags >> 5) & 0x03u);
        group.creature_size = creature_profile
            ? (int)(creature_profile->attributes & 0x0003)
            : 0;
        group.directions_packed =
            csb_v1_runtime_repeated_group_direction_pack(direction);
        group.cells_packed = (uint16_t)group_record[5];
        group.behavior = (int)(flags & 0x000Fu);
        group.active_group_index = 0;
        group.source_map_index = current_map_index;
        group.party_map_index = profile->current_level;
        if (csb_v1_teleporter_rotation_apply_group_pc34_compat(
                &teleporter,
                &group,
                &result) != 0) {
            break;
        }
        if (!csb_v1_runtime_move_group_thing_to_square(
                dungeon,
                group_thing,
                current_map_index,
                *inout_map_x,
                *inout_map_y,
                teleporter.target_map_index,
                teleporter.target_map_x,
                teleporter.target_map_y)) {
            break;
        }

        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_type != 4 || thing_size < 16) break;
        group_record[5] = (uint8_t)(result.cells_packed & 0xFFu);
        flags = csb_v1_runtime_read_u16(group_record + 14);
        flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 8)) |
                           (uint16_t)((result.directions_packed & 0x03u) << 8));
        csb_v1_runtime_write_u16(group_record + 14, flags);
        *inout_map_index = teleporter.target_map_index;
        *inout_map_x = teleporter.target_map_x;
        *inout_map_y = teleporter.target_map_y;
        moved_count++;
    }
    if (moved_count > 0 && (!out_group_alive || *out_group_alive)) {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_sync_active_group_state_from_record(
                profile,
                group_thing,
                group_record,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y,
                1,
                moved_count > 0);
        }
    }
    /* ReDMCSB MOVESENS.C F0267 lines 493-617 moves C04 groups through
     * creature-scope teleporters and open pits in the same PC34 100-step
     * chain. Teleporters call F0262 for direction/cell rotation; pits call
     * DUNGEON.C F0154 to resolve the lower target map/coordinate. This
     * bounded CSB runtime bridge handles generated groups with raw C04
     * records and mirrors the surviving group's native active-group side
     * state; buzz audio and full F0191 fall-damage aftermath remain separate
     * work. */
    return moved_count;
}

static void csb_v1_runtime_apply_group_behavior_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int thing;
    int guard;
    int distance_x;
    int distance_y;

    if (!profile || !record || !profile->dungeon_handle) return;
    if (profile->champion_count <= 0) return;
    if (record->mapIndex != profile->current_level) return;
    distance_x = abs(profile->party_x - record->mapX);
    distance_y = abs(profile->party_y - record->mapY);

    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB GROUP.C F0209 lines 2098-2139 processes C37.  A wandering
     * group that sees the party switches to C6 attack when in same-row/column
     * attack range, otherwise to C7 approach and queues the next C37.  This
     * CSB bridge applies the real-format group behavior byte mutation first;
     * follow-up movement/attack event expansion remains in the source-locked
     * creature-AI layer. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *thing_record;
        uint16_t flags;
        int thing_type;
        int thing_size;
        int behavior;
        int next_behavior;
        int movement_ticks;
        int creature_count;
        int creature_size;
        uint16_t group_thing;
        const struct CreatureBehaviorProfile_Compat *creature_profile;

        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 4 && thing_size >= 16) {
            flags = csb_v1_runtime_read_u16(thing_record + 14);
            behavior = (int)(flags & 0x000Fu);
            group_thing = (uint16_t)thing;
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            creature_profile = CREATURE_GetProfile_Compat((int)thing_record[4]);
            creature_size = creature_profile
                ? (int)(creature_profile->attributes & 0x0003u)
                : 0;
            if ((behavior == 0 || behavior == 2 || behavior == 3) &&
                (distance_x == 0 || distance_y == 0)) {
                next_behavior = (distance_x + distance_y <= 1) ? 6 : 7;
                flags = (uint16_t)((flags & 0xFFF0u) |
                                   (uint16_t)(next_behavior & 0x0F));
                csb_v1_runtime_write_u16(thing_record + 14, flags);
                csb_v1_runtime_set_active_group_target(
                    profile,
                    group_thing,
                    record->mapIndex,
                    record->mapX,
                    record->mapY,
                    profile->party_x,
                    profile->party_y);
                if (next_behavior == 6) {
                    csb_v1_runtime_turn_active_group_toward_attack(
                        profile,
                        group_thing,
                        thing_record,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        csb_v1_runtime_direction_from_source_to_destination(
                            record->mapX,
                            record->mapY,
                            profile->party_x,
                            profile->party_y),
                        creature_count,
                        creature_size);
                    csb_v1_runtime_schedule_c38_attack_events(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        flags);
                }
                if (next_behavior == 7) {
                    csb_v1_runtime_set_active_group_direction_all(
                        profile,
                        group_thing,
                        thing_record,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        csb_v1_runtime_direction_from_source_to_destination(
                            record->mapX,
                            record->mapY,
                            profile->party_x,
                            profile->party_y));
                    /* ReDMCSB GROUP.C F0209 lines 2135-2140 sets behavior
                     * C7 approach and increments the next C37 time by one
                     * tick; lines 2450-2463 then add C37 through F0208 with
                     * the existing movement-tick priority. */
                    csb_v1_runtime_schedule_c37_group_event(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        1u);
                }
                return;
            }
            if (behavior == 7) {
                CSB_V1_RuntimeActiveGroupState *active_state;
                int party_visible = (distance_x == 0 || distance_y == 0);
                int target_map_index = record->mapIndex;
                int target_x = record->mapX;
                int target_y = record->mapY;
                int approach_target_x = profile->party_x;
                int approach_target_y = profile->party_y;
                int moved = 0;
                int deferred = 0;
                int initial_move_direction = 0;
                int is_archenemy = creature_profile &&
                    ((creature_profile->attributes &
                      CREATURE_ATTR_MASK_ARCHENEMY) != 0);

                movement_ticks = csb_v1_runtime_creature_movement_ticks(
                    (int)thing_record[4]);
                if (party_visible && distance_x + distance_y <= 1) {
                    flags = (uint16_t)((flags & 0xFFF0u) | 6u);
                    csb_v1_runtime_write_u16(thing_record + 14, flags);
                    csb_v1_runtime_set_active_group_target(
                        profile,
                        group_thing,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        profile->party_x,
                        profile->party_y);
                    csb_v1_runtime_turn_active_group_toward_attack(
                        profile,
                        group_thing,
                        thing_record,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        csb_v1_runtime_direction_from_source_to_destination(
                            record->mapX,
                            record->mapY,
                            profile->party_x,
                            profile->party_y),
                        creature_count,
                        creature_size);
                    csb_v1_runtime_schedule_c38_attack_events(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        flags);
                    return;
                }
                if (party_visible) {
                    csb_v1_runtime_set_active_group_target(
                        profile,
                        group_thing,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        profile->party_x,
                        profile->party_y);
                } else {
                    active_state = csb_v1_runtime_active_group_state_for_thing(
                        profile,
                        group_thing);
                    if (!active_state) return;
                    approach_target_x = active_state->target_map_x;
                    approach_target_y = active_state->target_map_y;
                    if (approach_target_x == record->mapX &&
                        approach_target_y == record->mapY) {
                        flags = (uint16_t)(flags & 0xFFF0u);
                        csb_v1_runtime_write_u16(thing_record + 14, flags);
                        csb_v1_runtime_set_active_group_direction_group(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            (int)((flags >> 8) & 0x03u),
                            creature_count,
                            creature_size);
                        return;
                    }
                }
                if (approach_target_y != record->mapY) {
                    target_y += (approach_target_y > record->mapY) ? 1 : -1;
                } else if (approach_target_x != record->mapX) {
                    target_x += (approach_target_x > record->mapX) ? 1 : -1;
                } else {
                    return;
                }
                initial_move_direction =
                    csb_v1_runtime_direction_from_source_to_destination(
                        record->mapX,
                        record->mapY,
                        target_x,
                        target_y);
                if (!csb_v1_runtime_group_destination_is_blocked(
                        dungeon,
                        record->mapIndex,
                        target_x,
                        target_y)) {
                    if (csb_v1_runtime_group_destination_has_party_or_group(
                            profile,
                            record->mapIndex,
                            target_x,
                            target_y)) {
                        csb_v1_runtime_schedule_move_group_event(
                            profile,
                            group_thing,
                            record->mapIndex,
                            target_x,
                            target_y,
                            0);
                        deferred = 1;
                    } else {
                        csb_v1_runtime_sync_active_group_state_from_record(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            0,
                            0);
                        moved = csb_v1_runtime_move_group_thing_to_square(
                            dungeon,
                            group_thing,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            record->mapIndex,
                            target_x,
                            target_y);
                    }
                    if (moved) {
                        int group_alive = 1;
                        int consequence_moves;
                        csb_v1_runtime_request_creature_movement_sound(
                            profile,
                            (int)thing_record[4],
                            target_x,
                            target_y);
                        consequence_moves =
                            csb_v1_runtime_apply_group_consequences_at_square(
                            profile,
                            group_thing,
                            &target_map_index,
                            &target_x,
                            &target_y,
                            &group_alive);
                        if (!group_alive) {
                            return;
                        }
                        thing_record = csb_v1_runtime_mutable_thing_record(
                            dungeon,
                            group_thing,
                            &thing_type,
                            &thing_size);
                        if (thing_record && thing_type == 4 &&
                            thing_size >= 16) {
                            if (consequence_moves == 0) {
                                csb_v1_runtime_set_active_group_direction_group(
                                    profile,
                                    group_thing,
                                    thing_record,
                                    target_map_index,
                                    target_x,
                                    target_y,
                                    initial_move_direction,
                                    creature_count,
                                    creature_size);
                            }
                            csb_v1_runtime_sync_active_group_state_from_record(
                                profile,
                                group_thing,
                                thing_record,
                                target_map_index,
                                target_x,
                                target_y,
                                1,
                                1);
                        }
                    }
                } else {
                    int double_target_x = record->mapX;
                    int double_target_y = record->mapY;
                    if (initial_move_direction == 0) {
                        double_target_y -= 2;
                    } else if (initial_move_direction == 1) {
                        double_target_x += 2;
                    } else if (initial_move_direction == 2) {
                        double_target_y += 2;
                    } else {
                        double_target_x -= 2;
                    }
                    if (is_archenemy &&
                        !csb_v1_runtime_group_destination_is_blocked(
                            dungeon,
                            record->mapIndex,
                            double_target_x,
                            double_target_y) &&
                        !csb_v1_runtime_group_destination_has_party_or_group(
                            profile,
                            record->mapIndex,
                            double_target_x,
                            double_target_y)) {
                        target_x = double_target_x;
                        target_y = double_target_y;
                        csb_v1_runtime_sync_active_group_state_from_record(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            0,
                            0);
                        moved = csb_v1_runtime_move_group_thing_to_square(
                            dungeon,
                            group_thing,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            record->mapIndex,
                            target_x,
                            target_y);
                    }
                    if (moved) {
                        int group_alive = 1;
                        int consequence_moves;
                        csb_v1_runtime_request_buzz_sound(
                            profile,
                            target_x,
                            target_y);
                        csb_v1_runtime_request_creature_movement_sound(
                            profile,
                            (int)thing_record[4],
                            target_x,
                            target_y);
                        consequence_moves =
                            csb_v1_runtime_apply_group_consequences_at_square(
                            profile,
                            group_thing,
                            &target_map_index,
                            &target_x,
                            &target_y,
                            &group_alive);
                        if (!group_alive) {
                            return;
                        }
                        thing_record = csb_v1_runtime_mutable_thing_record(
                            dungeon,
                            group_thing,
                            &thing_type,
                            &thing_size);
                        if (thing_record && thing_type == 4 &&
                            thing_size >= 16) {
                            if (consequence_moves == 0) {
                                csb_v1_runtime_set_active_group_direction_group(
                                    profile,
                                    group_thing,
                                    thing_record,
                                    target_map_index,
                                    target_x,
                                    target_y,
                                    initial_move_direction,
                                    creature_count,
                                    creature_size);
                            }
                            csb_v1_runtime_sync_active_group_state_from_record(
                                profile,
                                group_thing,
                                thing_record,
                                target_map_index,
                                target_x,
                                target_y,
                                1,
                                1);
                        }
                    } else {
                        csb_v1_runtime_sync_active_group_state_from_record(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            0,
                            0);
                        csb_v1_runtime_set_active_group_direction_group(
                            profile,
                            group_thing,
                            thing_record,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            initial_move_direction,
                            creature_count,
                            creature_size);
                    }
                }
                /* ReDMCSB GROUP.C F0209 lines 2228-2272 walks C7 approach
                 * toward the target using F0202 movement checks, then lines
                 * 2273-2284 allow an archenemy double-square move that ignores
                 * the first square and requests M560_SOUND_BUZZ at the second
                 * square before moving the group. Lines
                 * 2450-2463 schedule the next C37.  This bounded bridge
                 * relinks a real-format C04 group, creates destination
                 * square-first slots through the bounded F0163 bridge when
                 * needed, schedules C60/C61-style retry when blocked by
                 * party/group occupancy, then applies bounded creature-scope
                 * teleporter/pit chains. Full F0202 occupancy breadth and
                 * attack expansion remain separate work. */
                if (!deferred) {
                    csb_v1_runtime_schedule_c37_group_event(
                        profile,
                        moved ? target_map_index : record->mapIndex,
                        moved ? target_x : record->mapX,
                        moved ? target_y : record->mapY,
                        (int)thing_record[4],
                        (uint32_t)((movement_ticks > 1) ? movement_ticks : 1));
                }
            }
            return;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_move_group_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    uint16_t group_thing;
    int source_level;
    int source_x;
    int source_y;
    int target_level;
    int target_x;
    int target_y;
    int group_alive = 1;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    group_thing = (uint16_t)(((uint16_t)(record->effect & 0xFF) << 8) |
                             (uint16_t)(record->cell & 0xFF));
    target_level = record->mapIndex;
    target_x = record->mapX;
    target_y = record->mapY;
    if (group_thing == 0xFFFEu || group_thing == 0xFFFFu) return;
    if (!csb_v1_runtime_find_group_thing_location(
            dungeon,
            group_thing,
            &source_level,
            &source_x,
            &source_y)) {
        return;
    }
    if (csb_v1_runtime_group_destination_is_blocked(
            dungeon,
            target_level,
            target_x,
            target_y)) {
        return;
    }
    if (csb_v1_runtime_group_destination_has_party_or_group(
            profile,
            target_level,
            target_x,
            target_y)) {
        csb_v1_runtime_schedule_move_group_event(
            profile,
            group_thing,
            target_level,
            target_x,
            target_y,
            record->eventType == DM1_EVENT_MOVE_GROUP_AUDIBLE);
        return;
    }
    {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_sync_active_group_state_from_record(
                profile,
                group_thing,
                group_record,
                source_level,
                source_x,
                source_y,
                0,
                0);
        }
    }
    if (!csb_v1_runtime_move_group_thing_to_square(
            dungeon,
            group_thing,
            source_level,
            source_x,
            source_y,
            target_level,
            target_x,
            target_y)) {
        return;
    }
    {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_request_creature_movement_sound(
                profile,
                (int)group_record[4],
                target_x,
                target_y);
        }
    }
    /* ReDMCSB: MOVESENS.C F0265 lines 169-189 schedules C60/C61 with
     * destination map/x/y and group thing in C.Slot after a blocked group
     * move. TIMELINE.C later retries F0267 on that group. This bounded CSB
     * bridge carries C.Slot in c_cell/c_effect and retries movement when
     * the party/group obstruction is gone. */
    (void)csb_v1_runtime_apply_group_consequences_at_square(
        profile,
        group_thing,
        &target_level,
        &target_x,
        &target_y,
        &group_alive);
    if (group_alive) {
        int thing_type = -1;
        int thing_size = 0;
        uint8_t *group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size >= 16) {
            csb_v1_runtime_sync_active_group_state_from_record(
                profile,
                group_thing,
                group_record,
                target_level,
                target_x,
                target_y,
                1,
                1);
        }
    }
}

static int csb_v1_runtime_stat_or_default(
    const CSB_V1_Champion *champion,
    int stat_index,
    int stat_kind)
{
    int value;

    if (!champion ||
        stat_index < 0 ||
        stat_index >= CSB_V1_STAT_COUNT ||
        stat_kind < 0 ||
        stat_kind > CSB_V1_STAT_MAX) {
        return 30;
    }
    value = (int)champion->Statistics[stat_index][stat_kind];
    return (value > 0) ? value : 30;
}

static int csb_v1_runtime_imported_skill_level(
    const CSB_V1_Champion *champion,
    int skill_index)
{
    int level;

    if (!champion ||
        skill_index < 0 ||
        skill_index >= CSB_V1_FULL_SKILL_COUNT) {
        return 1;
    }

    if (champion->SkillExperienceValid) {
        int64_t experience;

        /* CSBWin SaveGame.cpp:1838 swaps four CHARDESC records whose SKILL
         * rows are CSB.h CHARDESC::skill[20] tempAdjust/experience pairs.
         * ReDMCSB CHAMPION.C F0303 lines 752-768 adds temporary experience,
         * averages hidden skills with their base skill, then halves from the
         * 500 XP threshold to derive the live skill level. */
        experience = (int64_t)champion->SkillExperience[skill_index] +
                     (int64_t)champion->SkillTemporaryExperience[skill_index];
        if (skill_index > 3) {
            const int base_skill = (skill_index - 4) >> 2;
            experience +=
                (int64_t)champion->SkillExperience[base_skill] +
                (int64_t)champion->SkillTemporaryExperience[base_skill];
            experience >>= 1;
        }
        if (experience < 0) experience = 0;
        level = 1;
        while (experience >= 500 && level < 16) {
            experience >>= 1;
            level++;
        }
        return level;
    }

    if (skill_index >= CSB_V1_SKILL_COUNT) {
        return 1;
    }
    level = (int)champion->Skills[skill_index];
    if (level < 1) level = 1;
    if (level > 16) level = 16;
    return level;
}

int csb_v1_runtime_get_champion_skill_level(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int skill_index)
{
    if (!profile ||
        !profile->party_state_valid ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    return csb_v1_runtime_imported_skill_level(
        &profile->party_state.Champions[champion_index],
        skill_index);
}

static int csb_v1_runtime_fill_creature_combat_snapshot(
    int creature_type,
    int creature_index,
    struct CombatantCreatureSnapshot_Compat *out)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    if (!creature_profile) return 0;

    /* ReDMCSB DUNGEON.C G0243 supplies immutable creature attack fields;
     * live C04 real-format group records supply the per-creature slot. */
    out->creatureType = creature_type;
    out->attack = creature_profile->baseAttack;
    out->defense = creature_profile->baseDefense;
    out->dexterity = creature_profile->dexterity;
    out->baseHealth = creature_profile->baseHealth;
    out->poisonAttack = creature_profile->poisonAttack;
    out->attackType = creature_profile->attackType;
    out->attributes = creature_profile->attributes;
    out->woundProbabilities = creature_profile->woundProbabilities;
    out->properties = creature_profile->properties;
    out->doubledMapDifficulty = 0;
    out->creatureIndex = creature_index;
    out->healthBefore = 0;
    return 1;
}

static int csb_v1_runtime_fill_defender_combat_snapshot(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    struct CombatantChampionSnapshot_Compat *out)
{
    const CSB_V1_Champion *champion;

    if (!profile || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (champion->CurrentHealth <= 0 ||
        (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0321 consumes a snapshot of current champion
     * statistics, wounds, defenses, and party shields.  CSB V1's imported
     * champion block currently carries the source statistics and compact
     * skill row but not yet the full DM1 armor/wound/rest/shield side state,
     * so those fields stay at bounded zero until the shared inventory/
     * lifecycle bridge is attached. */
    out->championIndex = champion_index;
    out->currentHealth = champion->CurrentHealth;
    out->dexterity = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_DEX,
        CSB_V1_STAT_CUR);
    out->skillLevelParry = csb_v1_runtime_imported_skill_level(champion, 7);
    out->statisticVitality = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_VIT,
        CSB_V1_STAT_CUR);
    out->statisticAntifire = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_ANTIFIRE,
        CSB_V1_STAT_CUR);
    out->statisticAntimagic = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_ANTIMAGIC,
        CSB_V1_STAT_CUR);
    out->statisticWisdom = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_WIS,
        CSB_V1_STAT_CUR);
    out->statisticLuck = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_LUCK,
        CSB_V1_STAT_CUR);
    out->statisticLuckMax = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_LUCK,
        CSB_V1_STAT_MAX);
    out->statisticLuckMin = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_LUCK,
        CSB_V1_STAT_MIN);
    out->wounds = (int)champion->Wounds;
    return 1;
}

static uint32_t csb_v1_runtime_creature_attack_seed(
    const CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int creature_type,
    int creature_index,
    int champion_index)
{
    uint32_t seed;

    seed = 0xC5B1C038u ^
           ((uint32_t)profile->game_time * 1103515245u) ^
           ((uint32_t)(record->mapX & 0xFF) << 24) ^
           ((uint32_t)(record->mapY & 0xFF) << 16) ^
           ((uint32_t)(creature_type & 0xFF) << 8) ^
           (uint32_t)((creature_index & 0x03) |
                      ((champion_index & 0x03) << 2));
    return (seed != 0u) ? seed : 1u;
}

static void csb_v1_runtime_mark_champion_dead(
    CSB_V1_RuntimeProfile *profile,
    int champion_index)
{
    int next_leader;

    if (!profile ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return;
    }
    (void)csb_v1_champion_kill(
        &profile->party_state.Champions[champion_index]);
    if (profile->party_state.LeaderIndex == champion_index ||
        profile->leader_index == champion_index) {
        next_leader = csb_v1_runtime_first_living_champion(
            &profile->party_state);
        profile->party_state.LeaderIndex = next_leader;
        profile->leader_index = next_leader;
        if (next_leader < 0) {
            /* ReDMCSB CHAMPION.C F0319 lines 1662-1668 sets
             * G0303_B_PartyDead when no champion still has CurrentHealth
             * after damage application. */
            profile->game_over = 1;
        }
    }
}

static void csb_v1_runtime_schedule_poison_champion_event(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int poison_attack)
{
    struct DM1_Event_V1 event;

    if (!profile || champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        poison_attack <= 0) {
        return;
    }
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        profile->current_level,
        profile->game_time + 36u);
    event.type = DM1_EVENT_POISON_CHAMPION;
    event.priority = (uint8_t)champion_index;
    event.b_mapX = (uint8_t)profile->party_x;
    event.b_mapY = (uint8_t)profile->party_y;
    event.c_effect = (uint8_t)(poison_attack & 0xff);
    if (dm1v1_event_add(&profile->timeline_queue, &event) >= 0 &&
        profile->party_state.Champions[champion_index].PoisonEventCount <
            255u) {
        profile->party_state.Champions[champion_index].PoisonEventCount++;
    }
}

static void csb_v1_runtime_apply_poison_attack_to_champion(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int poison_attack)
{
    CSB_V1_Champion *champion;
    int poison_damage;
    int next_attack;
    unsigned int dose;

    if (!profile ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        poison_attack <= 0) {
        return;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (champion->CurrentHealth <= 0 ||
        (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
        return;
    }

    /* ReDMCSB CHAMPION.C F0322 lines 1949-1960 applies immediate
     * max(1, Attack >> 6) damage, increments PoisonEventCount when
     * rescheduling C75, and stores Attack-1 in EVENT.B.Attack.  The
     * bounded CSB event bridge stores that 8-bit attack in c_effect. */
    poison_damage = poison_attack >> 6;
    if (poison_damage < 1) poison_damage = 1;
    if (poison_damage >= champion->CurrentHealth) {
        champion->CurrentHealth = 0;
        csb_v1_runtime_mark_champion_dead(profile, champion_index);
        return;
    }
    champion->CurrentHealth = (int16_t)(champion->CurrentHealth -
                                        poison_damage);

    dose = (unsigned int)champion->PoisonDose +
           (unsigned int)poison_attack;
    if (dose > 0xffffu) dose = 0xffffu;
    champion->PoisonDose = (uint16_t)dose;

    next_attack = poison_attack - 1;
    if (next_attack > 0) {
        csb_v1_runtime_schedule_poison_champion_event(
            profile,
            champion_index,
            next_attack);
    }
}

static void csb_v1_runtime_apply_poison_event_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    int champion_index;

    if (!profile || !record) return;
    champion_index = record->aux0;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return;
    }
    /* ReDMCSB TIMELINE.C C75 lines 1991-1993 decrements the current
     * poison event count before F0322 reschedules Attack-1. */
    if (profile->party_state.Champions[champion_index].PoisonEventCount >
        0u) {
        profile->party_state.Champions[champion_index].PoisonEventCount--;
    }
    csb_v1_runtime_apply_poison_attack_to_champion(
        profile,
        champion_index,
        record->effect);
}

static int csb_v1_runtime_apply_explosion_party_action(
    CSB_V1_RuntimeProfile *profile,
    const struct CombatAction_Compat *action,
    struct RngState_Compat *rng)
{
    int applied;
    int random_window;
    int base_attack;
    int i;

    if (!profile || !action || !rng ||
        !profile->party_state_valid ||
        action->kind != COMBAT_ACTION_APPLY_DAMAGE_CHAMPION ||
        action->rawAttackValue <= 0) {
        return 0;
    }

    applied = 0;
    random_window = (action->rawAttackValue >> 3) + 1;
    base_attack = action->rawAttackValue - random_window;
    random_window <<= 1;

    /* ReDMCSB PROJEXPL.C F0213 lines 169-174 and F0220 lines
     * 858-862 dispatch party-square fireball/lightning/poison-cloud
     * explosions through CHAMPION.C F0324.  F0324 fans out to every
     * living champion, randomizes attack by +/- 1/8, and then calls F0321
     * for shield/defense/wound scaling before F0319 death handling. */
    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; ++i) {
        CSB_V1_Champion *champion = &profile->party_state.Champions[i];
        struct CombatantChampionSnapshot_Compat defender;
        int randomized_attack;
        int scaled_attack = 0;
        int selected_wounds = 0;

        if (champion->CurrentHealth <= 0 ||
            (champion->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) {
            continue;
        }
        randomized_attack = base_attack +
            F0732_COMBAT_RngRandom_Compat(rng, random_window);
        if (randomized_attack < 1) randomized_attack = 1;
        if (!csb_v1_runtime_fill_defender_combat_snapshot(
                profile,
                i,
                &defender) ||
            !F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
                action->attackTypeCode,
                randomized_attack,
                action->allowedWounds,
                &defender,
                rng,
                &scaled_attack,
                NULL) ||
            scaled_attack <= 0) {
            continue;
        }
        if (!F0739c_COMBAT_SelectChampionWoundsF0321Rng_Compat(
                scaled_attack,
                action->allowedWounds,
                &defender,
                rng,
                &selected_wounds,
                NULL)) {
            selected_wounds = 0;
        }
        champion->Wounds = (uint16_t)(champion->Wounds |
                                      (uint16_t)selected_wounds);
        if (scaled_attack >= champion->CurrentHealth) {
            champion->CurrentHealth = 0;
            csb_v1_runtime_mark_champion_dead(profile, i);
        } else {
            champion->CurrentHealth =
                (int16_t)(champion->CurrentHealth - scaled_attack);
        }
        applied++;
    }
    return applied;
}

static int csb_v1_runtime_group_cell_value(int cells, int index)
{
    if (cells == 0xFF) return 0xFF;
    if (index < 0 || index > 3) return 0;
    return (cells >> (index * 2)) & 0x03;
}

static int csb_v1_runtime_group_cells_set_value(
    int cells,
    int index,
    int value)
{
    int shift;

    if (cells == 0xFF) return cells;
    if (index < 0 || index > 3) return cells;
    shift = index * 2;
    cells &= ~(0x03 << shift);
    cells |= (value & 0x03) << shift;
    return cells & 0xFF;
}

static int csb_v1_runtime_group_direction_value(uint16_t directions, int index)
{
    if (index < 0 || index > 3) return 0;
    return (int)((directions >> (index * 2)) & 0x03u);
}

static uint16_t csb_v1_runtime_group_directions_set_value(
    uint16_t directions,
    int index,
    int value)
{
    int shift;

    if (index < 0 || index > 3) return directions;
    shift = index * 2;
    directions = (uint16_t)(directions & ~(uint16_t)(0x03u << shift));
    directions =
        (uint16_t)(directions | (uint16_t)((value & 0x03) << shift));
    return directions;
}

static void csb_v1_runtime_unlink_group_thing_from_square(
    CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y)
{
    uint8_t *first_ptr;
    uint8_t *record;
    uint8_t *previous_record;
    uint16_t thing;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon) return;
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);
    if (!first_ptr) return;

    previous_record = NULL;
    thing = csb_v1_runtime_read_u16(first_ptr);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return;
        next_thing = csb_v1_runtime_read_u16(record);
        if (thing == group_thing && thing_type == 4) {
            if (previous_record) {
                csb_v1_runtime_write_u16(previous_record, next_thing);
            } else {
                csb_v1_runtime_write_u16(first_ptr, next_thing);
            }
            return;
        }
        previous_record = record;
        thing = next_thing;
    }
}

static int csb_v1_runtime_append_thing_to_square_tail(
    CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int level,
    int map_x,
    int map_y)
{
    uint8_t *first_ptr;
    uint8_t *record;
    uint16_t current;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon || thing == 0xFFFEu || thing == 0xFFFFu) return 0;
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);

    record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        thing,
        &thing_type,
        &thing_size);
    if (!record || thing_size < 2) return 0;
    csb_v1_runtime_write_u16(record, 0xFFFEu);
    if (!first_ptr) {
        first_ptr = csb_v1_runtime_create_square_first_thing_ptr(
            dungeon,
            level,
            map_x,
            map_y,
            thing);
        return first_ptr ? 1 : 0;
    }

    current = csb_v1_runtime_read_u16(first_ptr);
    if (current == 0xFFFEu || current == 0xFFFFu) {
        csb_v1_runtime_write_u16(first_ptr, thing);
        return 1;
    }
    for (guard = 0; guard < 128; ++guard) {
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            current,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(record);
        if (next_thing == 0xFFFEu || next_thing == 0xFFFFu) {
            csb_v1_runtime_write_u16(record, thing);
            return 1;
        }
        current = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_unlink_thing_from_square(
    CSB_V1_DungeonData *dungeon,
    uint16_t target_thing,
    int level,
    int map_x,
    int map_y)
{
    uint8_t *first_ptr;
    uint8_t *record;
    uint8_t *previous_record;
    uint16_t thing;
    uint16_t next_thing;
    int thing_type;
    int thing_size;
    int guard;

    if (!dungeon || target_thing == 0xFFFEu || target_thing == 0xFFFFu) {
        return 0;
    }
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);
    if (!first_ptr) return 0;

    previous_record = NULL;
    thing = csb_v1_runtime_read_u16(first_ptr);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(record);
        if ((thing & 0x3FFFu) == (target_thing & 0x3FFFu)) {
            if (previous_record) {
                csb_v1_runtime_write_u16(previous_record, next_thing);
            } else {
                csb_v1_runtime_write_u16(first_ptr, next_thing);
            }
            csb_v1_runtime_write_u16(record, 0xFFFEu);
            return 1;
        }
        previous_record = record;
        thing = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_projectile_result_places_associated_object(
    int result_kind)
{
    switch (result_kind) {
    case PROJECTILE_RESULT_DESPAWN_ENERGY:
    case PROJECTILE_RESULT_HIT_CHAMPION:
    case PROJECTILE_RESULT_HIT_CREATURE:
    case PROJECTILE_RESULT_HIT_DOOR:
    case PROJECTILE_RESULT_HIT_WALL:
    case PROJECTILE_RESULT_HIT_OTHER_PROJECTILE:
        return 1;
    default:
        return 0;
    }
}

static int csb_v1_runtime_stairs_exit_direction(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y)
{
    static const int step_east[4] = { 0, 1, 0, -1 };
    static const int step_north[4] = { -1, 0, 1, 0 };
    int raw_square;
    int north_south;
    int check_x;
    int check_y;
    int check_raw;
    int check_type = 1;
    int blocked = 0;

    if (!dungeon || level < 0 || level >= dungeon->level_count) return 0;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        map_x,
        map_y);
    if (raw_square < 0) return 0;

    north_south = (raw_square & 0x08) ? 0 : 1;
    check_x = map_x + step_east[north_south ? 1 : 0];
    check_y = map_y + step_north[north_south ? 1 : 0];
    check_raw = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        check_x,
        check_y);
    if (check_raw >= 0) {
        check_type = (dungeon->square_bytes == 1)
            ? ((check_raw >> 5) & 0x07)
            : (check_raw & 0x1F);
        blocked = (check_type == 0 || check_type == 3) ? 1 : 0;
    }
    /* ReDMCSB DUNGEON.C F0155 lines 1560-1582: stairs without the
     * north/south orientation bit check EAST and return EAST/WEST; stairs
     * with the bit check NORTH and return NORTH/SOUTH depending on whether
     * that neighbor is wall/stairs. */
    return (blocked << 1) + north_south;
}

static int csb_v1_runtime_has_map_location_metadata(
    const CSB_V1_DungeonData *dungeon)
{
    int i;

    if (!dungeon) return 0;
    for (i = 0; i < dungeon->level_count; ++i) {
        if (dungeon->map_levels[i] != 0 ||
            dungeon->map_offset_x[i] != 0 ||
            dungeon->map_offset_y[i] != 0) {
            return 1;
        }
    }
    return 0;
}

static int csb_v1_runtime_map_source_level(
    const CSB_V1_DungeonData *dungeon,
    int map_index)
{
    if (!dungeon ||
        map_index < 0 ||
        map_index >= dungeon->level_count ||
        !csb_v1_runtime_has_map_location_metadata(dungeon)) {
        return map_index;
    }
    return dungeon->map_levels[map_index];
}

static int csb_v1_runtime_location_after_level_change(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int level_delta,
    int *inout_map_x,
    int *inout_map_y,
    int *out_map_index)
{
    int source_level;
    int target_source_level;
    int global_x;
    int global_y;
    int i;

    if (out_map_index) *out_map_index = -1;
    if (!dungeon || !inout_map_x || !inout_map_y || !out_map_index ||
        map_index < 0 || map_index >= dungeon->level_count) {
        return 0;
    }
    if (!csb_v1_runtime_has_map_location_metadata(dungeon)) {
        int target = map_index + level_delta;
        if (target < 0 || target >= dungeon->level_count) return 0;
        *out_map_index = target;
        return 1;
    }

    /* ReDMCSB DUNGEON.C F0154 lines 1508-1554: level changes convert
     * local map coordinates to global OffsetMapX/Y coordinates, add the
     * level delta to MAP.A.Level, then choose the map on that source level
     * whose offset/width/height contains the same global coordinate. */
    source_level = csb_v1_runtime_map_source_level(dungeon, map_index);
    target_source_level = source_level + level_delta;
    global_x = dungeon->map_offset_x[map_index] + *inout_map_x;
    global_y = dungeon->map_offset_y[map_index] + *inout_map_y;
    for (i = 0; i < dungeon->level_count; ++i) {
        int min_x = dungeon->map_offset_x[i];
        int min_y = dungeon->map_offset_y[i];
        int max_x = min_x + dungeon->level_widths[i] - 1;
        int max_y = min_y + dungeon->level_heights[i] - 1;

        if (csb_v1_runtime_map_source_level(dungeon, i) !=
                target_source_level ||
            global_x < min_x || global_x > max_x ||
            global_y < min_y || global_y > max_y) {
            continue;
        }
        *inout_map_x = global_x - min_x;
        *inout_map_y = global_y - min_y;
        *out_map_index = i;
        return 1;
    }
    return 0;
}

static int csb_v1_runtime_apply_object_consequences_at_square(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t *inout_thing,
    int source_map_x,
    int *inout_map_index,
    int *inout_map_x,
    int *inout_map_y)
{
    int moved_count = 0;
    int chain_guard;

    if (!dungeon || !inout_thing || !inout_map_index ||
        !inout_map_x || !inout_map_y) {
        return 0;
    }
    for (chain_guard = 0; chain_guard < 100; ++chain_guard) {
        CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
        CSB_V1_TeleporterRotationRuntimeObjectResultPc34 result;
        int raw_square;
        int square_type;
        int scope = 0;
        int self_target;

        if (*inout_map_index < 0 ||
            *inout_map_index >= dungeon->level_count) {
            break;
        }
        raw_square = csb_v1_dungeon_get_raw_square(
            dungeon,
            *inout_map_index,
            *inout_map_x,
            *inout_map_y);
        if (raw_square < 0) {
            break;
        }
        square_type = (dungeon->square_bytes == 1)
            ? ((raw_square >> 5) & 0x07)
            : (raw_square & 0x1F);
        if (square_type != PROJECTILE_ELEMENT_TELEPORTER) {
            int target_level;
            int old_level;
            int old_x;
            int old_y;
            uint16_t old_thing;

            old_level = *inout_map_index;
            old_x = *inout_map_x;
            old_y = *inout_map_y;
            old_thing = *inout_thing;
            if (square_type == 2) {
                int target_x = *inout_map_x;
                int target_y = *inout_map_y;
                if ((raw_square & 0x08) == 0 ||
                    (raw_square & 0x01) != 0) {
                    break;
                }
                if (!csb_v1_runtime_location_after_level_change(
                        dungeon,
                        *inout_map_index,
                        1,
                        &target_x,
                        &target_y,
                        &target_level)) {
                    break;
                }
                if (!csb_v1_runtime_unlink_thing_from_square(
                        dungeon,
                        *inout_thing,
                        old_level,
                        old_x,
                        old_y)) {
                    break;
                }
                *inout_map_index = target_level;
                *inout_map_x = target_x;
                *inout_map_y = target_y;
            } else if (square_type == 3) {
                static const int step_east[4] = { 0, 1, 0, -1 };
                static const int step_north[4] = { -1, 0, 1, 0 };
                uint16_t moved_thing;
                int direction;
                int cell;

                if ((raw_square & 0x04) == 0) {
                    int target_x = *inout_map_x;
                    int target_y = *inout_map_y;
                    if (!csb_v1_runtime_location_after_level_change(
                            dungeon,
                            *inout_map_index,
                            1,
                            &target_x,
                            &target_y,
                            &target_level)) {
                        break;
                    }
                    *inout_map_index = target_level;
                    *inout_map_x = target_x;
                    *inout_map_y = target_y;
                }
                direction = csb_v1_runtime_stairs_exit_direction(
                    dungeon,
                    *inout_map_index,
                    *inout_map_x,
                    *inout_map_y);
                cell = (*inout_thing >> 14) & 0x03;
                cell = (((cell - direction + 1) & 0x02) >> 1) + direction;
                cell &= 0x03;
                moved_thing = (uint16_t)((*inout_thing & 0x3FFFu) |
                                         (uint16_t)(cell << 14));
                *inout_map_x += step_east[direction];
                *inout_map_y += step_north[direction];
                if (!csb_v1_runtime_unlink_thing_from_square(
                        dungeon,
                        *inout_thing,
                        old_level,
                        old_x,
                        old_y)) {
                    *inout_map_index = old_level;
                    *inout_map_x = old_x;
                    *inout_map_y = old_y;
                    break;
                }
                *inout_thing = moved_thing;
            } else {
                break;
            }
            if (!csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    *inout_thing,
                    *inout_map_index,
                    *inout_map_x,
                    *inout_map_y)) {
                *inout_map_index = old_level;
                *inout_map_x = old_x;
                *inout_map_y = old_y;
                *inout_thing = old_thing;
                (void)csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    old_thing,
                    old_level,
                    old_x,
                    old_y);
                break;
            }
            moved_count++;
            csb_v1_runtime_process_object_floor_sensors_at(
                profile,
                dungeon,
                *inout_thing,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y);
            continue;
        }
        if ((raw_square & 0x08) == 0) break;
        if (csb_v1_runtime_decode_group_teleporter_at_square(
                dungeon,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y,
                raw_square,
                &teleporter,
                &scope) <= 0 ||
            scope == 0x01) {
            break;
        }
        if (csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
                &teleporter,
                *inout_thing,
                source_map_x,
                &result) != 0) {
            break;
        }
        self_target = teleporter.target_map_index == *inout_map_index &&
                      teleporter.target_map_x == *inout_map_x &&
                      teleporter.target_map_y == *inout_map_y;
        if (!csb_v1_runtime_unlink_thing_from_square(
                dungeon,
                *inout_thing,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y)) {
            break;
        }
        *inout_thing = result.thing;
        *inout_map_index = teleporter.target_map_index;
        *inout_map_x = teleporter.target_map_x;
        *inout_map_y = teleporter.target_map_y;
        if (!csb_v1_runtime_append_thing_to_square_tail(
                dungeon,
                *inout_thing,
                *inout_map_index,
                *inout_map_x,
                *inout_map_y)) {
            break;
        }
        moved_count++;
        csb_v1_runtime_process_object_floor_sensors_at(
            profile,
            dungeon,
            *inout_thing,
            *inout_map_index,
            *inout_map_x,
            *inout_map_y);
        if (self_target) break;
    }
    /* ReDMCSB MOVESENS.C F0267 lines 450-530 lets non-party, non-group
     * objects use object/party-capable teleporters, rejects creature-only
     * teleporters, rotates object cells only for relative teleporters unless
     * the object came from the CM2 projectile-associated-object path, and
     * continues into open non-imaginary pits and non-projectile stairs in the
     * same PC34 100-step chain. This CSB bridge also dispatches bounded C004
     * object floor sensors after successful materialization/movement; buzz
     * audio and broader floor sensor types remain separate runtime work. */
    return moved_count;
}

static int csb_v1_runtime_materialize_projectile_associated_object(
    CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    const struct ProjectileTickResult_Compat *tick_result)
{
    CSB_V1_DungeonData *dungeon;
    uint16_t associated_thing;
    uint16_t placed_thing;
    int thing_type;
    int map_index;
    int map_x;
    int map_y;
    int cell;

    if (!profile || !projectile || !tick_result) return 0;
    if (!tick_result->despawn ||
        !csb_v1_runtime_projectile_result_places_associated_object(
            tick_result->resultKind)) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    if (!dungeon) return 0;

    associated_thing = (uint16_t)projectile->reserved1;
    if (associated_thing == 0u ||
        associated_thing == 0xFFFEu ||
        associated_thing == 0xFFFFu) {
        return 0;
    }
    thing_type = (associated_thing >> 10) & 0x0F;
    if (thing_type == 14 || associated_thing >= DM1_THING_FIRST_EXPLOSION) {
        return 0;
    }

    map_index = tick_result->newMapIndex;
    map_x = tick_result->newMapX;
    map_y = tick_result->newMapY;
    cell = tick_result->newCell & 3;
    if (map_index < 0 || map_x < 0 || map_y < 0) return 0;

    placed_thing = (uint16_t)((associated_thing & 0x3FFFu) |
                              (uint16_t)(cell << 14));
    /* ReDMCSB PROJEXPL.C F0215 lines 239-259: deleting a projectile whose
     * Projectile.Slot is not an explosion moves that associated object to
     * the projectile square via F0267_MOVE_GetMoveResult_CPSCE. This CSB
     * bridge performs the real-format thing-list writeback for squares whose
     * first-thing slot already exists; first-thing table expansion remains a
     * later full F0267 integration slice. */
    if (!csb_v1_runtime_append_thing_to_square_tail(
            dungeon,
            placed_thing,
            map_index,
            map_x,
            map_y)) {
        return 0;
    }
    csb_v1_runtime_process_object_floor_sensors_at(
        profile,
        dungeon,
        placed_thing,
        map_index,
        map_x,
        map_y);
    return csb_v1_runtime_apply_object_consequences_at_square(
               profile,
               dungeon,
               &placed_thing,
               CSB_V1_TELEPORTER_ROTATION_SOURCE_PROJECTILE_ASSOCIATED_OBJECT_PC34,
               &map_index,
               &map_x,
               &map_y) >= 0;
}

static int csb_v1_runtime_collect_square_launcher_things(
    const CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    struct ProjectileLauncherSquareThing_Compat *out,
    int out_capacity)
{
    int thing;
    int count = 0;
    int guard;

    if (!dungeon || !out || out_capacity <= 0) return 0;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (thing < 0) return 0;

    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record || thing_size < 2) break;
        if (count < out_capacity) {
            out[count].thing = (unsigned short)thing;
            out[count].cell =
                csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                    (uint16_t)thing);
            out[count].thingType = thing_type;
            ++count;
        }
        thing = (int)csb_v1_runtime_read_u16(record);
    }
    return count;
}

static uint16_t csb_v1_runtime_thing_with_cell(
    int thing_type,
    int thing_index,
    int cell)
{
    return (uint16_t)(((cell & 0x03) << 14) |
                      ((thing_type & 0x0F) << 10) |
                      (thing_index & 0x03FF));
}

static int csb_v1_runtime_find_unused_object_record(
    CSB_V1_DungeonData *dungeon,
    int thing_type,
    uint8_t **out_record,
    int *out_index)
{
    int i;
    int byte_count = 4;

    if (out_record) *out_record = NULL;
    if (out_index) *out_index = -1;
    if (!dungeon || !dungeon->raw_data ||
        (thing_type != DM1_DROP_THING_TYPE_WEAPON &&
         thing_type != DM1_DROP_THING_TYPE_ARMOUR &&
         thing_type != DM1_DROP_THING_TYPE_JUNK)) {
        return 0;
    }

    for (i = 0; i < dungeon->thing_type_counts[thing_type]; ++i) {
        int offset = dungeon->thing_data_bases[thing_type] + i * byte_count;
        if (offset < 0 || offset + byte_count > dungeon->raw_size) return 0;
        if (csb_v1_runtime_read_u16(dungeon->raw_data + offset) == 0xFFFFu) {
            if (out_record) *out_record = dungeon->raw_data + offset;
            if (out_index) *out_index = i;
            return 1;
        }
    }
    return 0;
}

static uint16_t csb_v1_runtime_allocate_fixed_possession_thing(
    CSB_V1_DungeonData *dungeon,
    const struct DM1FixedPossessionDrop_Compat *drop)
{
    uint8_t *record;
    uint16_t item_bits;
    int index;

    if (!dungeon || !drop) return 0xFFFFu;
    if (!csb_v1_runtime_find_unused_object_record(
            dungeon,
            drop->thingType,
            &record,
            &index)) {
        return 0xFFFFu;
    }

    item_bits = (uint16_t)(drop->itemType & 0x7F);
    if (drop->cursed) item_bits |= 0x0100u;
    csb_v1_runtime_write_u16(record + 0, 0xFFFEu);
    csb_v1_runtime_write_u16(record + 2, item_bits);
    return csb_v1_runtime_thing_with_cell(
        drop->thingType,
        index,
        drop->cell);
}

static int csb_v1_runtime_new_object_launcher_icon_to_object(
    int icon_index,
    int *out_thing_type,
    int *out_item_type)
{
    int thing_type = DM1_DROP_THING_TYPE_WEAPON;
    int item_type;

    /* ReDMCSB DUNGEON.C F0167 lines 2140-2200 maps object-generator and
     * projectile-launcher icon indices to object thing types.  This bounded
     * CSB runtime bridge covers the launcher objects used by original-style
     * traps and leaves the full object-info table for the broader item DB. */
    if (icon_index >= 4 && icon_index <= 7) icon_index = 4;
    switch (icon_index) {
    case 4:   item_type = 2; break;   /* C004 torch -> C02 weapon torch */
    case 32:  item_type = 8; break;   /* C032 dagger */
    case 51:  item_type = 27; break;  /* C051 arrow */
    case 52:  item_type = 28; break;  /* C052 slayer */
    case 54:  item_type = 30; break;  /* C054 rock */
    case 55:  item_type = 31; break;  /* C055 poison dart */
    case 56:  item_type = 32; break;  /* C056 throwing star */
    case 128:
        item_type = 25;               /* C128 boulder */
        thing_type = DM1_DROP_THING_TYPE_JUNK;
        break;
    default:
        return 0;
    }
    if (out_thing_type) *out_thing_type = thing_type;
    if (out_item_type) *out_item_type = item_type;
    return 1;
}

static uint16_t csb_v1_runtime_allocate_new_object_launcher_thing(
    CSB_V1_DungeonData *dungeon,
    int icon_index)
{
    uint8_t *record;
    int thing_type;
    int item_type;
    int index;

    if (!dungeon ||
        !csb_v1_runtime_new_object_launcher_icon_to_object(
            icon_index,
            &thing_type,
            &item_type)) {
        return 0xFFFFu;
    }
    if (!csb_v1_runtime_find_unused_object_record(
            dungeon,
            thing_type,
            &record,
            &index)) {
        return 0xFFFFu;
    }
    csb_v1_runtime_write_u16(record + 0, 0xFFFEu);
    csb_v1_runtime_write_u16(record + 2, (uint16_t)(item_type & 0x7Fu));
    return csb_v1_runtime_thing_with_cell(thing_type, index, 0);
}

static void csb_v1_runtime_drop_creature_fixed_possessions(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    int creature_type,
    int source_cell,
    int level,
    int map_x,
    int map_y)
{
    struct DM1FixedPossessionDrop_Compat drops[DM1_MAX_FIXED_POSSESSION_DROPS];
    struct RngState_Compat rng;
    int drop_count = 0;
    int weapon_dropped = 0;
    int i;

    if (!profile || !dungeon) return;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)map_x << 8) ^
            ((uint32_t)map_y << 16) ^
            ((uint32_t)creature_type << 24) ^
            0xF0186u);
    if (!F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
            creature_type,
            source_cell,
            &rng,
            drops,
            DM1_MAX_FIXED_POSSESSION_DROPS,
            &drop_count,
            &weapon_dropped)) {
        return;
    }
    (void)weapon_dropped;

    /* ReDMCSB GROUP.C F0186 lines 580-645 resolves fixed creature
     * possessions, allocates unused C05/C06/C10 records, stores item type
     * plus cursed bit, assigns a floor cell, and moves the thing to the
     * source square through F0267.  This CSB bridge materializes the raw
     * object records and square thing-list append; sounds remain later. */
    for (i = 0; i < drop_count; ++i) {
        uint16_t thing = csb_v1_runtime_allocate_fixed_possession_thing(
            dungeon,
            &drops[i]);
        if (thing == 0xFFFFu) continue;
        if (!csb_v1_runtime_append_thing_to_square_tail(
                dungeon,
                thing,
                level,
                map_x,
                map_y)) {
            uint8_t *record;
            int thing_type;
            int thing_size;
            record = csb_v1_runtime_mutable_thing_record(
                dungeon,
                thing,
                &thing_type,
                &thing_size);
            if (record && thing_size >= 2) {
                csb_v1_runtime_write_u16(record, 0xFFFFu);
            }
        } else {
            int drop_level = level;
            int drop_x = map_x;
            int drop_y = map_y;
            csb_v1_runtime_process_object_floor_sensors_at(
                profile,
                dungeon,
                thing,
                drop_level,
                drop_x,
                drop_y);
            (void)csb_v1_runtime_apply_object_consequences_at_square(
                profile,
                dungeon,
                &thing,
                -1,
                &drop_level,
                &drop_x,
                &drop_y);
        }
    }
}

static void csb_v1_runtime_drop_group_slot_possessions(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y)
{
    struct RngState_Compat rng;
    uint16_t thing;
    int guard;

    if (!profile || !dungeon || !group_record) return;
    thing = csb_v1_runtime_read_u16(group_record + 2);
    if (thing == 0xFFFEu || thing == 0xFFFFu) return;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)map_x << 8) ^
            ((uint32_t)map_y << 16) ^
            0xF0188u);

    /* ReDMCSB GROUP.C F0188 lines 724-731 walks GROUP.Slot, rewrites each
     * carried thing with a random floor cell, and moves it onto the group
     * square before F0189 deletes the group.  Fixed possession allocation and
     * sound mode are separate CSB runtime slices. */
    for (guard = 0; guard < 64 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        uint8_t *record;
        uint16_t next_thing;
        uint16_t dropped_thing;
        int thing_type;
        int thing_size;

        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) break;
        next_thing = csb_v1_runtime_read_u16(record);
        dropped_thing = (uint16_t)((thing & 0x3FFFu) |
                                   (uint16_t)(F0732_COMBAT_RngRandom_Compat(
                                                  &rng,
                                                  4) << 14));
        if (!csb_v1_runtime_append_thing_to_square_tail(
                dungeon,
                dropped_thing,
                level,
                map_x,
                map_y)) {
            break;
        }
        {
            int drop_level = level;
            int drop_x = map_x;
            int drop_y = map_y;
            csb_v1_runtime_process_object_floor_sensors_at(
                profile,
                dungeon,
                dropped_thing,
                drop_level,
                drop_x,
                drop_y);
            (void)csb_v1_runtime_apply_object_consequences_at_square(
                profile,
                dungeon,
                &dropped_thing,
                -1,
                &drop_level,
                &drop_x,
                &drop_y);
        }
        thing = next_thing;
    }
    csb_v1_runtime_write_u16(group_record + 2, 0xFFFEu);
}

static const unsigned char g_csb_v1_giggler_steal_slots_pc34[8] = {
    CSB_V1_SLOT_ACTION_HAND,
    CSB_V1_SLOT_READY_HAND,
    CSB_V1_SLOT_READY_HAND,
    CSB_V1_SLOT_READY_HAND,
    CSB_V1_SLOT_READY_HAND,
    CSB_V1_SLOT_READY_HAND,
    CSB_V1_SLOT_READY_HAND,
    CSB_V1_SLOT_READY_HAND
};

static uint32_t csb_v1_runtime_champion_occupied_slot_mask(
    const CSB_V1_Champion *champion)
{
    uint32_t mask = 0u;
    int i;

    if (!champion) return 0u;
    for (i = 0; i < CSB_V1_SLOT_COUNT && i < 32; ++i) {
        uint16_t thing = champion->Slots[i];
        if (thing != 0xFFFFu && thing != 0xFFFEu) {
            mask |= (uint32_t)(1u << i);
        }
    }
    return mask;
}

static int csb_v1_runtime_link_stolen_thing_to_group_slot(
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    uint16_t stolen_thing)
{
    uint16_t group_slot;

    if (!dungeon || !group_record ||
        stolen_thing == 0xFFFFu ||
        stolen_thing == 0xFFFEu) {
        return 0;
    }

    group_slot = csb_v1_runtime_read_u16(group_record + 2);
    if (group_slot != 0xFFFFu && group_slot != 0xFFFEu) {
        uint8_t *stolen_record;
        int thing_type;
        int thing_size;

        stolen_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            stolen_thing,
            &thing_type,
            &thing_size);
        if (!stolen_record || thing_size < 2) return 0;
        csb_v1_runtime_write_u16(stolen_record, group_slot);
    }
    /* ReDMCSB GROUP.C F0193 lines 1041-1054 links a stolen object into
     * GROUP.Slot.  It intentionally preserves BUG0_12 for an empty Giggler:
     * the first stolen object's Next word is not cleared before becoming the
     * group slot head. */
    csb_v1_runtime_write_u16(group_record + 2, stolen_thing);
    return 1;
}

static int csb_v1_runtime_apply_giggler_steal_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    const struct DM1_DispatchRecord_V1 *record,
    int creature_index,
    int champion_index)
{
    CSB_V1_Champion *champion;
    struct DM1GigglerStealResult_Compat steal;
    struct RngState_Compat rng;
    uint32_t remaining_mask;
    uint16_t flags;
    int dexterity;
    int attempt;

    if (!profile || !dungeon || !group_record || !record ||
        champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount) {
        return 0;
    }

    champion = &profile->party_state.Champions[champion_index];
    dexterity = csb_v1_runtime_stat_or_default(
        champion,
        CSB_V1_STAT_DEX,
        CSB_V1_STAT_CUR);
    F0730_COMBAT_RngInit_Compat(
        &rng,
        csb_v1_runtime_creature_attack_seed(
            profile,
            record,
            DM1_CREATURE_TYPE_GIGGLER,
            creature_index,
            champion_index));
    if (!F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
            dexterity,
            csb_v1_runtime_champion_occupied_slot_mask(champion),
            0,
            &rng,
            &steal)) {
        return 0;
    }

    remaining_mask = steal.stolenSlotMask;
    for (attempt = 0; attempt < 8 && remaining_mask != 0u; ++attempt) {
        int slot = g_csb_v1_giggler_steal_slots_pc34[
            (steal.initialCounter + attempt) & 7];
        uint32_t slot_mask = (uint32_t)(1u << slot);
        uint16_t stolen_thing;

        if ((remaining_mask & slot_mask) == 0u) continue;
        remaining_mask &= ~slot_mask;
        stolen_thing = champion->Slots[slot];
        if (csb_v1_runtime_link_stolen_thing_to_group_slot(
                dungeon,
                group_record,
                stolen_thing)) {
            champion->Slots[slot] = 0xFFFFu;
        }
    }

    flags = csb_v1_runtime_read_u16(group_record + 14);
    if (steal.shouldFlee) {
        flags = (uint16_t)((flags & 0xFFF0u) |
                           (uint16_t)(steal.newBehavior & 0x0F));
        csb_v1_runtime_write_u16(group_record + 14, flags);
        csb_v1_runtime_schedule_c37_group_event(
            profile,
            record->mapIndex,
            record->mapX,
            record->mapY,
            DM1_CREATURE_TYPE_GIGGLER,
            (uint32_t)((steal.fleeDelayTicks > 0) ?
                           steal.fleeDelayTicks :
                           1));
    } else if (!profile->game_over) {
        csb_v1_runtime_schedule_c38_followup_event(
            profile,
            record->mapIndex,
            record->mapX,
            record->mapY,
            DM1_CREATURE_TYPE_GIGGLER,
            creature_index,
            (uint32_t)csb_v1_runtime_creature_attack_ticks(
                DM1_CREATURE_TYPE_GIGGLER));
    }
    return 1;
}

static void csb_v1_runtime_rewrite_group_events_after_creature_death(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    int creature_index)
{
    struct DM1_EventQueue_V1 *queue;
    uint16_t active_indices[DM1_EVENT_MAX_COUNT];
    int active_count;
    int i;

    if (!profile || creature_index < 0 || creature_index > 3) return;
    queue = &profile->timeline_queue;
    active_count = queue->eventCount;
    if (active_count < 0) active_count = 0;
    if (active_count > DM1_EVENT_MAX_COUNT) active_count = DM1_EVENT_MAX_COUNT;
    for (i = 0; i < active_count; ++i) {
        active_indices[i] = queue->timeline[i];
    }

    for (i = 0; i < active_count; ++i) {
        int event_index = active_indices[i];
        struct DM1_Event_V1 *event;
        int event_type;
        int event_creature_index = -1;

        if (event_index < 0 || event_index >= queue->maxEvents) continue;
        event = &queue->events[event_index];
        event_type = event->type;
        if (event_type == DM1_EVENT_NONE ||
            DM1_MAP_TIME_MAP(event->map_time) != (uint32_t)map_index ||
            event->b_mapX != (uint8_t)map_x ||
            event->b_mapY != (uint8_t)map_y) {
            continue;
        }
        if (event_type >= DM1_EVENT_UPDATE_ASPECT_CREATURE_0 &&
            event_type <= DM1_EVENT_UPDATE_ASPECT_CREATURE_3) {
            event_creature_index =
                event_type - DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
        } else if (event_type >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
                   event_type <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            event_creature_index =
                event_type - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
        } else {
            continue;
        }

        /* ReDMCSB GROUP.C F0190 lines 852-875 deletes queued aspect/attack
         * events for the killed creature and decrements higher creature-index
         * event types before fixing timeline heap placement. */
        if (event_creature_index == creature_index) {
            (void)dm1v1_event_delete(queue, event_index);
        } else if (event_creature_index > creature_index) {
            event->type--;
            (void)dm1v1_event_fix_existing_placement(queue, event_index);
        }
    }
}

static void csb_v1_runtime_delete_group_events_at_square(
    CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y)
{
    struct DM1_EventQueue_V1 *queue;
    uint16_t active_indices[DM1_EVENT_MAX_COUNT];
    int active_count;
    int i;

    if (!profile) return;
    queue = &profile->timeline_queue;
    active_count = queue->eventCount;
    if (active_count < 0) active_count = 0;
    if (active_count > DM1_EVENT_MAX_COUNT) active_count = DM1_EVENT_MAX_COUNT;
    for (i = 0; i < active_count; ++i) {
        active_indices[i] = queue->timeline[i];
    }

    for (i = 0; i < active_count; ++i) {
        int event_index = active_indices[i];
        struct DM1_Event_V1 *event;
        int event_type;

        if (event_index < 0 || event_index >= queue->maxEvents) continue;
        event = &queue->events[event_index];
        event_type = event->type;
        /* ReDMCSB GROUP.C F0181 lines 340-366 deletes C29..C41 group
         * reaction/aspect/behavior events for a square after F0189 removes
         * the final creature group. */
        if (event_type >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
            event_type <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 &&
            DM1_MAP_TIME_MAP(event->map_time) == (uint32_t)map_index &&
            event->b_mapX == (uint8_t)map_x &&
            event->b_mapY == (uint8_t)map_y) {
            (void)dm1v1_event_delete(queue, event_index);
        }
    }
}

static int csb_v1_runtime_f0190_smoke_attack_for_creature(int creature_type)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;
    int size;

    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    size = creature_profile ? (creature_profile->attributes & 0x0003) : 0;
    if (size == 0) return 110;
    if (size == 1) return 190;
    return 255;
}

static CSB_V1_RuntimeActiveGroupState *
csb_v1_runtime_active_group_state_for(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int create)
{
    uint16_t i;
    int first_empty = -1;

    if (!profile || ((group_thing >> 10) & 0x0Fu) != 4u) {
        return NULL;
    }
    for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
        CSB_V1_RuntimeActiveGroupState *state =
            &profile->active_group_state[i];
        if (!state->valid) {
            if (first_empty < 0) first_empty = (int)i;
            continue;
        }
        if (state->group_thing == group_thing &&
            state->map_index == level &&
            state->map_x == map_x &&
            state->map_y == map_y) {
            return state;
        }
    }
    if (!create || first_empty < 0 ||
        profile->active_group_state_count >=
            CSB_V1_RUNTIME_ACTIVE_GROUP_CAP) {
        return NULL;
    }
    {
        CSB_V1_RuntimeActiveGroupState *state =
            &profile->active_group_state[first_empty];
        memset(state, 0, sizeof(*state));
        state->valid = 1;
        state->group_thing = group_thing;
        state->map_index = level;
        state->map_x = map_x;
        state->map_y = map_y;
        state->target_map_x = map_x;
        state->target_map_y = map_y;
        ++profile->active_group_state_count;
        return state;
    }
}

static void csb_v1_runtime_clear_active_group_state(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y)
{
    CSB_V1_RuntimeActiveGroupState *state =
        csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            0);
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (profile->active_group_state_count > 0u) {
        --profile->active_group_state_count;
    }
    if (profile->half_square_direction_debounce_valid &&
        profile->half_square_direction_debounce_group == group_thing) {
        profile->half_square_direction_debounce_valid = 0;
    }
}

static CSB_V1_RuntimeActiveGroupState *
csb_v1_runtime_active_group_state_for_thing(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing)
{
    uint16_t i;

    if (!profile || ((group_thing >> 10) & 0x0Fu) != 4u) {
        return NULL;
    }
    for (i = 0u; i < CSB_V1_RUNTIME_ACTIVE_GROUP_CAP; ++i) {
        CSB_V1_RuntimeActiveGroupState *state =
            &profile->active_group_state[i];
        if (state->valid && state->group_thing == group_thing) {
            return state;
        }
    }
    return NULL;
}

static void csb_v1_runtime_sync_active_group_state_from_record(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    const uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int preserve_home,
    int moved)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint16_t flags;
    uint16_t directions;
    int direction;
    uint8_t cells;
    int existed;

    /* ReDMCSB GROUP.C F0183/F0184/F0200 keeps active-group Cells,
     * Directions, Aspect[4], GroupThingIndex, Prior/Home map coordinates,
     * and LastMoveTime beside the raw C04 group record. */
    if (!profile || !group_record) return;
    flags = csb_v1_runtime_read_u16(group_record + 14);
    direction = (int)((flags >> 8) & 0x03u);
    cells = group_record[5];
    directions = csb_v1_runtime_repeated_group_direction_pack(direction);

    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    existed = state ? 1 : 0;
    if (existed) {
        directions = state->directions;
    }
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    if (moved) {
        state->prior_map_x = state->map_x;
        state->prior_map_y = state->map_y;
    } else {
        state->prior_map_x = map_x;
        state->prior_map_y = map_y;
    }
    state->map_index = level;
    state->map_x = map_x;
    state->map_y = map_y;
    if (!existed) {
        state->target_map_x = map_x;
        state->target_map_y = map_y;
    }
    if (!preserve_home || !existed) {
        state->home_map_x = map_x;
        state->home_map_y = map_y;
    }
    state->cells = cells;
    state->directions = directions;
    state->last_move_time = moved
        ? profile->game_time
        : (profile->game_time >= 127u ? profile->game_time - 127u : 0u);
}

static void csb_v1_runtime_set_active_group_target(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int target_x,
    int target_y)
{
    CSB_V1_RuntimeActiveGroupState *state;

    if (!profile || target_x < 0 || target_y < 0) return;
    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    /* ReDMCSB GROUP.C F0209 lines 2111-2112, 2137-2138, and
     * 2247-2252 keep ActiveGroup.TargetMapX/Y as the last visible party
     * square so C7 approach can continue walking after the party is no
     * longer visible. */
    state->target_map_x = target_x;
    state->target_map_y = target_y;
}

static void csb_v1_runtime_set_active_group_aspect_attacking(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_type,
    int creature_index,
    int attacking)
{
    CSB_V1_RuntimeActiveGroupState *state;
    struct RngState_Compat rng;
    uint32_t seed;
    int random_value;

    if (!profile || creature_index < 0 || creature_index > 3) return;
    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;

    if (creature_type < 0 || creature_type >= DM1_CREATURE_TYPE_COUNT) {
        if (attacking) {
            state->aspect[creature_index] =
                (uint8_t)(state->aspect[creature_index] | 0x80u);
        } else {
            state->aspect[creature_index] =
                (uint8_t)(state->aspect[creature_index] & ~0x80u);
        }
        return;
    }

    /* ReDMCSB GROUP.C F0179 lines 222-305 rewrites ActiveGroup.Aspect
     * with horizontal/vertical offset bits plus attack/non-attack flip
     * state, preserving only IS_ATTACKING and FLIP_BITMAP from the previous
     * value before the update. Firestaff reuses the shared DM1/CSB creature
     * graphic-info helper for that bit layout and keeps the RNG local to
     * this bounded runtime slice. */
    seed = profile->dungeon_seed ^
           (uint32_t)(profile->game_time * 2654435761u) ^
           ((uint32_t)group_thing << 10) ^
           ((uint32_t)(creature_index & 3) << 4) ^
           (attacking ? 0xA17Au : 0x51C3u);
    F0730_COMBAT_RngInit_Compat(&rng, seed);
    random_value = F0732_COMBAT_RngRandom_Compat(&rng, 65536);
    state->aspect[creature_index] = dm1_creature_cycle_aspect_frame(
        creature_type,
        state->aspect[creature_index],
        attacking,
        random_value);
}

static void csb_v1_runtime_set_active_group_direction_all(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint16_t flags;

    if (!profile || !group_record) return;
    direction &= 3;
    flags = csb_v1_runtime_read_u16(group_record + 14);
    flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 8)) |
                       (uint16_t)(direction << 8));
    csb_v1_runtime_write_u16(group_record + 14, flags);

    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    /* ReDMCSB GROUP.C F0205/F0206 mutate ActiveGroup.Directions as a
     * 2-bit per-creature field, and F0184 normalizes it back into the GROUP
     * record when the active group is removed.  This bounded CSB bridge
     * writes the shared group facing into all four slots while Firestaff's
     * raw C04 record can still store only the normalized direction. */
    state->directions = csb_v1_runtime_repeated_group_direction_pack(direction);
}

static int csb_v1_runtime_direction_delta(int from_direction, int to_direction)
{
    return (from_direction - to_direction) & 3;
}

static void csb_v1_runtime_set_active_group_direction_creature(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_index,
    int creature_count,
    int two_half_square_creatures)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint16_t flags;
    int final_direction;

    if (!profile || !group_record || creature_index < 0 ||
        creature_index > 3) {
        return;
    }
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    if (creature_index >= creature_count) return;
    direction &= 3;

    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) {
        state = csb_v1_runtime_active_group_state_for(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            1);
    }
    if (!state) return;
    /* ReDMCSB GROUP.C F0205 lines 1598-1607 debounces two half-square
     * creatures by active-group pointer and game time before mutating
     * directions.  Firestaff's runtime profile owns the equivalent marker so
     * multiple CSB profiles/tests in one process do not share process-global
     * debounce state. */
    if (two_half_square_creatures &&
        profile->half_square_direction_debounce_valid &&
        profile->half_square_direction_debounce_time == profile->game_time &&
        profile->half_square_direction_debounce_group == group_thing) {
        return;
    }

    flags = csb_v1_runtime_read_u16(group_record + 14);
    flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 8)) |
                       (uint16_t)(direction << 8));
    csb_v1_runtime_write_u16(group_record + 14, flags);

    final_direction = direction;
    /* ReDMCSB GROUP.C F0205 lines 1607-1621 changes opposite turns one step
     * at a time.  This bounded bridge uses the deterministic CSB runtime
     * stream seed instead of the process-global source RNG. */
    if (csb_v1_runtime_direction_delta(
            csb_v1_runtime_group_direction_value(
                state->directions,
                creature_index),
            direction) == 2) {
        struct RngState_Compat rng;
        F0730_COMBAT_RngInit_Compat(
            &rng,
            profile->dungeon_seed ^ profile->game_time ^
                ((uint32_t)group_thing << 7) ^
                ((uint32_t)creature_index << 17) ^
                0xF0205u);
        final_direction =
            (direction + 1 +
             (F0732_COMBAT_RngRandom_Compat(&rng, 65536) & 0x0002)) & 3;
    }

    state->directions = csb_v1_runtime_group_directions_set_value(
        state->directions,
        creature_index,
        final_direction);
    if (two_half_square_creatures) {
        /* ReDMCSB F0205 mirrors the direction to the paired half-square
         * creature when F0206 selects the second creature of a two-half group. */
        state->directions = csb_v1_runtime_group_directions_set_value(
            state->directions,
            creature_index ^ 1,
            final_direction);
        profile->half_square_direction_debounce_valid = 1;
        profile->half_square_direction_debounce_time = profile->game_time;
        profile->half_square_direction_debounce_group = group_thing;
    }
}

static void csb_v1_runtime_set_active_group_direction_group(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size)
{
    struct RngState_Compat rng;
    int creature_index;
    int two_half_square_creatures;

    if (!profile || !group_record) return;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    creature_index = creature_count - 1;
    two_half_square_creatures =
        (creature_index != 0 && creature_size == 1) ? 1 : 0;
    if (two_half_square_creatures) {
        creature_index--;
    }
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)group_thing << 5) ^
            ((uint32_t)direction << 19) ^
            0xF0206u);
    do {
        /* ReDMCSB GROUP.C F0206 lines 1632-1645 always lets creature 0 turn
         * and randomly includes higher slots.  This bounded runtime bridge
         * keeps the same shape with Firestaff's deterministic local stream. */
        if (creature_index == 0 ||
            F0732_COMBAT_RngRandom_Compat(&rng, 2) != 0) {
            csb_v1_runtime_set_active_group_direction_creature(
                profile,
                group_thing,
                group_record,
                level,
                map_x,
                map_y,
                direction,
                creature_index,
                creature_count,
                two_half_square_creatures);
        }
    } while (creature_index-- > 0);
}

static void csb_v1_runtime_turn_active_group_toward_attack(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    uint8_t *group_record,
    int level,
    int map_x,
    int map_y,
    int direction,
    int creature_count,
    int creature_size)
{
    CSB_V1_RuntimeActiveGroupState *state;
    struct RngState_Compat rng;
    int i;
    int two_half_square_creatures;

    if (!profile || !group_record) return;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    direction &= 3;

    csb_v1_runtime_sync_active_group_state_from_record(
        profile,
        group_thing,
        group_record,
        level,
        map_x,
        map_y,
        0,
        0);
    state = csb_v1_runtime_active_group_state_for_thing(profile, group_thing);
    if (!state) return;

    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)group_thing << 3) ^
            ((uint32_t)direction << 21) ^
            0xF0209u);

    /* ReDMCSB GROUP.C F0209 lines 2114-2128 turns attacking groups toward
     * G0382_i_CurrentGroupPrimaryDirectionToParty per creature, only always
     * selecting creature 0; F0205 lines 1609-1621 then performs the one-step
     * opposite-turn clamp and mirrors two half-square pairs. */
    for (i = creature_count - 1; i >= 0; --i) {
        if (csb_v1_runtime_group_direction_value(state->directions, i) ==
            direction) {
            continue;
        }
        if (i != 0 && F0732_COMBAT_RngRandom_Compat(&rng, 2) != 0) {
            continue;
        }
        two_half_square_creatures =
            (i != 0 && creature_size == 1) ? 1 : 0;
        csb_v1_runtime_set_active_group_direction_creature(
            profile,
            group_thing,
            group_record,
            level,
            map_x,
            map_y,
            direction,
            i,
            creature_count,
            two_half_square_creatures);
    }
}

static void csb_v1_runtime_compact_active_group_state_after_kill(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_index,
    int creature_count)
{
    CSB_V1_RuntimeActiveGroupState *state;
    uint8_t old_cells;
    uint16_t old_directions;
    int i;

    if (!profile || creature_index < 0 || creature_index >= creature_count) {
        return;
    }
    state = csb_v1_runtime_active_group_state_for(
        profile,
        group_thing,
        level,
        map_x,
        map_y,
        0);
    if (!state) return;

    old_cells = state->cells;
    old_directions = state->directions;
    /* ReDMCSB GROUP.C F0190 lines 892-905 compacts cells/directions for
     * surviving creatures after a partial kill.  F0205/F0206 keep directions
     * in the ACTIVE_GROUP side table, so Firestaff must pack this native state
     * along with the raw GROUP record. */
    for (i = creature_index; i < creature_count - 1 && i < 3; ++i) {
        if (old_cells != 0xFFu) {
            state->cells = (uint8_t)csb_v1_runtime_group_cells_set_value(
                state->cells,
                i,
                csb_v1_runtime_group_cell_value(old_cells, i + 1));
        }
        state->directions = csb_v1_runtime_group_directions_set_value(
            state->directions,
            i,
            csb_v1_runtime_group_direction_value(old_directions, i + 1));
        state->aspect[i] = state->aspect[i + 1];
    }
    if (creature_count > 0 && creature_count <= 4) {
        if (state->cells != 0xFFu) {
            state->cells = (uint8_t)(state->cells &
                (uint8_t)((1u << ((creature_count - 1) * 2)) - 1u));
        }
        state->directions = (uint16_t)(state->directions &
            (uint16_t)((1u << ((creature_count - 1) * 2)) - 1u));
        state->aspect[creature_count - 1] = 0u;
    }
}

static void csb_v1_runtime_write_f0190_flee_delay_to_active_group(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int flee_delay)
{
    CSB_V1_RuntimeActiveGroupState *state;

    if (!profile || flee_delay <= 0) return;
    state = csb_v1_runtime_active_group_state_for(
        profile,
        group_thing,
        level,
        map_x,
        map_y,
        1);
    if (!state) return;
    state->delay_fleeing_from_target =
        (uint8_t)(flee_delay > 255 ? 255 : flee_delay);
}

static void csb_v1_runtime_write_f0190_flee_delay_to_item16(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int flee_delay)
{
    uint16_t i;
    uint8_t delay;

    if (!profile || flee_delay <= 0) return;
    delay = (uint8_t)(flee_delay > 255 ? 255 : flee_delay);

    for (i = 0u; i < profile->csbwin_runtime_item16_count; ++i) {
        CSB_V1_CSBWinRuntimeItem16 *item =
            &profile->csbwin_runtime_item16[i];
        if (!item->valid ||
            !item->live_ai_owned ||
            item->live_ai_group_thing != group_thing ||
            item->live_ai_map_index != level ||
            item->live_ai_map_x != map_x ||
            item->live_ai_map_y != map_y) {
            continue;
        }
        item->delay_or_flee_timer = delay;
    }

    if (!profile->csbwin_body_runtime_summary_valid) return;
    for (i = 0u; i < profile->csbwin_item16_summary_count; ++i) {
        CSB_V1_CSBWin512Item16Summary *item =
            &profile->csbwin_item16[i];
        if (!item->valid ||
            csb_v1_runtime_csbwin_item16_group_thing(item->monster_index) !=
                group_thing ||
            item->current_x != (uint8_t)map_x ||
            item->current_y != (uint8_t)map_y) {
            continue;
        }
        item->ubyte5 = delay;
    }
}

static void csb_v1_runtime_apply_f0190_fear_after_partial_kill(
    CSB_V1_RuntimeProfile *profile,
    uint8_t *group_record,
    uint16_t group_thing,
    uint16_t *inout_flags,
    int creature_type,
    int creature_count,
    int level,
    int map_x,
    int map_y,
    struct RngState_Compat *rng)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;
    struct DM1GroupBehaviorContext_Compat ctx;
    int should_flee = 0;
    int flee_delay = 0;

    if (!profile || !group_record || !inout_flags || !rng ||
        level != profile->current_level ||
        ((*inout_flags) & 0x000Fu) != 6u) {
        return;
    }
    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    if (!creature_profile) return;
    memset(&ctx, 0, sizeof(ctx));
    ctx.currentGroupMapX = map_x;
    ctx.currentGroupMapY = map_y;
    ctx.creatureType = creature_type;
    ctx.creatureInfo.properties = creature_profile->properties;

    /* ReDMCSB GROUP.C F0190 lines 887-890 tests fear only for attacking
     * groups on the party map, stores DelayFleeingFromTarget on ACTIVE_GROUP,
     * and switches GROUP.Behavior to C5.  Firestaff's CSB native active-group
     * array is still bounded, but a CSBWin-imported ITEM16 record that has
     * claimed this live C04 group is the matching active-monster side state. */
    if (F0821_DM1_GROUP_ShouldFrighten_Compat(
            &ctx,
            creature_count,
            rng,
            &should_flee,
            &flee_delay) &&
        should_flee) {
        *inout_flags = (uint16_t)((*inout_flags & ~(uint16_t)0x000Fu) | 5u);
        csb_v1_runtime_write_u16(group_record + 14, *inout_flags);
        csb_v1_runtime_write_f0190_flee_delay_to_active_group(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            flee_delay);
        csb_v1_runtime_write_f0190_flee_delay_to_item16(
            profile,
            group_thing,
            level,
            map_x,
            map_y,
            flee_delay);
    }
}

static void csb_v1_runtime_spawn_f0190_death_smoke(
    CSB_V1_RuntimeProfile *profile,
    int creature_type,
    int killed_cell,
    int map_index,
    int map_x,
    int map_y)
{
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_advance;
    int slot = -1;

    if (!profile) return;
    memset(&input, 0, sizeof(input));
    /* ReDMCSB GROUP.C F0190 lines 907-917 creates C040 smoke at the
     * killed creature cell, with attack 110/190/255 by creature size. */
    input.explosionType = C040_EXPLOSION_SMOKE;
    input.attack = csb_v1_runtime_f0190_smoke_attack_for_creature(creature_type);
    input.mapIndex = map_index;
    input.mapX = map_x;
    input.mapY = map_y;
    input.cell = (killed_cell == EXPLOSION_CELL_CENTERED)
        ? EXPLOSION_CELL_CENTERED : (killed_cell & 3);
    input.centered = (input.cell == EXPLOSION_CELL_CENTERED) ? 1 : 0;
    input.currentTick = (int)profile->game_time;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    if (F0821_EXPLOSION_Create_Compat(
            &input,
            &profile->explosions,
            &slot,
            &first_advance)) {
        csb_v1_runtime_schedule_explosion_advance_event(profile, &first_advance);
    }
}

static void csb_v1_runtime_pack_dead_group_creature(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_index,
    struct RngState_Compat *rng)
{
    uint16_t flags;
    int raw_count;
    int creature_count;
    int cells;
    int killed_cell;
    int creature_type;
    int i;

    if (!dungeon || !group_record ||
        creature_index < 0 || creature_index > 3) {
        return;
    }
    flags = csb_v1_runtime_read_u16(group_record + 14);
    raw_count = (int)((flags >> 5) & 0x03u);
    creature_count = raw_count + 1;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    if (creature_index >= creature_count) return;
    cells = group_record[5];
    killed_cell = (cells == 0xFF)
        ? EXPLOSION_CELL_CENTERED
        : csb_v1_runtime_group_cell_value(cells, creature_index);
    creature_type = group_record[4];
    csb_v1_runtime_spawn_f0190_death_smoke(
        profile,
        creature_type,
        killed_cell,
        level,
        map_x,
        map_y);
    csb_v1_runtime_drop_creature_fixed_possessions(
        profile,
        dungeon,
        creature_type,
        killed_cell,
        level,
        map_x,
        map_y);

    if (creature_count <= 1) {
        /* ReDMCSB GROUP.C F0190 lines 831-840 calls F0189_GROUP_Delete
         * when the last creature dies.  This bounded CSB bridge removes the
         * C04 thing from the square chain, drops the carried Slot chain, and
         * marks the real-format record unused; fixed possessions, sounds, and
         * active-group side state are later slices. */
        csb_v1_runtime_drop_group_slot_possessions(
            profile,
            dungeon,
            group_record,
            level,
            map_x,
            map_y);
        csb_v1_runtime_delete_group_events_at_square(
            profile,
            level,
            map_x,
            map_y);
        csb_v1_runtime_clear_active_group_state(
            profile,
            group_thing,
            level,
            map_x,
            map_y);
        csb_v1_runtime_unlink_group_thing_from_square(
            dungeon,
            group_thing,
            level,
            map_x,
            map_y);
        memset(group_record, 0, 16);
        csb_v1_runtime_write_u16(group_record + 0, 0xFFFFu);
        csb_v1_runtime_write_u16(group_record + 2, 0xFFFEu);
        return;
    }

    if ((flags & 0x000fu) == 6u) {
        csb_v1_runtime_rewrite_group_events_after_creature_death(
            profile,
            level,
            map_x,
            map_y,
            creature_index);
        csb_v1_runtime_apply_f0190_fear_after_partial_kill(
            profile,
            group_record,
            group_thing,
            &flags,
            creature_type,
            creature_count,
            level,
            map_x,
            map_y,
            rng);
    }

    /* ReDMCSB GROUP.C F0190 lines 892-905 compacts Health, directions,
     * cells, active aspect, then decrements GROUP.Count.  CSB's bounded
     * real-format bridge owns Health/Cells and mirrors the native
     * active-group Cells/Directions/Aspect side-table compaction here. */
    csb_v1_runtime_compact_active_group_state_after_kill(
        profile,
        group_thing,
        level,
        map_x,
        map_y,
        creature_index,
        creature_count);
    for (i = creature_index; i < creature_count - 1; ++i) {
        uint16_t next_hp =
            csb_v1_runtime_read_u16(group_record + 6 + (i + 1) * 2);
        int next_cell = csb_v1_runtime_group_cell_value(cells, i + 1);
        csb_v1_runtime_write_u16(group_record + 6 + i * 2, next_hp);
        cells = csb_v1_runtime_group_cells_set_value(cells, i, next_cell);
    }
    csb_v1_runtime_write_u16(group_record + 6 + (creature_count - 1) * 2, 0);
    if (cells != 0xFF) {
        cells &= (1 << ((creature_count - 1) * 2)) - 1;
    }
    group_record[5] = (uint8_t)(cells & 0xFF);
    flags = (uint16_t)((flags & ~(uint16_t)(0x03u << 5)) |
                       (uint16_t)(((raw_count - 1) & 0x03) << 5));
    csb_v1_runtime_write_u16(group_record + 14, flags);
}

static int csb_v1_runtime_apply_group_fall_damage(
    CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *group_record;
    struct RngState_Compat rng;
    uint16_t flags;
    int thing_type = -1;
    int thing_size = 0;
    int creature_count;
    int random_window;
    int base_attack;
    int killed_some = 0;
    int i;

    if (!profile || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    group_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        group_thing,
        &thing_type,
        &thing_size);
    if (!group_record || thing_type != 4 || thing_size < 16) return 0;

    flags = csb_v1_runtime_read_u16(group_record + 14);
    creature_count = (int)((flags >> 5) & 0x03u) + 1;
    if (creature_count < 1) creature_count = 1;
    if (creature_count > 4) creature_count = 4;
    random_window = (20 >> 3) + 1;
    base_attack = 20 - random_window;
    random_window <<= 1;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)map_index << 4) ^
            ((uint32_t)map_x << 8) ^
            ((uint32_t)map_y << 16) ^
            0xF0191u);

    /* ReDMCSB MOVESENS.C F0267 lines 596-617 applies GROUP.C F0191 with
     * attack 20 after a moving C04 group falls through a pit.  GROUP.C F0191
     * lines 952-973 fans out attack +/- 1/8 from Count down to creature 0.
     * This bounded bridge reuses the existing real-format F0190 pack/drop/
     * smoke helper; ActiveGroup side state and audio remain separate work. */
    for (i = creature_count - 1; i >= 0; --i) {
        uint16_t hp;
        int damage;

        group_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            group_thing,
            &thing_type,
            &thing_size);
        if (!group_record || thing_type != 4 || thing_size < 16) {
            return killed_some ? 2 : 0;
        }
        hp = csb_v1_runtime_read_u16(group_record + 6 + i * 2);
        if (hp == 0) continue;
        damage = base_attack + F0732_COMBAT_RngRandom_Compat(
            &rng,
            random_window);
        if (damage < 1) damage = 1;
        if (damage >= (int)hp) {
            killed_some = 1;
            csb_v1_runtime_pack_dead_group_creature(
                profile,
                dungeon,
                group_record,
                group_thing,
                map_index,
                map_x,
                map_y,
                i,
                &rng);
            if (creature_count <= 1) {
                return 2;
            }
        } else {
            csb_v1_runtime_write_u16(
                group_record + 6 + i * 2,
                (uint16_t)((int)hp - damage));
        }
    }
    group_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        group_thing,
        &thing_type,
        &thing_size);
    if (!group_record || thing_type != 4 || thing_size < 16) {
        return killed_some ? 2 : 0;
    }
    if (csb_v1_runtime_read_u16(group_record + 0) == 0xFFFFu) {
        return 2;
    }
    return killed_some ? 1 : 0;
}

static int csb_v1_runtime_apply_explosion_group_action(
    CSB_V1_RuntimeProfile *profile,
    const struct CombatAction_Compat *action,
    struct RngState_Compat *rng)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *thing_record;
    int first_thing;
    int thing;
    int guard;
    int thing_type;
    int thing_size;
    int applied = 0;

    if (!profile || !action || !rng ||
        action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP ||
        action->rawAttackValue <= 0 ||
        !profile->dungeon_handle) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        action->targetMapIndex,
        action->targetMapX,
        action->targetMapY);
    if (first_thing < 0) return 0;

    for (guard = 0, thing = first_thing;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record || thing_size < 16) return applied;
        if (thing_type == 4) {
            uint16_t flags;
            int creature_count;
            int random_window;
            int base_attack;
            int i;

            flags = csb_v1_runtime_read_u16(thing_record + 14);
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            if (creature_count < 1) creature_count = 1;
            if (creature_count > 4) creature_count = 4;
            random_window = (action->rawAttackValue >> 3) + 1;
            base_attack = action->rawAttackValue - random_window;
            random_window <<= 1;

            /* ReDMCSB GROUP.C F0191 lines 952-973 applies the same
             * +/- 1/8 all-creature attack fanout used by explosion group
             * impacts in PROJEXPL.C F0213/F0220.  This real-format bridge
             * mutates GROUP.Health[4]; active-group aspect cleanup, drops,
             * and fixed possessions remain later CSB runtime slices. */
            for (i = 0; i < creature_count; ) {
                uint8_t *hp_ptr = thing_record + 6 + i * 2;
                uint16_t hp = csb_v1_runtime_read_u16(hp_ptr);
                int damage;
                if (hp == 0) {
                    i++;
                    continue;
                }
                damage = base_attack +
                    F0732_COMBAT_RngRandom_Compat(rng, random_window);
                if (damage < 1) damage = 1;
                if (damage >= (int)hp) {
                    csb_v1_runtime_pack_dead_group_creature(
                        profile,
                        dungeon,
                        thing_record,
                        (uint16_t)thing,
                        action->targetMapIndex,
                        action->targetMapX,
                        action->targetMapY,
                        i,
                        rng);
                    creature_count--;
                    if (creature_count <= 0) {
                        applied++;
                        return applied;
                    }
                } else {
                    csb_v1_runtime_write_u16(
                        hp_ptr,
                        (uint16_t)((int)hp - damage));
                    i++;
                }
                applied++;
            }
            return applied;
        }
        thing = csb_v1_runtime_read_u16(thing_record + 0);
    }
    return applied;
}

static int csb_v1_runtime_projectile_weapon_type_is_kept_sharp(int weapon_type)
{
    return weapon_type == 8  ||  /* C08_WEAPON_DAGGER */
           weapon_type == 27 ||  /* C27_WEAPON_ARROW */
           weapon_type == 28 ||  /* C28_WEAPON_SLAYER */
           weapon_type == 31 ||  /* C31_WEAPON_POISON_DART */
           weapon_type == 32;    /* C32_WEAPON_THROWING_STAR */
}

static int csb_v1_runtime_link_projectile_thing_to_group_slot_tail(
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    uint16_t associated_thing)
{
    uint16_t group_slot;
    uint16_t tail;
    int guard;

    if (!dungeon || !group_record ||
        associated_thing == 0xFFFEu ||
        associated_thing == 0xFFFFu) {
        return 0;
    }
    group_slot = csb_v1_runtime_read_u16(group_record + 2);
    if (group_slot == 0xFFFEu || group_slot == 0xFFFFu) {
        uint8_t *record;
        int thing_type;
        int thing_size;
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            associated_thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        csb_v1_runtime_write_u16(record, 0xFFFEu);
        csb_v1_runtime_write_u16(group_record + 2, associated_thing);
        return 1;
    }

    tail = group_slot;
    for (guard = 0; guard < 64 && tail != 0xFFFEu && tail != 0xFFFFu;
         ++guard) {
        uint8_t *record;
        uint16_t next_thing;
        int thing_type;
        int thing_size;
        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            tail,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        next_thing = csb_v1_runtime_read_u16(record);
        if (next_thing == 0xFFFEu || next_thing == 0xFFFFu) {
            uint8_t *associated_record =
                csb_v1_runtime_mutable_thing_record(
                    dungeon,
                    associated_thing,
                    &thing_type,
                    &thing_size);
            if (!associated_record || thing_size < 2) return 0;
            csb_v1_runtime_write_u16(associated_record, 0xFFFEu);
            csb_v1_runtime_write_u16(record, associated_thing);
            return 1;
        }
        tail = next_thing;
    }
    return 0;
}

static int csb_v1_runtime_maybe_attach_projectile_weapon_to_group_slot(
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    const struct ProjectileInstance_Compat *projectile)
{
    const struct CreatureBehaviorProfile_Compat *creature_profile;
    uint8_t *weapon_record;
    uint16_t associated_thing;
    int thing_type;
    int thing_size;
    int weapon_type;
    int creature_type;

    if (!dungeon || !group_record || !projectile) return 0;
    associated_thing = (uint16_t)projectile->reserved1;
    if (associated_thing == 0u ||
        associated_thing == 0xFFFEu ||
        associated_thing == 0xFFFFu ||
        ((associated_thing >> 10) & 0x0Fu) != 5u ||
        (projectile->flags & PROJECTILE_FLAG_CREATES_EXPLOSION) != 0) {
        return 0;
    }
    creature_type = group_record[4];
    creature_profile = CREATURE_GetProfile_Compat(creature_type);
    if (!creature_profile ||
        ((creature_profile->attributes &
          CREATURE_ATTR_MASK_KEEP_THROWN_SHARP_WEAPONS) == 0)) {
        return 0;
    }
    weapon_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        associated_thing,
        &thing_type,
        &thing_size);
    if (!weapon_record || thing_type != 5 || thing_size < 4) return 0;
    weapon_type = (int)(csb_v1_runtime_read_u16(weapon_record + 2) & 0x7Fu);
    if (!csb_v1_runtime_projectile_weapon_type_is_kept_sharp(weapon_type)) {
        return 0;
    }

    /* ReDMCSB PROJEXPL.C F0217 lines 540-553 selects GROUP.Slot for
     * surviving dagger/arrow/slayer/poison-dart/throwing-star impacts
     * against creatures with KEEP_THROWN_SHARP_WEAPONS; F0215 lines
     * 239-256 then appends the projectile associated thing to that slot
     * list instead of moving it to the floor square. */
    return csb_v1_runtime_link_projectile_thing_to_group_slot_tail(
        dungeon,
        group_record,
        associated_thing);
}

static int csb_v1_runtime_apply_projectile_group_action(
    CSB_V1_RuntimeProfile *profile,
    const struct CombatAction_Compat *action,
    struct ProjectileInstance_Compat *projectile)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *thing_record;
    int first_thing;
    int thing;
    int guard;
    int thing_type;
    int thing_size;
    struct RngState_Compat rng;

    if (!profile || !action || !projectile ||
        action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP ||
        action->rawAttackValue <= 0 ||
        !profile->dungeon_handle) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        action->targetMapIndex,
        action->targetMapX,
        action->targetMapY);
    if (first_thing < 0) return 0;
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)(projectile->slotIndex & 0xFF) << 4) ^
            ((uint32_t)(projectile->mapX & 0xFF) << 12) ^
            ((uint32_t)(projectile->mapY & 0xFF) << 20) ^
            0xF0190u);

    for (guard = 0, thing = first_thing;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record || thing_size < 16) return 0;
        if (thing_type == 4) {
            uint16_t flags;
            uint16_t hp;
            uint8_t *hp_ptr;
            int creature_count;
            int creature_index = -1;
            int cells;
            int i;

            flags = csb_v1_runtime_read_u16(thing_record + 14);
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            if (creature_count < 1) creature_count = 1;
            if (creature_count > 4) creature_count = 4;
            cells = thing_record[5];
            if (cells == 0xFF) {
                creature_index = 0;
            } else {
                for (i = 0; i < creature_count; ++i) {
                    if (csb_v1_runtime_group_cell_value(cells, i) ==
                        (action->targetCell & 3)) {
                        creature_index = i;
                        break;
                    }
                }
            }
            if (creature_index < 0 || creature_index >= creature_count) {
                return 0;
            }
            hp_ptr = thing_record + 6 + creature_index * 2;
            hp = csb_v1_runtime_read_u16(hp_ptr);
            if (hp == 0) return 0;
            if (action->rawAttackValue >= (int)hp) {
                csb_v1_runtime_pack_dead_group_creature(
                    profile,
                    dungeon,
                    thing_record,
                    (uint16_t)thing,
                    action->targetMapIndex,
                    action->targetMapX,
                    action->targetMapY,
                    creature_index,
                    &rng);
                return 1;
            }

            csb_v1_runtime_write_u16(
                hp_ptr,
                (uint16_t)((int)hp - action->rawAttackValue));
            if (csb_v1_runtime_maybe_attach_projectile_weapon_to_group_slot(
                    dungeon,
                    thing_record,
                    projectile)) {
                projectile->reserved1 = 0xFFFF;
            }
            return 1;
        }
        thing = csb_v1_runtime_read_u16(thing_record + 0);
    }
    return 0;
}

static void csb_v1_runtime_apply_creature_attack_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *thing_record;
    uint16_t flags;
    int thing;
    int guard;
    int creature_index;
    int creature_count;
    int thing_type;
    int thing_size;
    int champion_index;
    int creature_cell;
    int damage;
    struct CombatantCreatureSnapshot_Compat attacker;
    struct CombatantChampionSnapshot_Compat defender;
    struct CombatResult_Compat combat;
    struct RngState_Compat rng;

    if (!profile || !record || !profile->dungeon_handle ||
        !profile->party_state_valid) {
        return;
    }
    creature_index = record->eventType - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    if (creature_index < 0 || creature_index > 3) return;

    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB GROUP.C F0209 lines 1443-1515 processes C38-C41 as
     * per-creature attack decisions after C6 attack entry and calls the
     * common creature melee damage path (PROJEXPL.C F0230, then
     * CHAMPION.C F0321).  CSB keeps the real-format group lookup and target
     * selection here, then delegates the bounded damage roll to the shared
     * M10 combat resolver used by DM1.  Full CSB runtime RNG state, poison,
     * armor inventory, rest wake, ranged attacks, and broader aspect timing
     * remain later slices. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 4 && thing_size >= 16) {
            flags = csb_v1_runtime_read_u16(thing_record + 14);
            if ((flags & 0x000Fu) != 6u) return;
            creature_count = (int)((flags >> 5) & 0x03u) + 1;
            if (creature_index >= creature_count) return;
            creature_cell = (thing_record[3] == 0xFFu)
                ? 0
                : ((int)thing_record[3] >> (creature_index * 2)) & 0x03;
            champion_index = csb_v1_runtime_target_champion_for_adjacent_attack(
                profile,
                record->mapX,
                record->mapY,
                creature_cell);
            if (champion_index < 0) {
                champion_index =
                    csb_v1_runtime_first_living_champion(&profile->party_state);
            }
            if (champion_index < 0) return;
            csb_v1_runtime_set_active_group_direction_creature(
                profile,
                (uint16_t)thing,
                thing_record,
                record->mapIndex,
                record->mapX,
                record->mapY,
                csb_v1_runtime_direction_from_source_to_destination(
                    record->mapX,
                    record->mapY,
                    profile->party_x,
                    profile->party_y),
                creature_index,
                creature_count,
                0);
            csb_v1_runtime_set_active_group_aspect_attacking(
                profile,
                (uint16_t)thing,
                record->mapIndex,
                record->mapX,
                record->mapY,
                (int)thing_record[4],
                creature_index,
                1);
            csb_v1_runtime_request_creature_attack_sound(
                profile,
                (int)thing_record[4],
                record->mapX,
                record->mapY);
            if ((int)thing_record[4] == DM1_CREATURE_TYPE_GIGGLER) {
                (void)csb_v1_runtime_apply_giggler_steal_timeline_record(
                    profile,
                    dungeon,
                    thing_record,
                    record,
                    creature_index,
                    champion_index);
                return;
            }
            memset(&combat, 0, sizeof(combat));
            if (!csb_v1_runtime_fill_creature_combat_snapshot(
                    (int)thing_record[4],
                    creature_index,
                    &attacker)) {
                return;
            }
            if (!csb_v1_runtime_fill_defender_combat_snapshot(
                    profile,
                    champion_index,
                    &defender)) {
                return;
            }
            (void)F0730_COMBAT_RngInit_Compat(
                &rng,
                csb_v1_runtime_creature_attack_seed(
                    profile,
                    record,
                    attacker.creatureType,
                    creature_index,
                    champion_index));
            if (!F0736_COMBAT_ResolveCreatureMelee_Compat(
                    &attacker,
                    &defender,
                    &rng,
                    &combat)) {
                return;
            }
            damage = (combat.outcome == COMBAT_OUTCOME_HIT_DAMAGE &&
                      combat.damageApplied > 0)
                ? combat.damageApplied
                : 0;
            if (damage <= 0) {
                if (!profile->game_over) {
                    csb_v1_runtime_schedule_c38_followup_event(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        creature_index,
                        (uint32_t)csb_v1_runtime_creature_attack_ticks(
                            (int)thing_record[4]));
                }
                return;
            }
            if (profile->party_state.Champions[champion_index].CurrentHealth <=
                damage) {
                profile->party_state.Champions[champion_index].Wounds =
                    (uint16_t)(profile->party_state
                                   .Champions[champion_index]
                                   .Wounds |
                               (uint16_t)combat.woundMaskAdded);
                csb_v1_runtime_mark_champion_dead(profile, champion_index);
            } else {
                profile->party_state.Champions[champion_index].Wounds =
                    (uint16_t)(profile->party_state
                                   .Champions[champion_index]
                                   .Wounds |
                               (uint16_t)combat.woundMaskAdded);
                profile->party_state.Champions[champion_index].CurrentHealth =
                    (int16_t)(profile->party_state
                                  .Champions[champion_index]
                                  .CurrentHealth -
                              damage);
                if (combat.poisonAttackPending > 0) {
                    csb_v1_runtime_apply_poison_attack_to_champion(
                        profile,
                        champion_index,
                        combat.poisonAttackPending);
                }
            }
            if (!profile->game_over) {
                /* ReDMCSB GROUP.C F0209 lines 2343-2422 computes the next
                 * C38 attack time and F0208 lines 1820-1834 may convert the
                 * paired earlier aspect update into C33..C36 with C.Ticks
                 * carrying the remaining attack delay; when C33 dispatches
                 * it prepares the matching C38.  This bounded CSB bridge
                 * keeps the source AttackTicks base and explicit C33->C38
                 * handoff; RNG jitter and live ActiveGroup aspect sprites
                 * remain later slices. */
                uint32_t attack_delay =
                    (uint32_t)csb_v1_runtime_creature_attack_ticks(
                        (int)thing_record[4]);
                csb_v1_runtime_schedule_c38_followup_event(
                    profile,
                    record->mapIndex,
                    record->mapX,
                    record->mapY,
                    (int)thing_record[4],
                    creature_index,
                    attack_delay);
            }
            return;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_trigger_floor_sensor_event(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    struct DM1_Event_V1 event;
    int raw_square;
    int square_type;
    int event_type;

    if (!profile || !result) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data) return;
    if (level < 0 || level >= dungeon->level_count) return;

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        target_x,
        target_y);
    if (raw_square < 0) return;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    event_type = csb_v1_runtime_square_event_type_for_sensor_target(square_type);
    if (event_type == DM1_EVENT_NONE) return;

    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        level,
        profile->game_time);
    event.type = (uint8_t)event_type;
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)target_cell;
    event.c_effect = (uint8_t)sensor_effect;
    if (dm1v1_event_add(&profile->timeline_queue, &event) >= 0) {
        result->sensor_event_count++;
        result->sensor_last_event_type = event_type;
    }
}

static int csb_v1_runtime_queue_remote_square_event(
    CSB_V1_RuntimeProfile *profile,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell)
{
    const CSB_V1_DungeonData *dungeon;
    struct DM1_Event_V1 event;
    int raw_square;
    int square_type;
    int event_type;

    if (!profile) return 0;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data) return 0;

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        profile->current_level,
        target_x,
        target_y);
    if (raw_square < 0) return 0;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    event_type = csb_v1_runtime_square_event_type_for_sensor_target(square_type);
    if (event_type == DM1_EVENT_NONE) return 0;

    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        profile->current_level,
        profile->game_time);
    event.type = (uint8_t)event_type;
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)target_cell;
    event.c_effect = (uint8_t)sensor_effect;
    return dm1v1_event_add(&profile->timeline_queue, &event) >= 0 ? 1 : 0;
}

static int csb_v1_runtime_object_type_from_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing)
{
    const uint8_t *record;
    int thing_type;
    int thing_size;

    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &thing_size);
    if (!record || thing_type <= 4 || thing_type >= 14 || thing_size < 4) {
        return -1;
    }
    return (int)(csb_v1_runtime_read_u16(record + 2) & 0x007Fu);
}

static void csb_v1_runtime_process_object_floor_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_DungeonData *dungeon,
    uint16_t placed_thing,
    int level,
    int map_x,
    int map_y)
{
    int first_thing;
    int thing;
    int object_type;
    int guard;

    if (!profile || !dungeon || !dungeon->raw_data) return;
    if (profile->dungeon_handle != dungeon) return;
    if (level < 0 || level >= dungeon->level_count) return;
    object_type = csb_v1_runtime_object_type_from_thing(dungeon, placed_thing);
    if (object_type < 0) return;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (first_thing < 0 || first_thing == 0xFFFE || first_thing == 0xFFFF) {
        return;
    }

    /* ReDMCSB MOVESENS.C F0276 lines 1608-1655 classifies the moving
     * THING, scans the square for matching object types before add, then
     * lines 1691-1694 trigger C004 only when the sensor data matches
     * F0032_OBJECT_GetType(P0590_T_Thing) and no same-type object is already
     * present. Firestaff calls this after link, so the scan ignores the
     * just-placed THING and treats any other same-type object as the source
     * L0775_B_SquareContainsThingOfSameType guard. */
    thing = first_thing;
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int same_type_present = 0;
        int scan;
        int scan_guard;
        int sensor_effect;
        int target_cell;
        int target_x;
        int target_y;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record) break;
        if (thing_type >= 4) break;
        if (thing_type != 3 || thing_size < 8) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        type_data = csb_v1_runtime_read_u16(record + 2);
        flags_word = csb_v1_runtime_read_u16(record + 4);
        target_word = csb_v1_runtime_read_u16(record + 6);
        sensor_type = (int)(type_data & 0x007Fu);
        sensor_data = (int)(type_data >> 7);
        if (sensor_type != DM1_SENSOR_FLOOR_OBJECT ||
            sensor_data != object_type) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        scan = first_thing;
        for (scan_guard = 0;
             scan_guard < 128 && scan != 0xFFFE && scan != 0xFFFF;
             ++scan_guard) {
            int scan_type;
            const uint8_t *scan_record = csb_v1_dungeon_get_thing_record(
                dungeon,
                (uint16_t)scan,
                &scan_type,
                NULL,
                NULL);
            if (!scan_record) break;
            if ((uint16_t)scan != placed_thing &&
                scan_type > 4 &&
                scan_type < 14 &&
                csb_v1_runtime_object_type_from_thing(
                    dungeon,
                    (uint16_t)scan) == object_type) {
                same_type_present = 1;
                break;
            }
            scan = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)scan);
        }
        if (same_type_present) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        sensor_effect = (int)((flags_word >> 3) & 0x03u);
        if ((flags_word >> 5) & 0x01u) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        if (sensor_effect == DM1_EFFECT_HOLD) {
            sensor_effect = DM1_EFFECT_SET;
        }
        target_cell = (int)((target_word >> 4) & 0x03u);
        target_x = (int)((target_word >> 6) & 0x1Fu);
        target_y = (int)((target_word >> 11) & 0x1Fu);
        csb_v1_runtime_trigger_remote_sensor_event(
            profile,
            level,
            sensor_effect,
            target_x,
            target_y,
            target_cell);
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static int csb_v1_runtime_find_wall_cell_object_of_type(
    const CSB_V1_DungeonData *dungeon,
    int first_thing,
    int cell,
    int object_type,
    uint16_t *out_thing)
{
    int thing = first_thing;
    int guard;

    if (out_thing) *out_thing = 0xFFFFu;
    if (!dungeon || object_type < 0) return 0;
    for (guard = 0;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        int thing_type;
        const uint8_t *record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            NULL);
        if (!record) break;
        if ((thing & 3) == (cell & 3) && thing_type > 4 && thing_type < 14 &&
            csb_v1_runtime_object_type_from_thing(
                dungeon,
                (uint16_t)thing) == object_type) {
            if (out_thing) *out_thing = (uint16_t)thing;
            return 1;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    return 0;
}

static int csb_v1_runtime_find_first_square_object(
    const CSB_V1_DungeonData *dungeon,
    int first_thing,
    uint16_t *out_thing)
{
    int thing = first_thing;
    int guard;

    if (out_thing) *out_thing = 0xFFFFu;
    if (!dungeon) return 0;
    for (guard = 0;
         guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        int thing_type;
        const uint8_t *record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            NULL);
        if (!record) break;
        if (thing_type > 4 && thing_type < 14) {
            if (out_thing) *out_thing = (uint16_t)thing;
            return 1;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
    return 0;
}

static int csb_v1_runtime_has_later_wall_cell_sensor(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int cell)
{
    int guard;

    if (!dungeon || thing == 0xFFFEu || thing == 0xFFFFu) return 0;
    thing = csb_v1_runtime_sensor_next_thing(dungeon, thing);
    for (guard = 0;
         guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        const uint8_t *record = csb_v1_dungeon_get_thing_record(
            dungeon,
            thing,
            &thing_type,
            NULL,
            NULL);
        if (!record || thing_type >= 4) break;
        if (thing_type == 3 && ((thing & 3) == (uint16_t)(cell & 3))) {
            return 1;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, thing);
    }
    return 0;
}

static int csb_v1_runtime_remove_wall_sensor_after_previous(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    uint16_t previous_thing,
    uint16_t sensor_thing)
{
    uint8_t *previous_record;
    uint8_t *sensor_record;
    uint16_t next_thing;
    int previous_size;
    int sensor_type;
    int sensor_size;

    if (!dungeon || previous_thing == sensor_thing ||
        sensor_thing == 0xFFFEu || sensor_thing == 0xFFFFu) {
        return 0;
    }
    previous_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        previous_thing,
        NULL,
        &previous_size);
    sensor_record = csb_v1_runtime_mutable_thing_record(
        dungeon,
        sensor_thing,
        &sensor_type,
        &sensor_size);
    if (!previous_record || previous_size < 2 ||
        !sensor_record || sensor_size < 2 ||
        sensor_type != 3) {
        return 0;
    }
    (void)level;
    (void)map_x;
    (void)map_y;
    next_thing = csb_v1_runtime_read_u16(sensor_record);
    csb_v1_runtime_write_u16(previous_record, next_thing);
    csb_v1_runtime_write_u16(sensor_record, 0xFFFFu);
    return 1;
}

static int csb_v1_runtime_rotate_wall_cell_sensors(
    CSB_V1_DungeonData *dungeon,
    int level,
    int map_x,
    int map_y,
    int cell)
{
    uint8_t *first_ptr;
    uint8_t *prev_first_link = NULL;
    uint8_t *first_sensor_record = NULL;
    uint8_t *last_sensor_record = NULL;
    uint16_t thing;
    uint16_t first_sensor_thing = 0xFFFFu;
    uint16_t after_first;
    uint16_t last_next;
    int guard;

    if (!dungeon || !dungeon->raw_data) return 0;
    first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        map_x,
        map_y);
    if (!first_ptr) return 0;

    thing = csb_v1_runtime_read_u16(first_ptr);
    prev_first_link = first_ptr;
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        int thing_size;
        uint8_t *record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 3 &&
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(thing) ==
                (cell & 3)) {
            first_sensor_thing = thing;
            first_sensor_record = record;
            break;
        }
        prev_first_link = record;
        thing = csb_v1_runtime_read_u16(record);
    }
    if (!first_sensor_record) return 0;

    thing = csb_v1_runtime_read_u16(first_sensor_record);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        int thing_size;
        uint8_t *record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == 3 &&
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(thing) ==
                (cell & 3)) {
            last_sensor_record = record;
            break;
        }
        thing = csb_v1_runtime_read_u16(record);
    }
    if (!last_sensor_record) return 0;

    thing = csb_v1_runtime_read_u16(last_sensor_record);
    for (guard = 0; guard < 128 && thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        int thing_type;
        int thing_size;
        uint8_t *record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            thing,
            &thing_type,
            &thing_size);
        if (!record || thing_size < 2 || thing_type != 3) break;
        if (csb_v1_teleporter_rotation_thing_cell_pc34_compat(thing) ==
            (cell & 3)) {
            last_sensor_record = record;
        }
        thing = csb_v1_runtime_read_u16(record);
    }

    /* ReDMCSB MOVESENS.C F0271 lines 1113-1138: after a local
     * C02 rotation effect, unlink the first matching sensor and append
     * it after the last matching sensor in that square/cell's sensor run. */
    after_first = csb_v1_runtime_read_u16(first_sensor_record);
    csb_v1_runtime_write_u16(prev_first_link, after_first);
    last_next = csb_v1_runtime_read_u16(last_sensor_record);
    csb_v1_runtime_write_u16(first_sensor_record, last_next);
    csb_v1_runtime_write_u16(last_sensor_record, first_sensor_thing);
    return 1;
}

static int csb_v1_runtime_trigger_wall_ornament_click_core(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    int object_type_override,
    uint16_t *leader_hand_thing)
{
    CSB_V1_DungeonData *dungeon;
    int first_thing;
    int thing;
    int previous_thing;
    int guard;
    int queued = 0;
    uint16_t hand_thing = 0xFFFFu;
    int object_type = object_type_override;

    if (!profile) return 0;
    if (!profile->dungeon_handle) return 0;
    dungeon = (CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1) return 0;
    if (csb_v1_dungeon_get_raw_square(
            dungeon,
            profile->current_level,
            map_x,
            map_y) < 0) {
        return 0;
    }

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        profile->current_level,
        map_x,
        map_y);
    if (first_thing < 0 || first_thing == 0xFFFE || first_thing == 0xFFFF) {
        return 0;
    }
    if (leader_hand_thing) {
        hand_thing = *leader_hand_thing;
        if (hand_thing != 0xFFFEu && hand_thing != 0xFFFFu) {
            object_type = csb_v1_runtime_object_type_from_thing(
                dungeon,
                hand_thing);
        }
    }

    /* ReDMCSB: MOVESENS.C F0275/F0276 handles wall ornament clicks by
     * matching the clicked cell, resolving Revert/HOLD, applying local
     * storage/rotation side effects, then calling F0272_SENSOR_TriggerEffect. */
    previous_thing = first_thing;
    thing = first_thing;
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF;
         ++guard) {
        uint8_t *record;
        int thing_type;
        int thing_size;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int sensor_cell;
        int square_has_object = 0;
        int same_type_present = 0;
        int different_type_present = 0;
        int trigger = 1;
        int sensor_effect;
        int target_cell;
        int target_x;
        int target_y;
        int scan;
        int scan_guard;
        uint16_t storage_thing = 0xFFFFu;
        uint16_t square_thing = 0xFFFFu;
        uint16_t generated_thing = 0xFFFFu;
        int storage_action = 0;
        int do_not_trigger = 0;
        int rotate_after = 0;
        int remove_current_sensor = 0;

        record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!record) break;
        if (thing_type >= 4) break;
        if (thing_type != 3 || thing_size < 8) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        sensor_cell = thing & 3;
        if (sensor_cell != (cell & 3)) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        scan = first_thing;
        for (scan_guard = 0;
             scan_guard < 128 && scan != 0xFFFE && scan != 0xFFFF;
             ++scan_guard) {
            int scan_type;
            int scan_object_type;
            const uint8_t *scan_record = csb_v1_dungeon_get_thing_record(
                dungeon,
                (uint16_t)scan,
                &scan_type,
                NULL,
                NULL);
            (void)scan_record;
            if (!scan_record) break;
            if ((scan & 3) == (cell & 3) && scan_type > 4 && scan_type < 14) {
                square_has_object = 1;
                scan_object_type = csb_v1_runtime_object_type_from_thing(
                    dungeon,
                    (uint16_t)scan);
                if (scan_object_type == object_type) {
                    same_type_present = 1;
                } else {
                    different_type_present = 1;
                }
            }
            scan = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)scan);
        }

        type_data = csb_v1_runtime_read_u16(record + 2);
        flags_word = csb_v1_runtime_read_u16(record + 4);
        target_word = csb_v1_runtime_read_u16(record + 6);
        sensor_type = (int)(type_data & 0x007Fu);
        sensor_data = (int)(type_data >> 7);
        switch (sensor_type) {
        case 1: /* C001_SENSOR_WALL_ORNAMENT_CLICK */
            trigger = (object_type < 0 && !square_has_object) ? 1 : 0;
            break;
        case 2: /* C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT */
            trigger = (object_type >= 0 &&
                       !same_type_present &&
                       sensor_data == object_type) ? 1 : 0;
            break;
        case 3: /* C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT */
            trigger = (object_type >= 0 &&
                       !different_type_present &&
                       sensor_data != object_type) ? 1 : 0;
            break;
        case DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED:
        case DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE:
        case DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR:
            if (!leader_hand_thing ||
                csb_v1_runtime_has_later_wall_cell_sensor(
                    dungeon,
                    (uint16_t)thing,
                    cell)) {
                trigger = 0;
                break;
            }
            trigger = (object_type >= 0 && sensor_data == object_type) ? 1 : 0;
            if (sensor_type == DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE) {
                rotate_after = 1;
            } else if (sensor_type ==
                       DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR) {
                remove_current_sensor = 1;
            }
            storage_action = 3;
            break;
        case DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE:
            if (!leader_hand_thing ||
                csb_v1_runtime_has_later_wall_cell_sensor(
                    dungeon,
                    (uint16_t)thing,
                    cell)) {
                trigger = 0;
                break;
            }
            trigger = (object_type < 0) ? 1 : 0;
            if (trigger) rotate_after = 1;
            storage_action = 4;
            break;
        case DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE:
            if (!leader_hand_thing) {
                trigger = 0;
                break;
            }
            if (object_type < 0) {
                if (!csb_v1_runtime_find_wall_cell_object_of_type(
                        dungeon,
                        first_thing,
                        cell,
                        sensor_data,
                        &storage_thing)) {
                    trigger = 0;
                    break;
                }
                storage_action = 1;
            } else {
                if (object_type != sensor_data ||
                    csb_v1_runtime_find_wall_cell_object_of_type(
                        dungeon,
                        first_thing,
                        cell,
                        sensor_data,
                        NULL)) {
                    trigger = 0;
                    break;
                }
                storage_action = 2;
                do_not_trigger = (int)(((flags_word >> 3) & 0x03u) ==
                                       DM1_EFFECT_HOLD);
            }
            trigger = 1;
            break;
        case DM1_SENSOR_WALL_OBJECT_EXCHANGER:
            if (!leader_hand_thing ||
                csb_v1_runtime_has_later_wall_cell_sensor(
                    dungeon,
                    (uint16_t)thing,
                    cell)) {
                trigger = 0;
                break;
            }
            if (object_type != sensor_data ||
                !csb_v1_runtime_find_first_square_object(
                    dungeon,
                    first_thing,
                    &square_thing)) {
                trigger = 0;
                break;
            }
            storage_action = 5;
            trigger = 1;
            break;
        default:
            trigger = 0;
            break;
        }

        if ((flags_word >> 5) & 0x01u) {
            trigger ^= 1;
        }
        sensor_effect = (int)((flags_word >> 3) & 0x03u);
        if (sensor_effect == DM1_EFFECT_HOLD) {
            sensor_effect = do_not_trigger ? DM1_EFFECT_CLEAR : DM1_EFFECT_SET;
        } else if (!trigger || do_not_trigger) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }
        if (!trigger) {
            previous_thing = thing;
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        if (storage_action == 1) {
            if (!csb_v1_runtime_unlink_thing_from_square(
                    dungeon,
                    storage_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            *leader_hand_thing = storage_thing;
            object_type = sensor_data;
        } else if (storage_action == 2) {
            uint16_t stored_thing = csb_v1_runtime_thing_with_cell(
                (hand_thing >> 10) & 0x0F,
                hand_thing & 0x03FFu,
                cell);
            if (!csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    stored_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            *leader_hand_thing = 0xFFFFu;
            object_type = -1;
        } else if (storage_action == 3) {
            uint8_t *hand_record = csb_v1_runtime_mutable_thing_record(
                dungeon,
                hand_thing,
                NULL,
                NULL);
            if (!hand_record) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            csb_v1_runtime_write_u16(hand_record, 0xFFFFu);
            *leader_hand_thing = 0xFFFFu;
            object_type = -1;
        } else if (storage_action == 4) {
            generated_thing = csb_v1_runtime_allocate_new_object_launcher_thing(
                dungeon,
                sensor_data);
            if (generated_thing != 0xFFFFu && generated_thing != 0xFFFEu) {
                *leader_hand_thing = generated_thing;
                object_type = csb_v1_runtime_object_type_from_thing(
                    dungeon,
                    generated_thing);
            }
        } else if (storage_action == 5) {
            uint16_t stored_thing;
            if (!csb_v1_runtime_unlink_thing_from_square(
                    dungeon,
                    square_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            stored_thing = csb_v1_runtime_thing_with_cell(
                (hand_thing >> 10) & 0x0F,
                hand_thing & 0x03FFu,
                cell);
            if (!csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    stored_thing,
                    profile->current_level,
                    map_x,
                    map_y)) {
                (void)csb_v1_runtime_append_thing_to_square_tail(
                    dungeon,
                    square_thing,
                    profile->current_level,
                    map_x,
                    map_y);
                previous_thing = thing;
                thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
                continue;
            }
            *leader_hand_thing = square_thing;
            object_type = csb_v1_runtime_object_type_from_thing(
                dungeon,
                square_thing);
        }
        if (storage_action == 1 || storage_action == 2 || rotate_after) {
            (void)csb_v1_runtime_rotate_wall_cell_sensors(
                dungeon,
                profile->current_level,
                map_x,
                map_y,
                cell);
        }
        if (remove_current_sensor) {
            (void)csb_v1_runtime_remove_wall_sensor_after_previous(
                dungeon,
                profile->current_level,
                map_x,
                map_y,
                (uint16_t)previous_thing,
                (uint16_t)thing);
        }

        target_cell = (int)((target_word >> 4) & 0x03u);
        target_x = (int)((target_word >> 6) & 0x1Fu);
        target_y = (int)((target_word >> 11) & 0x1Fu);
        queued += csb_v1_runtime_queue_remote_square_event(
            profile,
            sensor_effect,
            target_x,
            target_y,
            target_cell);
        break;
    }

    return queued;
}

int csb_v1_runtime_trigger_wall_ornament_click(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    int object_type)
{
    return csb_v1_runtime_trigger_wall_ornament_click_core(
        profile,
        map_x,
        map_y,
        cell,
        object_type,
        NULL);
}

int csb_v1_runtime_trigger_wall_ornament_click_ex(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    uint16_t *leader_hand_thing)
{
    return csb_v1_runtime_trigger_wall_ornament_click_core(
        profile,
        map_x,
        map_y,
        cell,
        -1,
        leader_hand_thing);
}

int csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell)
{
    uint16_t leader_hand;
    int queued;

    if (!profile || !profile->party_state_valid) return 0;
    leader_hand = csb_v1_runtime_normalize_leader_hand_thing(
        profile->party_state.LeaderHandThing);
    queued = csb_v1_runtime_trigger_wall_ornament_click_core(
        profile,
        map_x,
        map_y,
        cell,
        -1,
        &leader_hand);
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(leader_hand);
    if (profile->csbwin_gameblock2_summary_valid) {
        profile->csbwin_object_in_hand = profile->party_state.LeaderHandThing;
    }
    return queued;
}

static int csb_v1_runtime_scan_thing_chain_for_object_type(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int object_type)
{
    int guard;

    if (!dungeon || object_type < 0) return 0;
    for (guard = 0;
         guard < DM1_SENSOR_POSSESSION_MAX_SCAN_STEPS &&
             thing != 0xFFFEu && thing != 0xFFFFu;
         ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type > 4 && thing_type < 14 &&
            csb_v1_runtime_object_type_from_thing(dungeon, thing) ==
                object_type) {
            return 1;
        }
        thing = csb_v1_runtime_read_u16(record);
    }
    return 0;
}

static int csb_v1_runtime_object_info_index_from_record(
    int thing_type,
    const uint8_t *record,
    int record_size)
{
    uint16_t word;
    int subtype;

    if (!record || record_size < 4) return -1;
    word = csb_v1_runtime_read_u16(record + 2);
    switch (thing_type) {
    case THING_TYPE_SCROLL:
        return 0;
    case THING_TYPE_CONTAINER:
        if (record_size < 8) return -1;
        subtype = (int)((word >> 1) & 0x03u);
        if (subtype > 0) subtype = 0;
        return 1 + subtype;
    case THING_TYPE_POTION:
        subtype = (int)((word >> 8) & 0x7Fu);
        if (subtype > 20) return -1;
        return 2 + subtype;
    case THING_TYPE_WEAPON:
        subtype = (int)(word & 0x7Fu);
        if (subtype > 45) return -1;
        return 23 + subtype;
    case THING_TYPE_ARMOUR:
        subtype = (int)(word & 0x7Fu);
        if (subtype > 57) return -1;
        return 69 + subtype;
    case THING_TYPE_JUNK:
        subtype = (int)(word & 0x7Fu);
        if (subtype > 52) return -1;
        return 127 + subtype;
    default:
        return -1;
    }
}

static int csb_v1_runtime_object_icon_from_object_info(
    int object_info_index)
{
    static const unsigned char kObjectInfoIcon[180] = {
         30,144,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,
        166,167,195, 16, 18,  4, 14, 20, 23, 25, 27, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
         61, 62, 63, 64, 65, 66,135,143, 28, 80, 81, 82,112,114, 67, 83, 68, 84, 69, 70,
         85, 86, 71, 87,119, 72, 88,113, 89, 73, 74, 90,103,104, 96, 97, 98,105,106,108,
        107, 75, 91, 76, 92, 99,115,100, 77, 93,116,109,101, 78, 94,117,110,102, 79, 95,
        118,111,140,141,142,194,196,  0,  8, 10, 12,146,147,125,126,127,176,177,178,179,
        180,181,182,183,184,185,186,187,188,189,190,191,128,129,130,131,168,169,170,171,
        172,173,174,175,120,121,122,123,124,132,133,134,136,137,138,139,192,193,197,198
    };
    if (object_info_index < 0 || object_info_index >= 180) return -1;
    return (int)kObjectInfoIcon[object_info_index];
}

static int csb_v1_runtime_object_action_set_from_object_info(
    int object_info_index)
{
    static const unsigned char kPotionActionSet[20] = {
         0, 0, 0, 42, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 42
    };
    static const unsigned char kWeaponActionSet[46] = {
        43,  7,  5,  6,  8,  9, 10, 11, 12, 13,
        13, 14, 15, 15, 16, 17, 18, 19, 20, 21,
        22, 22, 23, 24, 24, 27, 27, 26, 26, 27,
        42, 40, 42,  5,  5, 28, 29, 30, 31, 32,
        33,  5, 35, 36, 27,  1
    };
    static const unsigned char kArmourActionSet[58] = {
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0, 41, 41,
        41, 41,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 41,  0,  0,  0,  0, 41,  0,  0,
         0,  0, 41,  0,  0,  0,  0,  0
    };
    static const unsigned char kJunkActionSet[53] = {
         0,  0,  0,  0,  0,  0, 37, 37, 37,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 38, 38,  0, 39,  0,  0,  0,  0,
         0,  0,  0
    };

    /* ReDMCSB: DUNGLOB.C G0237 supplies ObjectInfo.ActionSetIndex;
     * MENU.C F0386/F0389 consumes it for action icons and action menus. */
    if (object_info_index >= 2 && object_info_index < 22) {
        return (int)kPotionActionSet[object_info_index - 2];
    }
    if (object_info_index >= 23 && object_info_index < 69) {
        return (int)kWeaponActionSet[object_info_index - 23];
    }
    if (object_info_index >= 69 && object_info_index < 127) {
        return (int)kArmourActionSet[object_info_index - 69];
    }
    if (object_info_index >= 127 && object_info_index < 180) {
        return (int)kJunkActionSet[object_info_index - 127];
    }
    return 0;
}

static const char *csb_v1_runtime_object_name_from_record(
    int thing_type,
    const uint8_t *record,
    int record_size)
{
    static const char *const kWeaponTypeNames[] = {
        "EYE OF TIME", "STORMRING", "TORCH", "FLAMITT",
        "STAFF OF CLAWS", "BOLT BLADE", "FURY", "THE FIRESTAFF",
        "DAGGER", "FALCHION", "SWORD", "RAPIER",
        "SABRE", "SAMURAI SWORD", "DELTA", "DIAMOND EDGE",
        "VORPAL BLADE", "THE INQUISITOR", "AXE", "HARDCLEAVE",
        "MACE", "MACE OF ORDER", "MORNING STAR", "CLUB",
        "STONE CLUB", "BOW", "CROSSBOW", "ARROW",
        "SLAYER", "SLING", "ROCK", "POISON DART",
        "THROWING STAR", "STICK", "STAFF", "WAND",
        "TEOWAND", "YEW STAFF", "STAFF OF MANAR", "SNAKE STAFF",
        "THE CONDUIT", "DRAGON SPIT", "SCEPTRE OF LYF", "HORN OF FEAR",
        "SPEED BOW", "THE FIRESTAFF"
    };
    static const char *const kPotionTypeNames[] = {
        "MON POTION", "UM POTION", "DES POTION", "VEN POTION",
        "SAR POTION", "ZO POTION", "ROS POTION", "KU POTION",
        "DANE POTION", "NETA POTION", "BRO POTION", "MA POTION",
        "YA POTION", "EE POTION", "VI POTION", "WATER FLASK",
        "EMPTY FLASK"
    };
    static const char *const kArmourTypeNames[] = {
        "CAPE", "CLOAK OF NIGHT", "BARBARIAN HIDE", "SANDALS",
        "LEATHER BOOTS", "ELVEN BOOTS", "LEATHER JERKIN", "LEATHER PANTS",
        "SUEDE BOOTS", "BLUE PANTS", "GHI", "GHI TROUSERS",
        "CALISTA", "CROWN OF NERRA", "BEZERKER HELM", "HELMET",
        "BASINET", "NETA SHIRT", "CHAINMAIL", "PLATE MAIL",
        "MITHRAL MAIL", "MITHRAL HOSEN", "LEG MAIL", "FOOT PLATE",
        "SMALL SHIELD", "WOODEN SHIELD", "LARGE SHIELD", "SHIELD OF LYTE",
        "SHIELD OF DARC", "DEXHELM"
    };
    static const char *const kJunkTypeNames[] = {
        "COMPASS", "TORCH", "WATERSKIN", "JEWEL SYMAL",
        "ILLUMULET", "ASHES", "BONES", "SAR COIN",
        "GOLD COIN", "IRON KEY", "KEY OF B", "SOLID KEY",
        "SQUARE KEY", "TOURQUOISE KEY", "CROSS KEY", "ONYX KEY",
        "SKELETON KEY", "GOLD KEY", "WINGED KEY", "TOPAZ KEY",
        "SAPPHIRE KEY", "EMERALD KEY", "RUBY KEY", "RA KEY",
        "MASTER KEY", "BOULDER", "BLUE GEM", "ORANGE GEM",
        "GREEN GEM", "APPLE", "CORN", "BREAD",
        "CHEESE", "SCREAMER SLICE", "WORM ROUND", "DRUMSTICK",
        "DRAGON STEAK", "GEM OF AGES", "EKKHARD CROSS", "MOONSTONE",
        "THE HELLION", "PENDANT FERAL", "MAGICAL BOX", "MIRROR OF DAWN",
        "ROPE", "RABBIT FOOT", "CORBAMITE", "CHOKER",
        "LOCK PICKS", "MAGNIFIER", "ZOKATHRA SPELL", "EMPTY FLASK"
    };
    uint16_t word;
    int subtype;

    if (!record || record_size < 4) return NULL;
    word = csb_v1_runtime_read_u16(record + 2);
    switch (thing_type) {
    case THING_TYPE_SCROLL:
        return "SCROLL";
    case THING_TYPE_CONTAINER:
        return "CHEST";
    case THING_TYPE_POTION:
        subtype = (int)((word >> 8) & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kPotionTypeNames) / sizeof(kPotionTypeNames[0]))) {
            return kPotionTypeNames[subtype];
        }
        return "POTION";
    case THING_TYPE_WEAPON:
        subtype = (int)(word & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kWeaponTypeNames) / sizeof(kWeaponTypeNames[0]))) {
            return kWeaponTypeNames[subtype];
        }
        return "WEAPON";
    case THING_TYPE_ARMOUR:
        subtype = (int)(word & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kArmourTypeNames) / sizeof(kArmourTypeNames[0]))) {
            return kArmourTypeNames[subtype];
        }
        return "ARMOUR";
    case THING_TYPE_JUNK:
        subtype = (int)(word & 0x7Fu);
        if (subtype >= 0 &&
            subtype < (int)(sizeof(kJunkTypeNames) / sizeof(kJunkTypeNames[0]))) {
            return kJunkTypeNames[subtype];
        }
        return "JUNK";
    default:
        return NULL;
    }
}

int csb_v1_runtime_object_icon_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    uint16_t word;
    int thing_type;
    int record_size;
    int object_info_index;
    int icon_index;

    if (!profile || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return -1;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record) return -1;

    object_info_index = csb_v1_runtime_object_info_index_from_record(
        thing_type,
        record,
        record_size);
    icon_index = csb_v1_runtime_object_icon_from_object_info(
        object_info_index);
    if (icon_index < 0) return -1;

    /* ReDMCSB DUNGEON.C F0141 maps thing type/subtype to G0237 object info;
     * OBJECT.C F0033 then applies per-object dynamic icon adjustments for
     * compass direction, lit torches, closed scrolls, charge-bearing junk,
     * and charge-bearing magical weapons. */
    word = csb_v1_runtime_read_u16(record + 2);
    if (thing_type == THING_TYPE_WEAPON) {
        int charge = (int)((word >> 10) & 0x0Fu);
        int lit = (word & 0x8000u) ? 1 : 0;
        if (icon_index == 4 && lit) {
            static const unsigned char kChargeCountToTorchIconOffset[16] = {
                0,1,1,1,2,2,2,2,3,3,3,3,3,3,3,3
            };
            icon_index += (int)kChargeCountToTorchIconOffset[charge & 0x0F];
        } else if (charge &&
                   (icon_index == 14 || icon_index == 16 ||
                    icon_index == 18 || icon_index == 20 ||
                    icon_index == 23 || icon_index == 25)) {
            icon_index += 1;
        }
    } else if (thing_type == THING_TYPE_SCROLL) {
        int closed = (int)((word >> 10) & 0x3Fu);
        if (icon_index == 30 && closed) icon_index += 1;
    } else if (thing_type == THING_TYPE_JUNK) {
        int charge = (int)((word >> 14) & 0x03u);
        if (icon_index == 0) {
            icon_index += profile->party_dir & 0x03;
        } else if (charge &&
                   (icon_index == 8 || icon_index == 10 ||
                    icon_index == 12)) {
            icon_index += 1;
        }
    }
    return icon_index;
}

int csb_v1_runtime_object_action_set_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    int thing_type;
    int record_size;
    int object_info_index;

    if (!profile) return 0;
    dungeon = (const CSB_V1_DungeonData *)profile->dungeon_handle;
    if (!dungeon ||
        thing == THING_NONE ||
        thing == THING_ENDOFLIST) {
        return 0;
    }
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record) return 0;
    object_info_index = csb_v1_runtime_object_info_index_from_record(
        thing_type,
        record,
        record_size);
    return csb_v1_runtime_object_action_set_from_object_info(
        object_info_index);
}

int csb_v1_runtime_load_object_names_m564(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t *bytes,
    size_t byte_count)
{
    size_t offset = 0u;
    int name_index;

    if (!profile || !bytes || byte_count == 0u) return 0;
    memset(profile->object_names, 0, sizeof(profile->object_names));
    profile->object_name_table_valid = 0;

    /* ReDMCSB OBJECT.C F0031 lines ~58-109 loads
     * M564_GRAPHIC_OBJECT_NAMES for PC media as C199 icon-indexed strings.
     * Each string ends when the source byte has bit 7 set; the stored
     * character is byte & 0x7f, followed by a C null terminator. */
    for (name_index = 0;
         name_index < CSB_V1_OBJECT_NAME_COUNT;
         ++name_index) {
        size_t written = 0u;
        int terminated = 0;

        while (offset < byte_count) {
            unsigned char c = bytes[offset++];
            if (written < (size_t)CSB_V1_OBJECT_NAME_MAX_CHARS) {
                profile->object_names[name_index][written++] =
                    (char)(c & 0x7fu);
            }
            if ((c & 0x80u) != 0u) {
                terminated = 1;
                break;
            }
        }
        profile->object_names[name_index][written] = '\0';
        if (!terminated) {
            memset(profile->object_names, 0, sizeof(profile->object_names));
            return 0;
        }
    }

    profile->object_name_table_valid = 1;
    return 1;
}

int csb_v1_runtime_object_name(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    char *out,
    size_t out_size)
{
    const CSB_V1_DungeonData *dungeon;
    const uint8_t *record;
    const char *name;
    int thing_type;
    int record_size;
    int icon_index;

    if (!out || out_size == 0U) return 0;
    out[0] = '\0';
    if (!profile || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return 0;
    }
    dungeon = profile->dungeon_handle
        ? profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    record = csb_v1_dungeon_get_thing_record(
        dungeon,
        thing,
        &thing_type,
        NULL,
        &record_size);
    if (!record) return 0;

    /* ReDMCSB OBJECT.C F0031 loads C199 icon-indexed names and F0034 draws
     * the leader-hand object name after F0033 icon resolution.  Prefer the
     * CSB-owned decoded M564 table; the subtype fallback is retained only for
     * startup/probe paths before CSBGRAPH has been bound. */
    icon_index = csb_v1_runtime_object_icon_index(profile, thing);
    if (profile->object_name_table_valid &&
        icon_index >= 0 &&
        icon_index < CSB_V1_OBJECT_NAME_COUNT &&
        profile->object_names[icon_index][0] != '\0') {
        snprintf(out, out_size, "%s", profile->object_names[icon_index]);
        return out[0] != '\0';
    }

    name = csb_v1_runtime_object_name_from_record(
        thing_type,
        record,
        record_size);
    if (!name || name[0] == '\0') return 0;
    snprintf(out, out_size, "%s", name);
    return out[0] != '\0';
}

static int csb_v1_runtime_party_has_possession_object_type(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    int object_type)
{
    int champion_count;
    int champion_index;

    if (!profile || !dungeon || object_type < 0 ||
        !profile->party_state_valid) {
        return 0;
    }
    champion_count = profile->party_state.ChampionCount;
    if (champion_count < 0) champion_count = 0;
    if (champion_count > CSB_V1_MAX_CHAMPIONS) {
        champion_count = CSB_V1_MAX_CHAMPIONS;
    }
    for (champion_index = 0;
         champion_index < champion_count;
         ++champion_index) {
        const CSB_V1_Champion *champion =
            &profile->party_state.Champions[champion_index];
        int slot_index;

        if (champion->CurrentHealth <= 0) continue;
        for (slot_index = 0;
             slot_index < CSB_V1_SLOT_COUNT &&
                 slot_index < DM1_SENSOR_POSSESSION_SLOT_LAST;
             ++slot_index) {
            uint16_t slot_thing = champion->Slots[slot_index];
            if (slot_thing == 0xFFFEu || slot_thing == 0xFFFFu) {
                continue;
            }
            if (csb_v1_runtime_scan_thing_chain_for_object_type(
                    dungeon,
                    slot_thing,
                    object_type)) {
                return 1;
            }
        }
    }
    /* ReDMCSB MOVESENS.C F0274 lines 1271-1306 also checks the
     * leader-hand object once after champion slots.  CSB runtime's
     * current owned profile stores imported equipment slots but M11 owns
     * the transient cursor/leader-hand object, so that cursor fallback
     * remains part of the later full M11/CSB inventory handoff. */
    return 0;
}

static void csb_v1_runtime_process_party_floor_sensors_at_level(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y,
    int add_party,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int first_thing;
    int thing;
    int guard;

    if (!profile || !result) return;
    dungeon = (profile->dungeon_handle)
        ? (const CSB_V1_DungeonData *)profile->dungeon_handle
        : csb_v1_dungeon_get_current();
    if (!dungeon || !dungeon->raw_data) return;
    if (level < 0 || level >= dungeon->level_count) return;
    if (map_x < 0 || map_y < 0) return;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        level,
        map_x,
        map_y);
    if (first_thing < 0 || first_thing == 0xFFFE) return;

    /* ReDMCSB: MOVESENS.C F0267 lines 800-822 calls
     * F0276_SENSOR_ProcessThingAdditionOrRemoval when the party leaves and
     * enters a square.  F0276 lines 1658-1785 walks C03 sensor things until
     * the first non-sensor, checks floor sensor types C003/C005/C009 for the
     * party, resolves HOLD into SET/CLEAR, then calls F0272/F0268 to enqueue
     * the square-effect event.  This CSB runtime slice covers party floor
     * sensors, including C008 party-possession checks over imported champion
     * slots; object/group movement sensors remain separate runtime work. */
    thing = first_thing;
    for (guard = 0; guard < 128 && thing != 0xFFFE; ++guard) {
        const uint8_t *record;
        int thing_type;
        int thing_size;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int sensor_effect;
        int trigger;
        int target_x;
        int target_y;
        int target_cell;

        record = csb_v1_dungeon_get_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            NULL,
            &thing_size);
        if (!record) break;
        if (thing_type >= 4) break;
        if (thing_type != 3 || thing_size < 8) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        type_data = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
        flags_word = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
        target_word = (uint16_t)record[6] | ((uint16_t)record[7] << 8);
        sensor_type = (int)(type_data & 0x007Fu);
        sensor_data = (int)(type_data >> 7);
        if (sensor_type == 0) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        trigger = add_party ? 1 : 0;
        switch (sensor_type) {
        case 3: /* C003_SENSOR_FLOOR_PARTY */
            if (profile->champion_count <= 0) {
                trigger = 0;
            } else if (sensor_data != 0) {
                trigger = add_party &&
                    (sensor_data == ((profile->party_dir & 3) + 1));
            }
            break;
        case 5: /* C005_SENSOR_FLOOR_PARTY_ON_STAIRS */
            {
                int raw_square = csb_v1_dungeon_get_raw_square(
                    dungeon,
                    level,
                    map_x,
                    map_y);
                int square_type = (raw_square < 0) ? -1 :
                    ((dungeon->square_bytes == 1)
                        ? ((raw_square >> 5) & 0x07)
                        : (raw_square & 0x1F));
                trigger = (square_type == 3) ? trigger : 0;
            }
            break;
        case 8: /* C008_SENSOR_FLOOR_PARTY_POSSESSION */
            trigger = add_party &&
                csb_v1_runtime_party_has_possession_object_type(
                    profile,
                    dungeon,
                    sensor_data);
            break;
        case 9: /* C009_SENSOR_FLOOR_VERSION_CHECKER, PC34 engine <= 34 */
            trigger = add_party && (sensor_data <= 34);
            break;
        default:
            trigger = 0;
            break;
        }

        sensor_effect = (int)((flags_word >> 3) & 0x03u);
        if ((flags_word >> 5) & 0x01u) {
            trigger ^= 1;
        }
        if (sensor_effect == DM1_EFFECT_HOLD) {
            sensor_effect = trigger ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
        } else if (!trigger) {
            thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
            continue;
        }

        target_cell = (int)((target_word >> 4) & 0x03u);
        target_x = (int)((target_word >> 6) & 0x1Fu);
        target_y = (int)((target_word >> 11) & 0x1Fu);
        if ((flags_word >> 6) & 0x01u) {
            result->sensor_audible_count++;
        }
        result->sensor_trigger_count++;
        result->sensor_last_type = sensor_type;
        result->sensor_last_data = sensor_data;
        result->sensor_last_effect = sensor_effect;
        result->sensor_last_target_x = target_x;
        result->sensor_last_target_y = target_y;
        result->sensor_last_target_cell = target_cell;
        csb_v1_runtime_trigger_floor_sensor_event(
            profile,
            level,
            sensor_effect,
            target_x,
            target_y,
            target_cell,
            result);

        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_process_party_floor_sensors_at(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int add_party,
    CSB_V1_InputCommandRuntimeResult *result)
{
    if (!profile) return;
    csb_v1_runtime_process_party_floor_sensors_at_level(
        profile,
        profile->current_level,
        map_x,
        map_y,
        add_party,
        result);
}

static void csb_v1_runtime_apply_party_floor_sensor_consequences(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    if (!profile || !result) return;
    if (!result->movement_step_applied && !result->stair_transition_applied) {
        return;
    }

    result->sensor_source_remove_checked = 1;
    csb_v1_runtime_process_party_floor_sensors_at_level(
        profile,
        result->old_party_level,
        result->old_party_x,
        result->old_party_y,
        0,
        result);
    if (result->stair_transition_applied && result->movement_step_applied) {
        csb_v1_runtime_process_party_floor_sensors_at_level(
            profile,
            result->old_party_level,
            result->movement_destination_x,
            result->movement_destination_y,
            0,
            result);
    }
    if (result->new_party_level == result->old_party_level) {
        result->sensor_destination_add_checked = 1;
        csb_v1_runtime_process_party_floor_sensors_at(
            profile,
            profile->party_x,
            profile->party_y,
            1,
            result);
    }
}

static void csb_v1_runtime_apply_party_turn_floor_sensor_add_consequences(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    int level;
    int map_x;
    int map_y;

    if (!profile || !result) return;
    level = profile->current_level;
    map_x = profile->party_x;
    map_y = profile->party_y;
    if (level < 0) return;

    /* ReDMCSB: CLIKMENU.C F0365 lines 169-172 processes party floor
     * sensors on the current square before and after F0284 rotates the
     * party.  Directional C003 floor sensors therefore see the old facing
     * on removal and the new facing on addition.  The caller runs the
     * pre-rotation removal pass; this helper runs the post-rotation add. */
    result->sensor_destination_add_checked = 1;
    csb_v1_runtime_process_party_floor_sensors_at_level(
        profile,
        level,
        map_x,
        map_y,
        1,
        result);
}

static void csb_v1_runtime_mark_deferred_new_party_map_index(
    CSB_V1_InputCommandRuntimeResult *result)
{
    if (!result ||
        (!result->movement_step_applied &&
         !result->stair_transition_applied)) {
        return;
    }
    if (result->new_party_level == result->old_party_level) return;

    /* ReDMCSB MOVESENS.C F0267 lines 830-842: when party movement ends on
     * another map, the source engine does not run destination party sensors
     * immediately; it publishes G0327_i_NewPartyMapIndex for the outer game
     * loop/map handoff. Firestaff's bounded CSB runtime already updates the
     * active map directly, but it also exposes the source handoff signal so
     * M11/startup callers can observe the same boundary explicitly. */
    result->deferred_new_party_map_index_valid = 1;
    result->deferred_new_party_map_index = result->new_party_level;
}

static void csb_v1_runtime_trigger_remote_sensor_event(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int sensor_effect,
    int target_x,
    int target_y,
    int target_cell)
{
    CSB_V1_DungeonData *dungeon;
    struct DM1_Event_V1 event;
    int raw_square;
    int square_type;
    int event_type;

    if (!profile || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data) return;
    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        level,
        target_x,
        target_y);
    if (raw_square < 0) return;
    square_type = (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
    event_type = csb_v1_runtime_square_event_type_for_sensor_target(square_type);
    if (event_type == DM1_EVENT_NONE) return;

    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(level, profile->game_time);
    event.type = (uint8_t)event_type;
    event.b_mapX = (uint8_t)target_x;
    event.b_mapY = (uint8_t)target_y;
    event.c_cell = (uint8_t)target_cell;
    event.c_effect = (uint8_t)sensor_effect;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static uint8_t *csb_v1_runtime_square_byte_ptr(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int map_x,
    int map_y,
    int *out_square_type)
{
    CSB_V1_DungeonData *dungeon;
    int offset;

    if (out_square_type) *out_square_type = -1;
    if (!profile || !profile->dungeon_handle) return NULL;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->square_bytes != 1) return NULL;
    if (level < 0 || level >= dungeon->level_count) return NULL;
    if (map_x < 0 || map_x >= dungeon->level_widths[level] ||
        map_y < 0 || map_y >= dungeon->level_heights[level]) {
        return NULL;
    }
    offset = dungeon->level_offsets[level] +
             map_x * dungeon->level_heights[level] +
             map_y;
    if (offset < 0 || offset >= dungeon->raw_size) return NULL;
    if (out_square_type) *out_square_type = (dungeon->raw_data[offset] >> 5) & 0x07;
    return &dungeon->raw_data[offset];
}

static void csb_v1_runtime_projectile_step(int direction, int *out_dx, int *out_dy)
{
    int dx = 0;
    int dy = 0;
    switch (direction & 3) {
    case 0: dy = -1; break;
    case 1: dx = 1; break;
    case 2: dy = 1; break;
    case 3: dx = -1; break;
    default: break;
    }
    if (out_dx) *out_dx = dx;
    if (out_dy) *out_dy = dy;
}

static int csb_v1_runtime_projectile_teleporter_scope_allows(int scope)
{
    /* ReDMCSB MOVESENS.C F0267 lines 450-482 gives non-party,
     * non-group things a combined creatures|objects-or-party requirement,
     * then rejects only creature-only teleporters for non-groups.  C14
     * projectiles therefore use object/party-capable teleporters, not the
     * party-only scope test used by normal C003..C006 movement. */
    return scope != 0x01;
}

static void csb_v1_runtime_schedule_projectile_move_event(
    CSB_V1_RuntimeProfile *profile,
    const struct TimelineEvent_Compat *event)
{
    struct DM1_Event_V1 dm1_event;

    if (!profile || !event) return;
    if (event->kind != TIMELINE_EVENT_PROJECTILE_MOVE) return;
    if (event->aux0 < 0 || event->aux0 > 255) return;

    memset(&dm1_event, 0, sizeof(dm1_event));
    dm1_event.map_time = DM1_MAP_TIME_MAKE(
        event->mapIndex,
        event->fireAtTick);
    dm1_event.type = DM1_EVENT_MOVE_PROJECTILE;
    dm1_event.priority = (uint8_t)event->aux0;
    dm1_event.b_mapX = (uint8_t)event->mapX;
    dm1_event.b_mapY = (uint8_t)event->mapY;
    dm1_event.c_cell = (uint8_t)(event->cell & 3);
    dm1_event.c_effect = (uint8_t)(event->aux3 & 0xFF);
    (void)dm1v1_event_add(&profile->timeline_queue, &dm1_event);
}

static void csb_v1_runtime_schedule_explosion_advance_event(
    CSB_V1_RuntimeProfile *profile,
    const struct TimelineEvent_Compat *event)
{
    struct DM1_Event_V1 dm1_event;

    if (!profile || !event) return;
    if (event->kind != TIMELINE_EVENT_EXPLOSION_ADVANCE) return;
    if (event->aux0 < 0 || event->aux0 > 255) return;

    memset(&dm1_event, 0, sizeof(dm1_event));
    dm1_event.map_time = DM1_MAP_TIME_MAKE(
        event->mapIndex,
        event->fireAtTick);
    dm1_event.type = DM1_EVENT_EXPLOSION;
    dm1_event.priority = (uint8_t)event->aux0;
    dm1_event.b_mapX = (uint8_t)event->mapX;
    dm1_event.b_mapY = (uint8_t)event->mapY;
    dm1_event.c_cell = (uint8_t)(event->cell & 0xFF);
    dm1_event.c_effect = (uint8_t)(event->aux1 & 0xFF);
    (void)dm1v1_event_add(&profile->timeline_queue, &dm1_event);
}

static int csb_v1_runtime_projectile_instance_active(
    const struct ProjectileInstance_Compat *projectile)
{
    return projectile &&
           projectile->slotIndex >= 0 &&
           projectile->reserved3 != 0;
}

static int csb_v1_runtime_explosion_instance_active(
    const struct ExplosionInstance_Compat *explosion)
{
    return explosion &&
           explosion->slotIndex >= 0 &&
           explosion->reserved0 != 0;
}

static int csb_v1_runtime_square_type_from_raw(
    const CSB_V1_DungeonData *dungeon,
    int raw_square)
{
    if (!dungeon || raw_square < 0) return PROJECTILE_ELEMENT_WALL;
    return (dungeon->square_bytes == 1)
        ? ((raw_square >> 5) & 0x07)
        : (raw_square & 0x1F);
}

static int csb_v1_runtime_projectile_resolve_teleporter_chain(
    const CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    int start_map_index,
    int start_map_x,
    int start_map_y,
    int base_cell,
    int *out_map_index,
    int *out_map_x,
    int *out_map_y,
    int *out_direction,
    int *out_cell)
{
    const CSB_V1_DungeonData *dungeon;
    int map_index;
    int map_x;
    int map_y;
    int direction;
    int cell;
    int i;
    int applied = 0;

    if (out_map_index) *out_map_index = start_map_index;
    if (out_map_x) *out_map_x = start_map_x;
    if (out_map_y) *out_map_y = start_map_y;
    if (out_direction) {
        *out_direction = projectile ? (projectile->direction & 3) : -1;
    }
    if (out_cell) *out_cell = base_cell;
    if (!profile || !projectile || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data || dungeon->level_count <= 0) return 0;

    map_index = start_map_index;
    map_x = start_map_x;
    map_y = start_map_y;
    direction = projectile->direction & 3;
    cell = (base_cell >= 0) ? (base_cell & 3) : 0;

    /* ReDMCSB MOVESENS.C F0267 lines 466-530 chains teleporters up to the
     * PC34/I34E MEDIA529 100-step cap.  F0264 lines 148-170 marks C14
     * projectiles as levitating, so pits are intentionally not part of this
     * projectile chain helper. */
    for (i = 0; i < 100; ++i) {
        CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
        CSB_V1_TeleporterRotationRuntimeProjectileResultPc34 teleporter_result;
        uint16_t projectile_thing;
        int raw_square;
        int scope = 0;
        int self_target;

        if (map_index < 0 || map_index >= dungeon->level_count) break;
        raw_square = csb_v1_dungeon_get_raw_square(
            dungeon,
            map_index,
            map_x,
            map_y);
        if (raw_square < 0 ||
            csb_v1_runtime_square_type_from_raw(dungeon, raw_square) !=
                PROJECTILE_ELEMENT_TELEPORTER ||
            (raw_square & 0x08) == 0) {
            break;
        }
        if (csb_v1_runtime_decode_teleporter_at_square(
                dungeon,
                map_index,
                map_x,
                map_y,
                raw_square,
                &teleporter,
                &scope) <= 0) {
            break;
        }
        if (!csb_v1_runtime_projectile_teleporter_scope_allows(scope) ||
            teleporter.target_map_index < 0 ||
            teleporter.target_map_index >= dungeon->level_count) {
            break;
        }

        projectile_thing = (uint16_t)(((cell & 3) << 14) |
                                      (14u << 10) |
                                      (uint16_t)(projectile->slotIndex &
                                                 0x03FF));
        if (csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
                &teleporter,
                projectile_thing,
                direction,
                &teleporter_result) != 0) {
            break;
        }

        self_target = teleporter.target_map_index == map_index &&
                      teleporter.target_map_x == map_x &&
                      teleporter.target_map_y == map_y;
        map_index = teleporter.target_map_index;
        map_x = teleporter.target_map_x;
        map_y = teleporter.target_map_y;
        direction = teleporter_result.direction & 3;
        cell = csb_v1_teleporter_rotation_thing_cell_pc34_compat(
            teleporter_result.thing) & 3;
        applied++;
        if (self_target) break;
    }

    if (!applied) return 0;
    if (out_map_index) *out_map_index = map_index;
    if (out_map_x) *out_map_x = map_x;
    if (out_map_y) *out_map_y = map_y;
    if (out_direction) *out_direction = direction;
    if (out_cell) *out_cell = (base_cell >= 0) ? cell : base_cell;
    return applied;
}

static int csb_v1_runtime_party_champion_cell_mask(
    const CSB_V1_RuntimeProfile *profile)
{
    int i;
    int mask = 0;

    if (!profile || !profile->party_state_valid) return 0;

    /* ReDMCSB: MOVESENS.C F0266 lines 241-247 fills the projectile
     * impact cell table from F0285_CHAMPION_GetIndexInCell(), so empty
     * party cells must not be exposed as champion impact targets. */
    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; ++i) {
        const CSB_V1_Champion *champion = &profile->party_state.Champions[i];
        if (csb_v1_champion_is_dead(champion) ||
            champion->CurrentHealth <= 0) {
            continue;
        }
        mask |= 1 << ((int)champion->Cell & 3);
    }
    return mask;
}

static int csb_v1_runtime_build_projectile_digest(
    const CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    int projectile_index,
    struct CellContentDigest_Compat *out)
{
    const CSB_V1_DungeonData *dungeon;
    int raw_square;
    int dest_raw_square;
    int dx;
    int dy;
    int dest_x;
    int dest_y;
    int i;

    if (!profile || !projectile || !out || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data ||
        projectile->mapIndex < 0 ||
        projectile->mapIndex >= dungeon->level_count) {
        return 0;
    }

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        projectile->mapIndex,
        projectile->mapX,
        projectile->mapY);
    if (raw_square < 0) return 0;

    csb_v1_runtime_projectile_step(projectile->direction, &dx, &dy);
    dest_x = projectile->mapX + dx;
    dest_y = projectile->mapY + dy;

    memset(out, 0, sizeof(*out));
    out->sourceMapIndex = projectile->mapIndex;
    out->sourceMapX = projectile->mapX;
    out->sourceMapY = projectile->mapY;
    out->sourceSquareType =
        csb_v1_runtime_square_type_from_raw(dungeon, raw_square);
    out->destTeleporterNewDirection = -1;
    out->destDoorState = PROJECTILE_DOOR_STATE_NONE;

    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat *other =
            &profile->projectiles.entries[i];
        if (i == projectile_index ||
            !csb_v1_runtime_projectile_instance_active(other)) {
            continue;
        }
        if (other->mapIndex == projectile->mapIndex &&
            other->mapX == projectile->mapX &&
            other->mapY == projectile->mapY &&
            other->cell == projectile->cell) {
            out->sourceHasOtherProjectile = 1;
            break;
        }
    }

    out->destMapIndex = projectile->mapIndex;
    out->destMapX = dest_x;
    out->destMapY = dest_y;
    dest_raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        projectile->mapIndex,
        dest_x,
        dest_y);
    if (dest_raw_square < 0) {
        out->destIsMapBoundary = 1;
        out->destSquareType = PROJECTILE_ELEMENT_WALL;
        return 1;
    }

    out->destSquareType =
        csb_v1_runtime_square_type_from_raw(dungeon, dest_raw_square);
    if (out->destSquareType == PROJECTILE_ELEMENT_TELEPORTER &&
        (dest_raw_square & 0x08) != 0) {
        int chain_map_index = projectile->mapIndex;
        int chain_x = dest_x;
        int chain_y = dest_y;
        int chain_direction = projectile->direction;
        if (csb_v1_runtime_projectile_resolve_teleporter_chain(
                profile,
                projectile,
                projectile->mapIndex,
                dest_x,
                dest_y,
                -1,
                &chain_map_index,
                &chain_x,
                &chain_y,
                &chain_direction,
                NULL) > 0) {
            out->destTeleporterNewDirection = chain_direction;
            out->destMapIndex = chain_map_index;
            out->destMapX = chain_x;
            out->destMapY = chain_y;
            dest_x = out->destMapX;
            dest_y = out->destMapY;
            dest_raw_square = csb_v1_dungeon_get_raw_square(
                dungeon,
                out->destMapIndex,
                dest_x,
                dest_y);
            if (dest_raw_square >= 0) {
                out->destSquareType =
                    csb_v1_runtime_square_type_from_raw(
                        dungeon,
                        dest_raw_square);
            }
        }
    }
    if (out->destSquareType == PROJECTILE_ELEMENT_FAKEWALL) {
        out->destFakeWallIsImaginaryOrOpen =
            (dest_raw_square & 0x05) ? 1 : 0;
    }
    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int door_state = dest_raw_square & 0x07;
        if (door_state == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (door_state <= 4) {
            out->destDoorState = door_state;
        } else if (door_state == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
        }
        out->destDoorAllowsProjectilePassThrough = 0;
    }
    if (profile->current_level == projectile->mapIndex &&
        profile->party_x == dest_x &&
        profile->party_y == dest_y) {
        out->destChampionCellMask =
            csb_v1_runtime_party_champion_cell_mask(profile);
        out->destHasChampion = out->destChampionCellMask ? 1 : 0;
        out->destPartyDirection = profile->party_dir & 3;
    }
    {
        int first_thing = csb_v1_dungeon_get_first_thing(
            dungeon,
            projectile->mapIndex,
            dest_x,
            dest_y);
        if (first_thing >= 0 &&
            ((first_thing >> 10) & 0x0F) == 4) {
            int thing_type = -1;
            int thing_index = -1;
            int thing_size = 0;
            const uint8_t *group = csb_v1_dungeon_get_thing_record(
                dungeon,
                first_thing,
                &thing_type,
                &thing_index,
                &thing_size);
            out->destHasCreatureGroup = 1;
            out->destCreatureType =
                (group && thing_type == 4 && thing_size > 4) ? group[4] : 0;
            out->destCreatureCellMask = 0x0F;
            {
                const struct CreatureBehaviorProfile_Compat *creature_profile =
                    CREATURE_GetProfile_Compat(out->destCreatureType);
                out->destCreatureIsNonMaterial =
                    creature_profile &&
                    ((creature_profile->attributes &
                      CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            }
            (void)thing_index;
        }
    }
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat *other =
            &profile->projectiles.entries[i];
        int new_cell;
        if (i == projectile_index ||
            !csb_v1_runtime_projectile_instance_active(other)) {
            continue;
        }
        if (other->mapIndex != projectile->mapIndex ||
            other->mapX != dest_x ||
            other->mapY != dest_y) {
            continue;
        }
        if ((projectile->direction & 1) == (projectile->cell & 1)) {
            new_cell = (projectile->cell - 1) & 3;
        } else {
            new_cell = (projectile->cell + 1) & 3;
        }
        if (other->cell == new_cell) {
            out->destHasOtherProjectile = 1;
            break;
        }
    }
    return 1;
}

static int csb_v1_runtime_projectile_teleporter_rotated_cell(
    const CSB_V1_RuntimeProfile *profile,
    const struct ProjectileInstance_Compat *projectile,
    int base_cell,
    int *out_direction,
    int *out_cell)
{
    int dx;
    int dy;
    int dest_x;
    int dest_y;

    if (out_direction) *out_direction = -1;
    if (out_cell) *out_cell = -1;
    if (!profile || !projectile || !out_cell) {
        return 0;
    }
    csb_v1_runtime_projectile_step(projectile->direction, &dx, &dy);
    dest_x = projectile->mapX + dx;
    dest_y = projectile->mapY + dy;
    return csb_v1_runtime_projectile_resolve_teleporter_chain(
        profile,
        projectile,
        projectile->mapIndex,
        dest_x,
        dest_y,
        base_cell,
        NULL,
        NULL,
        NULL,
        out_direction,
        out_cell) > 0;
}

static int csb_v1_runtime_build_explosion_digest(
    const CSB_V1_RuntimeProfile *profile,
    const struct ExplosionInstance_Compat *explosion,
    struct CellContentDigest_Compat *out)
{
    const CSB_V1_DungeonData *dungeon;
    int raw_square;

    if (!profile || !explosion || !out || !profile->dungeon_handle) return 0;
    dungeon = profile->dungeon_handle;
    if (!dungeon->raw_data ||
        explosion->mapIndex < 0 ||
        explosion->mapIndex >= dungeon->level_count) {
        return 0;
    }

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        explosion->mapIndex,
        explosion->mapX,
        explosion->mapY);
    if (raw_square < 0) return 0;

    memset(out, 0, sizeof(*out));
    out->sourceMapIndex = explosion->mapIndex;
    out->sourceMapX = explosion->mapX;
    out->sourceMapY = explosion->mapY;
    out->sourceSquareType =
        csb_v1_runtime_square_type_from_raw(dungeon, raw_square);
    out->destMapIndex = explosion->mapIndex;
    out->destMapX = explosion->mapX;
    out->destMapY = explosion->mapY;
    out->destSquareType = out->sourceSquareType;
    out->destTeleporterNewDirection = -1;
    out->destDoorState = PROJECTILE_DOOR_STATE_NONE;

    if (out->destSquareType == PROJECTILE_ELEMENT_FAKEWALL) {
        out->destFakeWallIsImaginaryOrOpen =
            (raw_square & 0x05) ? 1 : 0;
    }
    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int door_state = raw_square & 0x07;
        if (door_state == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (door_state <= 4) {
            out->destDoorState = door_state;
        } else if (door_state == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
        }
        out->destDoorAllowsProjectilePassThrough = 0;
    }
    if (profile->current_level == explosion->mapIndex &&
        profile->party_x == explosion->mapX &&
        profile->party_y == explosion->mapY) {
        out->destChampionCellMask =
            csb_v1_runtime_party_champion_cell_mask(profile);
        out->destHasChampion = out->destChampionCellMask ? 1 : 0;
        out->destPartyDirection = profile->party_dir & 3;
    }
    {
        int first_thing = csb_v1_dungeon_get_first_thing(
            dungeon,
            explosion->mapIndex,
            explosion->mapX,
            explosion->mapY);
        if (first_thing >= 0 &&
            ((first_thing >> 10) & 0x0F) == 4) {
            int thing_type = -1;
            int thing_index = -1;
            int thing_size = 0;
            const uint8_t *group = csb_v1_dungeon_get_thing_record(
                dungeon,
                first_thing,
                &thing_type,
                &thing_index,
                &thing_size);
            out->destHasCreatureGroup = 1;
            out->destCreatureType =
                (group && thing_type == 4 && thing_size > 4) ? group[4] : 0;
            out->destCreatureCellMask = 0x0F;
            {
                const struct CreatureBehaviorProfile_Compat *creature_profile =
                    CREATURE_GetProfile_Compat(out->destCreatureType);
                out->destCreatureIsNonMaterial =
                    creature_profile &&
                    ((creature_profile->attributes &
                      CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            }
            (void)thing_index;
        }
    }
    return 1;
}

static void csb_v1_runtime_apply_projectile_move_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    struct ProjectileInstance_Compat *projectile;
    struct ProjectileInstance_Compat new_state;
    struct ProjectileTickResult_Compat tick_result;
    struct CellContentDigest_Compat digest;
    struct RngState_Compat rng;
    int slot;

    if (!profile || !record) return;
    slot = record->aux0;
    if (slot < 0 || slot >= PROJECTILE_LIST_CAPACITY) return;
    projectile = &profile->projectiles.entries[slot];
    if (!csb_v1_runtime_projectile_instance_active(projectile)) return;
    if (!csb_v1_runtime_build_projectile_digest(
            profile,
            projectile,
            slot,
            &digest)) {
        (void)F0813_PROJECTILE_Despawn_Compat(&profile->projectiles, slot);
        return;
    }

    /* ReDMCSB PROJEXPL.C F0219 is dispatched from TIMELINE.C F0261 C48/C49.
     * CSB's bounded runtime delegates the source projectile motion math to
     * M10 F0811, then owns only CSB real-format byte-map digesting and event
     * requeueing. */
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)projectile->mapX << 8) ^
            ((uint32_t)projectile->mapY << 16) ^
            (uint32_t)(slot << 24));
    if (!F0811_PROJECTILE_Advance_Compat(
            projectile,
            &digest,
            profile->game_time,
            &rng,
            &new_state,
            &tick_result)) {
        (void)F0813_PROJECTILE_Despawn_Compat(&profile->projectiles, slot);
        return;
    }
    if (tick_result.emittedExplosion) {
        struct ExplosionCreateInput_Compat explosion_input;
        struct TimelineEvent_Compat first_advance;
        int explosion_slot = -1;
        memset(&explosion_input, 0, sizeof(explosion_input));
        memset(&first_advance, 0, sizeof(first_advance));
        explosion_input.explosionType = tick_result.outExplosion.explosionType;
        explosion_input.attack = tick_result.outExplosion.attack;
        explosion_input.mapIndex = tick_result.outExplosion.mapIndex;
        explosion_input.mapX = tick_result.outExplosion.mapX;
        explosion_input.mapY = tick_result.outExplosion.mapY;
        explosion_input.cell = tick_result.outExplosion.cell;
        explosion_input.centered = tick_result.outExplosion.centered;
        explosion_input.poisonAttack = tick_result.outExplosion.poisonAttack;
        explosion_input.currentTick = (int)profile->game_time;
        explosion_input.ownerKind = tick_result.outExplosion.ownerKind;
        explosion_input.ownerIndex = tick_result.outExplosion.ownerIndex;
        explosion_input.creatorProjectileSlot =
            tick_result.outExplosion.creatorProjectileSlot;
        if (F0821_EXPLOSION_Create_Compat(
                &explosion_input,
                &profile->explosions,
                &explosion_slot,
                &first_advance)) {
            csb_v1_runtime_schedule_explosion_advance_event(
                profile,
                &first_advance);
        }
    }
    if (tick_result.emittedCombatAction &&
        tick_result.outAction.kind == COMBAT_ACTION_APPLY_DAMAGE_GROUP) {
        (void)csb_v1_runtime_apply_projectile_group_action(
            profile,
            &tick_result.outAction,
            projectile);
    }
    if (!tick_result.despawn &&
        tick_result.resultKind == PROJECTILE_RESULT_FLEW &&
        digest.destTeleporterNewDirection >= 0) {
        int rotated_direction = -1;
        int rotated_cell = -1;
        if (csb_v1_runtime_projectile_teleporter_rotated_cell(
                profile,
                projectile,
                new_state.cell,
                &rotated_direction,
                &rotated_cell)) {
            if (rotated_direction >= 0) {
                new_state.direction = rotated_direction & 3;
                tick_result.newDirection = new_state.direction;
            }
            new_state.cell = rotated_cell & 3;
            tick_result.newCell = new_state.cell;
            tick_result.outNextTick.cell = new_state.cell;
        }
    }
    if (tick_result.despawn) {
        (void)csb_v1_runtime_materialize_projectile_associated_object(
            profile,
            projectile,
            &tick_result);
        (void)F0813_PROJECTILE_Despawn_Compat(&profile->projectiles, slot);
        return;
    }
    *projectile = new_state;
    if (tick_result.resultKind == PROJECTILE_RESULT_FLEW) {
        csb_v1_runtime_schedule_projectile_move_event(
            profile,
            &tick_result.outNextTick);
    }
    if (tick_result.emittedDoorDestructionEvent ||
        tick_result.emittedDoorToggleEvent) {
        struct DM1_Event_V1 door_event;
        memset(&door_event, 0, sizeof(door_event));
        door_event.map_time = DM1_MAP_TIME_MAKE(
            tick_result.outNextTick.mapIndex,
            tick_result.outNextTick.fireAtTick);
        door_event.type = tick_result.emittedDoorDestructionEvent
            ? DM1_EVENT_DOOR_DESTRUCTION
            : DM1_EVENT_DOOR;
        door_event.b_mapX = (uint8_t)tick_result.outNextTick.mapX;
        door_event.b_mapY = (uint8_t)tick_result.outNextTick.mapY;
        door_event.c_cell = (uint8_t)(tick_result.outNextTick.cell & 3);
        door_event.c_effect = (uint8_t)tick_result.outNextTick.aux0;
        (void)dm1v1_event_add(&profile->timeline_queue, &door_event);
    }
}

static void csb_v1_runtime_apply_explosion_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    struct ExplosionInstance_Compat *explosion;
    struct ExplosionInstance_Compat new_state;
    struct ExplosionTickResult_Compat tick_result;
    struct CellContentDigest_Compat digest;
    struct RngState_Compat rng;
    int slot;

    if (!profile || !record) return;
    slot = record->aux0;
    if (slot < 0 || slot >= EXPLOSION_LIST_CAPACITY) return;
    explosion = &profile->explosions.entries[slot];
    if (!csb_v1_runtime_explosion_instance_active(explosion)) return;
    if (!csb_v1_runtime_build_explosion_digest(
            profile,
            explosion,
            &digest)) {
        (void)F0824_EXPLOSION_Despawn_Compat(&profile->explosions, slot);
        return;
    }

    /* ReDMCSB PROJEXPL.C F0220 is dispatched from TIMELINE.C F0261 C25.
     * CSB keeps real-format byte-map lookup here and delegates explosion
     * frame/attack/lifecycle parity to the shared M10 F0822 mirror. */
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)explosion->mapX << 8) ^
            ((uint32_t)explosion->mapY << 16) ^
            (uint32_t)(slot << 24));
    if (!F0822_EXPLOSION_Advance_Compat(
            explosion,
            &digest,
            profile->game_time,
            &rng,
            &new_state,
            &tick_result)) {
        (void)F0824_EXPLOSION_Despawn_Compat(&profile->explosions, slot);
        return;
    }

    if (tick_result.emittedCombatActionPartyCount > 0) {
        (void)csb_v1_runtime_apply_explosion_party_action(
            profile,
            &tick_result.outActionParty,
            &rng);
    }
    if (tick_result.emittedCombatActionGroupCount > 0) {
        (void)csb_v1_runtime_apply_explosion_group_action(
            profile,
            &tick_result.outActionGroup,
            &rng);
    }

    if (tick_result.emittedDoorDestructionEvent) {
        struct DM1_Event_V1 door_event;
        memset(&door_event, 0, sizeof(door_event));
        door_event.map_time = DM1_MAP_TIME_MAKE(
            tick_result.outNextTick.mapIndex,
            tick_result.outNextTick.fireAtTick);
        door_event.type = DM1_EVENT_DOOR_DESTRUCTION;
        door_event.priority = 0;
        door_event.b_mapX = (uint8_t)tick_result.outNextTick.mapX;
        door_event.b_mapY = (uint8_t)tick_result.outNextTick.mapY;
        door_event.c_cell = (uint8_t)(tick_result.outNextTick.cell & 0xFF);
        door_event.c_effect = 0;
        (void)dm1v1_event_add(&profile->timeline_queue, &door_event);
    }
    if (tick_result.despawn) {
        (void)F0824_EXPLOSION_Despawn_Compat(&profile->explosions, slot);
        return;
    }

    *explosion = new_state;
    if (tick_result.outNextTick.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE) {
        explosion->scheduledAtTick = (int)tick_result.outNextTick.fireAtTick;
        csb_v1_runtime_schedule_explosion_advance_event(
            profile,
            &tick_result.outNextTick);
    }
}

static void csb_v1_runtime_schedule_door_animation_followup(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int effect)
{
    struct DM1_Event_V1 event;

    if (!profile || !record) return;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        record->mapIndex,
        profile->game_time + 1u);
    event.type = DM1_EVENT_DOOR_ANIMATION;
    event.b_mapX = (uint8_t)record->mapX;
    event.b_mapY = (uint8_t)record->mapY;
    event.c_cell = (uint8_t)record->cell;
    event.c_effect = (uint8_t)effect;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_apply_door_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    uint8_t *square;
    int square_type;
    int door_state;
    int effect;
    int next_state;

    if (!profile || !record) return;
    square = csb_v1_runtime_square_byte_ptr(
        profile,
        record->mapIndex,
        record->mapX,
        record->mapY,
        &square_type);
    if (!square || square_type != 4) return;

    door_state = (int)(*square & 0x07u);
    if (door_state == 5) return;
    effect = record->effect;
    if (effect == DM1_EFFECT_TOGGLE) {
        effect = (door_state == 0) ? DM1_EFFECT_CLEAR : DM1_EFFECT_SET;
    }
    if (effect != DM1_EFFECT_SET && effect != DM1_EFFECT_CLEAR) return;
    if ((effect == DM1_EFFECT_SET && door_state == 0) ||
        (effect == DM1_EFFECT_CLEAR && door_state == 4)) {
        return;
    }

    next_state = door_state + ((effect == DM1_EFFECT_SET) ? -1 : 1);
    if (next_state < 0) next_state = 0;
    if (next_state > 4) next_state = 4;
    *square = (uint8_t)((*square & (uint8_t)~0x07u) | (uint8_t)next_state);
    if ((effect == DM1_EFFECT_SET && next_state != 0) ||
        (effect == DM1_EFFECT_CLEAR && next_state != 4)) {
        csb_v1_runtime_schedule_door_animation_followup(
            profile,
            record,
            effect);
    }
}

static void csb_v1_runtime_apply_square_flag_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int expected_square_type,
    uint8_t open_mask)
{
    uint8_t *square;
    int square_type;
    int effect;
    int is_open;

    if (!profile || !record) return;
    square = csb_v1_runtime_square_byte_ptr(
        profile,
        record->mapIndex,
        record->mapX,
        record->mapY,
        &square_type);
    if (!square || square_type != expected_square_type) return;

    effect = record->effect;
    if (effect == DM1_EFFECT_TOGGLE) {
        effect = (*square & open_mask) ? DM1_EFFECT_CLEAR : DM1_EFFECT_SET;
    }
    if (effect != DM1_EFFECT_SET && effect != DM1_EFFECT_CLEAR) return;
    is_open = (effect == DM1_EFFECT_SET) ? 1 : 0;
    if (is_open) {
        *square = (uint8_t)(*square | open_mask);
    } else {
        *square = (uint8_t)(*square & (uint8_t)~open_mask);
    }
}

static void csb_v1_runtime_schedule_enable_group_generator_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    uint32_t ticks)
{
    struct DM1_Event_V1 event;

    if (!profile || !record || ticks == 0u) return;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(
        record->mapIndex,
        profile->game_time + ticks);
    event.type = DM1_EVENT_ENABLE_GROUP_GENERATOR;
    event.b_mapX = (uint8_t)record->mapX;
    event.b_mapY = (uint8_t)record->mapY;
    (void)dm1v1_event_add(&profile->timeline_queue, &event);
}

static void csb_v1_runtime_materialize_corridor_generator_group(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    int sensor_data,
    uint16_t flags_word,
    uint16_t local_word)
{
    CSB_V1_DungeonData *dungeon;
    uint8_t *group_record;
    uint8_t *first_thing_ptr;
    struct GeneratorContext_Compat ctx;
    struct GeneratorResult_Compat result;
    struct RngState_Compat rng;
    uint16_t previous_first;
    uint16_t group_thing;
    uint16_t group_flags;
    int group_index;
    int i;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    if (!csb_v1_runtime_find_unused_group_record(
            dungeon,
            &group_record,
            &group_index)) {
        return;
    }
    first_thing_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (!first_thing_ptr) return;

    memset(&ctx, 0, sizeof(ctx));
    memset(&result, 0, sizeof(result));
    ctx.mapIndex = record->mapIndex;
    ctx.mapX = record->mapX;
    ctx.mapY = record->mapY;
    ctx.creatureType = sensor_data;
    ctx.creatureCountRaw = (int)((flags_word >> 7) & 0x0Fu);
    ctx.randomizeCount = (ctx.creatureCountRaw & 0x08) ? 1 : 0;
    ctx.healthMultiplier = (int)(local_word & 0x000Fu);
    ctx.ticksRaw = (int)(local_word >> 4);
    ctx.onceOnly = (int)((flags_word >> 2) & 0x01u);
    ctx.audible = (int)((flags_word >> 6) & 0x01u);
    ctx.mapDifficulty = 1;
    ctx.isOnPartyMap = (record->mapIndex == profile->current_level) ? 1 : 0;
    ctx.currentActiveGroupCount = 0;
    ctx.maxActiveGroupCount = 60;

    /* ReDMCSB TIMELINE.C F0245 lines 970-978 calls GROUP.C F0185 with
     * sensor data, health multiplier, count, random direction, and square
     * coordinates.  Firestaff's M10 F0860 owns the source-locked
     * count/random/health/cell calculation; this bridge binds its result to
     * CSB real-format C04 group slots and square-first-thing linkage. */
    F0730_COMBAT_RngInit_Compat(
        &rng,
        profile->dungeon_seed ^ profile->game_time ^
            ((uint32_t)record->mapX << 8) ^ ((uint32_t)record->mapY << 16));
    if (!F0860_RUNTIME_HandleGroupGenerator_Compat(
            &ctx,
            &rng,
            profile->game_time,
            &result) ||
        !result.spawned) {
        return;
    }

    previous_first = csb_v1_runtime_read_u16(first_thing_ptr);
    group_thing = (uint16_t)((4u << 10) | (uint16_t)(group_index & 0x03FF));
    csb_v1_runtime_write_u16(group_record + 0, previous_first);
    csb_v1_runtime_write_u16(group_record + 2, 0xFFFEu);
    group_record[4] = (uint8_t)(result.spawnedCreatureType & 0xFF);
    group_record[5] = (uint8_t)(result.spawnedGroupCells & 0xFF);
    for (i = 0; i < 4; ++i) {
        int hp = result.spawnedGroupHealth[i];
        if (hp < 0) hp = 0;
        if (hp > 0xFFFF) hp = 0xFFFF;
        csb_v1_runtime_write_u16(
            group_record + 6 + i * 2,
            (uint16_t)hp);
    }
    group_flags = (uint16_t)(((result.spawnedCreatureCount & 0x03) << 5) |
                             ((result.spawnedDirection & 0x03) << 8));
    csb_v1_runtime_write_u16(group_record + 14, group_flags);
    csb_v1_runtime_write_u16(first_thing_ptr, group_thing);
    csb_v1_runtime_sync_active_group_state_from_record(
        profile,
        group_thing,
        group_record,
        record->mapIndex,
        record->mapX,
        record->mapY,
        0,
        0);
    /* ReDMCSB GROUP.C F0180 lines 311-340 starts wandering by scheduling
     * C37 at game_time + 1 and prioritizes faster creatures as
     * 255 - MovementTicks. */
    csb_v1_runtime_schedule_c37_group_event(
        profile,
        record->mapIndex,
        record->mapX,
        record->mapY,
        result.spawnedCreatureType,
        1u);
}

static void csb_v1_runtime_apply_corridor_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int thing;
    int guard;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB TIMELINE.C F0245 lines 944-1001 walks C05 corridor square
     * things.  C02 textstrings toggle/set/clear Visible, while C006 floor
     * group generators disable once-only sensors or disable-and-schedule C65
     * after M046_TICKS.  Group materialization via GROUP.C F0185 remains a
     * separate CSB runtime binding because it needs live group-slot state. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *thing_record;
        int thing_type;
        int thing_size;

        thing_record = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!thing_record) break;
        if (thing_type == 2 && thing_size >= 4) {
            uint16_t text_word = csb_v1_runtime_read_u16(thing_record + 2);
            if (record->effect == DM1_EFFECT_TOGGLE) {
                text_word ^= 0x0001u;
            } else if (record->effect == DM1_EFFECT_SET) {
                text_word |= 0x0001u;
            } else {
                text_word &= (uint16_t)~0x0001u;
            }
            csb_v1_runtime_write_u16(thing_record + 2, text_word);
        } else if (thing_type == 3 && thing_size >= 8) {
            uint16_t type_data = csb_v1_runtime_read_u16(thing_record + 2);
            uint16_t flags_word = csb_v1_runtime_read_u16(thing_record + 4);
            uint16_t local_word = csb_v1_runtime_read_u16(thing_record + 6);
            int sensor_type = (int)(type_data & 0x007Fu);
            int sensor_data = (int)(type_data >> 7);
            int once_only = (int)((flags_word >> 2) & 0x01u);
            uint32_t ticks = (uint32_t)(local_word >> 4);

            if (sensor_type == 6) {
                csb_v1_runtime_materialize_corridor_generator_group(
                    profile,
                    record,
                    sensor_data,
                    flags_word,
                    local_word);
                if (once_only) {
                    type_data &= 0xFF80u;
                    csb_v1_runtime_write_u16(thing_record + 2, type_data);
                } else if (ticks != 0u) {
                    if (ticks > 127u) {
                        ticks = (ticks - 126u) << 6;
                    }
                    type_data &= 0xFF80u;
                    csb_v1_runtime_write_u16(thing_record + 2, type_data);
                    csb_v1_runtime_schedule_enable_group_generator_record(
                        profile,
                        record,
                        ticks);
                }
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_enable_group_generator_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int thing;
    int guard;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB TIMELINE.C F0246 lines 1009-1027 walks square things and
     * changes the first disabled C03 sensor type back to C006 group
     * generator. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *sensor;
        int thing_type;
        int thing_size;
        uint16_t type_data;

        sensor = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!sensor) break;
        if (thing_type == 3 && thing_size >= 8) {
            type_data = csb_v1_runtime_read_u16(sensor + 2);
            if ((type_data & 0x007Fu) == 0u) {
                type_data = (uint16_t)((type_data & 0xFF80u) | 6u);
                csb_v1_runtime_write_u16(sensor + 2, type_data);
                return;
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_wall_sensor_timeline_record(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record)
{
    CSB_V1_DungeonData *dungeon;
    int thing;
    int guard;

    if (!profile || !record || !profile->dungeon_handle) return;
    dungeon = profile->dungeon_handle;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        record->mapIndex,
        record->mapX,
        record->mapY);
    if (thing < 0) return;

    /* ReDMCSB TIMELINE.C F0248 lines 1175-1195 toggles same-cell wall
     * TextString visibility, then lines 1198-1308 handles wall C006
     * countdown and C005 AND/OR gate sensors by mutating M040_DATA and
     * feeding matching remote effects back through F0272_SENSOR_TriggerEffect.
     * Lines 1317-1339 handle C018 endgame sensors.  F0272 lines 1191-1197
     * also disables once-only triggered sensors and routes LocalEffect
     * sensors through F0270/F0271 instead of queuing a remote square event. */
    for (guard = 0; guard < 128 && thing != 0xFFFE && thing != 0xFFFF; ++guard) {
        uint8_t *sensor;
        int thing_type;
        int thing_size;
        uint16_t next_word;
        uint16_t type_data;
        uint16_t flags_word;
        uint16_t target_word;
        int sensor_type;
        int sensor_data;
        int sensor_effect;
        int revert_effect;
        int target_x;
        int target_y;
        int target_cell;
        int local_effect;
        int local_multiple;
        int once_only;
        int trigger = 0;
        int trigger_effect = DM1_EFFECT_SET;

        sensor = csb_v1_runtime_mutable_thing_record(
            dungeon,
            (uint16_t)thing,
            &thing_type,
            &thing_size);
        if (!sensor) break;
        if (thing_type == 2 && thing_size >= 4 &&
            csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                (uint16_t)thing) == (record->cell & 3)) {
            uint16_t text_word = csb_v1_runtime_read_u16(sensor + 2);
            if (record->effect == DM1_EFFECT_TOGGLE) {
                text_word ^= 0x0001u;
            } else if (record->effect == DM1_EFFECT_SET) {
                text_word |= 0x0001u;
            } else {
                text_word &= (uint16_t)~0x0001u;
            }
            csb_v1_runtime_write_u16(sensor + 2, text_word);
        } else if (thing_type == 3 && thing_size >= 8) {
            next_word = csb_v1_runtime_read_u16(sensor + 0);
            type_data = csb_v1_runtime_read_u16(sensor + 2);
            flags_word = csb_v1_runtime_read_u16(sensor + 4);
            target_word = csb_v1_runtime_read_u16(sensor + 6);
            sensor_type = (int)(type_data & 0x007Fu);
            sensor_data = (int)(type_data >> 7);
            sensor_effect = (int)((flags_word >> 3) & 0x03u);
            revert_effect = (int)((flags_word >> 5) & 0x01u);
            once_only = (int)((flags_word >> 2) & 0x01u);
            local_effect = (int)((flags_word >> 11) & 0x01u);
            local_multiple = (int)(target_word & 0x0FFFu);
            target_cell = (int)((target_word >> 4) & 0x03u);
            target_x = (int)((target_word >> 6) & 0x1Fu);
            target_y = (int)((target_word >> 11) & 0x1Fu);

            if (sensor_type == 6 && sensor_data > 0) {
                if (record->effect == DM1_EFFECT_SET) {
                    if (sensor_data < 511) sensor_data++;
                } else {
                    sensor_data--;
                }
                type_data = (uint16_t)((type_data & 0x007Fu) |
                                       ((uint16_t)sensor_data << 7));
                csb_v1_runtime_write_u16(sensor + 2, type_data);
                if (sensor_effect == DM1_EFFECT_HOLD) {
                    trigger = 1;
                    trigger_effect = ((sensor_data == 0) != revert_effect)
                        ? DM1_EFFECT_SET
                        : DM1_EFFECT_CLEAR;
                } else if (sensor_data == 0) {
                    trigger = 1;
                    trigger_effect = sensor_effect;
                }
            } else if (sensor_type == 5) {
                int bit_mask = 1 << (record->cell & 3);
                if (record->effect == DM1_EFFECT_TOGGLE) {
                    sensor_data ^= bit_mask;
                } else if (record->effect) {
                    sensor_data &= ~bit_mask;
                } else {
                    sensor_data |= bit_mask;
                }
                type_data = (uint16_t)((type_data & 0x007Fu) |
                                       ((uint16_t)sensor_data << 7));
                csb_v1_runtime_write_u16(sensor + 2, type_data);
                trigger = (((sensor_data & 0x000F) ==
                            ((sensor_data & 0x00F0) >> 4)) != revert_effect);
                if (sensor_effect == DM1_EFFECT_HOLD) {
                    trigger_effect = trigger ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
                    trigger = 1;
                } else {
                    trigger_effect = sensor_effect;
                }
            } else if (sensor_type == DM1_SENSOR_WALL_END_GAME) {
                struct DungeonSensor_Compat decoded_sensor;
                struct SensorTriggerResult_Compat endgame_result;
                csb_v1_runtime_decode_sensor_words(
                    next_word,
                    type_data,
                    flags_word,
                    target_word,
                    &decoded_sensor);
                memset(&endgame_result, 0, sizeof(endgame_result));
                if (F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
                        &decoded_sensor,
                        csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                            (uint16_t)thing),
                        record->effect,
                        record->cell,
                        &endgame_result) &&
                    endgame_result.triggered &&
                    endgame_result.endGameGameWon) {
                    profile->victory = 1;
                }
            } else if (csb_v1_runtime_sensor_type_is_explosion_launcher(
                           sensor_type) ||
                       csb_v1_runtime_sensor_type_is_new_object_launcher(
                           sensor_type) ||
                       csb_v1_runtime_sensor_type_is_square_object_launcher(
                           sensor_type)) {
                struct DungeonSensor_Compat decoded_sensor;
                struct ProjectileLauncherContext_Compat launcher_ctx;
                struct ProjectileLauncherResult_Compat launcher_result;
                struct ProjectileLauncherSquareThing_Compat square_things[64];
                int square_thing_count = 0;
                int launch_index;
                int is_square_object_launcher =
                    csb_v1_runtime_sensor_type_is_square_object_launcher(
                        sensor_type);
                int is_new_object_launcher =
                    csb_v1_runtime_sensor_type_is_new_object_launcher(
                        sensor_type);

                csb_v1_runtime_decode_sensor_words(
                    next_word,
                    type_data,
                    flags_word,
                    target_word,
                    &decoded_sensor);
                memset(&launcher_ctx, 0, sizeof(launcher_ctx));
                launcher_ctx.randomBit =
                    (int)((profile->dungeon_seed ^ profile->game_time ^
                           ((uint32_t)record->mapX << 4) ^
                           ((uint32_t)record->mapY << 8)) & 1u);
                launcher_ctx.newObjectThings[0] = 0xFFFFu;
                launcher_ctx.newObjectThings[1] = 0xFFFFu;
                if (is_new_object_launcher) {
                    launcher_ctx.newObjectThings[0] =
                        csb_v1_runtime_allocate_new_object_launcher_thing(
                            dungeon,
                            sensor_data);
                    if (sensor_type ==
                        DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ) {
                        launcher_ctx.newObjectThings[1] =
                            csb_v1_runtime_allocate_new_object_launcher_thing(
                                dungeon,
                                sensor_data);
                    }
                }
                if (is_square_object_launcher) {
                    square_thing_count =
                        csb_v1_runtime_collect_square_launcher_things(
                            dungeon,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            square_things,
                            (int)(sizeof(square_things) /
                                  sizeof(square_things[0])));
                    launcher_ctx.squareThings = square_things;
                    launcher_ctx.squareThingCount = square_thing_count;
                }
                memset(&launcher_result, 0, sizeof(launcher_result));
                if (F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
                        &decoded_sensor,
                        csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                            (uint16_t)thing),
                        record->mapX,
                        record->mapY,
                        record->cell,
                        &launcher_ctx,
                        &launcher_result) &&
                    launcher_result.triggered) {
                    if (launcher_result.sensorDisabled) {
                        type_data = (uint16_t)(type_data & 0xFF80u);
                        csb_v1_runtime_write_u16(sensor + 2, type_data);
                    }
                    if (is_square_object_launcher) {
                        int unlink_index;
                        /* ReDMCSB TIMELINE.C F0247 lines 1079-1100 finds
                         * same/next-cell square objects, then unlinks them
                         * through F0164 before F0212 creates launcher
                         * projectiles.  Sensor effects are intentionally not
                         * triggered by this ownership transfer. */
                        for (unlink_index = 0;
                             unlink_index < launcher_result.unlinkCount;
                             ++unlink_index) {
                            (void)csb_v1_runtime_unlink_thing_from_square(
                                dungeon,
                                launcher_result.unlinkThings[unlink_index],
                                record->mapIndex,
                                record->mapX,
                                record->mapY);
                        }
                    }
                    for (launch_index = 0;
                         launch_index < launcher_result.launchCount;
                         ++launch_index) {
                        const struct ProjectileLauncherLaunch_Compat *launch =
                            &launcher_result.launches[launch_index];
                        struct ProjectileCreateInput_Compat input;
                        struct TimelineEvent_Compat first_move;
                        int slot = -1;
                        int subtype;

                        if (!launch->valid) continue;
                        subtype =
                            csb_v1_runtime_projectile_subtype_from_explosion_thing(
                                launch->associatedThing);
                        memset(&input, 0, sizeof(input));
                        memset(&first_move, 0, sizeof(first_move));
                        input.category = (is_square_object_launcher ||
                                          is_new_object_launcher)
                            ? PROJECTILE_CATEGORY_KINETIC
                            : PROJECTILE_CATEGORY_MAGICAL;
                        input.subtype = (is_square_object_launcher ||
                                         is_new_object_launcher)
                            ? PROJECTILE_SUBTYPE_KINETIC_ARROW
                            : subtype;
                        input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
                        input.ownerIndex = -1;
                        input.mapIndex = record->mapIndex;
                        input.mapX = launch->mapX;
                        input.mapY = launch->mapY;
                        input.cell = launch->cell;
                        input.direction = launch->direction;
                        input.kineticEnergy = launch->kineticEnergy;
                        input.attack = launch->attack;
                        input.launcherStrength = launch->attack;
                        input.stepEnergy = launch->stepEnergy;
                        input.currentTick = (int)profile->game_time;
                        input.poisonAttack = (!is_square_object_launcher &&
                                              !is_new_object_launcher &&
                                              subtype ==
                                                  PROJECTILE_SUBTYPE_POISON_CLOUD)
                            ? launch->attack
                            : 0;
                        input.attackTypeCode = (is_square_object_launcher ||
                                                is_new_object_launcher)
                            ? COMBAT_ATTACK_BLUNT
                            : csb_v1_runtime_projectile_attack_type_from_subtype(
                                  subtype);
                        input.associatedThing = (int)launch->associatedThing;
                        input.firstMoveGraceFlag = 0;
                        if (F0810_PROJECTILE_Create_Compat(
                                &input,
                                &profile->projectiles,
                                &slot,
                                &first_move)) {
                            csb_v1_runtime_schedule_projectile_move_event(
                                profile,
                                &first_move);
                        }
                    }
                }
            }
            if (trigger) {
                if (once_only) {
                    type_data = (uint16_t)(type_data & 0xFF80u);
                    csb_v1_runtime_write_u16(sensor + 2, type_data);
                }
                if (local_effect) {
                    if (local_multiple == DM1_EFFECT_CLEAR ||
                        local_multiple == DM1_EFFECT_TOGGLE) {
                        (void)csb_v1_runtime_rotate_wall_cell_sensors(
                            dungeon,
                            record->mapIndex,
                            record->mapX,
                            record->mapY,
                            record->cell);
                    }
                } else {
                    csb_v1_runtime_trigger_remote_sensor_event(
                        profile,
                        record->mapIndex,
                        trigger_effect,
                        target_x,
                        target_y,
                        target_cell);
                }
            }
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_timeline_dispatch_side_effects(
    CSB_V1_RuntimeProfile *profile)
{
    int i;

    if (!profile) return;
    /* ReDMCSB: TIMELINE.C F0261 lines 1875-1901 dispatches C05/C06/C07/C08/C09/C10
     * to F0242/F0250/F0251/F0244; F0244 immediately routes doors through
     * C01 door-animation, and F0241 lines 754-809 steps the door state one
     * value per event.  This runtime bridge mutates real-format CSB byte-map
     * square flags and bounded wall/generator sensor state for the startup
     * playability path; projectile launchers, group movement, damage, sounds,
     * and DSA effects remain separate work. */
    for (i = 0; i < profile->last_timeline_dispatch.count; ++i) {
        const struct DM1_DispatchRecord_V1 *record =
            &profile->last_timeline_dispatch.records[i];
        switch (record->eventType) {
        case DM1_EVENT_CORRIDOR:
            csb_v1_runtime_apply_corridor_timeline_record(profile, record);
            break;
        case DM1_EVENT_WALL:
            csb_v1_runtime_apply_wall_sensor_timeline_record(profile, record);
            break;
        case DM1_EVENT_MOVE_PROJECTILE:
        case DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS:
            csb_v1_runtime_apply_projectile_move_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_EXPLOSION:
            csb_v1_runtime_apply_explosion_timeline_record(profile, record);
            break;
        case DM1_EVENT_DOOR:
        case DM1_EVENT_DOOR_ANIMATION:
            csb_v1_runtime_apply_door_timeline_record(profile, record);
            break;
        case DM1_EVENT_DOOR_DESTRUCTION:
            {
                uint8_t *square;
                int square_type;
                square = csb_v1_runtime_square_byte_ptr(
                    profile,
                    record->mapIndex,
                    record->mapX,
                    record->mapY,
                    &square_type);
                if (square && square_type == 4) {
                    *square = (uint8_t)((*square & (uint8_t)~0x07u) | 5u);
                }
            }
            break;
        case DM1_EVENT_FAKEWALL:
            csb_v1_runtime_apply_square_flag_timeline_record(
                profile,
                record,
                6,
                0x04u);
            break;
        case DM1_EVENT_TELEPORTER:
            csb_v1_runtime_apply_square_flag_timeline_record(
                profile,
                record,
                5,
                0x08u);
            break;
        case DM1_EVENT_PIT:
            csb_v1_runtime_apply_square_flag_timeline_record(
                profile,
                record,
                2,
                0x08u);
            break;
        case DM1_EVENT_ENABLE_GROUP_GENERATOR:
            csb_v1_runtime_apply_enable_group_generator_record(
                profile,
                record);
            break;
        case DM1_EVENT_UPDATE_BEHAVIOR_GROUP:
            csb_v1_runtime_apply_group_behavior_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_MOVE_GROUP_SILENT:
        case DM1_EVENT_MOVE_GROUP_AUDIBLE:
            csb_v1_runtime_apply_move_group_timeline_record(profile, record);
            break;
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_0:
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_1:
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_2:
        case DM1_EVENT_UPDATE_ASPECT_CREATURE_3:
            csb_v1_runtime_apply_creature_aspect_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0:
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1:
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_2:
        case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3:
            csb_v1_runtime_apply_creature_attack_timeline_record(
                profile,
                record);
            break;
        case DM1_EVENT_POISON_CHAMPION:
            csb_v1_runtime_apply_poison_event_record(profile, record);
            break;
        default:
            break;
        }
    }
}

/* ── Runtime profile API ────────────────────────────────────────────── */

void csb_v1_runtime_init(CSB_V1_RuntimeProfile *profile, const char *data_dir)
{
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));

    profile->variant_id     = CSB_V1_VARIANT_UNKNOWN;
    profile->difficulty    = CSB_V1_DIFFICULTY_HARD; /* default: 3 champions */
    profile->current_level = 0;
    profile->current_world = 0;
    profile->level_count   = 1;
    profile->world_count   = 1;
    profile->champion_count = 3;
    profile->leader_index = -1;
    profile->magic_caster_index = -1;
    profile->party_state_valid = 0;
    csb_v1_character_init_default(&profile->party_state);

    profile->party_x = CSB_V1_START_PARTY_X;
    profile->party_y = CSB_V1_START_PARTY_Y;
    profile->party_z = CSB_V1_START_PARTY_Z;
    profile->party_dir = CSB_V1_START_PARTY_DIR;

    profile->state     = CSB_STATE_TITLE;
    profile->paused    = 0;
    profile->victory   = 0;
    profile->game_over = 0;

    profile->game_ticks    = 0;
    profile->game_time     = 0;
    profile->total_play_ms = 0;
    profile->tick_count    = 0;
    dm1v1_event_queue_init(&profile->timeline_queue, profile->game_time);
    csb_v1_audio_runtime_init(&profile->audio_runtime);
    memset(&profile->last_timeline_dispatch, 0,
           sizeof(profile->last_timeline_dispatch));
    profile->timeline_dispatch_count = 0;
    csb_v1_skin_cache_init(&profile->skin_cache);
    DM1_V1_InputCommandQueue_InitPc34Compat(&profile->input_command_queue);
    memset(&profile->last_input_dispatch, 0,
           sizeof(profile->last_input_dispatch));
    profile->input_dispatch_count = 0;

    profile->data_dir = data_dir;
    profile->save_dir = csb_v1_runtime_save_dir();
}

int csb_v1_runtime_custom_background_skin_grid(
    CSB_V1_RuntimeProfile *profile,
    uint8_t *out_cell_skins,
    int out_cell_skin_capacity,
    int *out_width,
    int *out_height,
    int *out_loaded_level,
    int *out_default_skin)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int width;
    int height;
    int x;
    int y;
    int has_skin = 0;
    uint8_t default_skin;
    CSB_V1_RuntimeSkinCacheLookupCtx lookup_ctx;

    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_loaded_level) *out_loaded_level = -1;
    if (out_default_skin) *out_default_skin = 0;
    if (!profile || !out_cell_skins || out_cell_skin_capacity <= 0 ||
        !profile->dungeon_handle) {
        return 0;
    }

    dungeon = profile->dungeon_handle;
    level = profile->current_level;
    if (level < 0 || level >= dungeon->level_count) {
        return 0;
    }
    width = dungeon->level_widths[level];
    height = dungeon->level_heights[level];
    if (width <= 0 || height <= 0 ||
        width * height > out_cell_skin_capacity) {
        return 0;
    }

    memset(out_cell_skins, 0, (size_t)width * (size_t)height);
    /* CSBWin data.cpp SKIN_CACHE::GetSkin/GetDefaultSkin reads Expool
     * EDT_Skins records through Locate(); Firestaff resolves the runtime
     * CSBWin save tail first so saved SETSKIN state can override the loaded
     * dungeon DB11 defaults during startup/resume rendering. */
    lookup_ctx.profile = profile;
    lookup_ctx.dungeon = dungeon;
    default_skin = csb_v1_skin_cache_get_default_skin(
        &profile->skin_cache,
        csb_v1_runtime_skin_cache_record_lookup,
        &lookup_ctx,
        level);
    if (default_skin != 0u) {
        has_skin = 1;
    }
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint8_t skin = csb_v1_skin_cache_get_skin(
                &profile->skin_cache,
                csb_v1_runtime_skin_cache_record_lookup,
                &lookup_ctx,
                level,
                width,
                height,
                x,
                y);
            out_cell_skins[(size_t)y * (size_t)width + (size_t)x] = skin;
            if (skin != 0u) {
                has_skin = 1;
            }
        }
    }

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    if (out_loaded_level) *out_loaded_level = level;
    if (out_default_skin) *out_default_skin = (int)default_skin;
    return has_skin;
}

int csb_v1_runtime_add_timeline_event(CSB_V1_RuntimeProfile *profile,
                                      const struct DM1_Event_V1 *event)
{
    if (!profile || !event) return -1;
    profile->timeline_queue.gameTick = profile->game_time;
    return dm1v1_event_add(&profile->timeline_queue, event);
}

int csb_v1_runtime_get_last_timeline_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct DM1_TickDispatchResult_V1 *out_result)
{
    if (!profile || !out_result) return -1;
    *out_result = profile->last_timeline_dispatch;
    return out_result->count;
}

int csb_v1_runtime_enqueue_input_command(CSB_V1_RuntimeProfile *profile,
                                         int command,
                                         int x,
                                         int y)
{
    if (!profile) return 0;
    return DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
        &profile->input_command_queue, command, x, y);
}

int csb_v1_runtime_process_one_input_command(
    CSB_V1_RuntimeProfile *profile,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction)
{
    CSB_V1_InputCommandRuntimeResult result;

    if (!profile) return -1;
    return csb_v1_runtime_process_input_queue(
        profile,
        &profile->input_command_queue,
        disabled_movement_ticks,
        projectile_disabled_movement_ticks,
        last_projectile_disabled_movement_direction,
        &result);
}

int csb_v1_runtime_get_last_input_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputQueueProcessResultPc34Compat *out_result)
{
    if (!profile || !out_result) return -1;
    *out_result = profile->last_input_dispatch;
    return out_result->dequeued;
}

static int csb_v1_runtime_first_living_champion(const CSB_V1_PartyState *party)
{
    int i;
    if (!party) return -1;
    for (i = 0; i < party->ChampionCount && i < CSB_V1_MAX_CHAMPIONS; i++) {
        if (!csb_v1_champion_is_dead(&party->Champions[i]) &&
            party->Champions[i].CurrentHealth > 0) {
            return i;
        }
    }
    return -1;
}

static uint16_t csb_v1_runtime_normalize_leader_hand_thing(uint16_t thing)
{
    return thing == 0u ? 0xffffu : thing;
}

static uint16_t csb_v1_runtime_export_leader_hand_thing(
    const CSB_V1_RuntimeProfile *profile)
{
    if (!profile) return 0xffffu;
    if (profile->party_state_valid) {
        return csb_v1_runtime_normalize_leader_hand_thing(
            profile->party_state.LeaderHandThing);
    }
    return profile->csbwin_gameblock2_summary_valid
        ? csb_v1_runtime_normalize_leader_hand_thing(
              profile->csbwin_object_in_hand)
        : 0xffffu;
}

static int csb_v1_runtime_direction_from_source_to_destination(
    int source_x,
    int source_y,
    int dest_x,
    int dest_y)
{
    if (source_x == dest_x) return (source_y > dest_y) ? 0 : 2;
    if (source_y == dest_y) return (source_x > dest_x) ? 3 : 1;
    return 0;
}

static int csb_v1_runtime_champion_index_in_cell(
    const CSB_V1_PartyState *party,
    int cell)
{
    int i;
    if (!party) return -1;
    for (i = 0; i < party->ChampionCount && i < CSB_V1_MAX_CHAMPIONS; i++) {
        const CSB_V1_Champion *champion = &party->Champions[i];
        if (((int)champion->Cell & 3) == (cell & 3) &&
            !csb_v1_champion_is_dead(champion) &&
            champion->CurrentHealth > 0) {
            return i;
        }
    }
    return -1;
}

static int csb_v1_runtime_target_champion_for_adjacent_attack(
    const CSB_V1_RuntimeProfile *profile,
    int attacker_x,
    int attacker_y,
    int creature_cell)
{
    static const unsigned char ordered_cells[8][4] = {
        { 0, 1, 3, 2 },
        { 1, 0, 2, 3 },
        { 1, 2, 0, 3 },
        { 2, 1, 3, 0 },
        { 3, 2, 0, 1 },
        { 2, 3, 1, 0 },
        { 0, 3, 1, 2 },
        { 3, 0, 2, 1 }
    };
    int distance_x;
    int distance_y;
    int direction;
    int table_index;
    int i;

    if (!profile || !profile->party_state_valid) return -1;
    distance_x = abs(profile->party_x - attacker_x);
    distance_y = abs(profile->party_y - attacker_y);
    if (distance_x > 1 || distance_y > 1) return -1;

    /* ReDMCSB CHAMPION.C F0286 calls PROJEXPL.C F0229, which derives an
     * ordered four-cell attack list from attacker/party coordinates and the
     * source creature cell, then returns the first living champion in those
     * cells. */
    direction = csb_v1_runtime_direction_from_source_to_destination(
        profile->party_x,
        profile->party_y,
        attacker_x,
        attacker_y);
    table_index = direction << 1;
    if ((table_index & 0x0002) == 0) {
        creature_cell++;
    }
    table_index += (creature_cell >> 1) & 0x0001;
    table_index &= 7;
    for (i = 0; i < 4; i++) {
        int champion_index = csb_v1_runtime_champion_index_in_cell(
            &profile->party_state,
            ordered_cells[table_index][i]);
        if (champion_index >= 0) return champion_index;
    }
    return -1;
}

int csb_v1_runtime_set_party_state(CSB_V1_RuntimeProfile *profile,
                                   const CSB_V1_PartyState *party)
{
    int leader;
    if (!profile || !party) return -1;
    if (party->ChampionCount < 0 ||
        party->ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    profile->party_state = *party;
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(
            profile->party_state.LeaderHandThing);
    profile->party_state_valid = 1;
    profile->champion_count = party->ChampionCount;
    profile->party_dir = party->PartyDirection & 3;
    profile->magic_caster_index = party->MagicCasterIndex;

    leader = party->LeaderIndex;
    if (leader < 0 || leader >= party->ChampionCount ||
        csb_v1_champion_is_dead(&party->Champions[leader]) ||
        party->Champions[leader].CurrentHealth <= 0) {
        leader = csb_v1_runtime_first_living_champion(party);
    }
    profile->leader_index = leader;
    profile->party_state.LeaderIndex = leader;
    return 0;
}

int csb_v1_runtime_get_party_state(const CSB_V1_RuntimeProfile *profile,
                                   CSB_V1_PartyState *out_party)
{
    if (!profile || !out_party || !profile->party_state_valid) return -1;
    *out_party = profile->party_state;
    return out_party->ChampionCount;
}

int csb_v1_runtime_apply_csbwin_gameblock2_summary(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    if (!profile || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT) {
        return -1;
    }
    if (summary->num_character > CSB_V1_MAX_CHAMPIONS ||
        summary->party_x > CSB_V1_MAX_PARTY_X ||
        summary->party_y > CSB_V1_MAX_PARTY_Y ||
        summary->party_facing > 3u ||
        summary->appended_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
        summary->appended_size < summary->appended_preserved_size) {
        return -1;
    }

    /* CSBWin SaveGame.cpp lines 1775-1811 applies GAMEBLOCK2 after
     * `swapBlock2()`: time/RNG, party pose, hand/caster indexes, timer
     * metadata, cursor object, and ITEM16 capacity. Firestaff stores this
     * as a bounded startup/resume handoff until the decoded champion,
     * item, and timer bodies are imported into the live runtime. */
    profile->game_time = summary->game_time;
    profile->timeline_queue.gameTick = profile->game_time;
    profile->party_x = (int)summary->party_x;
    profile->party_y = (int)summary->party_y;
    profile->party_z = (int)summary->party_level;
    profile->current_level = (int)summary->party_level;
    csb_v1_dungeon_set_current_level(profile->current_level);
    profile->party_dir = (int)(summary->party_facing & 3u);
    profile->champion_count = (int)summary->num_character;
    profile->leader_index = (summary->hand_char < summary->num_character)
        ? (int)summary->hand_char
        : -1;
    profile->magic_caster_index = (summary->magic_caster < summary->num_character)
        ? (int)summary->magic_caster
        : -1;
    if (profile->party_state_valid) {
        profile->party_state.ChampionCount = profile->champion_count;
        profile->party_state.PartyDirection = (uint8_t)(profile->party_dir & 3);
        profile->party_state.LeaderIndex = profile->leader_index;
        profile->party_state.MagicCasterIndex = profile->magic_caster_index;
    }

    profile->csbwin_gameblock2_summary_valid = 1;
    profile->csbwin_random_seed = summary->random_seed;
    profile->csbwin_object_in_hand = summary->object_in_hand;
    if (profile->party_state_valid) {
        profile->party_state.LeaderHandThing =
            csb_v1_runtime_normalize_leader_hand_thing(
                summary->object_in_hand);
    }
    profile->csbwin_num_timer = summary->num_timer;
    profile->csbwin_first_avail_timer = summary->first_avail_timer;
    profile->csbwin_max_timers = summary->max_timers;
    profile->csbwin_item16_queue_len = summary->item16_queue_len;
    profile->csbwin_max_item16 = summary->max_item16;
    profile->csbwin_timer_sequence = summary->timer_sequence;
    profile->csbwin_last_monster_attack_time =
        summary->last_monster_attack_time;
    profile->csbwin_last_party_move_time = summary->last_party_move_time;
    profile->csbwin_party_move_disable_timer =
        summary->party_move_disable_timer;
    profile->csbwin_word11712 = summary->word11712;
    profile->csbwin_word11714 = summary->word11714;
    profile->csbwin_header_tail_valid = 1;
    memcpy(profile->csbwin_header_byte22808,
           summary->header.public_fields.csbwin_byte22808,
           sizeof(profile->csbwin_header_byte22808));
    profile->csbwin_appended_tail_valid =
        summary->appended_size != 0u ? 1 : 0;
    profile->csbwin_appended_tail_size = summary->appended_size;
    profile->csbwin_appended_tail_preserved_size =
        summary->appended_preserved_size;
    profile->csbwin_appended_tail_fnv1a = summary->appended_fnv1a;
    profile->csbwin_appended_tail_truncated = summary->appended_truncated;
    memset(profile->csbwin_appended_tail, 0,
           sizeof(profile->csbwin_appended_tail));
    if (summary->appended_preserved_size != 0u) {
        memcpy(profile->csbwin_appended_tail,
               summary->appended_preserved,
               summary->appended_preserved_size);
    }
    return 0;
}

static void csb_v1_runtime_copy_csbwin_champion_text(char *dst,
                                                     size_t dst_size,
                                                     const char *src)
{
    if (!dst || dst_size == 0u) return;
    memset(dst, 0, dst_size);
    if (!src) return;
    strncpy(dst, src, dst_size - 1u);
}

static uint16_t csb_v1_runtime_csbwin_attr_to_firestaff_stat(
    const CSB_V1_CSBWin512ChampionSummary *src,
    int csbwin_attr,
    int firestaff_component)
{
    static const int component_map[3] = {
        2, /* Firestaff minimum <- CSBWin ATTRIBUTE.ubMinimum */
        1, /* Firestaff current <- CSBWin ATTRIBUTE.ubCurrent */
        0  /* Firestaff maximum <- CSBWin ATTRIBUTE.ubMaximum */
    };
    if (!src || csbwin_attr < 0 || csbwin_attr >= 7 ||
        firestaff_component < 0 || firestaff_component >= 3) {
        return 0u;
    }
    return (uint16_t)src->attributes[csbwin_attr]
                                      [component_map[firestaff_component]];
}

int csb_v1_runtime_apply_csbwin_champion_summaries(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    static const int attr_to_stat[CSB_V1_STAT_COUNT] = {
        1, /* STR <- CSBWin attribute[1] Strength */
        2, /* DEX <- CSBWin attribute[2] Dexterity */
        3, /* WIS <- CSBWin attribute[3] Wisdom */
        4, /* VIT <- CSBWin attribute[4] Vitality */
        5, /* AntiMagic <- CSBWin attribute[5] */
        6, /* AntiFire <- CSBWin attribute[6] */
        0  /* Luck <- CSBWin attribute[0] */
    };
    int champion_count;
    int champion_index;

    if (!profile || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT ||
        summary->num_character > CSB_V1_MAX_CHAMPIONS ||
        summary->party_facing > 3u) {
        return -1;
    }

    champion_count = (int)summary->num_character;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = champion_count;
    profile->party_state.PartyDirection = (int)(summary->party_facing & 3u);
    profile->party_state.PartyMapX = (int)summary->party_x;
    profile->party_state.PartyMapY = (int)summary->party_y;
    profile->party_state.LeaderIndex =
        (summary->hand_char < summary->num_character)
            ? (int)summary->hand_char
            : -1;
    profile->party_state.MagicCasterIndex =
        (summary->magic_caster < summary->num_character)
            ? (int)summary->magic_caster
            : -1;
    profile->party_state.LeaderHandThing =
        csb_v1_runtime_normalize_leader_hand_thing(
            summary->object_in_hand);

    for (champion_index = 0; champion_index < CSB_V1_MAX_CHAMPIONS;
         ++champion_index) {
        CSB_V1_Champion *dst =
            &profile->party_state.Champions[champion_index];
        const CSB_V1_CSBWin512ChampionSummary *src =
            &summary->champions[champion_index];
        int stat_index;
        int skill_index;
        int slot_index;

        csb_v1_champion_init(dst);
        if (champion_index >= champion_count || !src->valid) {
            continue;
        }

        /* CSBWin SaveGame.cpp:1838 swapCharacterData() consumes four
         * CHARDESC records. CSBWin/CSB.h:2486-2597 gives fixed offsets for
         * identity, vitals, attributes, possessions, timers, load, and
         * portrait bytes at offset 336. */
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->Name, sizeof(dst->Name), src->name);
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->Title, sizeof(dst->Title), src->title);
        memcpy(dst->Portrait, src->portrait, sizeof(src->portrait));
        dst->CsbWinWord24 = src->word24;
        dst->CurrentHealth = src->hp;
        dst->MaximumHealth = src->max_hp;
        dst->CurrentStamina = src->stamina;
        dst->MaximumStamina = src->max_stamina;
        dst->CurrentMana = src->mana;
        dst->MaximumMana = src->max_mana;
        dst->CsbWinWord64 = src->word64;
        for (stat_index = 0; stat_index < CSB_V1_STAT_COUNT; ++stat_index) {
            const int csbwin_attr = attr_to_stat[stat_index];
            dst->Statistics[stat_index][CSB_V1_STAT_MIN] =
                csb_v1_runtime_csbwin_attr_to_firestaff_stat(
                    src, csbwin_attr, CSB_V1_STAT_MIN);
            dst->Statistics[stat_index][CSB_V1_STAT_CUR] =
                csb_v1_runtime_csbwin_attr_to_firestaff_stat(
                    src, csbwin_attr, CSB_V1_STAT_CUR);
            dst->Statistics[stat_index][CSB_V1_STAT_MAX] =
                csb_v1_runtime_csbwin_attr_to_firestaff_stat(
                    src, csbwin_attr, CSB_V1_STAT_MAX);
        }
        dst->SkillExperienceValid = 1u;
        for (skill_index = 0;
             skill_index < CSB_V1_FULL_SKILL_COUNT;
             ++skill_index) {
            dst->SkillExperience[skill_index] =
                src->skill_experience[skill_index];
            dst->SkillTemporaryExperience[skill_index] =
                src->skill_temp_adjust[skill_index];
        }
        for (skill_index = 0;
             skill_index < CSB_V1_SKILL_COUNT;
             ++skill_index) {
            dst->Skills[skill_index] =
                (uint8_t)csb_v1_runtime_imported_skill_level(
                    dst,
                    skill_index);
        }
        for (slot_index = 0; slot_index < CSB_V1_SLOT_COUNT; ++slot_index) {
            dst->Slots[slot_index] = src->possessions[slot_index];
        }
        dst->Cell = (uint8_t)(src->char_position & 3u);
        dst->Direction = (uint8_t)(src->facing & 3u);
        dst->DirectionMaximumDamageReceived = src->max_recent_damage;
        dst->CsbWinByte30 = src->byte30;
        dst->CsbWinByte31 = src->byte31;
        memcpy(dst->Incantation, src->incantation, sizeof(dst->Incantation));
        dst->CsbWinByte33 = src->byte33;
        dst->CsbWinFacing3 = src->facing3;
        dst->CsbWinUByte43 = src->ubyte43;
        dst->ActionIndex = (src->attack_type < 0)
            ? CSB_V1_ACTION_NONE
            : (uint8_t)src->attack_type;
        dst->EnableActionEventIndex = src->busy_timer;
        dst->HideDamageReceivedEventIndex = src->timer_index;
        dst->Attributes = (uint16_t)src->char_flags;
        dst->Wounds = (uint16_t)src->wounds;
        dst->PoisonEventCount = src->poison_count;
        dst->Food = src->food;
        dst->Water = src->water;
        dst->Load = src->load;
        dst->ShieldStrength = src->shield_strength;
        dst->Talents = src->talents;
        dst->Fingerprint = src->fingerprint;
        dst->CauseOfDamage = src->cause_of_damage;
        dst->MonsterCausingDamage = src->monster_causing_damage;
        dst->EventIndex = src->timer_index;
    }

    profile->champion_count = champion_count;
    profile->leader_index = profile->party_state.LeaderIndex;
    profile->magic_caster_index = profile->party_state.MagicCasterIndex;
    return 0;
}

int csb_v1_runtime_export_csbwin_champion_summaries(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWin512BodyReport *out_summary)
{
    static const int attr_to_stat[CSB_V1_STAT_COUNT] = {
        1, /* STR -> CSBWin attribute[1] Strength */
        2, /* DEX -> CSBWin attribute[2] Dexterity */
        3, /* WIS -> CSBWin attribute[3] Wisdom */
        4, /* VIT -> CSBWin attribute[4] Vitality */
        5, /* AntiMagic -> CSBWin attribute[5] */
        6, /* AntiFire -> CSBWin attribute[6] */
        0  /* Luck -> CSBWin attribute[0] */
    };
    int champion_count;
    int champion_index;

    if (!profile || !out_summary || !profile->party_state_valid) {
        return -1;
    }

    memset(out_summary, 0, sizeof(*out_summary));
    champion_count = profile->party_state.ChampionCount;
    if (champion_count < 0) champion_count = 0;
    if (champion_count > CSB_V1_MAX_CHAMPIONS) {
        champion_count = CSB_V1_MAX_CHAMPIONS;
    }

    /* CSBWin SaveGame.cpp:1838 writes four CHARDESC records after
     * swapCharacterData(); CSBWin/CSB.h:2486-2597 defines the fixed field
     * order. This export is a bounded runtime summary, not the encrypted
     * 512-byte CSBWin file writer. */
    out_summary->header_valid = 1;
    out_summary->sections_verified = CSB_V1_CSBWIN_512_SECTION_COUNT;
    out_summary->num_character = (uint16_t)champion_count;
    out_summary->party_x = (uint16_t)(profile->party_state.PartyMapX & 0xffff);
    out_summary->party_y = (uint16_t)(profile->party_state.PartyMapY & 0xffff);
    out_summary->party_level = (uint16_t)(profile->current_level & 0xffff);
    out_summary->party_facing =
        (uint16_t)(profile->party_state.PartyDirection & 3);
    out_summary->hand_char =
        (profile->party_state.LeaderIndex >= 0 &&
         profile->party_state.LeaderIndex < champion_count)
            ? (uint16_t)profile->party_state.LeaderIndex
            : 0xffffu;
    out_summary->magic_caster =
        (profile->party_state.MagicCasterIndex >= 0 &&
         profile->party_state.MagicCasterIndex < champion_count)
            ? (uint16_t)profile->party_state.MagicCasterIndex
            : 0xffffu;
    out_summary->object_in_hand = csb_v1_runtime_export_leader_hand_thing(
        profile);

    for (champion_index = 0; champion_index < CSB_V1_MAX_CHAMPIONS;
         ++champion_index) {
        const CSB_V1_Champion *src =
            &profile->party_state.Champions[champion_index];
        CSB_V1_CSBWin512ChampionSummary *dst =
            &out_summary->champions[champion_index];
        int stat_index;
        int skill_index;
        int slot_index;

        if (champion_index >= champion_count) {
            continue;
        }

        dst->valid = 1;
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->name, sizeof(dst->name), src->Name);
        csb_v1_runtime_copy_csbwin_champion_text(
            dst->title, sizeof(dst->title), src->Title);
        dst->word24 = src->CsbWinWord24;
        dst->facing = (uint8_t)(src->Direction & 3u);
        dst->char_position = (uint8_t)(src->Cell & 3u);
        dst->byte30 = src->CsbWinByte30;
        dst->byte31 = src->CsbWinByte31;
        dst->attack_type = (src->ActionIndex == CSB_V1_ACTION_NONE)
            ? -1
            : (int8_t)src->ActionIndex;
        dst->byte33 = src->CsbWinByte33;
        memcpy(dst->incantation, src->Incantation, sizeof(dst->incantation));
        dst->facing3 = src->CsbWinFacing3;
        dst->max_recent_damage =
            (uint8_t)(src->DirectionMaximumDamageReceived & 0xffu);
        dst->poison_count = src->PoisonEventCount;
        dst->ubyte43 = src->CsbWinUByte43;
        dst->busy_timer = src->EnableActionEventIndex;
        dst->timer_index = src->HideDamageReceivedEventIndex;
        dst->char_flags = (int16_t)src->Attributes;
        dst->wounds = (int16_t)src->Wounds;
        dst->hp = src->CurrentHealth;
        dst->max_hp = src->MaximumHealth;
        dst->stamina = src->CurrentStamina;
        dst->max_stamina = src->MaximumStamina;
        dst->mana = src->CurrentMana;
        dst->max_mana = src->MaximumMana;
        dst->word64 = src->CsbWinWord64;
        dst->food = src->Food;
        dst->water = src->Water;
        for (stat_index = 0; stat_index < CSB_V1_STAT_COUNT; ++stat_index) {
            const int csbwin_attr = attr_to_stat[stat_index];
            dst->attributes[csbwin_attr][0] =
                (uint8_t)(src->Statistics[stat_index][CSB_V1_STAT_MAX] &
                          0xffu);
            dst->attributes[csbwin_attr][1] =
                (uint8_t)(src->Statistics[stat_index][CSB_V1_STAT_CUR] &
                          0xffu);
            dst->attributes[csbwin_attr][2] =
                (uint8_t)(src->Statistics[stat_index][CSB_V1_STAT_MIN] &
                          0xffu);
        }
        if (src->SkillExperienceValid) {
            for (skill_index = 0;
                 skill_index < CSB_V1_FULL_SKILL_COUNT;
                 ++skill_index) {
                dst->skill_experience[skill_index] =
                    src->SkillExperience[skill_index];
                dst->skill_temp_adjust[skill_index] =
                    src->SkillTemporaryExperience[skill_index];
            }
        }
        for (slot_index = 0; slot_index < CSB_V1_SLOT_COUNT; ++slot_index) {
            dst->possessions[slot_index] = src->Slots[slot_index];
        }
        dst->load = src->Load;
        dst->shield_strength = src->ShieldStrength;
        dst->talents = src->Talents;
        dst->fingerprint = src->Fingerprint;
        dst->cause_of_damage = src->CauseOfDamage;
        dst->monster_causing_damage = src->MonsterCausingDamage;
        memcpy(dst->portrait, src->Portrait, sizeof(dst->portrait));
    }

    return champion_count;
}

int csb_v1_runtime_apply_csbwin_body_runtime_summaries(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    if (!profile || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT ||
        summary->item16_summary_count >
            CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES ||
        summary->timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        summary->timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES) {
        return -1;
    }

    /* CSBWin SaveGame.cpp:535-543 swapCharacterData() and
     * SaveGame.cpp:1822-1855 body load restore character-tail spell state,
     * ITEM16 active-monster records, timers, and timer queue after the
     * GAMEBLOCK2 handoff. This runtime step is intentionally still a
     * bounded summary copy: it preserves verified decoded state for startup
     * resume while the full event/item materialization remains separate. */
    profile->csbwin_body_runtime_summary_valid = 1;
    profile->csbwin_character_tail_brightness =
        summary->character_tail_brightness;
    profile->csbwin_character_tail_see_thru_walls =
        summary->character_tail_see_thru_walls;
    profile->csbwin_character_tail_magic_footprints_active =
        summary->character_tail_magic_footprints_active;
    profile->csbwin_character_tail_party_shield =
        summary->character_tail_party_shield;
    profile->csbwin_character_tail_fire_shield =
        summary->character_tail_fire_shield;
    profile->csbwin_character_tail_spell_shield =
        summary->character_tail_spell_shield;
    profile->csbwin_character_tail_num_footprint_entries =
        summary->character_tail_num_footprint_entries;
    profile->csbwin_character_tail_freeze_life_timer =
        summary->character_tail_freeze_life_timer;
    profile->csbwin_character_tail_first_magic_footprint =
        summary->character_tail_first_magic_footprint;
    profile->csbwin_character_tail_last_magic_footprint =
        summary->character_tail_last_magic_footprint;
    memcpy(profile->csbwin_character_tail_party_footprints,
           summary->character_tail_party_footprints,
           sizeof(profile->csbwin_character_tail_party_footprints));
    memcpy(profile->csbwin_character_tail_byte13220,
           summary->character_tail_byte13220,
           sizeof(profile->csbwin_character_tail_byte13220));
    profile->csbwin_character_tail_invisible =
        summary->character_tail_invisible;

    profile->csbwin_item16_summary_count = summary->item16_summary_count;
    profile->csbwin_item16_summary_total = summary->item16_summary_total;
    memcpy(profile->csbwin_item16,
           summary->item16,
           sizeof(profile->csbwin_item16));
    profile->csbwin_timer_summary_count = summary->timer_summary_count;
    profile->csbwin_timer_summary_total = summary->timer_summary_total;
    memcpy(profile->csbwin_timers,
           summary->timers,
           sizeof(profile->csbwin_timers));
    profile->csbwin_timer_queue_summary_count =
        summary->timer_queue_summary_count;
    profile->csbwin_timer_queue_summary_total =
        summary->timer_queue_summary_total;
    memcpy(profile->csbwin_timer_queue,
           summary->timer_queue,
           sizeof(profile->csbwin_timer_queue));
    return 0;
}

int csb_v1_runtime_materialize_csbwin_item16_summaries(
    CSB_V1_RuntimeProfile *profile)
{
    uint16_t item_index;
    int imported = 0;

    if (!profile || !profile->csbwin_body_runtime_summary_valid) {
        return -1;
    }
    if (profile->csbwin_item16_summary_count >
        CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES) {
        return -1;
    }

    memset(profile->csbwin_runtime_item16, 0,
           sizeof(profile->csbwin_runtime_item16));
    profile->csbwin_runtime_item16_count = 0u;
    profile->csbwin_runtime_item16_total =
        profile->csbwin_item16_summary_total;

    /* CSBWin CSB.h:2257-2280 defines ITEM16 as active-monster state:
     * word0 DB4 monster index, packed facings/positions, d.Time low byte,
     * target/previous/current coordinates, and four SINGLE_MONSTER_STATUS
     * bytes. SaveGame.cpp:491-499 swaps only word0 after loading. A negative
     * word0 marks an unused slot, so Firestaff skips 0xffff here while
     * preserving the bounded active records for later AI/runtime ownership. */
    for (item_index = 0u;
         item_index < profile->csbwin_item16_summary_count;
         ++item_index) {
        const CSB_V1_CSBWin512Item16Summary *src =
            &profile->csbwin_item16[item_index];
        CSB_V1_CSBWinRuntimeItem16 *dst;

        if (!src->valid || src->monster_index == 0xffffu) {
            continue;
        }
        if (profile->csbwin_runtime_item16_count >=
            CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES) {
            break;
        }

        dst = &profile->csbwin_runtime_item16
            [profile->csbwin_runtime_item16_count];
        memset(dst, 0, sizeof(*dst));
        dst->valid = 1;
        dst->monster_index = src->monster_index;
        dst->live_ai_group_thing = 0xFFFFu;
        dst->live_ai_map_index = -1;
        dst->live_ai_map_x = -1;
        dst->live_ai_map_y = -1;
        dst->facings = src->facings;
        dst->positions = src->positions;
        dst->last_move_time_lsb = src->ubyte4;
        dst->delay_or_flee_timer = src->ubyte5;
        dst->target_x = src->target_x;
        dst->target_y = src->target_y;
        dst->previous_x = src->previous_x;
        dst->previous_y = src->previous_y;
        dst->current_x = src->current_x;
        dst->current_y = src->current_y;
        memcpy(dst->single_monster_status,
               src->single_monster_status,
               sizeof(dst->single_monster_status));
        ++profile->csbwin_runtime_item16_count;
        ++imported;
    }

    return imported;
}

static uint16_t csb_v1_runtime_csbwin_item16_group_thing(uint16_t monster_index)
{
    if (((monster_index >> 10) & 0x0Fu) == 4u) {
        return monster_index;
    }
    return (uint16_t)((4u << 10) | (monster_index & 0x03FFu));
}

static int csb_v1_runtime_has_c37_for_square(
    const CSB_V1_RuntimeProfile *profile,
    int map_index,
    int map_x,
    int map_y)
{
    int i;

    if (!profile || map_index < 0 || map_x < 0 || map_y < 0) return 0;
    for (i = 0; i < profile->timeline_queue.eventCount; ++i) {
        int event_index = profile->timeline_queue.timeline[i];
        const struct DM1_Event_V1 *event =
            NULL;
        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
            continue;
        }
        event = &profile->timeline_queue.events[event_index];
        if (event->type == DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
            DM1_MAP_TIME_MAP(event->map_time) == (uint8_t)map_index &&
            event->b_mapX == (uint8_t)map_x &&
            event->b_mapY == (uint8_t)map_y) {
            return 1;
        }
    }
    return 0;
}

int csb_v1_runtime_claim_csbwin_item16_ai_ownership(
    CSB_V1_RuntimeProfile *profile)
{
    uint16_t item_index;
    int claimed = 0;

    if (!profile || !profile->dungeon_handle) return 0;

    for (item_index = 0u;
         item_index < profile->csbwin_runtime_item16_count;
         ++item_index) {
        CSB_V1_CSBWinRuntimeItem16 *item =
            &profile->csbwin_runtime_item16[item_index];
        uint16_t group_thing;
        int map_index = -1;
        int map_x = -1;
        int map_y = -1;
        uint8_t *group_record;
        int thing_type;
        int thing_size;
        int creature_type = 0;

        if (!item->valid || item->live_ai_owned) {
            continue;
        }

        group_thing =
            csb_v1_runtime_csbwin_item16_group_thing(item->monster_index);
        if (!csb_v1_runtime_find_group_thing_location(
                profile->dungeon_handle,
                group_thing,
                &map_index,
                &map_x,
                &map_y)) {
            continue;
        }

        item->live_ai_owned = 1;
        item->live_ai_group_thing = group_thing;
        item->live_ai_map_index = map_index;
        item->live_ai_map_x = map_x;
        item->live_ai_map_y = map_y;
        group_record = csb_v1_runtime_mutable_thing_record(
            profile->dungeon_handle,
            group_thing,
            &thing_type,
            &thing_size);
        if (group_record && thing_type == 4 && thing_size > 4) {
            creature_type = (int)group_record[4];
        }
        if (!csb_v1_runtime_has_c37_for_square(
                profile,
                map_index,
                map_x,
                map_y)) {
            csb_v1_runtime_schedule_c37_group_event(
                profile,
                map_index,
                map_x,
                map_y,
                creature_type,
                1u);
            item->live_ai_c37_queued = 1;
        }
        ++claimed;
    }

    /* CSBWin CSB.h ITEM16 stores active-monster records keyed by the DB4
     * monster/group index, while ReDMCSB GROUP.C F0209 resumes live monster
     * behavior through C37 square events.  This bridge claims each decoded
     * ITEM16 whose C04 group thing is still present in the loaded dungeon and
     * ensures there is a C37 owner tick unless the imported timer queue
     * already supplied one for that square. */
    return claimed;
}

int csb_v1_runtime_materialize_csbwin_timer_queue(
    CSB_V1_RuntimeProfile *profile)
{
    uint16_t queue_index;
    int imported = 0;

    if (!profile || !profile->csbwin_body_runtime_summary_valid) {
        return -1;
    }
    if (profile->csbwin_timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES ||
        profile->csbwin_timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES) {
        return -1;
    }

    /* CSBWin Timer.cpp:728-772 orders timers by full m_time, then
     * timerFunction, then m_timerUByte5, then m_timerSequence when enabled.
     * The decoded CSBWin timer queue already captures the source order; this
     * handoff rebuilds Firestaff's timeline heap from that queue, preserving
     * m_time as Map_Time and m_timerUByte5 as the Type_Priority priority byte.
     * Unsupported side effects remain harmless dispatch records until their
     * runtime handlers are implemented. */
    dm1v1_event_queue_init(&profile->timeline_queue, profile->game_time);
    for (queue_index = 0u;
         queue_index < profile->csbwin_timer_queue_summary_count;
         ++queue_index) {
        uint16_t timer_index = profile->csbwin_timer_queue[queue_index];
        const CSB_V1_CSBWin512TimerSummary *timer;
        struct DM1_Event_V1 event;

        if (timer_index >= profile->csbwin_timer_summary_count) {
            continue;
        }
        timer = &profile->csbwin_timers[timer_index];
        if (!timer->valid || timer->function == DM1_EVENT_NONE) {
            continue;
        }

        memset(&event, 0, sizeof(event));
        event.map_time = timer->time;
        event.type = timer->function;
        event.priority = timer->ubyte5;
        event.b_mapX = timer->ubyte6;
        event.b_mapY = timer->ubyte7;
        event.c_cell = timer->ubyte8;
        event.c_effect = timer->ubyte9;
        if (dm1v1_event_add(&profile->timeline_queue, &event) >= 0) {
            ++imported;
        }
    }
    return imported;
}

int csb_v1_runtime_apply_csbwin_resume_report(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary)
{
    if (!profile || !summary || !summary->header_valid ||
        summary->sections_verified < CSB_V1_CSBWIN_512_SECTION_COUNT ||
        summary->num_character > CSB_V1_MAX_CHAMPIONS ||
        summary->party_x > CSB_V1_MAX_PARTY_X ||
        summary->party_y > CSB_V1_MAX_PARTY_Y ||
        summary->party_facing > 3u ||
        summary->item16_summary_count >
            CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES ||
        summary->timer_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        summary->timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES) {
        return -1;
    }

    /* CSBWin SaveGame.cpp:1768-1855 loads GAMEBLOCK2, ITEM16,
     * character data, timers, then the timer queue. Firestaff keeps the
     * lower-level handoff helpers testable, but startup/resume callers use
     * this ordered boundary so a verified CSBWin body cannot be applied as a
     * half-imported runtime state. */
    if (csb_v1_runtime_apply_csbwin_gameblock2_summary(
            profile, summary) != 0) {
        return -1;
    }
    if (csb_v1_runtime_apply_csbwin_champion_summaries(
            profile, summary) != 0) {
        return -1;
    }
    if (csb_v1_runtime_apply_csbwin_body_runtime_summaries(
            profile, summary) != 0) {
        return -1;
    }
    if (csb_v1_runtime_materialize_csbwin_item16_summaries(profile) < 0) {
        return -1;
    }
    if (csb_v1_runtime_materialize_csbwin_timer_queue(profile) < 0) {
        return -1;
    }
    (void)csb_v1_runtime_claim_csbwin_item16_ai_ownership(profile);
    return 0;
}

int csb_v1_runtime_apply_csbwin_resume_file(
    CSB_V1_RuntimeProfile *profile,
    const char *path,
    size_t max_size)
{
    enum { DEFAULT_MAX_BYTES = 4 * 1024 * 1024 };
    FILE *fp;
    long file_size_long;
    size_t file_size;
    uint8_t *bytes;
    size_t got;
    int rc;
    CSB_V1_CSBWin512BodyReport report;

    if (!profile || !path || path[0] == '\0') {
        return -1;
    }
    if (max_size == 0u) {
        max_size = (size_t)DEFAULT_MAX_BYTES;
    }

    fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    file_size_long = ftell(fp);
    if (file_size_long < 0) {
        fclose(fp);
        return -1;
    }
    file_size = (size_t)file_size_long;
    if (file_size > max_size) {
        fclose(fp);
        return -1;
    }
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    bytes = (uint8_t *)malloc(file_size > 0u ? file_size : 1u);
    if (!bytes) {
        fclose(fp);
        return -1;
    }
    got = fread(bytes, 1u, file_size, fp);
    fclose(fp);
    if (got != file_size) {
        free(bytes);
        return -1;
    }

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbwin_512_verify_save_body(bytes, file_size, 0u, &report);
    free(bytes);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return -1;
    }
    return csb_v1_runtime_apply_csbwin_resume_report(profile, &report);
}

static int csb_v1_runtime_build_csbwin_core_summary(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWin512BodyReport *summary)
{
    int rc;
    uint16_t i;
    uint16_t timer_count;
    uint16_t item_count;

    if (!profile || !summary || !profile->party_state_valid) {
        return -1;
    }

    rc = csb_v1_runtime_export_csbwin_champion_summaries(profile, summary);
    if (rc < 0) {
        return -1;
    }

    summary->header_valid = 1;
    summary->header.verdict = CSB_V1_CSBWIN_512_VERDICT_CSB;
    summary->header.key_index = CSB_V1_CSBWIN_512_KEY_CSB;
    summary->timer_record_size = 16u;
    summary->header.public_fields.csbwin_random_game_id =
        (uint32_t)csb_v1_runtime_effective_game_id(profile);
    summary->header.public_fields.csbwin_total_move_count =
        profile->tick_count;
    if (profile->csbwin_header_tail_valid) {
        memcpy(summary->header.public_fields.csbwin_byte22808,
               profile->csbwin_header_byte22808,
               sizeof(summary->header.public_fields.csbwin_byte22808));
    }
    if (profile->csbwin_appended_tail_valid) {
        if (profile->csbwin_appended_tail_preserved_size >
                CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES ||
            profile->csbwin_appended_tail_size !=
                profile->csbwin_appended_tail_preserved_size) {
            return -1;
        }
        summary->appended_offset = 0u;
        summary->appended_size = profile->csbwin_appended_tail_size;
        summary->appended_preserved_size =
            profile->csbwin_appended_tail_preserved_size;
        summary->appended_fnv1a = profile->csbwin_appended_tail_fnv1a;
        summary->appended_truncated =
            profile->csbwin_appended_tail_truncated;
        memcpy(summary->appended_preserved,
               profile->csbwin_appended_tail,
               summary->appended_preserved_size);
    }
    summary->game_time = profile->game_time;
    summary->random_seed = profile->csbwin_gameblock2_summary_valid
        ? profile->csbwin_random_seed
        : profile->dungeon_seed;
    summary->object_in_hand = csb_v1_runtime_export_leader_hand_thing(profile);
    summary->last_monster_attack_time =
        profile->csbwin_last_monster_attack_time;
    summary->last_party_move_time = profile->csbwin_last_party_move_time;
    summary->party_move_disable_timer =
        profile->csbwin_party_move_disable_timer;
    summary->word11712 = profile->csbwin_word11712;
    summary->word11714 = profile->csbwin_word11714;

    if (profile->csbwin_body_runtime_summary_valid) {
        summary->character_tail_brightness =
            profile->csbwin_character_tail_brightness;
        summary->character_tail_see_thru_walls =
            profile->csbwin_character_tail_see_thru_walls;
        summary->character_tail_magic_footprints_active =
            profile->csbwin_character_tail_magic_footprints_active;
        summary->character_tail_party_shield =
            profile->csbwin_character_tail_party_shield;
        summary->character_tail_fire_shield =
            profile->csbwin_character_tail_fire_shield;
        summary->character_tail_spell_shield =
            profile->csbwin_character_tail_spell_shield;
        summary->character_tail_num_footprint_entries =
            profile->csbwin_character_tail_num_footprint_entries;
        summary->character_tail_freeze_life_timer =
            profile->csbwin_character_tail_freeze_life_timer;
        summary->character_tail_first_magic_footprint =
            profile->csbwin_character_tail_first_magic_footprint;
        summary->character_tail_last_magic_footprint =
            profile->csbwin_character_tail_last_magic_footprint;
        memcpy(summary->character_tail_party_footprints,
               profile->csbwin_character_tail_party_footprints,
               sizeof(summary->character_tail_party_footprints));
        memcpy(summary->character_tail_byte13220,
               profile->csbwin_character_tail_byte13220,
               sizeof(summary->character_tail_byte13220));
        summary->character_tail_invisible =
            profile->csbwin_character_tail_invisible;
    }

    item_count = profile->csbwin_body_runtime_summary_valid
        ? profile->csbwin_item16_summary_total
        : profile->csbwin_runtime_item16_total;
    if (item_count > CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES) {
        return -1;
    }
    summary->max_item16 = item_count;
    summary->item16_queue_len = item_count;
    summary->item16_summary_total = item_count;
    summary->item16_summary_count = item_count;
    if (profile->csbwin_body_runtime_summary_valid) {
        memcpy(summary->item16,
               profile->csbwin_item16,
               (size_t)item_count * sizeof(summary->item16[0]));
    } else {
        for (i = 0u; i < item_count; ++i) {
            const CSB_V1_CSBWinRuntimeItem16 *src =
                &profile->csbwin_runtime_item16[i];
            CSB_V1_CSBWin512Item16Summary *dst = &summary->item16[i];
            if (!src->valid) {
                continue;
            }
            dst->valid = 1;
            dst->monster_index = src->monster_index;
            dst->facings = src->facings;
            dst->positions = src->positions;
            dst->ubyte4 = src->last_move_time_lsb;
            dst->ubyte5 = src->delay_or_flee_timer;
            dst->target_x = src->target_x;
            dst->target_y = src->target_y;
            dst->previous_x = src->previous_x;
            dst->previous_y = src->previous_y;
            dst->current_x = src->current_x;
            dst->current_y = src->current_y;
            memcpy(dst->single_monster_status,
                   src->single_monster_status,
                   sizeof(dst->single_monster_status));
        }
    }

    if (profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount >
            (int)CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        profile->timeline_queue.eventCount >
            (int)CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES) {
        return -1;
    }
    timer_count = (uint16_t)profile->timeline_queue.eventCount;
    summary->max_timers = timer_count;
    summary->num_timer = timer_count;
    summary->first_avail_timer = timer_count;
    summary->timer_sequence = profile->csbwin_timer_sequence;
    summary->timer_summary_total = timer_count;
    summary->timer_summary_count = timer_count;
    summary->timer_queue_summary_total = timer_count;
    summary->timer_queue_summary_count = timer_count;

    /* CSBWin SaveGame.cpp writes the TIMER array and then the timer queue
     * after GAMEBLOCK2/ITEM16/CHARDESC. For Firestaff's bounded core export,
     * write the current runtime timeline heap order as the CSBWin queue so a
     * re-import reaches the same event boundary without relying on stale
     * imported timer bytes. */
    for (i = 0u; i < timer_count; ++i) {
        int event_index = profile->timeline_queue.timeline[i];
        const struct DM1_Event_V1 *event;
        CSB_V1_CSBWin512TimerSummary *timer = &summary->timers[i];
        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
            return -1;
        }
        event = &profile->timeline_queue.events[event_index];
        timer->valid = 1;
        timer->time = event->map_time;
        timer->function = (uint8_t)event->type;
        timer->ubyte5 = event->priority;
        timer->ubyte6 = event->b_mapX;
        timer->ubyte7 = event->b_mapY;
        timer->ubyte8 = event->c_cell;
        timer->ubyte9 = event->c_effect;
        timer->sequence =
            (uint16_t)(profile->csbwin_timer_sequence + i);
        timer->level = (uint8_t)DM1_MAP_TIME_MAP(event->map_time);
        summary->timer_queue[i] = i;
    }

    summary->sections_verified = CSB_V1_CSBWIN_512_SECTION_COUNT;
    return 0;
}

static int csb_v1_runtime_locate_appended_expool_record_internal(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size)
{
    CSB_V1_CSBWin512BodyReport report;
    const uint8_t *report_bytes = NULL;
    size_t report_size = 0u;
    size_t offset;

    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!profile || !profile->csbwin_appended_tail_valid ||
        profile->csbwin_appended_tail_truncated ||
        profile->csbwin_appended_tail_size == 0u ||
        profile->csbwin_appended_tail_size !=
            profile->csbwin_appended_tail_preserved_size ||
        profile->csbwin_appended_tail_preserved_size >
            CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES) {
        return 0;
    }

    memset(&report, 0, sizeof(report));
    report.appended_size = profile->csbwin_appended_tail_size;
    report.appended_preserved_size =
        profile->csbwin_appended_tail_preserved_size;
    report.appended_fnv1a = profile->csbwin_appended_tail_fnv1a;
    report.appended_truncated = profile->csbwin_appended_tail_truncated;
    if ((report.appended_size % CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES) == 0u) {
        report.appended_expool_candidate = 1;
        report.appended_expool_block_count =
            (uint16_t)(report.appended_size /
                       CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES);
    }
    memcpy(report.appended_preserved,
           profile->csbwin_appended_tail,
           report.appended_preserved_size);

    /* CSBWin data.cpp EXPOOL::Locate returns a pointer into the DB11 block.
     * The lower-level helper works on a verifier report, so translate the
     * found offset back into the runtime profile's preserved tail storage. */
    if (!csb_v1_csbwin_512_appended_expool_locate_record(
            &report, record_id, &report_bytes, &report_size) ||
        !report_bytes ||
        report_bytes < report.appended_preserved) {
        return 0;
    }
    offset = (size_t)(report_bytes - report.appended_preserved);
    if (offset > profile->csbwin_appended_tail_preserved_size ||
        report_size >
            profile->csbwin_appended_tail_preserved_size - offset) {
        return 0;
    }
    if (out_bytes) *out_bytes = profile->csbwin_appended_tail + offset;
    if (out_size) *out_size = report_size;
    return 1;
}

int csb_v1_runtime_locate_csbwin_appended_expool_record(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size)
{
    return csb_v1_runtime_locate_appended_expool_record_internal(
        profile,
        record_id,
        out_bytes,
        out_size);
}

int csb_v1_runtime_export_csbwin_core_save_to_memory(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_size)
{
    CSB_V1_CSBWin512BodyReport summary;

    if (!profile || !out || !out_size) {
        return -1;
    }
    *out_size = 0u;
    memset(&summary, 0, sizeof(summary));
    if (csb_v1_runtime_build_csbwin_core_summary(profile, &summary) != 0) {
        return -1;
    }
    return csb_v1_csbwin_512_build_writable_core_save(
        &summary,
        out,
        out_capacity,
        out_size) == CSB_V1_CSBWIN_512_OK ? 0 : -1;
}

int csb_v1_runtime_export_csbwin_core_save_to_path(
    const CSB_V1_RuntimeProfile *profile,
    const char *path)
{
    uint8_t bytes[CSB_V1_CSBWIN_BLOCK1_BYTES + 128u +
                  CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES * 16u +
                  3328u +
                  CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES * 16u +
                  CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES * 2u +
                  CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES];
    size_t size = 0u;
    FILE *fp;
    size_t wrote;

    if (!profile || !path || path[0] == '\0') {
        return -1;
    }
    if (csb_v1_runtime_export_csbwin_core_save_to_memory(
            profile, bytes, sizeof(bytes), &size) != 0) {
        return -1;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return -1;
    }
    wrote = fwrite(bytes, 1u, size, fp);
    if (fclose(fp) != 0) {
        return -1;
    }
    return wrote == size ? 0 : -1;
}

int csb_v1_runtime_set_leader(CSB_V1_RuntimeProfile *profile,
                              int champion_index)
{
    CSB_V1_Champion *champion;
    if (!profile || !profile->party_state_valid) return -1;
    if (champion_index == profile->leader_index) return 0;
    if (champion_index < 0) {
        profile->leader_index = -1;
        profile->party_state.LeaderIndex = -1;
        return 0;
    }
    if (champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    champion = &profile->party_state.Champions[champion_index];
    /* Mirrors the source-locked F0368 guard and selection side effect:
     * ignore dead/empty champions, then write G0411_i_LeaderIndex and align
     * the selected champion direction to G0308_i_PartyDirection.
     * Source: ReDMCSB CLIKCHAM.C F0368_COMMAND_SetLeader lines 51-68. */
    if (csb_v1_champion_is_dead(champion) || champion->CurrentHealth <= 0) {
        return -1;
    }
    profile->leader_index = champion_index;
    profile->party_state.LeaderIndex = champion_index;
    champion->Direction = (uint8_t)(profile->party_dir & 3);
    return 0;
}

int csb_v1_runtime_select_champion_portrait_render_source(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    CSB_V1_ChampionPortraitRenderSource *out_source)
{
    const CSB_V1_Champion *champion;
    if (out_source) {
        memset(out_source, 0, sizeof(*out_source));
        out_source->champion_index = -1;
    }
    if (!profile || !out_source || !profile->party_state_valid) return -1;
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    champion = &profile->party_state.Champions[champion_index];
    /* ReDMCSB: PANEL.C F0354 lines 2195-2239 draws the status-box portrait
     * directly from M516_CHAMPIONS[ChampionIndex].Portrait.  Firestaff keeps
     * the same ownership boundary here: Utility Disk/CMP import may populate
     * CSB_V1_Champion.Portrait, and the renderer receives only a source view
     * into the runtime-owned champion snapshot. */
    out_source->portrait = champion->Portrait;
    out_source->portrait_byte_count = CSB_V1_PORTRAIT_BYTE_COUNT;
    out_source->portrait_width = CSB_V1_PORTRAIT_WIDTH;
    out_source->portrait_height = CSB_V1_PORTRAIT_HEIGHT;
    out_source->portrait_byte_width = CSB_V1_PORTRAIT_BYTE_WIDTH;
    out_source->champion_index = champion_index;
    out_source->is_leader = (champion_index == profile->leader_index);
    out_source->name = champion->Name;
    out_source->title = champion->Title;
    return 0;
}

int csb_v1_runtime_rotate_party(CSB_V1_RuntimeProfile *profile,
                                 int target_dir)
{
    /* Source: ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection lines
     * 117-130.  The PC 3.4 C version (MEDIA182) computes
     *   delta = (P0600_i_Direction - G0308_i_PartyDirection); if delta<0
     *   then delta += 4;
     * then loops over G0305_ui_PartyChampionCount champions, applying
     *   Champion.Cell      = (Champion.Cell + delta) & 3;
     *   Champion.Direction = (Champion.Direction + delta) & 3;
     * and finally writes G0308_i_PartyDirection = P0600_i_Direction.
     * The M021_NORMALIZE() macro is just (x & 3). */
    int delta;
    int current_dir;
    int i;

    if (!profile) return -1;
    if (!profile->party_state_valid) return -1;
    if (target_dir < 0 || target_dir > 3) return -1;

    current_dir = profile->party_dir & 3;
    if (target_dir == current_dir) {
        /* Source-locked F0284 early return.  No champion state is
         * touched and the caller still gets 0. */
        return 0;
    }

    delta = target_dir - current_dir;
    if (delta < 0) delta += 4;

    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; i++) {
        uint8_t *cell = &profile->party_state.Champions[i].Cell;
        uint8_t *dir = &profile->party_state.Champions[i].Direction;
        *cell = (uint8_t)(((int)*cell + delta) & 3);
        *dir  = (uint8_t)(((int)*dir  + delta) & 3);
    }

    profile->party_dir = (uint8_t)target_dir;
    profile->party_state.PartyDirection = (uint8_t)target_dir;
    return 0;
}

int csb_v1_runtime_process_input_queue(
    CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction,
    CSB_V1_InputCommandRuntimeResult *out_result)
{
    CSB_V1_InputCommandRuntimeResult local_result;
    int target_dir;

    if (!profile || !queue) return -1;
    memset(&local_result, 0, sizeof(local_result));

    local_result.old_party_x = profile->party_x;
    local_result.old_party_y = profile->party_y;
    local_result.old_party_dir = profile->party_dir & 3;
    local_result.new_party_x = profile->party_x;
    local_result.new_party_y = profile->party_y;
    local_result.new_party_dir = profile->party_dir & 3;
    local_result.old_party_level = profile->current_level;
    local_result.new_party_level = profile->current_level;

    /* Source: ReDMCSB COMMAND.C F0380 lines 2045-2156 owns the command
     * queue dequeue/gate/dispatch boundary.  The shared V1 queue helper
     * preserves that C001/C002 vs C003-C006 split before this CSB runtime
     * adapter applies only the state transition that has a CSB profile
     * boundary today. */
    local_result.queue_result =
        DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
            queue,
            profile->party_dir,
            disabled_movement_ticks,
            projectile_disabled_movement_ticks,
            last_projectile_disabled_movement_direction);

    if (!local_result.queue_result.dequeued) {
        if (out_result) *out_result = local_result;
        return 0;
    }

    profile->last_input_dispatch = local_result.queue_result;
    profile->input_dispatch_count++;

    switch (local_result.queue_result.command) {
    case DM1_V1_COMMAND_TURN_LEFT:
        if (csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL)) {
            csb_v1_runtime_take_current_stairs(profile, &local_result);
            csb_v1_runtime_mark_deferred_new_party_map_index(&local_result);
            csb_v1_runtime_apply_party_floor_sensor_consequences(
                profile,
                &local_result);
            break;
        }
        target_dir = (profile->party_dir + 3) & 3;
        local_result.sensor_source_remove_checked = 1;
        csb_v1_runtime_process_party_floor_sensors_at_level(
            profile,
            local_result.old_party_level,
            local_result.old_party_x,
            local_result.old_party_y,
            0,
            &local_result);
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        csb_v1_runtime_apply_party_turn_floor_sensor_add_consequences(
            profile,
            &local_result);
        break;
    case DM1_V1_COMMAND_TURN_RIGHT:
        if (csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL)) {
            csb_v1_runtime_take_current_stairs(profile, &local_result);
            csb_v1_runtime_mark_deferred_new_party_map_index(&local_result);
            csb_v1_runtime_apply_party_floor_sensor_consequences(
                profile,
                &local_result);
            break;
        }
        target_dir = (profile->party_dir + 1) & 3;
        local_result.sensor_source_remove_checked = 1;
        csb_v1_runtime_process_party_floor_sensors_at_level(
            profile,
            local_result.old_party_level,
            local_result.old_party_x,
            local_result.old_party_y,
            0,
            &local_result);
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        csb_v1_runtime_apply_party_turn_floor_sensor_add_consequences(
            profile,
            &local_result);
        break;
    case DM1_V1_COMMAND_MOVE_FORWARD:
    case DM1_V1_COMMAND_MOVE_RIGHT:
    case DM1_V1_COMMAND_MOVE_BACKWARD:
    case DM1_V1_COMMAND_MOVE_LEFT:
        {
            CSB_V1_MovementCommandStepRuntimeResultPc34Compat step_result;
            int step_status =
                (local_result.queue_result.command ==
                     DM1_V1_COMMAND_MOVE_BACKWARD &&
                 csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL))
                    ? 0
                    :
                csb_v1_movement_command_step_runtime_apply_pc34_compat(
                    profile,
                    local_result.queue_result.command,
                    csb_v1_runtime_default_wall_probe,
                    NULL,
                    &step_result);
            if (step_status < 0) {
                return -1;
            }
            if (local_result.queue_result.command ==
                    DM1_V1_COMMAND_MOVE_BACKWARD &&
                csb_v1_runtime_current_square_is_stairs(profile, NULL, NULL)) {
                local_result.movement_command_handled = 1;
                local_result.movement_step_attempted = 1;
                local_result.disabled_movement_ticks_after = 1;
                csb_v1_runtime_take_current_stairs(profile, &local_result);
            } else {
                local_result.unsupported_runtime_command =
                    step_result.unsupported_runtime_command;
                local_result.movement_command_handled =
                    step_result.command_handled;
                local_result.movement_step_attempted =
                    step_result.step_attempted;
                local_result.movement_step_applied = step_result.step_applied;
                local_result.movement_blocked_by_wall =
                    step_result.blocked_by_wall;
                local_result.movement_destination_x =
                    step_result.destination_x;
                local_result.movement_destination_y =
                    step_result.destination_y;
                local_result.disabled_movement_ticks_after =
                    step_result.disabled_movement_ticks_after;
                csb_v1_runtime_sample_destination_square(profile, &local_result);
                if (local_result.movement_blocked_by_wall) {
                    if (local_result.movement_destination_square_type == 4) {
                        local_result.movement_blocked_by_door = 1;
                    } else if (local_result.movement_destination_square_type == 6) {
                        local_result.movement_blocked_by_fakewall = 1;
                    }
                }
                csb_v1_runtime_apply_destination_chain(profile, &local_result);
                csb_v1_runtime_apply_destination_stairs(profile, &local_result);
            }
            csb_v1_runtime_mark_deferred_new_party_map_index(&local_result);
            csb_v1_runtime_apply_party_floor_sensor_consequences(
                profile,
                &local_result);
        }
        break;
    default:
        local_result.unsupported_runtime_command = 1;
        break;
    }

    local_result.new_party_x = profile->party_x;
    local_result.new_party_y = profile->party_y;
    local_result.new_party_dir = profile->party_dir & 3;
    local_result.runtime_state_changed =
        (local_result.old_party_x != local_result.new_party_x) ||
        (local_result.old_party_y != local_result.new_party_y) ||
        (local_result.old_party_dir != local_result.new_party_dir) ||
        (local_result.teleporter_transition_applied != 0) ||
        (local_result.stair_transition_applied != 0) ||
        (local_result.pit_fall_applied != 0) ||
        (local_result.sensor_event_count != 0);

    if (out_result) *out_result = local_result;
    return 1;
}

void csb_v1_runtime_cleanup(CSB_V1_RuntimeProfile *profile) {
    if (!profile) return;
    /*
     * Unload the dungeon loaded by csb_v1_runtime_boot().
     * csb_v1_dungeon_unload() frees the dungeon data (raw_data,
     * dsa_offsets) via csb_v1_dungeon_free() and clears s_current_dungeon,
     * resetting dungeon-layer accessors to ENDOF.
     *
     * FIX (pass608): dungeon is now heap-allocated in csb_v1_runtime_boot()
     * and profile->dungeon_handle owns the allocation.  After calling
     * csb_v1_dungeon_unload() we also free(profile->dungeon_handle) to
     * release the heap struct and NULL the pointer.
     *
     * csb_v1_dungeon_free() does NOT free the struct itself (only inner
     * pointers), so free(dungeon_handle) is safe after csb_v1_dungeon_unload().
     */
    csb_v1_dungeon_unload();
    if (profile->dungeon_handle) {
        free(profile->dungeon_handle);
        profile->dungeon_handle = NULL;
    }
    csb_v1_skin_cache_cleanup(&profile->skin_cache);
}


int csb_v1_runtime_boot(CSB_V1_RuntimeProfile *profile,
                          const char *data_dir,
                          const char *version_hint)
{
    CSB_V1_AssetResult dun_result;
    CSB_V1_AssetResult gfx_result;
    const char *dun_path;
    const char *gfx_path;
    const char *search_dir;

    if (!profile) return -1;

    search_dir = data_dir ? data_dir : ".",

    /* Step 1: Find dungeon by CSB hash (ReDMCSB DUNGEON.C F0237) */
    dun_path = csb_v1_runtime_find_dungeon(search_dir, &dun_result);
    if (!dun_path) return -1;
    profile->dungeon_path = dun_path;
    profile->dungeon_asset = dun_result;

    /* Step 1b: Load the dungeon data (CSB V1 Phase 2 — real asset ingestion)
     * Uses csb_v1_dungeon_load_from_file() to read the actual DUNGEON.DAT
     * into the current dungeon context.  Dungeon-layer accessors in
     * csb_v1_dungeon_world_pc34_compat.h become live after this.
     *
     * FIX (pass608): dungeon MUST be heap-allocated.  The previous
     * implementation created a stack-local CSB_V1_DungeonData and passed
     * &dungeon to csb_v1_dungeon_set_current().  After boot() returns,
     * the stack variable goes out of scope and s_current_dungeon becomes
     * a dangling pointer.  This is a critical memory safety bug.
     *
     * The dungeon_handle field in CSB_V1_RuntimeProfile owns the heap
     * allocation.  csb_v1_dungeon_set_current() transfers inner-data
     * ownership (raw_data/dsa_offsets) but the struct itself is freed
     * by the profile in csb_v1_runtime_cleanup().
     *
     * Source: CSBWin/CSBCode.cpp LoadDungeon lines 6800-6950 */
    {
        /* Heap-allocate to avoid dangling pointer in s_current_dungeon */
        CSB_V1_DungeonData *dungeon = calloc(1, sizeof(CSB_V1_DungeonData));
        if (!dungeon) {
            /* Fall through — dungeon-layer accessors return ENDOF */
        } else if (csb_v1_dungeon_load_from_file(dungeon, dun_path) == 0) {
            profile->dungeon_handle = dungeon;
            csb_v1_dungeon_set_current(dungeon); /* singleton now points to heap */
            csb_v1_dungeon_set_current_level(0);   /* start at level 0 */
        } else {
            free(dungeon);
            profile->dungeon_handle = NULL;
        }
        /* If load fails (corrupt/missing file), boot continues without dungeon.
         * Dungeon-layer accessors will return ENDOF until a dungeon is loaded. */
    }

    /* Step 2: Find graphics (ReDMCSB DISK.C / CSBWin AssetCache) */
    gfx_path = csb_v1_runtime_find_graphics(search_dir, version_hint, &gfx_result);
    profile->graphics_path = gfx_path ? gfx_path : "";
    profile->graphics_asset = gfx_result;

    /* Step 3: Detect variant from asset hashes */
    profile->variant_id = csb_v1_runtime_detect_variant(
        gfx_path, dun_path, NULL, NULL);

    /* Step 4: Initialize Chaos Magic spell grid (ReDMCSB CASTER.C F0211) */
    profile->chaos_magic.magic_initialized = 1;
    profile->chaos_magic.spell_grid_version = 0;
    profile->chaos_magic.chaos_level = 0;

    /* Step 5: Set initial state to TITLE (ReDMCSB ENTRANCE.C G0298) */
    profile->state = CSB_STATE_TITLE;

    return 0;
}

void csb_v1_runtime_tick(CSB_V1_RuntimeProfile *profile, uint32_t dt_ms)
{
    if (!profile || profile->paused) return;
    if (profile->game_over || profile->victory) return;

    profile->total_play_ms += dt_ms;

    /* The original runtime gates event expiry against G0313_ul_GameTime,
     * not against a single frame delta.  Accumulate wall time first, then
     * fire every due 55ms quantum so common frame slices such as 16+16+23ms
     * still produce one V1 tick.
     * Source: ReDMCSB TIMELINE.C F0235 lines 702-708
     * Source: ReDMCSB COMMAND.C F0380 lines 2383-2429
     * (C147/C148 toggle G0301_B_GameTimeTicking). */
    while (csb_v1_runtime_tick_due(profile, 0U)) {
        csb_v1_fire_tick(profile);
    }
}

int csb_v1_runtime_tick_v1(CSB_V1_RuntimeProfile *profile)
{
    if (!profile || profile->paused) return 0;
    if (profile->game_over || profile->victory) return 0;

    profile->total_play_ms += CSB_V1_TICK_MS_NOMINAL;
    csb_v1_fire_tick(profile);
    return 1;
}

int csb_v1_runtime_tick_due(const CSB_V1_RuntimeProfile *profile, uint32_t now_ms)
{
    uint64_t wall_ms;
    uint64_t game_ticks_now;
    if (!profile) return 0;

    wall_ms = (now_ms != 0U) ? (uint64_t)now_ms : profile->total_play_ms;
    game_ticks_now = wall_ms / CSB_V1_TICK_MS_NOMINAL;
    return (profile->tick_count < game_ticks_now) ? 1 : 0;
}

/* ── Source evidence ────────────────────────────────────────────────── */

const char *csb_v1_runtime_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C: F0806_F0806_ENTRANCE_int (game boot)\n"
        "ReDMCSB ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)\n"
        "ReDMCSB ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics)\n"
        "ReDMCSB ENTRANCE.C: F0580_ENTRANCE_DrawDoorAnimationStep\n"
        "ReDMCSB ENTRANCE.C: F0581_ENTRANCE_BlitDoors\n"
        "ReDMCSB ENTRANCE.C: C28_ENTRANCE_CSB palette index\n"
        "ReDMCSB ENTRANCE.C: G0298_B_NewGame state machine control\n"
        "ReDMCSB ENTRANCE.C: G0309_i_PartyMapIndex init (party start)\n"
        "ReDMCSB ENTRANCE.C: MEDIA529_F20E_F20J save path decision\n"
        "ReDMCSB ENTRANCE.C: M567_COMMAND_ENTRANCE_DRAW_CREDITS\n"
        "ReDMCSB SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful\n"
        "ReDMCSB SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful\n"
        "ReDMCSB LOADSAVE.C: F0435_STARTEND_LoadGame\n"
        "ReDMCSB LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame\n"
        "ReDMCSB DUNGEON.C: F0237_DUNGEON_DungeonLoad (hash-gated load)\n"
        "ReDMCSB CASTER.C: F0211_CASTER_ClearSpellEffects (spell grid boot)\n"
        "ReDMCSB CASTER.C: F0213 per-square invocation slots\n"
        "ReDMCSB BugsAndChanges.htm: CHANGE7_29 (new CSBGAME.DAT header)\n"
        "ReDMCSB CEDTINC7.C: G3764_THAT_S_THE_CSB_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTIL_DISK\n"
        "ReDMCSB CEDTINC8.C: G3921/G3755/G3764 utility disk strings\n"
        "ReDMCSB F0417: F0417_SAVEUTIL_GetChecksumAndObfuscate\n"
        "ReDMCSB COMPILE.H MEDIA332 (S20E/S21E Atari ST 2.0/2.1)\n"
        "ReDMCSB COMPILE.H MEDIA529 (A35E/A35M Amiga 3.5)\n"
        "ReDMCSB COMPILE.H MEDIA278 (P20JA/P20JB PC DOS 3.4)\n"
        "ReDMCSB COMPILE.H MEDIA278_I34E_I34M (PC DOS multilanguage)\n"
        "CSBWin SaveGame.cpp: LoadGame() / SaveGame() (2953 lines)\n"
        "CSBWin Character.cpp: Character::import_dm1_save()\n"
        "CSBWin Magic.cpp: ChaosMagic namespace\n"
        "CSBWin AssetCache: variant_id mapping for all platforms\n"
        "asset_status_m12.c: g_csbVersions[] MD5 table (all 4 variants)\n"
        "asset_find_by_hash.c: hash-based asset discovery API\n"
        "\n"
        "CSB vs DM1 runtime differences:\n"
        "  - Dungeon hash: 6695d2acebce49f95db1d8f3a5c733de (CSB)\n"
        "  - Save namespace: csb_save_N.fsav (CSB) vs save_NN.dat (DM1)\n"
        "  - Save header: CSB_MAGIC 0x43534201 (CSB) vs DM_MAGIC 0x444D0001\n"
        "  - Save key index: C29 (CSB) vs C10 (DM1) per MEDIA187/MEDIA332\n"
        "  - Difficulty scale: +25% per champion (CSB) vs DM1 flat\n"
        "  - Chaos Magic: present at CSB boot (F0211)\n"
        "  - Entry: same ENTRANCE, C28_ENTRANCE_CSB palette\n"
        "  - Champion import: F0153 from DM1 save supported at CSB boot\n";
}
