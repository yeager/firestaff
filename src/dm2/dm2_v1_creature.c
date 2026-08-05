/* dm2_v1_creature.c — DM2 V1 Creature AI + Instance Lifecycle
 * Phase 6 source-lock (2026-05-26), Phase 5 followup (2026-05-31)
 * ReDMCSB: SKULL.ASM, skproject/SKWIN/SkWinCore.cpp, DME.h, defines.h
 * SKULLWIN/c_ai.cpp, c_creature.cpp, c_creature.h
 *
 * DM2 creature AI:
 *   - 64-entry AIDefinition table (vs 42 in DM1)
 *   - CCM command-dispatch via b_1a state register
 *   - AI_ATTACK_FLAGS for spell/attack type routing
 *   - 13 new creature types vs DM1 (companions, Dragoth, Vexirk, etc.)
 *   - Instance lifecycle: spawn / tick / death → drop + sound
 */

#include "dm2_v1_creature.h"
#include "dm2_v1_creature_tables.h"
#include "dm2_v1_ccm.h"
#include "dm2_v1_drops.h"
#include "dm2_v1_sound.h"
#include <string.h>

#define DM2_CREATURE_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH 0x0001u

/* ── dAITableGenuine — 62-entry AI definition table (hardcoded) ───────────
 * Source: skproject/SKWIN/SkWinCore.cpp:741-810 (getAIName)
 * Extended mode override: EXTENDED_LOAD_AI_DEFINITION() at SkWinCore.cpp:233-400
 *
 * Per-entry fields: w0AIFlags, ArmorClass, b3, BaseHP, AttackStrength,
 *                   PoisonDamage, Defense, b9x, w10, w12, AttacksSpells,
 *                   w16, w18, w20, w22, w24, w26, b28, Weight, w30, w32, b34, b35
 *
 * The structure layout is from DME.h:1505-1545.  The active table is reset
 * for each graphics session and populated through the original GDAT
 * EXTENDED_LOAD_AI_DEFINITION-compatible route below; absent rows remain
 * unavailable rather than becoming zero-valued creature behaviour.
 *
 * Companion/minion AI indices (13-18) are DM2-specific — no DM1 equivalent.
 * Boss indices: 30 (Lord Dragoth), 55 (Vexirk King), 51 (Amplifier).
 */

