/* CSBWin TT_ParameterMessage saved-DSA runtime regression.
 * Source: CSBCode.cpp ProcessTimers:6436-6454; Timer.cpp:1641-1711,
 * 2118-2185; data.cpp EXPOOL::Read:1542-1568; DSA.cpp ProcessDSATimer6. */

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
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint16_t numparam_globalstore[] = { 0x02d5u, 0x0054u };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWin512TimerSummary timer;
    const uint32_t message_record_id = (1u << 24) | 3u;
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t message_bucket = 32u +
        ((message_record_id * 0xbb40e62du) >> 27);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);
    uint32_t before_global;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&profile, 0, sizeof(profile));
    memset(&location, 0, sizeof(location));
    memset(&action, 0, sizeof(action));
    memset(&timer, 0, sizeof(timer));

    /* One source roomSTONE square plus one type-47 DB3 record. */
    raw[8] = 0u; raw[9] = 0u; raw[10] = 0x2fu; raw[11] = 0x41u;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 2;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 8;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    location.actuator_thing = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);

    /* A source-owned three-word message node at slot 3 and an existing
     * eighteen-word global node, so GLOBALSTORE can commit transactionally. */
    put_le16(tail, 2u, 3u);
    put_le32(tail, (size_t)message_bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, message_record_id);
    put_le32(tail, 3u * 4u, 0x12345678u);
    put_le16(tail, 64u * 4u + 2u, 18u);
    put_le32(tail, (size_t)global_bucket * 4u, 65u);
    put_le32(tail, 65u * 4u, global_bucket == message_bucket ? 1u : 0u);
    put_le32(tail, 66u * 4u, global_record_id);

    csb_v1_runtime_init(&profile, NULL);
    /* ProcessDSATimer6 commits the final state through DSA.cpp PutState,
     * which writes the LocalState-0 nibble into the live DB3 actuator. */
    profile.dungeon_handle = &dungeon;
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
    action.state_index = 4u;
    action.column = 0u;
    action.program_words = numparam_globalstore;
    action.program_word_count = 2;
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].valid = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].local_state = 0u;
    profile.csbwin_extended_dsa_state.imported_headers[7].state_slot_count = 8u;

    timer.valid = 1;
    timer.source_index = 3u;
    timer.function = 101u;
    timer.level = 0u;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_timer_summary_count = 4u;
    profile.csbwin_timer_summary_total = 4u;
    profile.csbwin_max_timers = 4u;
    profile.csbwin_first_avail_timer = 0u;
    profile.csbwin_timers[3] = timer;
    check(csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
              &profile, &dungeon, &location, &timer) == 0 &&
              profile.csbwin_global_variables[1] == 0u,
          "unqueued parameter-message timer rejects before EXPOOL dispatch");
    profile.csbwin_num_timer = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_timer_queue[0] = timer.source_index;
    profile.csbwin_num_timer = 2u;
    profile.csbwin_timer_queue_summary_count = 2u;
    profile.csbwin_timer_queue_summary_total = 2u;
    profile.csbwin_timer_queue[1] = timer.source_index;
    check(csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
              &profile, &dungeon, &location, &timer) == 0 &&
              profile.csbwin_global_variables[1] == 0u,
          "duplicate parameter-message queue ownership rejects before dispatch");
    profile.csbwin_num_timer = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    check(csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
              &profile, &dungeon, &location, &timer) == 1 &&
              profile.csbwin_global_variables[1] == 1u,
          "saved parameter message reads its exact EXPOOL payload and executes");

    before_global = profile.csbwin_global_variables[1];
    ++timer.sequence;
    check(csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
              &profile, &dungeon, &location, &timer) == 0 &&
              profile.csbwin_global_variables[1] == before_global,
          "altered timer fields reject before DSA publication");
    --timer.sequence;

    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
              &profile, &dungeon, &location, &timer) == 0 &&
              profile.csbwin_global_variables[1] == before_global,
          "altered EXPOOL receipt rejects before DSA publication");
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail, profile.csbwin_appended_tail_size);

    put_le16(profile.csbwin_appended_tail, 2u, 29u);
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail, profile.csbwin_appended_tail_size);
    check(csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
              &profile, &dungeon, &location, &timer) == 0 &&
              profile.csbwin_global_variables[1] == before_global,
          "parameter records beyond the bounded runner ABI reject cleanly");
    return failures == 0 ? 0 : 1;
}
