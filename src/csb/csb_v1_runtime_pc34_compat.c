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
#define CSB_V1_RUNTIME_SAVE_VERSION 2u

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
} CSB_V1_RuntimeSaveImageV1;

#define CSB_V1_RUNTIME_SAVE_V1_SIZE \
    ((uint32_t)offsetof(CSB_V1_RuntimeSaveImageV1, projectiles))

static int csb_v1_runtime_first_living_champion(
    const CSB_V1_PartyState *party);
static int csb_v1_runtime_target_champion_for_adjacent_attack(
    const CSB_V1_RuntimeProfile *profile,
    int attacker_x,
    int attacker_y,
    int creature_cell);

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
    if (image->byte_size >= sizeof(*image)) {
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
                                  profile->party_z,
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
     * F0261_TIMELINE_Process_CPSEF() before incrementing
     * G0313_ul_GameTime.  TIMELINE.C F0240 lines 702-708 expires the
     * first heap event when event_time <= G0313_ul_GameTime. */
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

    /* ReDMCSB: CLIKMENU.C F0366 lines ~230-236 reaches C03_ELEMENT_STAIRS
     * and asks DUNGEON.C F0150 for the stair destination; DEFS.H defines
     * MASK0x0004 as the stairs-up flag.  This bounded CSB runtime handoff
     * only mirrors the map-index transition; full stair coordinate pairing,
     * sensors, and timing remain separate movement-consequence work. */
    stair_up = (raw_square & 0x04) ? 1 : 0;
    target_level = level + (stair_up ? -1 : 1);
    result->stair_up = stair_up;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (target_level < 0 || target_level >= dungeon->level_count) return;

    profile->current_level = target_level;
    csb_v1_dungeon_set_current_level(target_level);
    result->new_party_level = target_level;
    result->stair_transition_applied = 1;
}

static void csb_v1_runtime_apply_destination_pit(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    const CSB_V1_DungeonData *dungeon;
    int level;
    int raw_square;
    int target_level;

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

    /* ReDMCSB: MOVESENS.C F0267 lines 538-574 handles an open,
     * non-imaginary C02_ELEMENT_PIT by calling DUNGEON.C F0154 for a
     * downward map transition, then sets the current map and party
     * coordinates.  This bounded CSB runtime slice mirrors only the
     * adjacent level-index fall; full F0154 coordinate pairing, fall damage,
     * sounds, rope, chained pits, and view redraw timing stay separate. */
    target_level = level + 1;
    result->old_party_level = profile->current_level;
    result->new_party_level = profile->current_level;
    if (target_level < 0 || target_level >= dungeon->level_count) return;

    profile->current_level = target_level;
    csb_v1_dungeon_set_current_level(target_level);
    result->new_party_level = target_level;
    result->pit_fall_applied = 1;
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

static int csb_v1_runtime_move_group_thing_to_square(
    CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int level,
    int old_x,
    int old_y,
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

    if (!dungeon || (old_x == new_x && old_y == new_y)) return 0;
    source_first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        old_x,
        old_y);
    dest_first_ptr = csb_v1_runtime_square_first_thing_ptr(
        dungeon,
        level,
        new_x,
        new_y);
    if (!source_first_ptr || !dest_first_ptr) return 0;

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
            csb_v1_runtime_write_u16(
                group_record,
                csb_v1_runtime_read_u16(dest_first_ptr));
            csb_v1_runtime_write_u16(dest_first_ptr, group_thing);
            return 1;
        }
        previous_record = group_record;
        thing = next_thing;
    }
    return 0;
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
    if (distance_x != 0 && distance_y != 0) return;

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
        uint16_t group_thing;

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
            if (behavior == 0 || behavior == 2 || behavior == 3) {
                next_behavior = (distance_x + distance_y <= 1) ? 6 : 7;
                flags = (uint16_t)((flags & 0xFFF0u) |
                                   (uint16_t)(next_behavior & 0x0F));
                csb_v1_runtime_write_u16(thing_record + 14, flags);
                if (next_behavior == 6) {
                    csb_v1_runtime_schedule_c38_attack_events(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        flags);
                }
                if (next_behavior == 7) {
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
                int target_x = record->mapX;
                int target_y = record->mapY;
                int moved = 0;

                movement_ticks = csb_v1_runtime_creature_movement_ticks(
                    (int)thing_record[4]);
                if (distance_x + distance_y <= 1) {
                    flags = (uint16_t)((flags & 0xFFF0u) | 6u);
                    csb_v1_runtime_write_u16(thing_record + 14, flags);
                    csb_v1_runtime_schedule_c38_attack_events(
                        profile,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        (int)thing_record[4],
                        flags);
                    return;
                }
                if (distance_y > 0) {
                    target_y += (profile->party_y > record->mapY) ? 1 : -1;
                } else if (distance_x > 0) {
                    target_x += (profile->party_x > record->mapX) ? 1 : -1;
                }
                if (!csb_v1_runtime_group_destination_is_blocked(
                        dungeon,
                        record->mapIndex,
                        target_x,
                        target_y)) {
                    moved = csb_v1_runtime_move_group_thing_to_square(
                        dungeon,
                        group_thing,
                        record->mapIndex,
                        record->mapX,
                        record->mapY,
                        target_x,
                        target_y);
                }
                /* ReDMCSB GROUP.C F0209 lines 2228-2272 walks C7 approach
                 * toward the target using F0202 movement checks, then lines
                 * 2450-2463 schedule the next C37.  This bounded bridge only
                 * relinks a real-format C04 group when the destination square
                 * already has a thing-list slot; full F0202 occupancy,
                 * active-group side state, sounds, and attack expansion remain
                 * separate work. */
                csb_v1_runtime_schedule_c37_group_event(
                    profile,
                    record->mapIndex,
                    moved ? target_x : record->mapX,
                    moved ? target_y : record->mapY,
                    (int)thing_record[4],
                    (uint32_t)((movement_ticks > 1) ? movement_ticks : 1));
            }
            return;
        }
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
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
        skill_index >= CSB_V1_SKILL_COUNT) {
        return 1;
    }

    /* ReDMCSB PROJEXPL.C F0230 line 1390 subtracts
     * F0303_CHAMPION_GetSkillLevel(C07_SKILL_PARRY) << 1 from creature
     * attack.  CSB's current imported champion block carries the compact
     * 16-byte DM1/CSB skill row rather than Firestaff M10's full 20-skill
     * XP/lifecycle state, so this bounded bridge treats the stored byte as
     * an imported source skill level and clamps it to the F0303 level range.
     * Full CSB skill-XP reconstruction belongs with the original-save body
     * importer, not in the C38 attack dispatcher. */
    level = (int)champion->Skills[skill_index];
    if (level < 1) level = 1;
    if (level > 16) level = 16;
    return level;
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