static const char *const g_ai_names[DM2_AI_TABLE_SIZE] = {
    [0]  = "TREE (PILLAR)",
    [1]  = "LABORATORY TABLE",
    [2]  = "????",
    [3]  = "BUSH",
    [4]  = "PILLARS / ROD (PILLAR)",
    [5]  = "STALAGMITE (PILLAR)",
    [6]  = "BOULDER",
    [7]  = "FOUNTAIN",
    [8]  = "OBELISKS / TOMBS",
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
    [23] = "CAVERN BAT (BAT)",          /* like DM1 bat */
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
    [35] = "TOWER BAT (BAT)",             /* like DM1 bat */
    [36] = "ARCHER GUARD",              /* DM2: AI_ATTACK_FLAGS__SHOOT */
    [37] = "MAGICK REFLECTOR (MACHINE)", /* DM2: w0_1_1 reflector */
    [38] = "POWER CRYSTAL (MACHINE)",    /* DM2 */
    [39] = "EVIL FOUNTAIN (FOUNTAIN)",  /* DM1 variant */
    [40] = "SPIKED WALL / FLOOR SPIKES", /* DM1: AI_ATTACK_FLAGS__PUSH_BACK */
    [41] = "SPECTRE (GHOST)",           /* DM1 ghost */
    [42] = "VEG MOUTH (DIGGER WORM)",   /* DM2 */
    [43] = "EVIL ATTACK MINION (EVIL)", /* DM2 */
    [44] = "AXEMAN",                    /* DM1 axeman */
    [45] = "CAVERN / STONE TABLE / WALL HOLE?", /* DM2 */
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

/* AIDefinition table — pre-populated from dAITableGenuine[62],
 * then overridden from GDAT via EXTENDED_LOAD_AI_DEFINITION().
 * Source: skproject/SKWIN/_4976_03a2.h + xcoreenh.cpp:61-217. */
static DM2_AIDefinition g_ai_table[DM2_AI_TABLE_SIZE];
static uint8_t g_ai_table_loaded[DM2_AI_TABLE_SIZE];
static uint8_t g_creature_ai_row[DM2_AI_TABLE_SIZE];
static uint8_t g_creature_ai_row_loaded[DM2_AI_TABLE_SIZE];
/* DM2-006: imported CREATURES drop words (fields 0x0A..0x14) per
 * creature type, plus the skproject LCG stream consumed by
 * DROP_CREATURE_POSSESSION count rolls (c_random.cpp DM2_RAND16). */
static uint16_t g_creature_drop_words[DM2_AI_TABLE_SIZE][DM2_DROP_SLOT_COUNT];
static uint8_t g_creature_drop_words_loaded[DM2_AI_TABLE_SIZE];
/* CREATURES word field 0x01 per creature type — the source's
 * table1d607e index (DM2_QUERY_GDAT_CREATURE_WORD_VALUE(type, 1),
 * c_creature.cpp:441 + 612, c_record.cpp:1387).  Captured by the AI
 * table loader alongside the drop words. */
static uint16_t g_creature_gdat_word1[DM2_AI_TABLE_SIZE];
static uint8_t g_creature_gdat_word1_loaded[DM2_AI_TABLE_SIZE];
static DM2_V1_DropRng g_drop_rng;
static DM2_V1_CCMProgram g_ccm_programs[DM2_AI_TABLE_SIZE];
static uint8_t g_ccm_program_loaded[DM2_AI_TABLE_SIZE];
static int g_ccm_program_count = 0;
static int g_ccm_program_field = -1;
static DM2_V1_CreatureFieldRuntime g_field_runtime;

int dm2_v1_creature_ai_index_count(void) {
    return DM2_AI_TABLE_SIZE;
}

const char *dm2_v1_creature_ai_name(int ai_index) {
    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return "?";
    return g_ai_names[ai_index] ? g_ai_names[ai_index] : "?";
}

const DM2_AIDefinition *dm2_v1_creature_ai_spec(int creature_type) {
    const DM2_AIDefinition *spec = NULL;

    /* SK-projects skcrture.cpp::QUERY_CREATURE_AI_SPEC_FROM_TYPE:
     * CREATURES[type] word 0x05 owns the AIDefinition row.  Never turn a
     * raw creature type into an AI-table index: without that source-owned
     * binding this runtime must not manufacture creature behavior. */
    if (dm2_v1_creature_ai_spec_def(creature_type, &spec) != 1) {
        return NULL;
    }
    return spec;
}

int dm2_v1_creature_ai_spec_flags(int creature_type, uint16_t *out_flags) {
    /* DM2_QUERY_CREATURE_AI_SPEC_FLAGS (c_record.cpp:1346-1349) over
     * DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD (c_record.cpp:1351-1354):
     * CREATURES[type & 0xff] word@5 -> AIDefinition row -> word@0.  The
     * loader captured both legs (g_creature_ai_row + g_ai_table) from the
     * proven EXTENDED_LOAD_AI_DEFINITION GDAT path; follow them and fail
     * closed for any type the session did not define. */
    int ai_row;

    if (out_flags == NULL) {
        return 0;
    }
    *out_flags = 0u;
    if (creature_type < 0 || creature_type >= DM2_AI_TABLE_SIZE) {
        return 0;
    }
    if (!g_creature_ai_row_loaded[creature_type]) {
        return 0;
    }
    ai_row = g_creature_ai_row[creature_type];
    if (ai_row < 0 || ai_row >= DM2_AI_TABLE_SIZE ||
        !g_ai_table_loaded[ai_row]) {
        return 0;
    }
    *out_flags = g_ai_table[ai_row].w0AIFlags;
    return 1;
}

int dm2_v1_creature_ai_spec_def(int creature_type,
                                const DM2_AIDefinition **out_def) {
    /* Same provenance chain as dm2_v1_creature_ai_spec_flags
     * (c_record.cpp:1351-1354); exposes the whole row for field
     * consumers like DM2_ATTACK_CREATURE's BaseHP probe
     * (c_creature.cpp:420-423). */
    int ai_row;

    if (out_def == NULL) {
        return 0;
    }
    *out_def = NULL;
    if (creature_type < 0 || creature_type >= DM2_AI_TABLE_SIZE) {
        return 0;
    }
    if (!g_creature_ai_row_loaded[creature_type]) {
        return 0;
    }
    ai_row = g_creature_ai_row[creature_type];
    if (ai_row < 0 || ai_row >= DM2_AI_TABLE_SIZE ||
        !g_ai_table_loaded[ai_row]) {
        return 0;
    }
    *out_def = &g_ai_table[ai_row];
    return 1;
}

int dm2_v1_creature_ai_base_hp(int creature_type, uint16_t *out_hp) {
    /* AIDefinition word@4 over the same provenance chain
     * (c_record.cpp:1351-1354); hookable adapter for the CAII module's
     * DM2_V1_CaiiWordValueFn providers. */
    const DM2_AIDefinition *def = NULL;

    if (out_hp == NULL) {
        return 0;
    }
    *out_hp = 0u;
    if (dm2_v1_creature_ai_spec_def(creature_type, &def) != 1) {
        return 0;
    }
    *out_hp = def->BaseHP;
    return 1;
}

int dm2_v1_creature_ai_defense(int creature_type, uint16_t *out_defense) {
    /* AIDefinition byte@8 over the same provenance chain
     * (c_record.cpp:1351-1354); provider for the combat module's
     * DM2_V1_CombatCreatureDefenseFn hook (c_engage.cpp melee defense). */
    const DM2_AIDefinition *def = NULL;

    if (out_defense == NULL) {
        return 0;
    }
    *out_defense = 0u;
    if (dm2_v1_creature_ai_spec_def(creature_type, &def) != 1) {
        return 0;
    }
    *out_defense = def->Defense;
    return 1;
}

int dm2_v1_creature_gdat_word1(int creature_type, uint16_t *out_word) {
    if (out_word == NULL) {
        return 0;
    }
    *out_word = 0u;
    if (creature_type < 0 || creature_type >= DM2_AI_TABLE_SIZE) {
        return 0;
    }
    if (!g_creature_gdat_word1_loaded[creature_type]) {
        return 0;
    }
    *out_word = g_creature_gdat_word1[creature_type];
    return 1;
}

void dm2_v1_creature_reset_ai_table(void) {
    int i;
    memset(g_ai_table, 0, sizeof(g_ai_table));
    memset(g_ai_table_loaded, 0, sizeof(g_ai_table_loaded));
    memset(g_creature_ai_row, 0, sizeof(g_creature_ai_row));
    memset(g_creature_ai_row_loaded, 0, sizeof(g_creature_ai_row_loaded));
    memset(g_creature_drop_words, 0, sizeof(g_creature_drop_words));
    memset(g_creature_drop_words_loaded, 0,
           sizeof(g_creature_drop_words_loaded));
    memset(g_creature_gdat_word1, 0, sizeof(g_creature_gdat_word1));
    memset(g_creature_gdat_word1_loaded, 0,
           sizeof(g_creature_gdat_word1_loaded));
    /* EXTENDED_LOAD_AI_DEFINITION (xcoreenh.cpp:69): always copies
     * dAITableGenuine first, then overrides from GDAT. */
    for (i = 0; i < DM2_AI_TABLE_GENUINE_SIZE && i < DM2_AI_TABLE_SIZE; ++i) {
        g_ai_table[i] = dm2_v1_ai_table_genuine[i];
    }
}

int dm2_v1_creature_drop_slots_loaded(int creature_type) {
    if (creature_type < 0 || creature_type >= DM2_AI_TABLE_SIZE) return 0;
    return g_creature_drop_words_loaded[creature_type] != 0;
}

uint16_t dm2_v1_creature_drop_slot_word(int creature_type, int slot) {
    if (creature_type < 0 || creature_type >= DM2_AI_TABLE_SIZE) return 0;
    if (slot < 0 || slot >= DM2_DROP_SLOT_COUNT) return 0;
    if (!g_creature_drop_words_loaded[creature_type]) return 0;
    return g_creature_drop_words[creature_type][slot];
}

void dm2_v1_creature_drop_rng_reset(void) {
    /* c_random.cpp:10-13 — c_randomdata::init sets random = 0. */
    dm2_v1_drops_rng_init(&g_drop_rng);
}

void dm2_v1_creature_reset_ccm_programs(void) {
    memset(g_ccm_programs, 0, sizeof(g_ccm_programs));
    memset(g_ccm_program_loaded, 0, sizeof(g_ccm_program_loaded));
    g_ccm_program_count = 0;
    g_ccm_program_field = -1;
}

static void dm2_v1_creature_decode_ai_spec(const uint8_t *raw,
                                           DM2_AIDefinition *out) {
    out->w0AIFlags = (uint16_t)raw[0] | ((uint16_t)raw[1] << 8);
    out->ArmorClass = raw[2];
    out->b3 = (int8_t)raw[3];
    out->BaseHP = (uint16_t)raw[4] | ((uint16_t)raw[5] << 8);
    out->AttackStrength = raw[6];
    out->PoisonDamage = raw[7];
    out->Defense = raw[8];
    out->b9x = raw[9];
    out->w10 = (uint16_t)raw[10] | ((uint16_t)raw[11] << 8);
    out->w12 = (uint16_t)raw[12] | ((uint16_t)raw[13] << 8);
    out->AttacksSpells = (uint16_t)raw[14] | ((uint16_t)raw[15] << 8);
    out->w16 = (uint16_t)raw[16] | ((uint16_t)raw[17] << 8);
    out->w18 = (uint16_t)raw[18] | ((uint16_t)raw[19] << 8);
    out->w20 = (uint16_t)raw[20] | ((uint16_t)raw[21] << 8);
    out->w22 = (uint16_t)raw[22] | ((uint16_t)raw[23] << 8);
    out->w24 = (uint16_t)raw[24] | ((uint16_t)raw[25] << 8);
    out->w26 = (uint16_t)raw[26] | ((uint16_t)raw[27] << 8);
    out->b28 = raw[28];
    out->Weight = raw[29];
    out->w30 = (uint16_t)raw[30] | ((uint16_t)raw[31] << 8);
    out->w32 = (uint16_t)raw[32] | ((uint16_t)raw[33] << 8);
    out->b34 = raw[34];
    out->b35 = raw[35];
}

int dm2_v1_creature_load_ai_table_from_gdat(const DM2_V1_AssetLoader *loader) {
    int loaded = 0;
    int creature_type;

    if (!loader || !loader->loaded) return -1;

    /* EXTENDED_LOAD_AI_DEFINITION repopulates the live table from the active
     * GDAT session.  A new session must not inherit an AI row that is absent
     * from its own CREATURE_AI category. */
    dm2_v1_creature_reset_ai_table();

    /* skproject/SKWINSPX/src/v4/skcrture.cpp:28-36 first reads
     * CREATURES[type] dtWordValue(0x05), then indexes the AIDefinition table
     * by that result.  Keep that indirection instead of treating creature
     * type as a CREATURE_AI row. */
    for (creature_type = 0; creature_type < DM2_AI_TABLE_SIZE;
         ++creature_type) {
        uint16_t ai_row;
        uint16_t hit_points = 0u;
        uint8_t raw[sizeof(DM2_AIDefinition)];
        int drop_slots_seen = 0;

        /* skproject/SKWINSPX/src/v4/skcrture.cpp:2092-2100
         * DROP_CREATURE_POSSESSION reads CREATURES[type] word fields
         * CREATURE_STAT_DROP_FIRST..LAST (0x0A..0x14) directly, without
         * the AI-row indirection.  Import them alongside the AI table so
         * the death path can resolve drops in source order. */
        for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
            uint16_t word = 0u;
            if (dm2_v1_asset_load_word_value(
                    loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
                    DM2_DROP_SLOT_FIRST + slot, &word)) {
                g_creature_drop_words[creature_type][slot] = word;
                drop_slots_seen = 1;
            }
        }
        g_creature_drop_words_loaded[creature_type] =
            (uint8_t)drop_slots_seen;

        /* CREATURES word field 0x01: the source's table1d607e index
         * (DM2_QUERY_GDAT_CREATURE_WORD_VALUE(type, 1),
         * c_creature.cpp:441 + 612, c_record.cpp:1387).  Captured
         * alongside the drop words — independent of the AI-row
         * indirection. */
        {
            uint16_t gdat_word1 = 0u;
            if (dm2_v1_asset_load_word_value(
                    loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
                    0x01, &gdat_word1)) {
                g_creature_gdat_word1[creature_type] = gdat_word1;
                g_creature_gdat_word1_loaded[creature_type] = 1;
            }
        }

        if (!dm2_v1_asset_load_word_value(
                loader, DM2_GDAT_CATEGORY_CREATURES, creature_type, 0x05,
                &ai_row) || ai_row >= DM2_AI_TABLE_SIZE) {
            continue;
        }
        /* SKProject SkWinCore.cpp::EXTENDED_LOAD_AI_DEFINITION does not
         * load a packed 36-byte record. Its real DM2 path queries the 36
         * dtWordValue fields individually, treating absent optional fields
         * as zero. Field four is BaseHP's low byte and is the source's
         * admission probe for an AI row. */
        if (!dm2_v1_asset_load_word_value(
                loader, DM2_GDAT_CATEGORY_CREATURE_AI, ai_row, 4,
                &hit_points) || hit_points == 0u) {
            continue;
        }
        memset(raw, 0, sizeof(raw));
        for (int field = 0; field < (int)sizeof(raw); ++field) {
            uint16_t value = 0u;
            if (dm2_v1_asset_load_word_value(
                    loader, DM2_GDAT_CATEGORY_CREATURE_AI, ai_row,
                    field, &value)) {
                raw[field] = (uint8_t)(value & 0xffu);
            }
        }
        if (!g_ai_table_loaded[ai_row]) {
            dm2_v1_creature_decode_ai_spec(raw, &g_ai_table[ai_row]);
            g_ai_table_loaded[ai_row] = 1;
        }
        g_creature_ai_row[creature_type] = (uint8_t)ai_row;
        g_creature_ai_row_loaded[creature_type] = 1;
        ++loaded;
    }

    return loaded;
}

