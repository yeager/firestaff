/*
 * csb_v1_runtime_pc34_compat.h — CSB V1 Runtime Profile
 *
 * Chaos Strikes Back V1 runtime profile: separate from DM1.
 *
 * Boot sequence (ReDMCSB ENTRANCE.C F0806_F0806_ENTRANCE_int):
 *   1. Load ENTRANCE.GFX (bootsector or disk-based)
 *   2. Show title screen with game ID / version string
 *   3. F0807_ENTRANCE_DrawAnimationStep: optional door animation
 *   4. Present game selector then run game
 *
 * CSB vs DM1 key runtime differences:
 *   - Different dungeon data (DUNGEON.C hash: 6695d2acebce49f95db1d8f3a5c733de)
 *   - Different default difficulty
 *   - Champion difficulty scale: +25% per champion (CSB harder vs DM1 flat)
 *   - Save namespace: csb_save_%d.fsav (LOADSAVE.C F0433/F0435)
 *   - Asset names: GRAPHICS.DAT + CSBGRAPH.DAT (or CSB.DAT on floppy platforms)
 *   - Entry point: ENTRANCE (same as DM1, but C28_ENTRANCE_CSB palette)
 *   - Chaos Magic system initialized at boot (CASTER.C F0211)
 *   - Champion import from DM1 save before dungeon load
 *
 * ReDMCSB references:
 *   ENTRANCE.C: F0806Entrance_int  (game boot)
 *   ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)
 *   ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics init)
 *   SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful
 *   SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful
 *   LOADSAVE.C: F0435_STARTEND_LoadGame
 *   LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame
 *   DUNGEON.C:  F0237_DUNGEON_DungeonLoad (hash-verified dungeon load)
 *   CASTER.C:   F0211_CASTER_ClearSpellEffects  (CSB-wide spell reset)
 *   CEDTINC7.C: G3764_THAT_S_THE_CSB_UTILITY_DISK  (utility disk prompt)
 *   CEDTDATA.C: G3921 PLEASE_INSERT_UTIL_DISK string
 *   BugsAndChanges.htm: CHANGE7_29  (new header format)
 *   MEDIA529_F20E_F20J: F20E/F21E save path decision
 *   MEDIA332_F20E_F21E_A31E_F31E: CSB uses C29 key index
 *
 * CSBWin save path references:
 *   SaveGame.cpp: LoadGame(), SaveGame() (2953 lines)
 *   Character.cpp: Character::import_dm1_save()
 */

#ifndef FIRESTAFF_CSB_V1_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_RUNTIME_PC34_COMPAT_H

#include <stdint.h>
#include "csb_v1_game_state_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Game identifier ────────────────────────────────────────────────────── */
#define CSB_V1_GAME_ID_STR  "csb"
#define CSB_V1_SAVE_ID_STR  "csbgame"

#define CSB_V1_MAX_PARTY_X  32
#define CSB_V1_MAX_PARTY_Y  32

/* ── Deterministic tick config ────────────────────────────────────────── */
/*
 * CSB uses a 55ms per-tick clock (same as DM1).
 * A 32-bit counter rolls over after ~27 hours of play.
 * Using uint64_t for extended precision during probing.
 */
#define CSB_V1_TICK_MS_NOMINAL  55U    /* nominal V1 tick in ms */

/* ── Default start position ──────────────────────────────────────────── */
/*
 * CSB starts in the Hall of Champions on map index 0.
 * Party always begins at coordinates (5, 5) facing North (0).
 * ReDMCSB: ENTRANCE.C line ~430 (G0309_i_PartyMapIndex init)
 */
#define CSB_V1_START_PARTY_X      5
#define CSB_V1_START_PARTY_Y      5
#define CSB_V1_START_PARTY_Z      0    /* ground floor */
#define CSB_V1_START_PARTY_DIR    0    /* 0=North, 1=East, 2=South, 3=West */

/* ── Difficulty ───────────────────────────────────────────────────────── */
/*
 * CSB difficulty scale: each champion fight gets +25% effective stats.
 * Base multiplier is 1.0 (same as DM1).  Escalates per champion count.
 * ReDMCSB: PROJEXPL.C projectile + CSBWin Character.cpp difficulty scale
 *
 *   Champions in party | Effective difficulty (x100%)
 *   ------------------+--------------------------------
 *     1                |  1.00 (Easy)
 *     2                |  1.25 (+25% Normal)
 *     3                |  1.50 (+50% Hard — default)
 *     4                |  2.00 (+100% Extreme)
 */