static void csb_v1_runtime_pack_dead_group_creature(
    CSB_V1_DungeonData *dungeon,
    uint8_t *group_record,
    uint16_t group_thing,
    int level,
    int map_x,
    int map_y,
    int creature_index)
{
    uint16_t flags;
    int raw_count;
    int creature_count;
    int cells;
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

    if (creature_count <= 1) {
        /* ReDMCSB GROUP.C F0190 lines 831-840 calls F0189_GROUP_Delete
         * when the last creature dies.  This bounded CSB bridge removes the
         * C04 thing from the square chain and marks the real-format record
         * unused; fixed possessions, sounds, and active-group side state are
         * later slices. */
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

    cells = group_record[5];
    /* ReDMCSB GROUP.C F0190 lines 892-905 compacts Health, directions,
     * cells, active aspect, then decrements GROUP.Count.  CSB's bounded
     * real-format bridge owns Health and Cells here; directions/aspect event
     * rewriting remain with the wider active-group runtime work. */
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
                        dungeon,
                        thing_record,
                        (uint16_t)thing,
                        action->targetMapIndex,
                        action->targetMapX,
                        action->targetMapY,
                        i);
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
     * armor inventory, rest wake, active-group side state, sounds, and aspect
     * timing remain later slices. */
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
            if (creature_index > (int)((flags >> 5) & 0x03u)) return;
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
                    csb_v1_runtime_schedule_c38_attack_event(
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
                 * C38 attack time from CreatureInfo.AttackTicks plus random
                 * jitter after an attack decision.  This bounded CSB bridge
                 * requeues the same creature with the source AttackTicks base;
                 * RNG jitter and aspect-event timing remain later slices. */
                csb_v1_runtime_schedule_c38_attack_event(
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
        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_trigger_floor_sensor_event(
    CSB_V1_RuntimeProfile *profile,
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

    raw_square = csb_v1_dungeon_get_raw_square(
        dungeon,
        profile->current_level,
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
        profile->current_level,
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

static void csb_v1_runtime_process_party_floor_sensors_at(
    CSB_V1_RuntimeProfile *profile,
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
    if (map_x < 0 || map_y < 0) return;

    first_thing = csb_v1_dungeon_get_first_thing(
        dungeon,
        profile->current_level,
        map_x,
        map_y);
    if (first_thing < 0 || first_thing == 0xFFFE) return;

    /* ReDMCSB: MOVESENS.C F0267 lines 800-822 calls
     * F0276_SENSOR_ProcessThingAdditionOrRemoval when the party leaves and
     * enters a square.  F0276 lines 1658-1785 walks C03 sensor things until
     * the first non-sensor, checks floor sensor types C003/C005/C009 for the
     * party, resolves HOLD into SET/CLEAR, then calls F0272/F0268 to enqueue
     * the square-effect event.  This CSB runtime slice covers party floor
     * sensors only; wall clicks, possessions, objects, groups, launchers, and
     * sensor rotation remain separate runtime work. */
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
                    profile->current_level,
                    map_x,
                    map_y);
                int square_type = (raw_square < 0) ? -1 :
                    ((dungeon->square_bytes == 1)
                        ? ((raw_square >> 5) & 0x07)
                        : (raw_square & 0x1F));
                trigger = (square_type == 3) ? trigger : 0;
            }
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
            sensor_effect,
            target_x,
            target_y,
            target_cell,
            result);

        thing = csb_v1_runtime_sensor_next_thing(dungeon, (uint16_t)thing);
    }
}

static void csb_v1_runtime_apply_party_floor_sensor_consequences(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_InputCommandRuntimeResult *result)
{
    if (!profile || !result || !result->movement_step_applied) return;

    result->sensor_source_remove_checked = 1;
    csb_v1_runtime_process_party_floor_sensors_at(
        profile,
        result->old_party_x,
        result->old_party_y,
        0,
        result);
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
        out->destHasChampion = 1;
        out->destPartyDirection = profile->party_dir & 3;
        out->destChampionCellMask = 0x0F;
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
        out->destHasChampion = 1;
        out->destPartyDirection = profile->party_dir & 3;
        out->destChampionCellMask = 0x0F;
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
    if (tick_result.despawn) {
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
     * Lines 1317-1339 handle C018 endgame sensors. Projectile launchers and
     * rotation side effects remain separate runtime work. */
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
                           sensor_type)) {
                struct DungeonSensor_Compat decoded_sensor;
                struct ProjectileLauncherContext_Compat launcher_ctx;
                struct ProjectileLauncherResult_Compat launcher_result;
                int launch_index;

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
                        input.category = PROJECTILE_CATEGORY_MAGICAL;
                        input.subtype = subtype;
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
                        input.poisonAttack =
                            (subtype == PROJECTILE_SUBTYPE_POISON_CLOUD)
                                ? launch->attack
                                : 0;
                        input.attackTypeCode =
                            csb_v1_runtime_projectile_attack_type_from_subtype(
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
                csb_v1_runtime_trigger_remote_sensor_event(
                    profile,
                    record->mapIndex,
                    trigger_effect,
                    target_x,
                    target_y,
                    target_cell);
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
    memset(&profile->last_timeline_dispatch, 0,
           sizeof(profile->last_timeline_dispatch));
    profile->timeline_dispatch_count = 0;
    DM1_V1_InputCommandQueue_InitPc34Compat(&profile->input_command_queue);
    memset(&profile->last_input_dispatch, 0,
           sizeof(profile->last_input_dispatch));
    profile->input_dispatch_count = 0;

    profile->data_dir = data_dir;
    profile->save_dir = csb_v1_runtime_save_dir();
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
        target_dir = (profile->party_dir + 3) & 3;
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        break;
    case DM1_V1_COMMAND_TURN_RIGHT:
        target_dir = (profile->party_dir + 1) & 3;
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        break;
    case DM1_V1_COMMAND_MOVE_FORWARD:
    case DM1_V1_COMMAND_MOVE_RIGHT:
    case DM1_V1_COMMAND_MOVE_BACKWARD:
    case DM1_V1_COMMAND_MOVE_LEFT:
        {
            CSB_V1_MovementCommandStepRuntimeResultPc34Compat step_result;
            int step_status =
                csb_v1_movement_command_step_runtime_apply_pc34_compat(
                    profile,
                    local_result.queue_result.command,
                    csb_v1_runtime_default_wall_probe,
                    NULL,
                    &step_result);
            if (step_status < 0) {
                return -1;
            }
            local_result.unsupported_runtime_command =
                step_result.unsupported_runtime_command;
            local_result.movement_command_handled = step_result.command_handled;
            local_result.movement_step_attempted = step_result.step_attempted;
            local_result.movement_step_applied = step_result.step_applied;
            local_result.movement_blocked_by_wall = step_result.blocked_by_wall;
            local_result.movement_destination_x = step_result.destination_x;
            local_result.movement_destination_y = step_result.destination_y;
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