int dm2_v1_creature_load_ccm_programs_from_gdat(const DM2_V1_AssetLoader *loader,
                                                int field) {
#if defined(FIRESTAFF_DM2_CREATURE_TESTING)
    int loaded = 0;
    int i;

    if (!loader || !loader->loaded || field < 0 || field > 0xff) return -1;
    dm2_v1_creature_reset_ccm_programs();

    /* skproject/SKULLWIN/c_creature.cpp DM2_PROCEED_CCM consumes a
     * per-creature command byte stream. Firestaff stores imported streams
     * beside the AI table, addressed by the same CREATURE_AI category index.
     * Field 0 remains the 36-byte AIDefinition row; callers pass the GDAT
     * field that contains the command byteprogram for the current asset set. */
    for (i = 0; i < DM2_AI_TABLE_SIZE; ++i) {
        size_t raw_size = 0;
        const uint8_t *raw = dm2_v1_asset_load_sized(
            loader, DM2_GDAT_CATEGORY_CREATURE_AI, i, field, &raw_size);
        DM2_V1_CCMProgram program;
        if (!raw || raw_size == 0) continue;
        if (dm2_v1_ccm_decode_program(raw, raw_size, &program) !=
            (int)DM2_CCM_RESULT_OK) {
            continue;
        }
        g_ccm_programs[i] = program;
        g_ccm_program_loaded[i] = 1;
        ++loaded;
    }

    g_ccm_program_count = loaded;
    g_ccm_program_field = loaded > 0 ? field : -1;
    return loaded;
#else
    (void)loader;
    (void)field;
    /* The source does not establish a CREATURE_AI field as a CCM bytecode
     * container.  Pattern-decoding raw GDAT fields is a test diagnostic, not
     * an admissible player-session data path. */
    dm2_v1_creature_reset_ccm_programs();
    return 0;
#endif
}

int dm2_v1_creature_load_ccm_programs_from_gdat_auto(
    const DM2_V1_AssetLoader *loader,
    int *out_field) {
    if (out_field) *out_field = -1;
#if defined(FIRESTAFF_DM2_CREATURE_TESTING)
    static const int k_fields[] = {
        1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        0x10, 0x11, 0x12, 0x18, 0x19, 0x1a, 0x20
    };
    int i;

    if (!loader || !loader->loaded) return -1;

    /* skproject/SKWIN/SkWinCore.cpp EXTENDED_LOAD_AI_DEFINITION (~233-400)
     * and QUERY_CREATURE_AI_SPEC_FROM_TYPE (~2995) bind CREATURE_AI GDAT rows
     * to runtime AI state; SKULLWIN/c_creature.cpp DM2_PROCEED_CCM consumes
     * the adjacent per-creature CCM stream. Different PC GDAT variants encode
     * that stream in non-zero fields, so boot probes candidate fields and keeps
     * the first byteprogram set that decodes cleanly. */
    for (i = 0; i < (int)(sizeof(k_fields) / sizeof(k_fields[0])); ++i) {
        int loaded = dm2_v1_creature_load_ccm_programs_from_gdat(
            loader, k_fields[i]);
        if (loaded > 0) {
            if (out_field) *out_field = k_fields[i];
            return loaded;
        }
    }

    dm2_v1_creature_reset_ccm_programs();
    return 0;
#else
    (void)loader;
    /* DM2_PROCEED_CCM reads b_1a from the live creature/CCM record; it does
     * not probe arbitrary CREATURE_AI fields until a byte sequence decodes.
     * The source field owner has not been recovered, therefore production
     * cannot select one by pattern matching.  Explicit test imports retain
     * coverage for the decoder only. */
    dm2_v1_creature_reset_ccm_programs();
    return 0;
#endif
}

