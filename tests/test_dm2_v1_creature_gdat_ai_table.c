/* test_dm2_v1_creature_gdat_ai_table.c
 *
 * DM2 V1 GDAT-backed creature AI table import gate.
 *
 * Source-lock anchors:
 *   skproject/SKWIN/SkWinCore.cpp:233-400 EXTENDED_LOAD_AI_DEFINITION
 *   skproject/SKWIN/SkWinCore.cpp:2995 QUERY_CREATURE_AI_SPEC_FROM_TYPE
 *   skproject/SKWIN/DME.h:1505-1545 AIDefinition 36-byte layout
 *   docs/dm2-v1-creatures/dm2_spawn.md GDAT_CATEGORY_CREATURE_AI 0x19
 */

#include "dm2_v1_creature.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define CHECK(cond, name) do { \
    ++tests_run; \
    if (cond) { \
        ++tests_passed; \
        printf("PASS: %s\n", name); \
    } else { \
        printf("FAIL: %s\n", name); \
    } \
} while (0)

static void put16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)(v >> 8);
}

static void make_ai_raw(uint8_t out[36],
                        uint16_t flags,
                        uint8_t armor,
                        int8_t b3,
                        uint16_t hp,
                        uint8_t attack,
                        uint8_t poison,
                        uint8_t defense,
                        uint16_t attack_spells,
                        uint8_t weight,
                        uint16_t w30) {
    memset(out, 0, 36);
    put16(out + 0, flags);
    out[2] = armor;
    out[3] = (uint8_t)b3;
    put16(out + 4, hp);
    out[6] = attack;
    out[7] = poison;
    out[8] = defense;
    put16(out + 14, attack_spells);
    out[29] = weight;
    put16(out + 30, w30);
}

int main(void) {
    uint8_t raw_data[72];
    uint32_t raw_offsets[2] = { 0, 36 };
    uint32_t raw_sizes[2] = { 36, 36 };
    DM2_V1_GdatEntry entries[2];
    DM2_V1_AssetLoader loader;

    make_ai_raw(raw_data,
                DM2_AIFLAG_WORM_GLOP,
                7,
                -3,
                80,
                12,
                4,
                9,
                AI_ATTACK_FLAGS__MELEE | AI_ATTACK_FLAGS__POISON_BOLT,
                31,
                0);
    make_ai_raw(raw_data + 36,
                DM2_AIFLAG_ABSORBS_MISSILE,
                3,
                2,
                24,
                5,
                0,
                2,
                AI_ATTACK_FLAGS__SHOOT,
                11,
                DM2_AI_W30_TURNS_MISSILE);

    memset(entries, 0, sizeof(entries));
    entries[0].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[0].cls2 = DM2_AI_THORN_DEMON;
    entries[0].cls4 = 0;
    entries[0].data_index = 0;
    entries[1].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[1].cls2 = DM2_AI_CAVE_BAT;
    entries[1].cls4 = 0;
    entries[1].data_index = 1;

    memset(&loader, 0, sizeof(loader));
    loader.data = raw_data;
    loader.data_size = sizeof(raw_data);
    loader.loaded = 1;
    loader.raw_data_count = 2;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.entries = entries;
    loader.entry_count = 2;

    dm2_v1_creature_reset_ai_table();

    CHECK(dm2_v1_creature_load_ai_table_from_gdat(NULL) == -1,
          "NULL loader rejected");
    CHECK(dm2_v1_creature_ai_spec(DM2_AI_THORN_DEMON)->BaseHP == 0,
          "reset leaves AI table empty before GDAT import");
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 2,
          "GDAT import loads two AI definitions");

    const DM2_AIDefinition *thorn =
        dm2_v1_creature_ai_spec(DM2_AI_THORN_DEMON);
    const DM2_AIDefinition *bat =
        dm2_v1_creature_ai_spec(DM2_AI_CAVE_BAT);
    const DM2_AIDefinition *unused =
        dm2_v1_creature_ai_spec(DM2_AI_GIGGLER);

    CHECK(thorn->w0AIFlags == DM2_AIFLAG_WORM_GLOP &&
              thorn->ArmorClass == 7 &&
              thorn->b3 == -3 &&
              thorn->BaseHP == 80 &&
              thorn->AttackStrength == 12 &&
              thorn->PoisonDamage == 4 &&
              thorn->Defense == 9 &&
              thorn->AttacksSpells ==
                  (AI_ATTACK_FLAGS__MELEE | AI_ATTACK_FLAGS__POISON_BOLT) &&
              thorn->Weight == 31,
          "Thorn Demon spec decodes 36-byte little-endian AIDefinition");

    CHECK(bat->w0AIFlags == DM2_AIFLAG_ABSORBS_MISSILE &&
              bat->BaseHP == 24 &&
              bat->AttacksSpells == AI_ATTACK_FLAGS__SHOOT &&
              bat->w30 == DM2_AI_W30_TURNS_MISSILE,
          "Cavern Bat spec decodes missile/attack flags");

    CHECK(unused->BaseHP == 0 && unused->AttacksSpells == 0,
          "missing GDAT AI entries leave unrelated table slots unchanged");

    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 1, 2, 0, 0, 16);
    const DM2_V1_CreatureInstance *inst = dm2_v1_creature_get_instance(slot);
    CHECK(slot >= 0 && inst && inst->hp_max == 160 && inst->hp_current == 160,
          "spawn HP scales from imported BaseHP");

    dm2_v1_creature_reset_ai_table();
    CHECK(dm2_v1_creature_ai_spec(DM2_AI_THORN_DEMON)->BaseHP == 0 &&
              dm2_v1_creature_ai_spec(DM2_AI_CAVE_BAT)->BaseHP == 0,
          "reset clears imported GDAT AI definitions");

    printf("DM2 V1 creature GDAT AI table: %d/%d passed\n",
           tests_passed,
           tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
