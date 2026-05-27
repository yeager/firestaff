#ifndef THERON_V1_BOOT_H
#define THERON_V1_BOOT_H

#include <stdint.h>
#include <stddef.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron's Quest V1 Boot Profile — Phase 1: Runtime Profile Split
 *
 * Separates Theron's Quest boot/runtime from DM1/CSB/DM2/Nexus:
 *   - Asset discovery (PC Engine HuCard data layout)
 *   - Menu launch routing (M12 game-card → boot profile)
 *   - Save namespace: saves/theron/ (between-dungeon only)
 *   - Platform diagnostics (PC Engine HuCard JP/US)
 *   - Deterministic config (PC Engine fixed-tick, no chivalry)
 *
 * Platform reference:
 *   PC Engine HuCard — Hudson Soft / NEC, 1992 (JP) / 1993 (US)
 *   CPU: 7.16 MHz HuC6280 (65C02 derivative)
 *   Resolution: 256x224 (NTSC) / 320x224 (x1/x2 pixel modes)
 *   Palette: 512 colors, 16-color per-sprite tiles
 *   Data storage: HuCard ROM (HuCard, no CD) — mapped as  banks
 *
 * Provenance (Phase 0 — BLOCKED_ON_REFERENCE):
 *   No hash-verified asset set yet.
 *   g_theronVersions[] in asset_status_m12.c holds structural slots
 *   with empty MD5, awaiting Phase 0 lock before asset scan is wired.
 *
 * Save namespace design (distinct from DM1/CSB/DM2):
 *   Theron's Quest was a "light" version — smaller dungeon set,
 *   4-character party (Theron + 3 champions), 7 mini-dungeons.
 *   No in-dungeon save. Between-dungeon saves only (saves/theron/).
 *   This separates TQ save timing semantics from full DM games.
 *
 * Source references:
 *   THQUEST.ASM T000  — Theron's Quest title/startup entry
 *   THQUEST.ASM T200  — PC Engine platform detection / version label
 *   THQUEST.ASM T400  — Dungeon bank loading (HuCard ROM mapping)
 *   THQUEST.ASM T520  — Party placement and start position
 *   THQUEST.ASM T560  — Dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T800  — Champion persistence between dungeons
 *   THQUEST.ASM T080  — Save/load (between-dungeon only, no in-dungeon saves)
 * ══════════════════════════════════════════════════════════════════════ */

/* Theron's Quest supported platform/region IDs.
 * Two confirmed releases:
 *   JP —released 1992-09-18 (Hudson Soft, PC Engine DUO compatible)
 *   US — released 1993     (Hudson Soft USA, TurboGrafx-16) */
typedef enum {
    THERON_PLATFORM_PCE_JP,  /* PC Engine HuCard — Japanese 1st edition */
    THERON_PLATFORM_PCE_US,  /* TurboGrafx-16 — North American edition */
    THERON_PLATFORM_COUNT
} Theron_Platform;

/* Theron's Quest deterministic config.
 * Mirrors the PC Engine fixed-tick game loop (18.2 Hz VBlank,
 * matching DM1/CSB since the PC Engine was designed to replicate
 * that timing where possible).
 *
 * Key differences from DM1/CSB:
 *   - No chivalry system (simplified champion Need management)
 *   - Smaller party (4 vs 4 champions + party members)
 *   - 7 mini-dungeons rather than 15+ level dungeon
 *   - Quest structure: 7 dungeon goal sequence
 *   - No day/night outdoor cycle (all indoor on rails)
 */
typedef struct {
    /* Tick rate: PC Engine HuC6280 VBlank drives same ~18.2 Hz ticks.
     * Each tick ≈ 55ms. No interpolation in V1. */
    uint32_t tick_rate_hz;          /* default: 18 */
    uint32_t tick_rate_hz_frac;     /* default: 2 (18.2 Hz = 18 + 2/10) */
    uint32_t tick_ms;              /* ~55ms per tick */

    /* Movement: PC Engine version uses DM2-style fixed speeds.
     * Note: TQ has simpler movement — no diagonal strafing. */
    uint32_t dungeon_move_speed;    /* Q8: default 0x0080 (=0.5 sq/tick) */
    uint32_t outdoor_move_speed;    /* Q8: same as dungeon (no outdoor map) */

    /* Party: Theron + 3 champion slots (4 total). No minion slots.
     * Champions persist stats/skills between dungeons but lose
     * current inventory on dungeon restart (per design docs). */
    uint32_t max_champions;         /* 4 (1 Theron + 3 champions) */
    uint32_t max_party_members;     /* 4 */

    /* Quest: 7 mini-dungeons. No outdoor / day-night cycle.
     * Start is dungeon 1 in the Hall of Records. */
    uint32_t dungeon_count;         /* 7 mini-dungeons */
    uint32_t max_levels;           /* ~3 levels per mini-dungeon max */

    /* No time-of-day cycle in TQ (no outdoor exploration).
     * Reserved slot preserved for parity in config struct. */
    uint32_t reserved_day_cycle_minutes;
    uint32_t reserved_day_cycle_ticks;

    /* Deterministic RNG seed: from TQ dungeon header word at offset 8.
     * Mirrors DM2 structure so boot config builder reuses logic. */
    uint32_t dungeon_seed;          /* default fallback: 313 */

    /* TQ-specific: quest goal item (7 dungeon → 7 quest items).
     * quest_items_collected ranges 0..7 in saves/theron/namespace. */
    uint32_t quest_items_collected; /* persists across between-dungeon saves */

    uint32_t reserved[2];
} Theron_V1_DeterministicConfig;

