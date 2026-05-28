/* dm2_v1_creature.c — DM2 V1 Creature AI
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKWIN/SkWinCore.cpp, DME.h, defines.h
 * SKULLWIN/c_ai.cpp, c_creature.cpp, c_creature.h
 *
 * DM2 creature AI:
 *   - 64-entry AIDefinition table (vs 42 in DM1)
 *   - CCM command-dispatch via b_1a state register
 *   - AI_ATTACK_FLAGS for spell/attack type routing
 *   - 13 new creature types vs DM1 (companions, Dragoth, Vexirk, etc.)
 */

#include "dm2_v1_creature.h"
#include <string.h>

/* ── dAITableGenuine — 64-entry AI definition table (hardcoded) ───────────
 * Source: skproject/SKWIN/SkWinCore.cpp:741-810 (getAIName)
 * Extended mode override: EXTENDED_LOAD_AI_DEFINITION() at SkWinCore.cpp:233-400
 *
 * Per-entry fields: w0AIFlags, ArmorClass, b3, BaseHP, AttackStrength,
 *                   PoisonDamage, Defense, b9x, w10, w12, AttacksSpells,
 *                   w16, w18, w20, w22, w24, w26, b28, Weight, w30, w32, b34, b35
 *
 * Only AI index names are sourced; structure layout from DME.h:1505-1545.
 * Full AIDefinition table values are GDAT-loaded in extended mode.
 * Stub here uses zero-initialized table; real implementation reads GDAT.
 *
 * Companion/minion AI indices (13-18) are DM2-specific — no DM1 equivalent.
 * Boss indices: 30 (Lord Dragoth), 55 (Vexirk King), 51 (Amplifier).
 */

static const char *const g_ai_names[DM2_AI_TABLE_SIZE] = {
    [0]  = "TREE (PILLAR)",
    [1]  = "LABORATORY TABLE",
    [3]  = "BUSH",
    [4]  = "PILLARS/ROD (PILLAR)",
    [5]  = "STALAGMITE (PILLAR)",
    [6]  = "BOULDER",
    [7]  = "FOUNTAIN",
    [8]  = "OBELISKS/TOMBS",
    [9]  = "WOOD TABLE (TABLE)",
    [10] = "MAGICK CAULDRON",
    [11] = "SKULL BRAZIER",
    [12] = "TRADING TABLE",
    [13] = "SCOUT MINION (ALLY)",      /* DM2: companion */
    [14] = "ATTACK MINION (ALLY)",      /* DM2: summoned by spell 29 */
    [15] = "CARRY MINION (ALLY)",       /* DM2: companion */
    [16] = "FETCH MINION (ALLY)",       /* DM2: companion */
    [17] = "GUARD MINION (ALLY)",       /* DM2: summoned by spell 30 */
    [18] = "U-HAUL MINION (ALLY)",      /* DM2: summoned by spell 31 */
    [19] = "THORN DEMON",               /* DM2: drops sellable worm food */
    [20] = "OBELISK (PASSABLE)",        /* DM2: decorative */
    [21] = "VORTEX",                    /* DM2: pull hazard */
    [22] = "FLAME ORB",                 /* DM2: fire hazard */
    [23] = "CAVERN BAT",                /* like DM1 bat */
    [24] = "GLOP",                      /* DM2: w0_6_7 set */
    [25] = "ROCKY",                     /* DM2: jump ability */
    [26] = "GIGGLER",                   /* steal (AI_ATTACK_FLAGS__STEAL) */
    [27] = "THICKET THIEF",             /* steal (AI_ATTACK_FLAGS__STEAL) */
    [28] = "TIGER STRIPED WORM (WORM)", /* DM2: w0_6_7 */
    [29] = "TREANT (TREE GORGON)",      /* DM2 */
    [30] = "LORD DRAGOTH",              /* DM2: primary antagonist */
    [31] = "DRU TAN",                   /* DM2 */
    [32] = "CAVE IN",                   /* DM2: trap */
    [33] = "MERCHANTS",                 /* DM2: NPC/shop */
    [34] = "DRAGOTH MINION (EVIL)",     /* DM2: Dragoth spawn */
    [35] = "TOWER BAT",                  /* like DM1 bat */
    [36] = "ARCHER GUARD",              /* DM2: AI_ATTACK_FLAGS__SHOOT */
    [37] = "MAGICK REFLECTOR (MACHINE)", /* DM2: w0_1_1 reflector */
    [38] = "POWER CRYSTAL (MACHINE)",    /* DM2 */
    [39] = "EVIL FOUNTAIN",             /* DM1 variant */
    [40] = "SPIKED WALL/FLOOR SPIKES",  /* DM1: AI_ATTACK_FLAGS__PUSH_BACK */
    [41] = "SPECTRE (GHOST)",           /* DM1 ghost */
    [42] = "VEG MOUTH (DIGGER WORM)",   /* DM2 */
    [43] = "EVIL ATTACK MINION (EVIL)", /* DM2 */
    [44] = "AXEMAN",                    /* DM1 axeman */
    [45] = "CAVERN/STONE TABLE",        /* DM2 */
    [46] = "MUMMY",                     /* DM2: poison */
    [47] = "VOID DOOR (MACHINE)",       /* DM2 */
    [48] = "DARK VEXIRK (VEXIRK)",      /* DM2: w0_4_4 */
    [49] = "EVIL GUARD MINION (ENEMY)", /* DM2 */
    [50] = "SKELETON",                  /* DM1 skeleton */
    [51] = "AMPLIFIER (MACHINE)",        /* DM2: AI_ATTACK_FLAGS__FIREBALL */
    [52] = "WOLF",                      /* DM2 */
    [53] = "PIT GHOST (GHOST)",         /* DM1 variant: invisible (w0_a_a) */
    [54] = "DOOR GHOST (GHOST)",        /* DM1 variant */
    [55] = "VEXIRK KING (VEXIRK)",      /* DM2: elite boss */
    [56] = "? OBELISK LIKE ?",          /* DM2: unknown */
    [57] = "AXEMAN THIEF",              /* DM2 */
    [58] = "FLYING CHEST",              /* DM2 */
    [59] = "BARREL",                    /* DM2 */
    [60] = "PEDISTAL (PILLAR)",         /* DM2 */
    [61] = "GHOST",                     /* DM1 ghost */
    [62] = "EVIL ATTACK MINION (EVIL)", /* DM2: duplicate of index 43 */
};

