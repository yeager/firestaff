/* CSBWin CSBCode.cpp TAG0138ec CursorFilter ResumeSavedGame regression. */

#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
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

    for (i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

int main(void)
{
    uint8_t raw[512] = { 0 };
    uint8_t tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    const uint16_t actuator_thing =
        (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);
    const uint32_t key = (CSB_V1_EXPOOL_EDT_SPECIAL_LOCATIONS << 24) |
        CSB_V1_EXPOOL_ESL_CURSOR_FILTER;
    const uint32_t bucket = 32u + ((key * 0xbb40e62du) >> 27);
    uint16_t program[] = { 0x0686u, 0u, 0x00cdu };
    CSB_V1_DungeonData dungeon;
    CSB_V1_BootProfile boot;
    CSB_V1_RuntimeProfile *runtime;
    CSB_V1_DSAImportedAction action;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&action, 0, sizeof(action));
    dungeon.level_count = 1; dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1; dungeon.level_offsets[0] = 32;
    dungeon.square_bytes = 1; dungeon.square_first_thing_base = 64;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 0;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    dungeon.thing_data_bases[11] = 128; dungeon.thing_type_counts[11] = 1;
    dungeon.raw_data = raw; dungeon.raw_size = (int)sizeof(raw);
    raw[32] = 0x10u; put_le16(raw, 64u, actuator_thing);
    put_le16(raw, 0u, 0xfffeu); put_le16(raw, 2u, 0x412fu);
    put_le32(raw, 128u + 2u, 3u);
    put_le32(raw, 128u + (size_t)bucket * 4u, 1u);
    put_le32(raw, 128u + 1u * 4u, 0u);
    put_le32(raw, 128u + 2u * 4u, key);
    put_le32(raw, 128u + 3u * 4u, 0u);

    memset(&boot, 0, sizeof(boot));
    csb_v1_runtime_init(&boot.runtime, NULL);
    runtime = &boot.runtime;
    runtime->dungeon_handle = &dungeon;
    runtime->party_state_valid = 1;
    runtime->party_state.ChampionCount = 1;
    runtime->csbwin_gameblock2_summary_valid = 1;
    runtime->csbwin_extended_features_valid = 1;
    runtime->csbwin_extended_level_index_present = 1;
    runtime->csbwin_extended_level_dsa_index[0][2] = 7u;
    memcpy(runtime->csbwin_appended_tail, tail, sizeof(tail));
    runtime->csbwin_appended_tail_valid = 1;
    runtime->csbwin_appended_tail_size = sizeof(tail);
    runtime->csbwin_appended_tail_preserved_size = sizeof(tail);
    runtime->csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    action.dsa_id = 7u; action.state_index = 4u; action.column = 0u;
    action.program_words = program;
    action.program_word_count = (int)(sizeof(program) / sizeof(program[0]));
    runtime->csbwin_extended_dsa_state.imported_actions = &action;
    runtime->csbwin_extended_dsa_state.imported_action_count = 1;
    runtime->csbwin_extended_dsa_state.imported_headers[7].valid = 1;
    runtime->csbwin_extended_dsa_state.imported_headers[7].local_state = 0u;
    runtime->csbwin_extended_dsa_state.imported_headers[7].state_slot_count = 8u;

    check(csb_v1_runtime_execute_csbwin_cursor_resume_saved_game_filter(
              runtime, 0x1234u) == 1,
          "ResumeSavedGame reaches its authenticated CursorFilter DSA action");
    check(csb_v1_runtime_write_leader_hand_from_boot_profile_pc34(
              &boot, 0x1234u) == 1 &&
              runtime->party_state.LeaderHandThing == 0x1234u,
          "live GAMEBLOCK2 hand bridge publishes the restored hand after notifications");
    runtime->csbwin_appended_tail[0] ^= 0x01u;
    check(csb_v1_runtime_execute_csbwin_cursor_resume_saved_game_filter(
              runtime, 0x1234u) == 0,
          "altered FNV-owned Extended Features tail blocks ResumeSavedGame");
    check(csb_v1_runtime_write_leader_hand_from_boot_profile_pc34(
              &boot, 0x4321u) == 1 &&
              runtime->party_state.LeaderHandThing == 0x4321u,
          "blocked callback cannot replace or prevent source-owned hand restoration");
    return failures ? 1 : 0;
}