/* Theron boot profile — collected at startup before game loop begins.
 * All fields are set once and read-only during gameplay.
 *
 * Save namespace: saves/theron/slotN.tqsv
 *   Format: between-dungeon only. In-dungeon saves mocked.
 *   (TQ design: you can save only at dungeon entrance, not mid-dungeon)
 */
typedef struct {
    /* ── Identity ────────────────────────────────────────── */
    char             game_id[8];       /* "theron" */
    Theron_Platform  platform;         /* detected platform */
    char             platform_label[32]; /* e.g. "PC Engine JP" */
    char             version_id[16];   /* e.g. "pce-jp", "pce-us" */

    /* ── Asset paths ─────────────────────────────────────── */
    char    asset_root[512];    /* ~/.firestaff/data/theron/ */
    char    graphics_path[512]; /* resolved: THQUEST.GFX or GRAPHICS.DAT */
    char    dungeon_path[512];  /* resolved: THQUEST.DUN or DUNGEON.DAT */
    int     assets_verified;   /* 0 — Phase 0 gate (no hash yet) */

    /* ── Save namespace (between-dungeon only) ─────────── */
    char    save_root[512];     /* saves/theron/ — NOT saves/dm1/ */
    int     in_dungeon_save_allowed; /* 0 — design restriction */

    /* ── Detected file sizes (diagnostic) ───────────────── */
    size_t  graphics_size;
    size_t  dungeon_size;
    char    graphics_md5[33];  /* empty until Phase 0 hash lock */
    char    dungeon_md5[33];  /* empty until Phase 0 hash lock */

    /* ── Deterministic config ────────────────────────────── */
    Theron_V1_DeterministicConfig deterministic;

    /* ── Runtime references (set after boot) ──────────────── */
    void   *theron_state;     /* Theron_V1_GameState* — set post boot */
    void   *dungeon_data;     /* Theron_V1_DungeonData* — parsed TQ data */
    void   *graphics_dat;    /* graphics data handle */
} Theron_V1_BootProfile;

/* ── Boot API ──────────────────────────────────────────────────────── */

/* Initialize a boot profile with Theron V1 defaults.
 * Does not touch filesystem — only sets struct fields. */
void theron_v1_boot_profile_init(Theron_V1_BootProfile *profile);

/* Scan for Theron's Quest assets in data_dir.
 * Phase 0 gate: assets_verified stays 0 until Phase 0 hash lock.
 * Probes for THQUEST.GFX / THQUEST.DUN / GRAPHICS.DAT / DUNGEON.DAT
 * in theron/ subdir.
 *
 * Returns 0 on success (assets found, even if unverified),
 * -1 if no Theron assets detected. */
int theron_v1_boot_scan_assets(Theron_V1_BootProfile *profile,
                               const char *data_dir);

/* Probe a data_dir for Theron assets without full verification.
 * Used by menu to check availability without triggering hash lock.
 * Returns: 1 if assets found, 0 if not. */
int theron_v1_boot_probe_available(const char *data_dir);

/* Set the save root directory.
 * If save_dir is NULL, uses default: <data_dir>/../saves/theron/ */
void theron_v1_boot_set_save_root(Theron_V1_BootProfile *profile,
                                   const char *save_dir);

/* Build deterministic config from detected dungeon header.
 * Reads dungeon_seed from header word at offset 8 (same layout as DM2).
 * Source: THQUEST.ASM T560 — dungeon header parsing */
void theron_v1_boot_build_deterministic_config(Theron_V1_BootProfile *profile,
                                               const uint8_t *dungeon_header,
                                               int dungeon_size);

/* Enter the game: initialize game state from boot profile.
 * Sets profile->theron_state and profile->dungeon_data.
 * Returns 0 on success. */
int theron_v1_boot_enter_game(Theron_V1_BootProfile *profile);

/* Free resources allocated during boot (not the profile itself). */
void theron_v1_boot_cleanup(Theron_V1_BootProfile *profile);

/* ── Diagnostics API ──────────────────────────────────────────────── */

/* Fill a diagnostic report with human-readable text.
 * Returns bytes written (capped at buf_size). */
size_t theron_v1_diagnostic_report(const Theron_V1_BootProfile *profile,
                                    char *buf, size_t buf_size);

/* Print one-line platform/version summary to stdout. */
void theron_v1_boot_print_summary(const Theron_V1_BootProfile *profile);

/* Source evidence citation string. */
const char *theron_v1_boot_source_evidence(void);

#endif /* THERON_V1_BOOT_H */