/* AIDefinition table — zero-initialized stub.
 * Real implementation loads from GDAT via EXTENDED_LOAD_AI_DEFINITION().
 * Values populated from SKWIN/GDAT at SkWinCore.cpp:233-400.
 * Stub shows field offsets consistent with DME.h:1505-1545. */
static DM2_AIDefinition g_ai_table[DM2_AI_TABLE_SIZE];

int dm2_v1_creature_ai_index_count(void) {
    return DM2_AI_TABLE_SIZE;
}

const char *dm2_v1_creature_ai_name(int ai_index) {
    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return "?";
    return g_ai_names[ai_index] ? g_ai_names[ai_index] : "?";
}

const DM2_AIDefinition *dm2_v1_creature_ai_spec(int creature_type) {
    /* QUERY_CREATURE_AI_SPEC_FROM_TYPE at SkWinCore.cpp:2995
     * In extended mode: uses EXTENDED_LOAD_AI_DEFINITION() result
     * In fixed mode: uses hardcoded dAITableGenuine[]
     * Stub: index by creature_type (capped) */
    int idx = creature_type;
    if (idx < 0) idx = 0;
    if (idx >= DM2_AI_TABLE_SIZE) idx = DM2_AI_TABLE_SIZE - 1;
    (void)creature_type; /* suppress unused */
    return &g_ai_table[idx];
}

/* dm2_v1_creature_attacks_party — check if creature attacks at given distance
 * Source: SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM, DM2_CREATURE_ATTACKS_PARTY
 * Attack decision: based on AI_ATTACK_FLAGS and distance check.
 * b_1a command byte 0x17+ = fallback to CREATURE_ATTACKS_PARTY.
 * Melee range: distance == 1 tile. Ranged: AI_ATTACK_FLAGS__SHOOT.
 * Stub: non-mobile AIs (pillars, trees, objects, merchants) do not attack. */
int dm2_v1_creature_attacks_party(int ai_index, int distance) {
    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return 0;
    /* Stub: static/object AI indices do not attack (0,1,4,5,6,7,8,9,10,11,12,20,45,59,60) */
    if (ai_index == 0  || ai_index == 1  || ai_index == 4
     || ai_index == 5  || ai_index == 6  || ai_index == 7
     || ai_index == 8  || ai_index == 9  || ai_index == 10
     || ai_index == 11 || ai_index == 12 || ai_index == 20
     || ai_index == 33 || ai_index == 45 || ai_index == 59
     || ai_index == 60) {
        return 0;
    }
    return (distance <= 1 && distance >= 0) ? 1 : 0;
}

/* dm2_v1_creature_resolves_spell — map AI_ATTACK_FLAGS to spell effect
 * Source: SkWinCore.cpp:27038-27096 (OBJECT_EFFECT_* mapping)
 * Returns non-zero if creature has the spell-flag set.
 * Stub only — full resolution needs creature instance AttacksSpells field. */
int dm2_v1_creature_resolves_spell(int ai_index, uint16_t attack_flags) {
    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return 0;
    (void)ai_index;
    /* Stub: return 1 for spell-type flags if attack_flags has spell bits */
    if (attack_flags & (AI_ATTACK_FLAGS__FIREBALL |
                       AI_ATTACK_FLAGS__DISPELL  |
                       AI_ATTACK_FLAGS__LIGHTNING |
                       AI_ATTACK_FLAGS__POISON_CLOUD |
                       AI_ATTACK_FLAGS__POISON_BOLT |
                       AI_ATTACK_FLAGS__POISON_BLOB |
                       AI_ATTACK_FLAGS__PUSH_SPELL |
                       AI_ATTACK_FLAGS__PULL_SPELL)) {
        return 1;
    }
    return 0;
}

const char *dm2_v1_creature_source_evidence(void) {
    return
        "DM2 V1 Creature AI — Phase 6 source-lock\n"
        "ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: skproject/SKWIN/defines.h:705-716 (AI_ATTACK_FLAGS)\n"
        "Source: skproject/SKWIN/DME.h:1505-1560 (AIDefinition 36-byte struct, w0AIFlags bits)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:741-810 (getAIName, 64-entry table)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:2995 (QUERY_CREATURE_AI_SPEC_FROM_TYPE)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:16815-16936 (ALLOC_NEW_CREATURE, CREATE_MINION)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:415-437 (AI_ATTACK_FLAGS dispatch)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:27038-27096 (spell attack resolution)\n"
        "Source: skproject/SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM (CCM b_1a dispatch)\n"
        "Source: skproject/SKULLWIN/c_creature.h: b_1a, b_17 fields\n"
        "Source: skproject/SKULLWIN/c_ai.cpp: DM2_THINK_CREATURE (NPC planning tick)\n"
        "Source: skproject/SKWIN/SkGlobal.h:636 (CREATURE_AI_TAB_SIZE=64, MAXAI=255)\n";
}