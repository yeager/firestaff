/* dm2_v1_progression.c — DM2 V1 Progression Constants
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKWIN/SkGlobal.h
 * docs/dm2_champ_changes.md, docs/dm2_dungeon_design.md
 *
 * CRITICAL: DM2 V1 does NOT have XP-based character progression.
 * Stats are FIXED per champion type at creation.
 * Power progression: equipment, tech level, spell access, companions, reputation.
 */

#include "dm2_v1_progression.h"

/* ── Progression constants comparison table ────────────────────────────────
 * Source: SKULL.ASM, skproject/SKWIN/SkGlobal.h:636
 * DM2 vs DM1: key differences in AI table size, spell count, drop slots. */

const DM2_ProgressionConstant g_progression_constants[] = {
    /* AI table */
    { "CREATURE_AI_TABLE_SIZE", 64,  42,  "DM2: 64 entries (0-3E), DM1: 42 entries" },
    { "MAXAI",                  255, 62,  "DM2: extended mode max, DM1: 62" },

    /* Spells */
    { "MAX_SPELL_ORIGINAL",     34,  30,  "DM2: 34 fixed, DM1: ~30" },
    { "MAX_SPELL_CUSTOM",       255, 0,   "DM2: extended GDAT mode (DM1: none)" },

    /* Drops */
    { "DROP_SLOTS_PER_CREATURE", 11,  1,   "DM2: 11 via GDAT 0x0A-0x14, DM1: 1" },

    /* Level types */
    { "LEVEL_TYPE_COUNT",       3,   1,   "DM2: outdoor/indoor/building, DM1: indoor only" },

    /* Time cycle */
    { "TIME_CYCLE_MINUTES",     1440, 0,   "DM2: 24h cycle, DM1: no time-of-day" },

    /* Weather states */
    { "WEATHER_STATES",         4,   0,   "DM2: clear/rain/fog/storm, DM1: none" },

    /* Companions */
    { "MAX_COMPANIONS",         4,   0,   "DM2: NPC allies 13-18, DM1: none" },
    { "LOYALTY_MAX",            100, 0,   "DM2: companion loyalty meter, DM1: none" },

    /* Champion stat growth */
    { "CHAMP_LEVEL_MAX",        99,  99,  "DM2: capped but unused (no XP), DM1: used" },
    { "CHAMP_STAT_CAP",         999, 999, "DM1 and DM2: same cap" },

    /* Dungeon levels */
    { "MAX_DUNGEON_LEVELS",     28,  16,  "DM2: 28 (PC EN), DM1: 16" },

    /* Combat */
    { "RANGED_PENALTY_PER_TILE", 10, 10,  "DM2: -10%/tile, DM1: same" },
    { "MAX_ATTACK_RANGE",       255, 255, "Both: same max" },

    /* Sound */
    { "MUSIC_TRACK_COUNT",      28,  10,  "DM2: 28 HMP tracks, DM1: ~10" },
    { "SFX_VOICES",             16,  4,   "DM2: 16-slot ring buffer, DM1: 3-4" },
};

const int g_progression_constant_count =
    (int)(sizeof(g_progression_constants) / sizeof(g_progression_constants[0]));

const char *dm2_v1_progression_source_evidence(void) {
    return
        "DM2 V1 Progression Constants — Phase 6 source-lock\n"
        "ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: skproject/SKWIN/SkGlobal.h:37-163 (MAXSPELL_*, CREATURE_AI_TAB_SIZE, MAXAI)\n"
        "Source: skproject/SKWIN/SkGlobal.h:636 (EXTENDED_GDAT_CATEGORIES)\n"
        "Source: docs/dm2_champ_changes.md (no XP progression, fixed stat increases at creation)\n"
        "Source: docs/dm2_dungeon_design.md (3 level types, 28 dungeon levels)\n"
        "Source: docs/dm2_time.md (1440 min/day, time-of-day cycle)\n"
        "Source: docs/dm2_audio.md (28 music tracks, 16-slot SFX buffer)\n"
        "Source: SKWin.GDAT2.InternalCodes.txt (11 drop slots, DropTableSeed)\n"
        "CRITICAL NOTE: DM2 V1 stat increases are FIXED per champion type at creation.\n"
        "  No XP earned through play. Power progression via equipment, tech, spells, companions.\n";
}