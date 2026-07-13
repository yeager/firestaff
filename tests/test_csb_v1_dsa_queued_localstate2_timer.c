/* CSBWin compact-ParameterB saved-timer DSA regression.
 * Source: data.cpp DB3::MakeBig/ParameterB:1319-1351; DSA.cpp
 * GetState:548-571; SaveGame.cpp:1844-1858; CSBCode.cpp:6430-6470. */

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
    uint8_t raw[16] = { 0 };
    uint8_t tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint16_t store_global[] = { 0x0686u, 0x55aau, 0x0054u };
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWin512TimerSummary timer;
    uint32_t before;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&profile, 0, sizeof(profile));
    memset(&location, 0, sizeof(location));
    memset(&action, 0, sizeof(action));
    memset(&timer, 0, sizeof(timer));

    /* Compact DB3: word2 is DSA type/selector/state, word6 is ParameterB.
     * The zero high two bits are the only widened-state shape admitted. */
    raw[10] = 0x2fu; raw[11] = 0x01u;
    raw[14] = 1u;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 2;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 8;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    location.actuator_thing = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);

    put_le16(tail, 2u, 18u);
    put_le32(tail, (size_t)global_bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, global_record_id);

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[0][2] = 7u;
    profile.csbwin_global_variables_valid = 1;
    profile.csbwin_global_variable_count = 16u;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    action.dsa_id = 7u;
    action.state_index = 1u;
    action.column = 0u;
    action.program_words = store_global;
    action.program_word_count = 3;
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].valid = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].local_state = 2u;
    profile.csbwin_extended_dsa_state.imported_headers[7].state_slot_count = 2u;
    timer.valid = 1;
    timer.source_index = 0u;
    timer.function = 6u;
    profile.csbwin_timer_summary_count = 1u;
    profile.csbwin_timer_summary_total = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_timers[0] = timer;
    profile.csbwin_timer_queue[0] = 0u;

    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "queued TT_STONEROOM executes the compact ParameterB-selected action");

    before = profile.csbwin_global_variables[1];
    raw[15] = 0x80u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              profile.csbwin_global_variables[1] == before,
          "widened ParameterB bits reject before DSA publication");
    raw[15] = 0u;

    profile.csbwin_timer_queue[0] = 1u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              profile.csbwin_global_variables[1] == before,
          "out-of-range saved timer-queue entries reject cleanly");
    profile.csbwin_timer_queue[0] = 0u;

    profile.csbwin_timers[0].function = 101u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              profile.csbwin_global_variables[1] == before,
          "parameter-message timers stay on their authenticated payload route");
    return failures == 0 ? 0 : 1;
}
