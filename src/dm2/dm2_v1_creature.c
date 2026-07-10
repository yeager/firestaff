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
#include "dm2_v1_ccm.h"
#include "dm2_v1_drops.h"
#include "dm2_v1_sound.h"
#include <string.h>

#define DM2_CREATURE_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH 0x0001u

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
static uint8_t g_ai_table_loaded[DM2_AI_TABLE_SIZE];
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
    /* QUERY_CREATURE_AI_SPEC_FROM_TYPE at SkWinCore.cpp:2995
     * In extended mode: uses EXTENDED_LOAD_AI_DEFINITION() result
     * In fixed mode: uses hardcoded dAITableGenuine[]
     * Stub: index by creature_type (capped) */
    int idx = creature_type;
    if (idx < 0) idx = 0;
    if (idx >= DM2_AI_TABLE_SIZE) idx = DM2_AI_TABLE_SIZE - 1;
    (void)creature_type;
    return &g_ai_table[idx];
}

void dm2_v1_creature_reset_ai_table(void) {
    memset(g_ai_table, 0, sizeof(g_ai_table));
    memset(g_ai_table_loaded, 0, sizeof(g_ai_table_loaded));
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
    int i;

    if (!loader || !loader->loaded) return -1;

    /* skproject/SKWIN/SkWinCore.cpp EXTENDED_LOAD_AI_DEFINITION lines
     * ~233-400 loads 36 AIDefinition bytes per creature AI index from
     * GDAT_CATEGORY_CREATURE_AI (0x19).  QUERY_CREATURE_AI_SPEC_FROM_TYPE
     * line ~2995 then indexes this table by creature type. */
    for (i = 0; i < DM2_AI_TABLE_SIZE; ++i) {
        size_t raw_size = 0;
        const uint8_t *raw = dm2_v1_asset_load_sized(
            loader, DM2_GDAT_CATEGORY_CREATURE_AI, i, 0, &raw_size);
        if (!raw || raw_size < sizeof(DM2_AIDefinition)) continue;
        dm2_v1_creature_decode_ai_spec(raw, &g_ai_table[i]);
        g_ai_table_loaded[i] = 1;
        ++loaded;
    }

    return loaded;
}

int dm2_v1_creature_load_ccm_programs_from_gdat(const DM2_V1_AssetLoader *loader,
                                                int field) {
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
}