#define CSB_V1_DIFFICULTY_BASE        100   /* base percent, x100 scale */
#define CSB_V1_DIFFICULTY_PER_CHAMP    25   /* extra % x100 per champion */

typedef enum {
    CSB_V1_DIFFICULTY_EASY   = 100,
    CSB_V1_DIFFICULTY_NORMAL = 125,  /* 2 champions */
    CSB_V1_DIFFICULTY_HARD   = 150,  /* 3 champions (default) */
    CSB_V1_DIFFICULTY_EXTREME = 200  /* 4 champions */
} CSB_V1_Difficulty;

/* ── Asset discovery namespace ───────────────────────────────────────── */

/*
 * CSB asset discovery differs from DM1 in two ways:
 *   1. Graphics archive can be CSBGRAPH.DAT, CSB.DAT, or GRAPHICS.DAT
 *      (floppy vs CD/data vs hybrid media).
 *   2. Dungeon data uses a separate hash (6695d2acebce49f95db1d8f3a5c733de).
 *
 * File search order (ReDMCSB DISK.C + CSBWin AssetCache):
 *   GRAPHICS.DAT on any media type (always required)
 *   CSBGRAPH.DAT (CD/data release, optional overlay)
 *   CSB.DAT     (floppy release, complete GRAPHICS replacement)
 *   DUNGEON.DAT  (hash-verified)
 *
 * Data directory discovery:
 *   ~/.firestaff/data/csb/     (canonical)
 *   ~/.firestaff/data/          (shared DM1/CSB/DM2 search fallback)
 *   $PWD                       (dev mode fallback)
 */
typedef enum {
    CSB_V1_ASSET_GFX_ARCHIVE_NONE   = 0,
    CSB_V1_ASSET_GFX_ARCHIVE_CSB    = 1,  /* CSB.DAT */
    CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF = 2, /* CSBGRAPH.DAT */
    CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS = 3, /* GRAPHICS.DAT (floppy fallback) */
} CSB_V1_AssetGfxArchiveType;

typedef struct {
    const char *path;       /* absolute path found */
    CSB_V1_AssetGfxArchiveType kind;  /* archive variant found */
} CSB_V1_AssetResult;

/* ── Variant diagnostics ──────────────────────────────────────────────── */
/*
 * CSB ships on many platforms, each with different media layouts.
 * Runtime variant diagnostics identify platform for bug-for-bug fixes.
 *
 * CSBWin AssetCache + ReDMCSB COMPILE.H MEDIA_* tags:
 *   MEDIA278 / P20JA / P20JB  = PC DOS 3.4 (EN/Multi)
 *   MEDIA332 / S20E / S21E    = Atari ST 2.0 / 2.1 English
 *   MEDIA529 / A35E / A35M    = Amiga 3.5 (EN/Multi)
 *   MEDIA529 / F20J / F20E    = Atari ST TT variant (F20J = 060)
 *
 * These affect only diagnostics and platform-specific quirks.
 * Game logic is entirely source-locked to ReDMCSB.
 */
typedef enum {
    CSB_V1_VARIANT_UNKNOWN           = 0,
    CSB_V1_VARIANT_PC34_EN          = 1,   /* PC DOS 3.4 English       MEDIA278 */
    CSB_V1_VARIANT_PC34_MULTI        = 2,   /* PC DOS 3.4 Multilanguage MEDIA278 */
    CSB_V1_VARIANT_ST20_EN           = 3,   /* Atari ST 2.0 English   MEDIA332 */
    CSB_V1_VARIANT_ST21_EN           = 4,   /* Atari ST 2.1 English   MEDIA332 */
    CSB_V1_VARIANT_AMIGA35_EN         = 5,   /* Amiga 3.5 English       MEDIA529 */
    CSB_V1_VARIANT_AMIGA35_MULTI      = 6,   /* Amiga 3.5 Multilanguage MEDIA529 */
    CSB_V1_VARIANT_ST_F20J           = 7,   /* Atari ST TT (F20J)     MEDIA529 */
    CSB_V1_VARIANT_ST_F20E          = 8,   /* Atari ST (F20E)        MEDIA529 */
    CSB_V1_VARIANT_COUNT
} CSB_V1_VariantId;

