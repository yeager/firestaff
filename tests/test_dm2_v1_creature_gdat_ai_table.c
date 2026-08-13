/* test_dm2_v1_creature_gdat_ai_table.c
 *
 * DM2 V1 GDAT-backed creature AI table admission gate.
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

static void set_word_entry(DM2_V1_GdatEntry *entry,
                           int category, int index, int field,
                           uint16_t value) {
    memset(entry, 0, sizeof(*entry));
    entry->cls1 = (uint8_t)category;
    entry->cls2 = (uint8_t)index;
    entry->cls3 = (uint8_t)DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entry->cls4 = (uint8_t)field;
    entry->data_index = value;
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
    DM2_V1_GdatEntry entries[3];
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry mapping_entry;
    DM2_V1_AssetLoader mapping_loader;
    uint8_t mapping_data = 0;
    uint32_t mapping_offset = 0;
    uint32_t mapping_size = 0;
    uint16_t hp = 0;

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

    memset(entries, 0, sizeof(entries));
    entries[0].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[0].cls2 = DM2_AI_THORN_DEMON;
    entries[0].cls4 = 0;
    entries[0].data_index = 0;
    entries[1].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[1].cls2 = DM2_AI_CAVE_BAT;
    entries[1].cls4 = 0;
    entries[1].data_index = 1;
    entries[2].cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entries[2].cls2 = 0;
    entries[2].cls4 = 0;
    entries[2].data_index = 2;

    memset(&loader, 0, sizeof(loader));
    loader.data = raw_data;
    loader.data_size = sizeof(raw_data);
    loader.loaded = 1;
    loader.raw_data_count = 3;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.entries = entries;
    loader.entry_count = 3;

    dm2_v1_creature_reset_ai_table();

    CHECK(dm2_v1_creature_load_ai_table_from_gdat(NULL) == -1,
          "NULL loader rejected");
    CHECK(dm2_v1_creature_ai_spec(DM2_AI_THORN_DEMON) == NULL,
          "reset does not expose an AI row without a CREATURES owner binding");
    /* A packed 36-byte record is a test fixture, not the source's GDAT
     * query graph.  EXTENDED_LOAD_AI_DEFINITION first obtains the owner from
     * CREATURES[type].dtWordValue(0x05), then obtains each requested
     * CREATURE_AI word value.  No owner/word records here means no AI. */
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 0,
          "packed raw AI fixture cannot enter the runtime table");
    CHECK(dm2_v1_creature_ai_spec(DM2_AI_THORN_DEMON) == NULL &&
              dm2_v1_creature_ai_spec(DM2_AI_CAVE_BAT) == NULL &&
              dm2_v1_creature_ai_spec(0) == NULL,
          "unowned raw rows cannot expose creature behavior");
    CHECK(dm2_v1_creature_attacks_party(DM2_AI_THORN_DEMON, 1) == 0 &&
              dm2_v1_creature_resolves_spell(DM2_AI_THORN_DEMON,
                                             AI_ATTACK_FLAGS__POISON_BOLT) == 0 &&
              dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 1, 2, 0, 0, 16) < 0,
          "raw fixture cannot create attack or spawn behavior");

    dm2_v1_creature_reset_ai_table();
    CHECK(dm2_v1_creature_ai_spec(DM2_AI_THORN_DEMON) == NULL &&
              dm2_v1_creature_ai_spec(DM2_AI_CAVE_BAT) == NULL,
          "reset clears live creature owner bindings");

    /* SKProject c_record.cpp masks the source creature key to one byte;
     * type 200 must still resolve CREATURES[type].word@5.  The retail
     * executable-owned AI row is valid without a CREATURE_AI override. */
    set_word_entry(&mapping_entry, DM2_GDAT_CATEGORY_CREATURES,
                   200, 0x05, DM2_AI_THORN_DEMON);
    memset(&mapping_loader, 0, sizeof(mapping_loader));
    mapping_loader.data = &mapping_data;
    mapping_loader.data_size = 1;
    mapping_loader.loaded = 1;
    mapping_loader.raw_data_count = 1;
    mapping_loader.raw_offsets = &mapping_offset;
    mapping_loader.raw_sizes = &mapping_size;
    mapping_loader.entries = &mapping_entry;
    mapping_loader.entry_count = 1;
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&mapping_loader) == 1 &&
              dm2_v1_creature_ai_base_hp(200, &hp) == 1 && hp == 400,
          "8-bit CREATURES key resolves the genuine AI row without override");

    printf("DM2 V1 creature GDAT AI table: %d/%d passed\n",
           tests_passed,
           tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
