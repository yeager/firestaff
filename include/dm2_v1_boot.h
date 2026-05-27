#ifndef DM2_V1_BOOT_H
#define DM2_V1_BOOT_H

#include <stdint.h>
#include <stddef.h>

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V1 Boot Profile — Phase 1: Runtime Profile Split
 *
 * Separates DM2/Skullkeep boot/runtime from DM1/CSB:
 *   - Asset discovery: DM2GRAPHICS.DAT, DM2DUNGEON.DAT, GRAPHICS.DAT in dm2/
 *   - Save namespace:  saves/dm2/slotN.fssv
 *   - Platform/version diagnostics
 *   - Deterministic config defaults
 *
 * Source references:
 *   SKULL.ASM T560  — DM2 dungeon loading (DUNGEON_Load)
 *   SKULL.ASM T000  — DM2 startup / title screen
 *   SKULL.ASM T800  — DM2 outdoor / shop / NPC entry points
 *   SKULL.ASM T520  — DM2 party placement and start position
 *   SKULL.ASM T048  — DM2 platform detection / version
 * ══════════════════════════════════════════════════════════════════════ */

/* DM2 supported platform IDs.
 * Used in diagnostics, save headers, and platform-specific logic. */
typedef enum {
    DM2_PLATFORM_PC_EN,      /* PC English — primary reference */
    DM2_PLATFORM_PC_FR,      /* PC French */
    DM2_PLATFORM_PC_JEWEL,   /* PC German/English JewelCase */
    DM2_PLATFORM_COUNT
} DM2_Platform;

/* DM2 deterministic config.
 * Fixed gameplay constants that ensure reproducible runs.
 * These mirror the original DM2 fixed-point arithmetic timing. */
typedef struct {
    /* Tick rate: DM2 runs at the same 18.2 Hz VBlank as DM1/CSB.
     * Each tick = ~55ms. No interpolation in V1. */
    uint32_t tick_rate_hz;          /* default: 18 */
    uint32_t tick_rate_hz_frac;     /* default: 2 (18.2 Hz = 18 + 2/10) */
    uint32_t tick_ms;              /* ~55ms per tick */

    /* Movement: DM2 uses slightly different speed values.
     * outdoor movement speed is higher than dungeon movement.
     * These are fixed-point Q8 values. */
    uint32_t dungeon_move_speed;   /* Q8: default 0x0080 (=0.5 squares/tick) */
    uint32_t outdoor_move_speed;   /* Q8: default 0x0100 (=1.0 squares/tick) */

    /* Party: DM2 supports up to 4 champions + minion slots.
     * Companion AI runs on same tick as party movement. */
    uint32_t max_champions;        /* 4 */
    uint32_t max_party_members;   /* 5 incl. leader */

    /* Time-of-day cycle: DM2 outdoor areas have day/night.
     * Full cycle in 1440 minutes (24 hours). */
    uint32_t day_cycle_minutes;    /* 1440 */
    uint32_t day_cycle_ticks;      /* derived from tick_rate */

    /* Dungeon: DM2 has 28 levels in PC English version.
     * Level 0 = Entrance / Hall of Champions.
     * Levels 1-27 = various indoor/outdoor areas. */
    uint32_t max_levels;          /* 28 for PC EN */

    /* Deterministic RNG seed: DM2 dungeon seed from DUNGEON.DAT header.
     * Used to initialize the event RNG for reproducible runs. */
    uint32_t dungeon_seed;

    /* Reserved for future deterministic options */
    uint32_t reserved[4];
} DM2_V1_DeterministicConfig;

/* DM2 boot profile — collected at startup before game loop begins.
 * All fields are set once and read-only during gameplay. */