typedef struct {
    CSB_V1_VariantId   id;
    const char        *name;       /* human-readable: "Atari ST 2.1 EN" */
    const char        *media_ref;   /* ReDMCSB COMPILE.H media tag */
    const char        *md5_gfx;     /* GRAPHICS.DAT hash for this variant */
    const char        *md5_graf;    /* CSBGRAPH.DAT / CSB.DAT hash */
    const char        *md5_dungeon; /* DUNGEON.DAT hash (shared) */
} CSB_V1_VariantInfo;

/* ── Chaos Magic runtime state ──────────────────────────────────────── */
/*
 * CSB introduces a Chaos Magic system not present in DM1.
 * Initialized at boot (F0211_CASTER_ClearSpellEffects fires at world load).
 * Active during all dungeon exploration.
 * ReDMCSB: CASTER.C F0211 (cleared at world load)
 *          CASTER.C F0213 (per-square invocation slots)
 * CSBWin:  Magic.cpp ChaosMagic namespace
 */
typedef struct {
    uint32_t spell_grid_version;  /* version key for CSB-wide spell grid */
    uint8_t  chaos_level;        /* current ambient chaos level (0-3) */
    int      magic_initialized;   /* 1 = spell grid built */
} CSB_V1_ChaosMagicState;

/* ── Runtime profile ─────────────────────────────────────────────────── */
/*
 * CSB V1 runtime profile: everything that distinguishes CSB from DM1.
 *
 * Separated from:
 *   - DM1 profile: different dungeon hash, save namespace, difficulty
 *   - CSB V2 profile: V1 game logic identical; V2 has render changes
 *
 * The profile owns:
 *   - Game mode and state machine
 *   - Party position/direction
 *   - Chaos Magic state
 *   - Deterministic 55ms tick accumulator
 *   - Config/game variant
 *   - Asset path references
 */
typedef struct {
    /* ── Identity ─────────────────────────────── */
    CSB_V1_VariantId        variant_id;
    CSB_V1_Difficulty       difficulty;
    uint32_t                dungeon_seed;   /* from DUNGEON.DAT */
    uint16_t                dungeon_game_id; /* serial from dungeon header */
    CSB_V1_AssetResult      dungeon_asset;
    CSB_V1_AssetResult      graphics_asset;

    /* ── Dungeon world ──────────────────────────── */
    int                     current_level;   /* 0-based dungeon level */
    int                     current_world;   /* 0-based world index */
    int                     level_count;    /* total dungeon levels */
    int                     world_count;     /* worlds in this campaign */

    /* ── Party state ────────────────────────────── */
    int                     party_x;
    int                     party_y;
    int                     party_z;         /* floor / height level */
    int                     party_dir;       /* 0=North, 1=East, 2=South, 3=West */
    int                     champion_count;  /* champions in party */

    /* ── State machine ─────────────────────────── */
    int                     state;   /* CSB_STATE_* enum */
    int                     paused;
    int                     victory;
    int                     game_over;

    /* ── Timing ────────────────────────────────── */
    uint64_t                game_ticks;      /* ms accumulator */
    uint32_t                game_time;       /* V1 game_time */
    uint64_t                total_play_ms;   /* wall-clock ms */
    uint32_t                tick_count;       /* how many V1 ticks elapsed */

    /* ── Chaos Magic ────────────────────────────── */
    CSB_V1_ChaosMagicState  chaos_magic;

    /* ── Data paths ─────────────────────────────── */
    const char             *data_dir;
    const char             *save_dir;  /* resolved at init via _save_dir_x() */
    const char             *dungeon_path;
    const char             *graphics_path;

    /* ── Dungeon data (owned) ─────────────────────── */
    /* Heap-allocated dungeon loaded by csb_v1_runtime_boot().
     * Freed by csb_v1_runtime_cleanup() or during a subsequent boot.
     * Also accessible via csb_v1_dungeon_get_current() for dungeon-layer
     * accessor stubs in csb_v1_dungeon_world_pc34_compat.c. */
    CSB_V1_DungeonData *dungeon_handle;
} CSB_V1_RuntimeProfile;

