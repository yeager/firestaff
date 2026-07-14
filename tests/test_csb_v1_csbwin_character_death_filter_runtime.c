/* CSBWin Character.cpp KillCharacter CharDeathFilter runtime regression.
 * Source: Character.cpp:2532-2585; DSA.cpp ProcessDSATimer6:5363-5416;
 * data.cpp EXPOOL::Locate:1542-1568. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void put_le16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void put_le32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
    bytes[offset + 2u] = (uint8_t)(value >> 16);
    bytes[offset + 3u] = (uint8_t)(value >> 24);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    uint8_t raw[512] = { 0 };
    uint8_t tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    const uint16_t actuator_thing = (uint16_t)(
        CSB_V1_THING_TYPE_ACTUATOR << 10);
    const uint32_t special_key =
        (CSB_V1_EXPOOL_EDT_SPECIAL_LOCATIONS << 24) |
        CSB_V1_EXPOOL_ESL_CHAR_DEATH_FILTER;
    const uint32_t special_bucket = 32u +
        ((special_key * 0xbb40e62du) >> 27);
    const uint32_t equip_key =
        (CSB_V1_EXPOOL_EDT_SPECIAL_LOCATIONS << 24) |
        CSB_V1_EXPOOL_ESL_EQUIP_FILTER;
    const uint32_t equip_bucket = 32u +
        ((equip_key * 0xbb40e62du) >> 27);
    uint16_t program[] = { 0x02d5u, 0x004du };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAImportedAction actions[2];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&profile, 0, sizeof(profile));
    memset(actions, 0, sizeof(actions));

    /* One original-format square list with one DB3 type-47 actuator. The
     * DB3 selector is 2 and its compact DSAstate is 4. */
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 32;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 64;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 0;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    dungeon.thing_data_bases[11] = 128;
    dungeon.thing_type_counts[11] = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    raw[32] = 0x10u;
    put_le16(raw, 64u, actuator_thing);
    put_le16(raw, 0u, 0xfffeu);
    put_le16(raw, 2u, 0x412fu);

    /* A real DB11/EXPOOL bucket node points to the packed LOCATIONREL. */
    put_le16(raw, 128u + 2u, 3u);
    put_le32(raw, 128u + (size_t)special_bucket * 4u, 1u);
    put_le32(raw, 128u + 1u * 4u, 0u);
    put_le32(raw, 128u + 2u * 4u, special_key);
    put_le32(raw, 128u + 3u * 4u, 0u);

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.party_state.ChampionCount = 1;
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[0][2] = 7u;
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    actions[0].dsa_id = 7u;
    actions[0].state_index = 4u;
    actions[0].column = 0u;
    actions[0].program_words = program;
    actions[0].program_word_count =
        (int)(sizeof(program) / sizeof(program[0]));
    actions[1] = actions[0];
    actions[1].column = 1u;
    profile.csbwin_extended_dsa_state.imported_actions = actions;
    profile.csbwin_extended_dsa_state.imported_action_count = 2;
    profile.csbwin_extended_dsa_state.imported_headers[7].valid = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].local_state = 0u;
    profile.csbwin_extended_dsa_state.imported_headers[7].state_slot_count = 8u;

    check(csb_v1_runtime_execute_csbwin_character_death_filter(
              &profile, 0) == 1,
          "CSBWin CharDeathFilter executes only its selected authenticated DSA action");

    /* Re-point the original DB11 bucket to EquipFilter. Its old RN callback
     * uses column 1, then its new RN callback uses column 0. */
    put_le32(raw, 128u + (size_t)special_bucket * 4u, 0u);
    put_le32(raw, 128u + 2u * 4u, equip_key);
    put_le32(raw, 128u + (size_t)equip_bucket * 4u, 1u);
    check(csb_v1_runtime_execute_csbwin_equip_filter(
              &profile, 0, 3, 0x1234u, 0x5678u) == 1,
          "CSBWin EquipFilter preserves old-RN column-1 then new-RN column-0 dispatch");

    put_le32(raw, 128u + (size_t)equip_bucket * 4u, 0u);
    put_le32(raw, 128u + 2u * 4u, special_key);
    put_le32(raw, 128u + (size_t)special_bucket * 4u, 1u);
    profile.csbwin_appended_tail[0] ^= 0x01u;
    check(csb_v1_runtime_execute_csbwin_character_death_filter(
              &profile, 0) == 0,
          "altered FNV-owned Extended Features tail blocks CharDeathFilter execution");

    return failures ? 1 : 0;
}