typedef struct {
    /* ── Identity ────────────────────────────────────────── */
    char             game_id[8];        /* "dm2" */
    DM2_Platform     platform;          /* detected platform */
    char             platform_label[32]; /* e.g. "PC English" */
    char             version_id[16];    /* e.g. "pc-en", "pc-fr" */

    /* ── Asset paths ─────────────────────────────────────── */
    char    asset_root[512];   /* base data dir, e.g. ~/.firestaff/data/dm2/ */
    char    graphics_path[512]; /* resolved path to GRAPHICS.DAT / DM2GRAPHICS.DAT */
    char    dungeon_path[512]; /* resolved path to DUNGEON.DAT / DM2DUNGEON.DAT */
    int     use_dm2_filenames;  /* 1 if using DM2GRAPHICS.DAT/DM2DUNGEON.DAT */
    int     assets_verified;    /* 1 if MD5 hash matched a known version */

    /* ── Save namespace ───────────────────────────────────── */
    char    save_root[512];    /* saves/dm2/ */

    /* ── Detected file sizes (diagnostic) ─────────────────── */
    size_t  graphics_size;
    size_t  dungeon_size;
    char    graphics_md5[33];
    char    dungeon_md5[33];

    /* ── Deterministic config ──────────────────────────────── */
    DM2_V1_DeterministicConfig deterministic;

    /* ── Runtime references (set after boot) ──────────────── */
    void   *dm2_state;         /* DM2_V1_GameState* — set by dm2_v1_boot_enter_game() */
    void   *dungeon_data;      /* DM2_V1_DungeonData* — parsed dungeon */
    void   *graphics_dat;      /* graphics data handle */
} DM2_V1_BootProfile;

/* ── Boot API ──────────────────────────────────────────────────────── */

/* Initialize a boot profile with defaults.
 * Does not touch the filesystem — only sets struct fields. */
void dm2_v1_boot_profile_init(DM2_V1_BootProfile *profile);

/* Scan and verify DM2 assets in data_dir.
 * Sets asset_root, graphics_path, dungeon_path, assets_verified.
 * Returns 0 on success, -1 if no valid DM2 assets found. */
int dm2_v1_boot_scan_assets(DM2_V1_BootProfile *profile,
                            const char *data_dir);

/* Probe a data_dir for DM2 assets without full verification.
 * Used by the launcher menu to determine DM2 availability.
 * Returns: 1 if assets found, 0 if not. */
int dm2_v1_boot_probe_available(const char *data_dir);

/* Set the save root directory.
 * If save_dir is NULL, uses default: <data_dir>/../saves/dm2/ */
void dm2_v1_boot_set_save_root(DM2_V1_BootProfile *profile,
                                const char *save_dir);

/* Build deterministic config from detected dungeon header.
 * Reads dungeon_seed from the DUNGEON.DAT header word at offset 8.
 * Source: SKULL.ASM T560 — DUNGEON_Load header parsing */
void dm2_v1_boot_build_deterministic_config(DM2_V1_BootProfile *profile,
                                            const uint8_t *dungeon_header,
                                            int dungeon_size);

/* Enter the game: initialize the game state from the boot profile.
 * Sets profile->dm2_state and profile->dungeon_data.
 * Returns 0 on success. */
int dm2_v1_boot_enter_game(DM2_V1_BootProfile *profile);

/* Free resources allocated during boot (but not the profile itself). */
void dm2_v1_boot_cleanup(DM2_V1_BootProfile *profile);

/* ── Diagnostics API ──────────────────────────────────────────────── */

/* Fill a diagnostic report buffer with human-readable text.
 * Returns bytes written (capped at buf_size). */
size_t dm2_v1_diagnostic_report(const DM2_V1_BootProfile *profile,
                                char *buf, size_t buf_size);

/* Print a one-line platform/version summary to stdout. */
void dm2_v1_boot_print_summary(const DM2_V1_BootProfile *profile);

/* Source evidence citation string.
 * Used in assert comments and debug output. */
const char *dm2_v1_boot_source_evidence(void);

#endif /* DM2_V1_BOOT_H */