int dm2_v1_creature_load_ccm_programs_from_gdat_auto(
    const DM2_V1_AssetLoader *loader,
    int *out_field) {
    static const int k_fields[] = {
        1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        0x10, 0x11, 0x12, 0x18, 0x19, 0x1a, 0x20
    };
    int i;

    if (out_field) *out_field = -1;
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

static int dm2_v1_creature_ai_has_gdat_spec(int ai_index) {
    return ai_index >= 0 && ai_index < DM2_AI_TABLE_SIZE &&
           g_ai_table_loaded[ai_index] != 0;
}

static int dm2_v1_creature_is_static_ai_index(int ai_index) {
    return ai_index == 0  || ai_index == 1  || ai_index == 4
        || ai_index == 5  || ai_index == 6  || ai_index == 7
        || ai_index == 8  || ai_index == 9  || ai_index == 10
        || ai_index == 11 || ai_index == 12 || ai_index == 20
        || ai_index == 33 || ai_index == 45 || ai_index == 59
        || ai_index == 60;
}

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
 * objects suppressed by w0AIFlags. Data-free fallback preserves the
 * original no-assets probe behavior for rows not yet imported. */
int dm2_v1_creature_attacks_party(int ai_index, int distance) {
    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return 0;
    if (distance < 0) return 0;

    if (dm2_v1_creature_ai_has_gdat_spec(ai_index)) {
        const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(ai_index);
        uint16_t attacks = spec ? spec->AttacksSpells : 0;

        if (!spec || (spec->w0AIFlags & DM2_AIFLAG_STATIC) != 0) return 0;
        if ((attacks & AI_ATTACK_FLAGS__MELEE) != 0 && distance <= 1) return 1;
        if (dm2_v1_creature_attack_flags_are_ranged(attacks) && distance <= 6) return 1;
        return 0;
    }

    if (dm2_v1_creature_is_static_ai_index(ai_index)) return 0;
    return distance <= 1 ? 1 : 0;
}

/* dm2_v1_creature_resolves_spell — map AI_ATTACK_FLAGS to spell effect
 * Source: SkWinCore.cpp:27038-27096 (OBJECT_EFFECT_* mapping)
 * Returns non-zero if creature has the requested spell-flag set.
 * GDAT path intersects the requested flags with the imported
 * AIDefinition.AttacksSpells. Data-free fallback preserves the old
 * flag-only classification for rows not yet imported. */
int dm2_v1_creature_resolves_spell(int ai_index, uint16_t attack_flags) {
    if (ai_index < 0 || ai_index >= DM2_AI_TABLE_SIZE) return 0;
    if (!dm2_v1_creature_attack_flags_are_ranged(attack_flags)) return 0;

    if (dm2_v1_creature_ai_has_gdat_spec(ai_index)) {
        const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(ai_index);
        return spec && (spec->AttacksSpells & attack_flags) != 0;
    }

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
    int slot = -1;
    for (int i = 0; i < DM2_MAX_CREATURE_INSTANCES; i++) {
        if (!g_creature_pool[i].alive) { slot = i; break; }
    }
    if (slot < 0) return -1;

    DM2_V1_CreatureInstance *c = &g_creature_pool[slot];
    memset(c, 0, sizeof(*c));
    c->instance_id = g_next_instance_id++;
    c->ai_index    = (ai_index >= 0 && ai_index < DM2_AI_TABLE_SIZE) ? ai_index : 0;
    c->world_x     = world_x;
    c->world_y     = world_y;
    c->map_index   = map_index;
    c->direction   = direction & 3;
    c->alive       = 1;
    c->is_visible  = 1;
    c->b_1a        = DM2_CCM_WALK_NOW;
    c->b_17        = 0;
    c->attack_cooldown = 0;
    c->poison_ticks    = 0;
    dm2_v1_creature_write_render_state(c, 0);

    int mult = (health_multiplier > 0) ? health_multiplier : 8;
    const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(ai_index);
    int hp = spec ? (int)spec->BaseHP * mult / 8 : 10;
    if (hp <= 0) hp = 1;  /* minimum 1 HP so zero-init stub creatures can die */
    c->hp_max     = hp;
    c->hp_current = c->hp_max;
    ++c->render_revision;

    return slot;
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

    /* Stub: Thorn Demon always drops sellable worm food.
     * Real: GDAT creature category 0x0A, sub-entries 0x0A-0x14, 11 slots.
     * Source: SKWin.GDAT2.InternalCodes.txt, dm2_v1_drops.c */
    DM2_V1_DropTable dt = {0};
    if (snap_ai == DM2_AI_THORN_DEMON) {
        dt.slots[0].item_id = DM2_DROP_THORN_DEMON_WORM_FOOD;
        dt.slots[0].count   = 1;
    }
    DM2_DropEntry drop = {0};
    int drop_hit = dm2_v1_drops_generate(&dt, (uint32_t)instance_id, &drop);

    /* Populate the observer so the CTest gate can assert the loot-state
     * contract deterministically.  drop_hit==1 + drop.item_id!=0 means
     * a non-empty drop entry; for non-Thorn-Demon AI the stub returns 0
     * and the observer records dropped=0 (death still observed). */
    memset(&g_last_death_drop, 0, sizeof(g_last_death_drop));
    g_last_death_drop.instance_id = instance_id;
    g_last_death_drop.ai_index    = snap_ai;
    g_last_death_drop.world_x     = snap_x;
    g_last_death_drop.world_y     = snap_y;
    g_last_death_drop.map_index   = snap_map;
    if (drop_hit && drop.item_id != 0) {
        g_last_death_drop.dropped = 1;
        g_last_death_drop.item_id = drop.item_id;
        g_last_death_drop.count   = drop.count;
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
        case DM2_CCM_OP_ATTACK_HANDLER:
        case DM2_CCM_OP_STEAL_ITEM:
        case DM2_CCM_OP_SPECIAL_ACTION:
        case DM2_CCM_OP_MERCHANT_BEHAVIOR:
        case DM2_CCM_OP_KILL_ON_TIMER_POS:
        case DM2_CCM_OP_ROTATES_TARGET:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON:
            args[0] = c->b_17;
            break;
        case DM2_CCM_OP_SHOOT_ITEM:
            args[0] = c->b_17;
            args[1] = c->direction;
            break;
        case DM2_CCM_OP_CAST_SPELL:
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
        int nonmaterial = dm2_v1_creature_ai_spec(c->ai_index)->w0AIFlags &
                          DM2_AIFLAG_NONMATERIAL;
        door_valid = 1;
        door_blocks = dm2_v1_creature_door_blocks_creature(
            door_state, door_attributes, nonmaterial != 0);
    }
    imported_op = dm2_v1_creature_imported_ccm_op(c, &imported_pc);
    if (imported_op) {
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
    g_last_ccm_tick.attack_cooldown_before = before_cooldown;

    if (rc == (int)DM2_CCM_RESULT_OK) {
        if (state.flags[9]) {
            c->b_1a = DM2_CCM_WALK_NOW;
            c->attack_cooldown = 18;
        } else if (state.flags[5] || state.flags[8] || state.flags[10]) {
            c->b_1a = DM2_CCM_WALK_NOW;
            c->attack_cooldown = 18;
        } else if (state.flags[3]) {
            c->b_1a = DM2_CCM_WALK_NOW;
            c->attack_cooldown = 9;
        } else if (state.flags[0] && before_cooldown == 0 &&
                   dm2_v1_creature_attacks_party(c->ai_index, 1)) {
            c->b_1a = DM2_CCM_CREATURE_ATTACKS_PARTY;
        } else if (state.flags[0] && before_cooldown == 0 &&
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
