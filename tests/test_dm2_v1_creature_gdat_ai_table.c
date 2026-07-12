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
    uint8_t raw_data[108];
    uint32_t raw_offsets[3] = { 0, 36, 72 };
    uint32_t raw_sizes[3] = { 36, 36, 36 };
    DM2_V1_GdatEntry entries[6];
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
    make_ai_raw(raw_data + 72,
                DM2_AIFLAG_STATIC,
                1,
                0,
                12,
                0,
                0,
                0,
                0,
                255,
                0);
    put16(raw_data + 32, 0x01a5u);

    memset(entries, 0, sizeof(entries));
    /* skproject QUERY_CREATURE_AI_SPEC_FROM_TYPE reads CREATURES[type]
     * dtWordValue field 0x05 before resolving the CREATURE_AI row.  Deliberately
     * use non-type row numbers to prove the runtime does not conflate them. */
    entries[0].cls1 = DM2_GDAT_CATEGORY_CREATURES;
    entries[0].cls2 = DM2_AI_THORN_DEMON;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[0].cls4 = 0x05;
    entries[0].data_index = 7;
    entries[1].cls1 = DM2_GDAT_CATEGORY_CREATURES;
    entries[1].cls2 = DM2_AI_CAVE_BAT;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[1].cls4 = 0x05;
    entries[1].data_index = 11;
    entries[2].cls1 = DM2_GDAT_CATEGORY_CREATURES;
    entries[2].cls2 = 0;
    entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[2].cls4 = 0x05;
    entries[2].data_index = 3;
    entries[3].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[3].cls2 = 7;
    entries[3].cls4 = 0;
    entries[3].data_index = 0;
    entries[4].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[4].cls2 = 11;
    entries[4].cls4 = 0;
    entries[4].data_index = 1;
    entries[5].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[5].cls2 = 3;
    entries[5].cls4 = 0;
    entries[5].data_index = 2;

    memset(&loader, 0, sizeof(loader));
    loader.data = raw_data;
    loader.data_size = sizeof(raw_data);
    loader.loaded = 1;
    loader.raw_data_count = 3;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.entries = entries;
    loader.entry_count = 6;

    dm2_v1_creature_reset_ai_table();

    CHECK(dm2_v1_creature_load_ai_table_from_gdat(NULL) == -1,
          "NULL loader rejected");
    CHECK(dm2_v1_creature_ai_spec(DM2_AI_THORN_DEMON)->BaseHP == 0,
          "reset leaves AI table empty before GDAT import");
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 3,
          "GDAT import loads three AI definitions");

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
    {
        uint16_t rectno = 0u;
        DM2_V1_CreatureItemClickEvidence evidence;
        CHECK(dm2_v1_creature_item_click_rect_evidence(
                  DM2_AI_THORN_DEMON, &rectno) == 1 && rectno == 0x01a5u &&
                  dm2_v1_creature_item_click_evidence(
                      DM2_AI_THORN_DEMON, &evidence) == 1 &&
                  evidence.valid == 1 && evidence.creature_type ==
                      DM2_AI_THORN_DEMON && evidence.ai_flags_w30 == 0u &&
                  evidence.rectno_w32 == 0x01a5u &&
                  dm2_v1_creature_item_click_evidence(
                      DM2_AI_CAVE_BAT, &evidence) == 0 &&
                  dm2_v1_creature_item_click_rect_evidence(
                      DM2_AI_GIGGLER, &rectno) == 0,
              "GDAT creature row exposes only w30-gated w32 click-rect evidence");
    }

    CHECK(dm2_v1_creature_attacks_party(DM2_AI_THORN_DEMON, 1) == 1 &&
              dm2_v1_creature_attacks_party(DM2_AI_THORN_DEMON, 7) == 0,
          "imported melee/spell creature attacks only inside GDAT range gate");

    CHECK(dm2_v1_creature_attacks_party(DM2_AI_CAVE_BAT, 4) == 1 &&
              dm2_v1_creature_attacks_party(DM2_AI_CAVE_BAT, 7) == 0,
          "imported shooter creature attacks inside ranged GDAT gate");

    CHECK(dm2_v1_creature_resolves_spell(DM2_AI_THORN_DEMON,
                                         AI_ATTACK_FLAGS__POISON_BOLT) == 1 &&
              dm2_v1_creature_resolves_spell(DM2_AI_THORN_DEMON,
                                             AI_ATTACK_FLAGS__FIREBALL) == 0,
          "spell resolution intersects requested flags with imported GDAT attacks");

    CHECK(dm2_v1_creature_attacks_party(0, 0) == 0,
          "imported static AI row suppresses attack routing");

    CHECK(unused == NULL &&
              dm2_v1_creature_attacks_party(DM2_AI_GIGGLER, 1) == 0 &&
              dm2_v1_creature_resolves_spell(DM2_AI_GIGGLER,
                                             AI_ATTACK_FLAGS__FIREBALL) == 0,
          "missing mapped row is explicitly unavailable with no action");

    CHECK(dm2_v1_creature_spawn(DM2_AI_GIGGLER, 1, 2, 0, 0, 8) == -1,
          "missing mapped row blocks spawn instead of inventing HP");

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
