#include "theron_v1_track02_creature_spawn.h"

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

/* Category-based stat computation from disassembly at UD 0x0870E5.
 * param1/param2 come from the spawn zone descriptor.
 * rand_seed provides determinism (original uses PCE rand). */
int theron_v1_track02_compute_spawn_stats(
    unsigned int category, uint8_t param1, uint8_t param2,
    uint16_t rand_seed, Theron_SpawnStats *out)
{
    if (!out) return 0;
    out->hp = 0;
    out->attack = 0;
    out->defense = 0;
    int hp = 0, atk = 0, def = 0;
    int r = (int)(rand_seed & 0xFF);

    switch (category) {
    case 0: /* dice(4): HP = rand % 4 + param1 */
        hp = (r % 4) + param1;
        atk = (r % 3) + 2;
        def = 1;
        break;
    case 1: /* rand * 21: HP = (rand * 21) >> 8 + param1 */
        hp = ((r * 21) >> 8) + param1;
        atk = ((r * 15) >> 8) + 3;
        def = ((r * 10) >> 8) + 1;
        break;
    case 2: /* rand * 25 * 1.5: HP = ((rand * 25) >> 8) * 3 / 2 + param1 */
        hp = (((r * 25) >> 8) * 3 / 2) + param1;
        atk = (((r * 18) >> 8) * 3 / 2) + 4;
        def = ((r * 12) >> 8) + 2;
        break;
    case 3: /* dice(5) * scaling + 1.5*adj */
        hp = ((r % 5) * param1 + (param2 * 3 / 2));
        atk = ((r % 5) + 1) * 2 + param2;
        def = ((r % 4) + 1) + param2;
        break;
    default:
        /* No disassembly-backed formula exists outside categories 0-3. */
        return 0;
    }

    if (hp > THERON_CREATURE_HP_CAP) hp = THERON_CREATURE_HP_CAP;
    if (hp < 1) hp = 1;
    if (atk < 1) atk = 1;
    if (def < 1) def = 1;

    out->hp = (int16_t)hp;
    out->attack = (int16_t)atk;
    out->defense = (int16_t)def;
    return 1;
}