int dm2_v1_creature_loaded_ccm_program_count(void) {
    return g_ccm_program_count;
}

int dm2_v1_creature_loaded_ccm_program_field(void) {
    return g_ccm_program_field;
}

void dm2_v1_creature_set_field_runtime(
    const DM2_V1_CreatureFieldRuntime *runtime) {
    if (runtime) {
        g_field_runtime = *runtime;
    } else {
        memset(&g_field_runtime, 0, sizeof(g_field_runtime));
    }
}

void dm2_v1_creature_reset_field_runtime(void) {
    memset(&g_field_runtime, 0, sizeof(g_field_runtime));
}

int dm2_v1_creature_door_open_pct_from_state(int door_state) {
    if (door_state == 5) return 100;
    if (door_state < 0) door_state = 0;
    if (door_state > 4) door_state = 4;
    return (4 - door_state) * 25;
}

#if 0 /* Former synthetic CCM walk bridge; no production caller. */
static int dm2_v1_creature_door_blocks_creature(int door_state,
                                                uint16_t door_attributes,
                                                int creature_nonmaterial) {
    if (creature_nonmaterial) return 0;
    if (door_state != 3 && door_state != 4) return 0;
    if ((door_attributes & DM2_CREATURE_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH) != 0) {
        return 0;
    }
    return 1;
}
#endif

static int dm2_v1_creature_attack_flags_are_ranged(uint16_t flags) {
    return (flags & (AI_ATTACK_FLAGS__SHOOT |
                     AI_ATTACK_FLAGS__FIREBALL |
                     AI_ATTACK_FLAGS__DISPELL |
                     AI_ATTACK_FLAGS__LIGHTNING |
                     AI_ATTACK_FLAGS__POISON_CLOUD |
                     AI_ATTACK_FLAGS__POISON_BOLT |
                     AI_ATTACK_FLAGS__POISON_BLOB |
                     AI_ATTACK_FLAGS__PUSH_SPELL |
                     AI_ATTACK_FLAGS__PULL_SPELL)) != 0;
}

/* dm2_v1_creature_attacks_party — check if creature attacks at given distance
 * Source: SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM, DM2_CREATURE_ATTACKS_PARTY
 * Attack decision: based on AI_ATTACK_FLAGS and distance check.
 * b_1a command byte 0x17+ = fallback to CREATURE_ATTACKS_PARTY.
 * Melee range: distance == 1 tile. Ranged: AI_ATTACK_FLAGS__SHOOT.
 * GDAT path: use AIDefinition.AttacksSpells loaded by
 * EXTENDED_LOAD_AI_DEFINITION (SkWinCore.cpp:233-400), with static
 * objects suppressed by w0AIFlags. An unimported row is rejected; it never
 * receives a data-free attack classification. */
int dm2_v1_creature_attacks_party(int ai_index, int distance) {
    const DM2_AIDefinition *spec;
    uint16_t attacks;

    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return 0;
    if (distance < 0) return 0;
    spec = dm2_v1_creature_ai_spec(ai_index);
    if (!spec || (spec->w0AIFlags & DM2_AIFLAG_STATIC) != 0) return 0;
    attacks = spec->AttacksSpells;
    if ((attacks & AI_ATTACK_FLAGS__MELEE) != 0 && distance <= 1) return 1;
    return dm2_v1_creature_attack_flags_are_ranged(attacks) && distance <= 6;
}

/* dm2_v1_creature_resolves_spell — map AI_ATTACK_FLAGS to spell effect
 * Source: SkWinCore.cpp:27038-27096 (OBJECT_EFFECT_* mapping)
 * Returns non-zero if creature has the requested spell-flag set.
 * GDAT path intersects the requested flags with the imported
 * AIDefinition.AttacksSpells. An unimported row is rejected rather than
 * receiving a flag-only spell classification. */
int dm2_v1_creature_resolves_spell(int ai_index, uint16_t attack_flags) {
    const DM2_AIDefinition *spec;

    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return 0;
    if (!dm2_v1_creature_attack_flags_are_ranged(attack_flags)) return 0;
    spec = dm2_v1_creature_ai_spec(ai_index);
    return spec != NULL && (spec->AttacksSpells & attack_flags) != 0;
}

/* ── Creature instance pool ──────────────────────────────────────────────
 * Source: SkWinCore.cpp:16815-16936 (ALLOC_NEW_CREATURE)
 *         SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM
 *         SKULLWIN/c_ai.cpp: DM2_THINK_CREATURE
 *
 * Instance pool: DM2_MAX_CREATURE_INSTANCES=64 per dungeon map.
 * HP scaling: BaseHP * healthMultiplier / 8 (healthMultiplier from GDAT). */

static DM2_V1_CreatureInstance g_creature_pool[DM2_MAX_CREATURE_INSTANCES];
static int g_next_instance_id = 0;
static int g_tick_counter = 0;
static DM2_V1_CreatureCCMTickObserver g_last_ccm_tick;

/* ── Death/drop observer (Phase 5 followup, 2026-06-22) ────────────────
 * Captures the most recent death_check event so the CTest gate can assert
 * the loot-state contract deterministically (item_id, count, slot, AI).
 * Initialized to all-zero / dropped=0 / count=0 so a fresh module load
 * before any death is observable as "no death yet" rather than garbage. */
static DM2_V1_CreatureDeathDropObserver g_last_death_drop;
static int g_death_observer_count = 0;

static void dm2_v1_creature_write_render_state(DM2_V1_CreatureInstance *c,
                                               int advance_tick)
{
    if (!c) return;
    if (advance_tick) {
        ++c->animation_tick;
    }
    /* skproject/SKWIN/SkWinCore.cpp DRAW_MAP_CHIP consumes the creature's
     * current animation base after DM2_PROCEED_CCM has written b_1a.  Keep
     * that frame on the live record, so a later viewport pass cannot replace
     * it with a renderer-local clock when the GDAT map-chip atlas is ready. */
    if (c->b_1a == DM2_CCM_CREATURE_ATTACKS_PARTY) {
        c->animation_frame = 2;
    } else if (c->attack_cooldown > 0) {
        c->animation_frame = 1;
    } else {
        c->animation_frame = (uint8_t)(c->animation_tick & 1u);
    }
    ++c->render_revision;
}

