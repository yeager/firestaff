#ifndef FIRESTAFF_DM2_V1_PROGRESSION_H
#define FIRESTAFF_DM2_V1_PROGRESSION_H
#include <stdint.h>

/* DM2 V1 — Progression and Game Constants
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKWIN/SkGlobal.h
 * docs/dm2_champ_changes.md, docs/dm2_dungeon_design.md
 *
 * CRITICAL: DM2 V1 does NOT have traditional champion XP progression.
 * Stat increases are FIXED per champion type at creation — not earned.
 * Power progression: equipment quality, tech level, spell access, companions, reputation.
 *
 * Key constants: 64 AI entries, 34 spells, 11 drop slots, 4 weather states.
 */

/* ── AI table sizes ───────────────────────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.h:636 */

#define DM2_CREATURE_AI_TABLE_SIZE   64   /* vs 42 in DM1 */
#define DM2_MAXAI                     255  /* extended mode */

/* ── Spell table size ───────────────────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.h:37-55 */

#define DM2_MAX_SPELL_ORIGINAL        34   /* 0-33 fixed mode */
#define DM2_MAX_SPELL_CUSTOM          255  /* extended GDAT mode */

/* ── Drop table size ────────────────────────────────────────────────
 * Source: SKWin.GDAT2.InternalCodes.txt (11 slots per creature) */

#define DM2_DROP_SLOTS_PER_CREATURE   11   /* vs 1 in DM1 */

/* ── Dungeon level types ────────────────────────────────────────────
 * Source: docs/dm2_dungeon_design.md
 * DM2 has outdoor areas — DM1 and CSB are purely indoor dungeon. */

#define DM2_LEVEL_OUTDOOR  0   /* sky/ground/tree/building, weather zones */
#define DM2_LEVEL_INDOOR   1   /* first-person dungeon (same model as DM1) */
#define DM2_LEVEL_BUILDING  2   /* multi-floor buildings within outdoor */
#define DM2_LEVEL_COUNT     3

/* ── Outdoor / time constants ─────────────────────────────────────────
 * Source: include/dm2_v1_game.h, docs/dm2_time.md */

#define DM2_TIME_MINUTES_PER_DAY      1440  /* 24h * 60 */
#define DM2_TIME_START_MINUTES        720   /* noon */
#define DM2_WEATHER_STATES            4     /* clear/rain/fog/storm */

/* ── Companion/minion constants ─────────────────────────────────────
 * Source: skproject/SKWIN/SkWinCore.cpp:741-810
 * DM2 new: ally companions (13-18) and evil minions (34,43,49,62) */

#define DM2_MAX_COMPANIONS            4    /* party companion limit */
#define DM2_COMPANION_LOYALTY_MAX     100  /* loyalty meter range */
#define DM2_COMPANION_LOYALTY_MIN     0

/* ── Champion stat growth ───────────────────────────────────────────
 * Source: docs/dm2_champ_changes.md
 * DM2 V1: stat increases are FIXED at champion creation, NOT XP-driven.
 * Class bonuses are table lookups at creation time only. */

#define DM2_CHAMP_STAT_CAP            999  /* per-stat maximum */
#define DM2_CHAMP_LEVEL_MAX            99   /* theoretical max (unused in V1) */

/* ── Dungeon progression ─────────────────────────────────────────────
 * Source: SKULL.ASM header parsing, docs/dm2_dungeon_design.md */

#define DM2_MAX_DUNGEON_LEVELS        28   /* PC English DM2 */
#define DM2_LEVEL_EXIT_Y               0   /* exit marker y position */
#define DM2_LEVEL_EXIT_X               0

/* ── Combat constants ────────────────────────────────────────────────
 * Source: SKULL.ASM combat, SkWinCore.cpp:17521-17670
 * Ranged penalty: -10% per extra tile beyond first */

#define DM2_RANGED_PENALTY_PER_TILE    10   /* percent damage reduction */
#define DM2_MAX_ATTACK_RANGE           255  /* max tile distance */

/* ── Progression constants comparison table ─────────────────────────── */

typedef struct {
    const char *name;
    int         dm2_value;
    int         dm1_value;
    const char *note;
} DM2_ProgressionConstant;

extern const DM2_ProgressionConstant g_progression_constants[];
extern const int g_progression_constant_count;

const char *dm2_v1_progression_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_PROGRESSION_H */