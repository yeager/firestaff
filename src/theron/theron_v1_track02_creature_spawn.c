#include "theron_v1_track02_creature_spawn.h"

#include <string.h>

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 *
 * Spawn zones from UD 0x274058-0x274150 (5 of 7 creatures have zones).
 * Creature pointer table from UD 0x274018 (7 entries x 8 bytes).
 * Category formulas from disassembled spawn code at UD 0x0870E5 (PCE $B0E5).
 *
 * THIEF and DEMON have spawn_data_offset = 0x0000 (scripted encounters). */

static const Theron_SpawnZoneDesc g_spawn_zones[5] = {
    /* [0] AKUTUBA UD 0x274058 */ { 47, 44, 3, 5, 14, 2 },
    /* [1] DRATOR  UD 0x2740D7 */ { 27, 24, 2, 4, 16, 2 },
    /* [2] FORMIC  UD 0x274102 */ { 23, 20, 2, 4, 16, 2 },
    /* [3] SARMON  UD 0x274129 */ { 23, 20, 2, 4, 16, 2 },
    /* [4] SHADO   UD 0x274150 */ {  0, 24, 8, 10, 18, 2 },
};

static const Theron_SpawnCategoryFormula g_formulas[THERON_CREATURE_SPAWN_CATEGORY_COUNT] = {
    /* cat 0: dice(4) */           { 0,  0, 4, 0 },
    /* cat 1: rand * 21 */         { 1, 21, 0, 0 },
    /* cat 2: rand * 25, then *1.5 */ { 2, 25, 0, 1 },
    /* cat 3: dice(5) * scaling + 1.5*adj */ { 3, 0, 5, 1 },
};

static const Theron_CreaturePointerEntry g_pointer_table[8] = {
    /* [0] AKUTUBA */ { 0x0172, 0x278A, 0x0058, 0x016B },
    /* [1] DRATOR  */ { 0x0172, 0x278A, 0x00D7, 0x016B },
    /* [2] FORMIC  */ { 0x0198, 0x278A, 0x0102, 0x016B },
    /* [3] SARMON  */ { 0x0172, 0x278A, 0x0129, 0x016B },
    /* [4] SHADO   */ { 0x01AE, 0x278A, 0x0150, 0x016B },
    /* [5] THIEF   */ { 0x01C2, 0x278A, 0x0000, 0x0000 },
    /* [6] DEMON   */ { 0x01CF, 0x278A, 0x0000, 0x0000 },
    /* [7] (unused) */ { 0x01DC, 0x278A, 0x0000, 0x0000 },
};

const Theron_SpawnZoneDesc *theron_v1_track02_spawn_zone(unsigned int creature_index) {
    if (creature_index >= 5) return NULL;
    return &g_spawn_zones[creature_index];
}

const Theron_SpawnCategoryFormula *theron_v1_track02_spawn_formula(unsigned int category) {
    if (category >= THERON_CREATURE_SPAWN_CATEGORY_COUNT) return NULL;
    return &g_formulas[category];
}

size_t theron_v1_track02_spawn_zone_count(void) {
    return 5;
}

const Theron_CreaturePointerEntry *theron_v1_track02_creature_pointer(unsigned int index) {
    if (index >= 8) return NULL;
    return &g_pointer_table[index];
}

/* The disassembly identifies category branches and constants at UD 0x0870E5,
 * but it does not yet identify the bank-switched RNG return contract or the
 * consumers that supply the final values.  A host seed would therefore create
 * synthetic gameplay data.  Keep this API fail-closed until a live HuC6280
 * capture binds those bytes and consumers.  The old arithmetic sketch belongs
 * only in explicitly labelled fixture code, never in this source-bound path.
 */
int theron_v1_track02_compute_spawn_stats(
    unsigned int category, uint8_t param1, uint8_t param2,
    uint16_t rand_seed, Theron_SpawnStats *out)
{
    (void)category;
    (void)param1;
    (void)param2;
    (void)rand_seed;
    if (out) {
        out->hp = 0;
        out->attack = 0;
        out->defense = 0;
    }
    return 0;
}

static uint16_t add_u16(uint16_t left, uint16_t right) {
    return (uint16_t)(left + right);
}

static uint16_t cap_u16(uint16_t value, uint16_t cap) {
    return value > cap ? cap : value;
}

int theron_v1_track02_apply_spawn_consumer_witness(
    const Theron_SpawnConsumerWitness *witness,
    Theron_SpawnConsumerReceipt *out)
{
    uint16_t hp;
    uint16_t attack;
    uint16_t defense;
    uint16_t b8;
    uint16_t b4b5;
    uint8_t rng;

    if (out) memset(out, 0, sizeof(*out));
    if (!witness || !out || !witness->authenticated_execution ||
        witness->category >= THERON_CREATURE_SPAWN_CATEGORY_COUNT ||
        witness->b6 == 0u) {
        return 0;
    }

    hp = witness->hp_accumulator;
    attack = witness->attack_accumulator;
    defense = witness->defense_accumulator;
    b8 = witness->b8_before_branch;
    b4b5 = witness->b4b5;

    /* Reproduce only the visible instructions in LB0E5.  $5A76 is represented
     * by the captured helper_b8 return; using a host multiplication here
     * would turn the receipt into synthetic game data. */
    switch (witness->category) {
    case 1u:
        b8 = (uint16_t)((uint8_t)(witness->helper_b8 << 1)); /* ASL $B8 */
        out->object_write_1 = witness->lb0fe_return_1;
        out->object_write_2 = witness->lb0fe_return_2;
        break;
    case 2u:
        hp = add_u16(hp, witness->helper_b8);
        b8 = (uint16_t)((uint8_t)(witness->helper_b8 +
                                  ((witness->helper_b8 + 1u) >> 1)));
        out->object_write_1 = witness->lb0fe_return_1;
        break;
    case 3u:
        /* LB0DD shifts the B4/B5 pair.  B8 is a separate captured register
         * consumed by the following 1.5x sequence. */
        b4b5 = (uint16_t)(b4b5 >> 5);
        b8 = (uint16_t)(uint8_t)(b8 + (b8 >> 1));
        hp = cap_u16(add_u16(hp, b8), THERON_CREATURE_HP_CAP);
        out->object_write_1 = witness->lb0fe_return_1;
        break;
    default:
        /* Category zero falls through directly to the common tail. */
        break;
    }

    /* LB161: L4667 & 3, add to HP, clamp at $0384. */
    if (witness->category == 2u || witness->category == 3u) {
        rng = (uint8_t)(witness->rng_common_1 & 0x03u);
        if (rng >= witness->b6) rng = (uint8_t)(witness->b6 - 1u);
        hp = cap_u16(add_u16(hp, rng), THERON_CREATURE_HP_CAP);
    }

    /* LB19D: LD23A($B8)+$2980 and LD23A($B4)+$2990, with the caps visible in
     * the source span.  The helper outputs are captured values, not guesses. */
    attack = cap_u16(add_u16(attack, witness->ld23a_b8), 0x03e7u);
    defense = cap_u16(add_u16(defense, witness->ld23a_b4), 0x270fu);

    out->valid = 1;
    out->hp_accumulator = hp;
    out->attack_accumulator = attack;
    out->defense_accumulator = defense;
    out->helper_input_b8 = (uint8_t)b8;
    out->helper_input_b4 = (uint8_t)b4b5;
    out->helper_input_b5 = (uint8_t)(b4b5 >> 8);
    return 1;
}