/* dm2_v1_creature_spawn — spawn a creature instance.
 * Source: SkWinCore.cpp:16815 — ALLOC_NEW_CREATURE(type, mult, dir, x, y)
 * healthMultiplier: 0=default (8), 1–16 scale HP. DM2_CREATURE_SPAWN_MAX=64. */
int dm2_v1_creature_spawn(int ai_index, int world_x, int world_y,
                          int map_index, int direction, int health_multiplier) {
#ifndef FIRESTAFF_DM2_CREATURE_TESTING
    (void)ai_index;
    (void)world_x;
    (void)world_y;
    (void)map_index;
    (void)direction;
    (void)health_multiplier;
    /* SKProject ALLOC_NEW_CREATURE (skcrture.cpp:6380-6430) allocates a
     * DB4 record, applies the current map index, links it into the loaded
     * record chain, and derives health from the source AI record plus RNG.
     * This standalone helper owns none of those inputs; accepting host
     * coordinates and multiplier would manufacture a live creature. */
    return -1;
#else
    const DM2_AIDefinition *spec;
    int slot = -1;
    int mult;
    int hp;

    spec = dm2_v1_creature_ai_spec(ai_index);
    if (spec == NULL || ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) {
        return -1;
    }
    for (int i = 0; i < DM2_MAX_CREATURE_INSTANCES; i++) {
        if (!g_creature_pool[i].alive) { slot = i; break; }
    }
    if (slot < 0) return -1;

    DM2_V1_CreatureInstance *c = &g_creature_pool[slot];
    memset(c, 0, sizeof(*c));
    c->instance_id = g_next_instance_id++;
    c->ai_index    = ai_index;
    c->world_x     = world_x;
    c->world_y     = world_y;
    c->map_index   = map_index;
    c->direction   = direction & 3;
    c->alive       = 1;
    c->is_visible  = 1;
    c->b_1a        = DM2_CCM_WALK_NOW;
    c->b_17        = 0;
    c->gdat_animation_sequence = 0u;
    c->gdat_animation_info = 0xffffu;
    c->attack_cooldown = 0;
    c->poison_ticks    = 0;
    dm2_v1_creature_write_render_state(c, 0);

    mult = (health_multiplier > 0) ? health_multiplier : 8;
    hp = (int)spec->BaseHP * mult / 8;
    if (hp <= 0) return -1;
    c->hp_max     = hp;
    c->hp_current = c->hp_max;
    ++c->render_revision;

    return slot;
#endif
}

int dm2_v1_creature_count(void) {
    int n = 0;
    for (int i = 0; i < DM2_MAX_CREATURE_INSTANCES; i++) {
        if (g_creature_pool[i].alive) n++;
    }
    return n;
}

int dm2_v1_creature_at(int world_x, int world_y, int map_index) {
    for (int i = 0; i < DM2_MAX_CREATURE_INSTANCES; i++) {
        DM2_V1_CreatureInstance *c = &g_creature_pool[i];
        if (c->alive && c->world_x == world_x && c->world_y == world_y
            && c->map_index == map_index) {
            return i;
        }
    }
    return -1;
}

int dm2_v1_creature_deal_damage(int instance_id, int damage) {
    if (instance_id < 0 || instance_id >= DM2_MAX_CREATURE_INSTANCES) return -1;
    DM2_V1_CreatureInstance *c = &g_creature_pool[instance_id];
    if (!c->alive) return -1;
    c->hp_current -= damage;
    if (c->hp_current < 0) c->hp_current = 0;
    ++c->render_revision;
    return c->hp_current;
}

int dm2_v1_creature_instance_hp(int instance_id) {
    if (instance_id < 0 || instance_id >= DM2_MAX_CREATURE_INSTANCES) return -1;
    return g_creature_pool[instance_id].hp_current;
}

int dm2_v1_creature_instance_ai(int instance_id) {
    if (instance_id < 0 || instance_id >= DM2_MAX_CREATURE_INSTANCES) return -1;
    return g_creature_pool[instance_id].ai_index;
}

const DM2_V1_CreatureInstance *dm2_v1_creature_get_instance(int instance_id) {
    if (instance_id < 0 || instance_id >= DM2_MAX_CREATURE_INSTANCES) return NULL;
    return &g_creature_pool[instance_id];
}

int dm2_v1_creature_set_gdat_animation_state(int instance_id,
                                              uint16_t sequence,
                                              uint16_t info)
{
#ifndef FIRESTAFF_DM2_CREATURE_TESTING
    (void)instance_id;
    (void)sequence;
    (void)info;
    /* SKProject skcrture.cpp:1595-1658 derives both words while processing
     * the current creature's CCM/GDAT animation frame. A standalone caller
     * has neither owner nor receipt and therefore cannot alter live state. */
    return -1;
#else
    DM2_V1_CreatureInstance *instance;

    if (instance_id < 0 || instance_id >= DM2_MAX_CREATURE_INSTANCES) {
        return -1;
    }
    instance = &g_creature_pool[instance_id];
    if (!instance->alive) return -1;
    instance->gdat_animation_sequence = sequence;
    instance->gdat_animation_info = info;
    ++instance->render_revision;
    return 0;
#endif
}

int dm2_v1_creature_export_live_state(DM2_V1_CreatureLiveState *out_state) {
    if (!out_state) return -1;
    memcpy(out_state->instances, g_creature_pool, sizeof(g_creature_pool));
    out_state->next_instance_id = g_next_instance_id;
    out_state->tick_counter = g_tick_counter;
    return 0;
}

int dm2_v1_creature_live_state_valid(const DM2_V1_CreatureLiveState *state) {
    if (!state || state->next_instance_id < 0 || state->tick_counter < 0) {
        return -1;
    }
    for (int i = 0; i < DM2_MAX_CREATURE_INSTANCES; ++i) {
        const DM2_V1_CreatureInstance *c = &state->instances[i];
        if ((c->alive != 0 && c->alive != 1) ||
            (c->is_visible != 0 && c->is_visible != 1) ||
            c->direction < 0 || c->direction > 3 ||
            c->ai_index < 0 || c->ai_index >= DM2_AI_TABLE_SIZE) {
            return -1;
        }
    }
    return 0;
}

int dm2_v1_creature_restore_live_state(const DM2_V1_CreatureLiveState *state) {
    if (dm2_v1_creature_live_state_valid(state) != 0) {
        return -1;
    }
    memcpy(g_creature_pool, state->instances, sizeof(g_creature_pool));
    g_next_instance_id = state->next_instance_id;
    g_tick_counter = state->tick_counter;
    dm2_v1_creature_reset_ccm_tick_observer();
    return 0;
}

/* ── Test-only API ────────────────────────────────────────────────
 * Compiled in only when FIRESTAFF_DM2_CREATURE_TESTING=1.  Lets the
 * collision gate inject a synthetic AI definition (with REFLECTOR /
 * ABSORBS_MISSILE / NONMATERIAL / TURNS_MISSILE bits set) without
 * depending on GDAT-loaded real AI table values.
 *
 * The override is a full-struct copy; the slot becomes a clone of the
 * caller's spec.  clear_ai_overrides() restores the zero-init default
 * so subsequent tests see a clean table. */