/* ── Runtime profile API ─────────────────────────────────────────────── */

/* Initialize a fresh runtime profile with CSB defaults.
 * Sets difficulty, start position, and NULL paths.
 * Does NOT boot the dungeon or initialize Chaos Magic. */
void csb_v1_runtime_init(CSB_V1_RuntimeProfile *profile, const char *data_dir);

/* Clean up runtime resources.
 * Unloads the dungeon data loaded by csb_v1_runtime_boot().
 * Idempotent: safe to call when no dungeon is loaded.
 * After this call, dungeon-layer accessors return ENDOF until
 * csb_v1_runtime_boot() is called again with a valid dungeon. */
void csb_v1_runtime_cleanup(CSB_V1_RuntimeProfile *profile);

/* Boot the CSB dungeon and initialize Chaos Magic.
 * Finds DUNGEON.DAT by hash (csb_v1_runtime_find_dungeon), loads the
 * dungeon data into the current dungeon context
 * (csb_v1_dungeon_load_from_file + csb_v1_dungeon_set_current), sets
 * dungeon_seed/game_id from the dungeon header, and initializes the spell
 * grid (F0211_CASTER_ClearSpellEffects).
 *
 * On success, the dungeon is accessible via csb_v1_dungeon_get_current()
 * and dungeon-layer accessors in csb_v1_dungeon_world_pc34_compat.h are
 * live.  Call csb_v1_runtime_cleanup() to unload before shutdown.
 *
 * Returns 0 on success, -1 if dungeon not found or load fails.
 * On success, profile->dungeon_path and ->graphics_path are set. */
int csb_v1_runtime_boot(CSB_V1_RuntimeProfile *profile,
                          const char *data_dir,
                          const char *version_hint);

/* Advance the runtime clock by dt_ms milliseconds.
 * Quantizes into 55ms V1 ticks; fires one tick per quantum.
 * Advances chaos_magic state each tick. */
void csb_v1_runtime_tick(CSB_V1_RuntimeProfile *profile, uint32_t dt_ms);

/* Advance exactly one V1 tick (55ms nominal).
 * Deterministic stepping function.  Returns 1 if a tick fired, 0 if paused. */
int csb_v1_runtime_tick_v1(CSB_V1_RuntimeProfile *profile);

/* Check if a V1 tick is due at wall-clock time now_ms. */
int csb_v1_runtime_tick_due(const CSB_V1_RuntimeProfile *profile, uint32_t now_ms);

/* ── Variant diagnostics ─────────────────────────────────────────────── */
const char *csb_v1_runtime_variant_name(CSB_V1_VariantId id);
const CSB_V1_VariantInfo *csb_v1_runtime_get_variant_info(CSB_V1_VariantId id);

/* Detect variant by matching gfx + dungeon MD5 hashes.
 * Returns the best-matching VariantId, or CSB_V1_VARIANT_UNKNOWN. */
int csb_v1_runtime_detect_variant(const char *gfx_path,
                                    const char *dungeon_path,
                                    const char *md5_gfx,
                                    const char *md5_dungeon);

/* ── Difficulty helpers ─────────────────────────────────────────────── */
int csb_v1_runtime_calc_difficulty(int champion_count);
const char *csb_v1_runtime_difficulty_str(int difficulty_x100);

/* ── Asset discovery ────────────────────────────────────────────────── */

/* Find CSB DUNGEON.DAT by hash (6695d2acebce49f95db1d8f3a5c733de).
 * Searches data_dir/csb/ and data_dir/ recursively.
 * Returns absolute path, or NULL if not found. */
const char *csb_v1_runtime_find_dungeon(const char *data_dir,
                                         CSB_V1_AssetResult *out_result);

/* Find CSB graphics archive.
 * Searches csb.dat / CSBGRAPH.DAT / GRAPHICS.DAT in order.
 * Returns absolute path, or NULL if none found. */
const char *csb_v1_runtime_find_graphics(const char *data_dir,
                                           const char *version_hint,
                                           CSB_V1_AssetResult *out_result);

/* ── Save namespace ──────────────────────────────────────────────── */
const char *csb_v1_runtime_save_dir(void);
const char *csb_v1_runtime_save_path(int slot);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *csb_v1_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_RUNTIME_PC34_COMPAT_H */
