/* CSBWin restored timer-queue to live-tick DSA regression.
 * Source: SaveGame.cpp:1844-1858; CSBCode.cpp ProcessTimers:6430-6470;
 * Timer.cpp ProcessTT_STONEROOM:2118-2185; DSA.cpp ProcessDSATimer6. */

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
    uint8_t raw[128] = { 0 };
    uint8_t tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint16_t store_global[] = { 0x0686u, 0x55aau, 0x0054u };
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWin512TimerSummary timer;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&profile, 0, sizeof(profile));
    memset(&action, 0, sizeof(action));
    memset(&timer, 0, sizeof(timer));

    /* Source-shaped one-square byte map: column count at 60, first Thing at
     * 62, square byte at 80, then an eight-byte type-47 DB3 at 90. */
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 80;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 62;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 90;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    raw[80] = 0x10u;
    put_le16(raw, 62u, (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10));
    put_le16(raw, 90u, 0xfffeu);
    put_le16(raw, 92u, 0x012fu);
    put_le16(raw, 96u, 1u);

    put_le16(tail, 2u, 18u);
    put_le32(tail, (size_t)global_bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, global_record_id);

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
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
    timer.time = 0u;
    profile.csbwin_timer_summary_count = 1u;
    profile.csbwin_timer_summary_total = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_timers[0] = timer;
    profile.csbwin_timer_queue[0] = 0u;

    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "restored queue timer reaches authenticated DSA through live tick dispatch");

    profile.csbwin_global_variables[1] = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].source_index = 1u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0u,
          "stale restored timer identity cannot reach the live DSA dispatcher");

    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].function = 8u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "restored teleporter timer reaches ActivateDSA before blocked cell effects");

    profile.csbwin_global_variables[1] = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].ubyte9 = 3u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0u,
          "unsupported teleporter action cannot reach the live DSA dispatcher");

    profile.csbwin_timers[0].function = 9u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "restored pit timer reaches ActivateDSA before blocked cell effects");

    profile.csbwin_global_variables[1] = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].ubyte9 = 3u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0u,
          "unsupported pit action cannot reach the live DSA dispatcher");

    profile.csbwin_timers[0].function = 10u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "restored door timer reaches ActivateDSA before blocked requeue effects");

    profile.csbwin_global_variables[1] = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].ubyte9 = 3u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0u,
          "unsupported door action cannot reach the live DSA dispatcher");

    profile.csbwin_timers[0].function = 102u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "restored DSA message reaches zero-parameter OPENROOM dispatch");

    profile.csbwin_global_variables[1] = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].ubyte9 = 3u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0u,
          "unsupported DSA message action cannot reach the live dispatcher");
    return failures == 0 ? 0 : 1;
}