#ifdef FIRESTAFF_DM2_CREATURE_TESTING
void dm2_v1_creature_test_set_ai_spec(int ai_index,
                                       const DM2_AIDefinition *spec) {
    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return;
    if (!spec) return;
    g_ai_table[ai_index] = *spec;
    g_ai_table_loaded[ai_index] = 1;
    g_creature_ai_row[ai_index] = (uint8_t)ai_index;
    g_creature_ai_row_loaded[ai_index] = 1;
}

void dm2_v1_creature_test_clear_ai_overrides(void) {
    dm2_v1_creature_reset_ai_table();
}

void dm2_v1_creature_test_set_ccm_state(int instance_id,
                                        uint8_t b_1a,
                                        uint8_t b_17,
                                        int target_x,
                                        int target_y) {
    if (instance_id < 0 || instance_id >= DM2_MAX_CREATURE_INSTANCES) return;
    if (!g_creature_pool[instance_id].alive) return;
    g_creature_pool[instance_id].b_1a = b_1a;
    g_creature_pool[instance_id].b_17 = b_17;
    g_creature_pool[instance_id].target_x = target_x;
    g_creature_pool[instance_id].target_y = target_y;
}

void dm2_v1_creature_test_reset_instances(void) {
    memset(g_creature_pool, 0, sizeof(g_creature_pool));
    g_next_instance_id = 0;
    g_tick_counter = 0;
    dm2_v1_creature_reset_ccm_tick_observer();
}

void dm2_v1_creature_test_set_drop_slots(int creature_type,
                                         const uint16_t slot_words[11]) {
    if (creature_type < 0 || creature_type >= DM2_AI_TABLE_SIZE) return;
    if (!slot_words) return;
    for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
        g_creature_drop_words[creature_type][slot] = slot_words[slot];
    }
    g_creature_drop_words_loaded[creature_type] = 1;
}
#endif /* FIRESTAFF_DM2_CREATURE_TESTING */

/* dm2_v1_creature_death_check — death → drop + spatial sound.
 * Source: SKULLWIN/c_creature.cpp, SKULLWIN/c_sound.cpp, SKWin.GDAT2.InternalCodes.txt */
void dm2_v1_creature_death_check(int instance_id) {
    if (instance_id < 0 || instance_id >= DM2_MAX_CREATURE_INSTANCES) return;
    DM2_V1_CreatureInstance *c = &g_creature_pool[instance_id];
    if (!c->alive || c->hp_current > 0) return;

    /* Snapshot creature state for the observer before we mutate. */
    int snap_ai    = c->ai_index;
    int snap_x     = c->world_x;
    int snap_y     = c->world_y;
    int snap_map   = c->map_index;

    c->alive = 0;
    ++c->render_revision;

    /* SOUND_CREATURE_DEATH (constant) positional at creature position.
     * Source: SKULLWIN/c_sound.cpp death_sfx dispatch */
    (void)dm2_v1_sound_play_positional(DM2_SOUND_CREATURE_DEATH,
                                        c->world_x, c->world_y,
                                        c->world_x, c->world_y);

    /* DM2-006: when the GDAT CREATURES drop words were imported for this
     * creature type, resolve them in source order
     * (skcrture.cpp:2092-2118 DROP_CREATURE_POSSESSION,
     * CREATURE_STAT_DROP_FIRST..LAST 0x0A..0x14) instead of the fixed
     * fallback.  Item-record creation stays fail-closed until
     * ALLOC_NEW_DBITEM is bound; the observer receipts the source-ordered
     * resolution. */
    int source_ordered = 0;
    int source_slots_admitted = 0;
    int source_total_items = 0;
    int source_first_item = 0;
    if (dm2_v1_creature_drop_slots_loaded(snap_ai)) {
        DM2_V1_DropSlotReceipt receipts[DM2_DROP_SLOT_COUNT];
        source_slots_admitted = dm2_v1_drops_resolve_source_slots(
            g_creature_drop_words[snap_ai], &g_drop_rng, receipts,
            &source_total_items);
        source_ordered = 1;
        for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
            if (receipts[slot].admitted) {
                source_first_item = receipts[slot].item_id;
                break;
            }
        }
    }

    /* Populate the observer so the CTest gate can assert the loot-state
     * contract deterministically.  Without imported source words this is
     * a recorded death with no generated drops: DROP_CREATURE_POSSESSION
     * has no creature-type special case or fabricated fallback. */
    memset(&g_last_death_drop, 0, sizeof(g_last_death_drop));
    g_last_death_drop.instance_id = instance_id;
    g_last_death_drop.ai_index    = snap_ai;
    g_last_death_drop.world_x     = snap_x;
    g_last_death_drop.world_y     = snap_y;
    g_last_death_drop.map_index   = snap_map;
    if (source_ordered) {
        g_last_death_drop.source_ordered = 1;
        g_last_death_drop.source_slots_admitted = source_slots_admitted;
        g_last_death_drop.source_total_items = source_total_items;
        if (source_total_items > 0 && source_first_item != 0) {
            g_last_death_drop.dropped = 1;
            g_last_death_drop.item_id = source_first_item;
            g_last_death_drop.count   = source_total_items;
        }
    }
    g_death_observer_count++;
}

/* ── Death/drop observer accessors ─────────────────────────────────────── */

int dm2_v1_creature_last_death_drop(DM2_V1_CreatureDeathDropObserver *out) {
    if (!out) return 0;
    if (g_death_observer_count <= 0) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    *out = g_last_death_drop;
    return 1;
}

int dm2_v1_creature_death_observer_count(void) {
    return g_death_observer_count;
}

void dm2_v1_creature_reset_death_observer(void) {
    memset(&g_last_death_drop, 0, sizeof(g_last_death_drop));
    g_death_observer_count = 0;
}

int dm2_v1_creature_last_ccm_tick(DM2_V1_CreatureCCMTickObserver *out) {
    if (!out) return 0;
    if (!g_last_ccm_tick.valid) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    *out = g_last_ccm_tick;
    return 1;
}

void dm2_v1_creature_reset_ccm_tick_observer(void) {
    memset(&g_last_ccm_tick, 0, sizeof(g_last_ccm_tick));
}

/* This former bridge is retained as source research only.  It reconstructed
 * an executable command from reduced CreatureInstance fields and converted
 * private interpreter flags into world mutations.  SKProject instead owns
 * the stream in live DB4/CAII records and invokes handlers with dungeon,
 * timer and party callbacks.  Do not compile the reconstruction into a
 * player build. */
#if 0
static int dm2_v1_creature_ccm_argc(int opcode) {
    const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(opcode);
    return def ? def->arg_count : 0;
}

static void dm2_v1_creature_make_ccm_args(const DM2_V1_CreatureInstance *c,
                                          int opcode,
                                          int args[DM2_CCM_MAX_PROGRAM_ARGS]) {
    memset(args, 0, sizeof(int) * DM2_CCM_MAX_PROGRAM_ARGS);
    if (!c) return;

    /* skproject/SKULLWIN/c_creature.cpp DM2_PROCEED_CCM dispatches b_1a
     * with operands taken from creature context registers. Firestaff still
     * imports only b_1a/b_17 at runtime, so this bridge maps the available
     * instance fields into the CCM interpreter until GDAT byteprogram
     * streams are wired from real assets. */
    switch (opcode) {
        /* 2026-07-19 DM2-005 follow-up: opcode constants now carry the
         * source b_1a command bytes (c_creature.cpp:2930-3212). */
        case DM2_CCM_OP_ATTACKS_PARTY:
        case DM2_CCM_OP_ATTACKS_PARTY_26:
        case DM2_CCM_OP_ROTATES_TARGET_CREATURE:
        case DM2_CCM_OP_STEAL_FROM_CHAMPION:
        case DM2_CCM_OP_JUMPS:
        case DM2_CCM_OP_PUTS_DOWN_ITEM:
        case DM2_CCM_OP_TAKES_ITEM:
        case DM2_CCM_OP_PLACE_MERCHANDISE:
        case DM2_CCM_OP_TAKE_MERCHANDISE:
        case DM2_CCM_OP_KILL_ON_TIMER_POSITION:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON_3E:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON_3F:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON_40:
            args[0] = c->b_17;
            break;
        case DM2_CCM_OP_SHOOT_ITEM:
        case DM2_CCM_OP_SHOOT_ITEM_ALT:
            args[0] = c->b_17;
            args[1] = c->direction;
            break;
        case DM2_CCM_OP_CAST_SPELL:
        case DM2_CCM_OP_CAST_SPELL_28:
            args[0] = c->b_17;
            args[1] = c->target_x;
            args[2] = c->target_y;
            break;
        default:
            break;
    }
}

static const DM2_V1_CCMProgramOp *dm2_v1_creature_imported_ccm_op(
    const DM2_V1_CreatureInstance *c,
    int *out_pc) {
    const DM2_V1_CCMProgram *program;
    int pc;

    if (out_pc) *out_pc = -1;
    if (!c || c->ai_index < 0 || c->ai_index >= DM2_AI_TABLE_SIZE) return NULL;
    if (!g_ccm_program_loaded[c->ai_index]) return NULL;
    program = &g_ccm_programs[c->ai_index];
    if (program->count <= 0 || program->count > DM2_CCM_MAX_PROGRAM_OPS) return NULL;
    pc = (int)c->b_17;
    if (pc < 0 || pc >= program->count) pc = 0;
    if (out_pc) *out_pc = pc;
    return &program->ops[pc];
}

static void dm2_v1_creature_run_ccm_tick(DM2_V1_CreatureInstance *c,
                                         int slot) {
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    DM2_V1_CCMState state;
    int args[DM2_CCM_MAX_PROGRAM_ARGS];
    int opcode;
    int argc;
    int before_cooldown;
    int imported_pc = -1;
    const DM2_V1_CCMProgramOp *imported_op;
    int rc;
    int door_valid = 0;
    int door_state = -1;
    uint16_t door_attributes = 0;
    int door_x = 0;
    int door_y = 0;
    int door_blocks = 0;

    if (!c) return;
    opcode = (int)c->b_1a;
    before_cooldown = c->attack_cooldown;
    door_x = c->world_x + dx[c->direction & 3];
    door_y = c->world_y + dy[c->direction & 3];
    if (g_field_runtime.read_door &&
        g_field_runtime.read_door(g_field_runtime.user,
                                  c->map_index,
                                  door_x,
                                  door_y,
                                  &door_state,
                                  &door_attributes) == 1) {
        const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(c->ai_index);
        int nonmaterial = spec != NULL &&
                          (spec->w0AIFlags & DM2_AIFLAG_NONMATERIAL) != 0;
        door_valid = 1;
        door_blocks = dm2_v1_creature_door_blocks_creature(
            door_state, door_attributes, nonmaterial != 0);
    }
    imported_op = dm2_v1_creature_imported_ccm_op(c, &imported_pc);
    if (!imported_op) {
#if !defined(FIRESTAFF_DM2_CREATURE_TESTING)
        /* The source has no local b_17 -> operand reconstruction: its CCM
         * command and operands belong to the live imported record.  Do not
         * turn the reduced CreatureInstance fields into an executable CCM
         * action when that record is absent. */
        memset(&g_last_ccm_tick, 0, sizeof(g_last_ccm_tick));
        g_last_ccm_tick.valid = 1;
        g_last_ccm_tick.instance_id = slot;
        g_last_ccm_tick.ai_index = c->ai_index;
        g_last_ccm_tick.before_b_1a = opcode;
        g_last_ccm_tick.ccm_result = (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
        g_last_ccm_tick.program_pc_before = -1;
        g_last_ccm_tick.program_pc_after = -1;
        return;
#endif
    } else {
        opcode = (int)imported_op->opcode;
    }
    dm2_v1_ccm_init_state(&state);
    state.target_id = c->instance_id;
    state.target_x = c->target_x;
    state.target_y = c->target_y;
    state.target_level = c->map_index;
    if (imported_op) {
        int i;
        argc = imported_op->arg_count;
        memset(args, 0, sizeof(args));
        for (i = 0; i < argc && i < DM2_CCM_MAX_PROGRAM_ARGS; ++i) {
            args[i] = imported_op->args[i];
        }
    } else {
        /* Explicit fixture-only route; normal builds returned above because
         * no source-owned CCM program and operand record was available. */
        dm2_v1_creature_make_ccm_args(c, opcode, args);
        argc = dm2_v1_creature_ccm_argc(opcode);
    }
    rc = dm2_v1_ccm_step(&state, opcode, args, argc, g_tick_counter);

    memset(&g_last_ccm_tick, 0, sizeof(g_last_ccm_tick));
    g_last_ccm_tick.valid = 1;
    g_last_ccm_tick.instance_id = slot;
    g_last_ccm_tick.ai_index = c->ai_index;
    g_last_ccm_tick.before_b_1a = opcode;
    g_last_ccm_tick.ccm_opcode = state.last_opcode;
    g_last_ccm_tick.ccm_result = rc;
    g_last_ccm_tick.ccm_flag_attack_party = state.flags[9];
    g_last_ccm_tick.ccm_flag_walk = state.flags[0];
    g_last_ccm_tick.ccm_flag_steal = state.flags[3];
    g_last_ccm_tick.ccm_flag_shoot = state.flags[5];
    g_last_ccm_tick.ccm_flag_cast_spell = state.flags[8];
    g_last_ccm_tick.ccm_flag_explode_or_summon = state.flags[10];
    g_last_ccm_tick.ccm_flag_path = state.flags[11];
    g_last_ccm_tick.ccm_flag_rotate = state.flags[12];
    g_last_ccm_tick.ccm_flag_special = state.flags[13];
    g_last_ccm_tick.ccm_flag_item = state.flags[14] || state.flags[15];
    g_last_ccm_tick.ccm_requested_state = state.next_state;
    g_last_ccm_tick.ccm_target_id = state.target_id;
    g_last_ccm_tick.ccm_target_x = state.target_x;
    g_last_ccm_tick.ccm_target_y = state.target_y;
    g_last_ccm_tick.ccm_stack_top = state.stack_top;
    if (state.stack_top > 0) g_last_ccm_tick.ccm_stack_value0 = state.stack[0];
    if (state.stack_top > 1) g_last_ccm_tick.ccm_stack_value1 = state.stack[1];
    g_last_ccm_tick.imported_program = imported_op ? 1 : 0;
    g_last_ccm_tick.program_pc_before = imported_pc;
    g_last_ccm_tick.field_door_valid = door_valid;
    g_last_ccm_tick.field_door_x = door_x;
    g_last_ccm_tick.field_door_y = door_y;
    g_last_ccm_tick.field_door_state = door_state;
    g_last_ccm_tick.field_door_open_pct =
        door_valid ? dm2_v1_creature_door_open_pct_from_state(door_state) : -1;
    g_last_ccm_tick.field_blocks_movement = door_blocks;
    g_last_ccm_tick.direction_before = c->direction;
    g_last_ccm_tick.attack_cooldown_before = before_cooldown;

    if (rc == (int)DM2_CCM_RESULT_OK) {
        if (state.flags[9]) {
            c->b_1a = DM2_CCM_WALK_NOW;
            c->attack_cooldown = 18;
        } else if (state.flags[5] || state.flags[8] || state.flags[10]) {
            c->b_1a = DM2_CCM_WALK_NOW;
            c->attack_cooldown = 18;
        } else if (state.flags[3] || state.flags[14] || state.flags[15]) {
            c->b_1a = DM2_CCM_WALK_NOW;
            c->attack_cooldown = 9;
        } else {
            if (state.flags[12]) {
                c->direction = state.target_id & 3;
            }
            if (state.next_state >= 0) {
                c->b_1a = (uint8_t)state.next_state;
            }
            if ((state.flags[0] || state.flags[11]) && before_cooldown == 0 &&
                   dm2_v1_creature_attacks_party(c->ai_index, 1)) {
                c->b_1a = DM2_CCM_CREATURE_ATTACKS_PARTY;
            } else if ((state.flags[0] || state.flags[11]) && before_cooldown == 0 &&
                   (!door_valid || !door_blocks)) {
            /* skproject/SKULLWIN/c_ai.cpp DM2_THINK_CREATURE advances the
             * creature's map cell after DM2_PROCEED_CCM leaves a walk state,
             * while GROUP.C door tests gate closed DB0 door cells.  The
             * Firestaff field-runtime bridge keeps the same order: CCM first,
             * then write back the real world_x/world_y when the door state
             * allows movement. */
            c->world_x = door_x;
            c->world_y = door_y;
            g_last_ccm_tick.field_moved = 1;
            g_last_ccm_tick.field_move_distance = 1;
            }
        }
    }

    if (imported_op) {
        int next_pc = imported_pc + 1;
        if (rc != (int)DM2_CCM_RESULT_OK ||
            next_pc >= g_ccm_programs[c->ai_index].count) {
            next_pc = 0;
        }
        c->b_17 = (uint8_t)next_pc;
        g_last_ccm_tick.program_pc_after = next_pc;
    } else {
        g_last_ccm_tick.program_pc_after = -1;
    }

    g_last_ccm_tick.after_b_1a = c->b_1a;
    g_last_ccm_tick.direction_after = c->direction;
    g_last_ccm_tick.attack_cooldown_after = c->attack_cooldown;
}

#endif

static void dm2_v1_creature_run_ccm_tick(DM2_V1_CreatureInstance *c,
                                         int slot) {
    if (!c) return;

    /* Missing source-owned DB4/CAII command-stream and handler callbacks
     * means execution must remain unavailable.  Real GDAT-backed creature
     * data and rendering are kept separate from this fail-closed seam. */
    memset(&g_last_ccm_tick, 0, sizeof(g_last_ccm_tick));
    g_last_ccm_tick.valid = 1;
    g_last_ccm_tick.instance_id = slot;
    g_last_ccm_tick.ai_index = c->ai_index;
    g_last_ccm_tick.before_b_1a = c->b_1a;
    g_last_ccm_tick.ccm_opcode = c->b_1a;
    g_last_ccm_tick.ccm_result = -1;
    g_last_ccm_tick.program_pc_before = -1;
    g_last_ccm_tick.program_pc_after = -1;
    g_last_ccm_tick.after_b_1a = c->b_1a;
    g_last_ccm_tick.direction_before = c->direction;
    g_last_ccm_tick.direction_after = c->direction;
    g_last_ccm_tick.attack_cooldown_before = c->attack_cooldown;
    g_last_ccm_tick.attack_cooldown_after = c->attack_cooldown;
}

/* dm2_v1_creature_tick — advance all creature instances by one tick.
 * Source: SKULLWIN/c_ai.cpp: DM2_THINK_CREATURE, SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM
 *
 * Per creature per tick:
 *   1. Tick attack_cooldown countdown
 *   2. Tick poison_ticks (apply PoisonDamage per tick, source: SkWinCore.cpp)
 *   3. DM2_THINK_CREATURE: decide next CCM action from b_1a state
 *   4. DM2_PROCEED_CCM: execute CCM dispatch
 *   5. If HP <= 0: creature_death_check → drop + spatial sound
 *
 * CCM dispatch (b_1a primary state register):
 *   0x00 WALK_NOW / 0x02 WALK_CONT → movement
 *   0x01 ATTACK_HANDLER → attack resolution
 *   0x05 SPECIAL_ACTION → CCM06/CCM0B/CCM0C (switch/trap/trigger)
 *   0x09 STEAL_ITEM → thief-type item theft
 *   0x0a MERCHANT_BEHAVIOR → shop/NPC behavior
 *   0x0d SHOOT_ITEM → ranged throw/pickup
 *   0x0f KILL_ON_TIMER_POS → delayed-position kill
 *   0x13 ROTATES_TARGET → reorient another creature
 *   0x15 CAST_SPELL → monster spellcasting
 *   0x17 CREATURE_ATTACKS_PARTY → fallback melee
 *   0x26 EXPLODE_OR_SUMMON → self-destruct or minion spawn
 *
 * Stub: advance cooldowns, poison, proximity check for attack state.
 * Real CCM requires GDAT creature definitions + party world-position. */
void dm2_v1_creature_tick(void) {
    g_tick_counter++;
    for (int i = 0; i < DM2_MAX_CREATURE_INSTANCES; i++) {
        DM2_V1_CreatureInstance *c = &g_creature_pool[i];
        if (!c->alive) continue;

        if (c->attack_cooldown > 0) c->attack_cooldown--;

        if (c->poison_ticks > 0) {
            c->poison_ticks--;
            const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(c->ai_index);
            int pd = spec ? (int)spec->PoisonDamage : 0;
            if (pd > 0 && c->hp_current > 0) {
                c->hp_current -= pd;
            }
        }

        dm2_v1_creature_run_ccm_tick(c, i);
        dm2_v1_creature_write_render_state(c, 1);

        /* Only trigger death check for alive creatures whose HP just hit 0.
         * alive=1 && hp_current<=0 means HP reached 0 this tick — death not yet processed.
         * After death_check sets alive=0, subsequent ticks skip this block. */
        if (c->alive && c->hp_current <= 0) {
            dm2_v1_creature_death_check(i);
        }
    }
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
