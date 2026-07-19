/* CSBWin TT_DOOR DSA-to-animation ownership regression.
 * Source: CSBWin Timer.cpp ActivateDSA:1453-1492 and
 * ProcessTT_DOOR:1509-1541; CSBCode.cpp ProcessTimers:6431-6433. */

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

int main(void)
{
    uint8_t raw[128] = { 0 };
    uint16_t pure_stack[] = { 0x0686u, 0x1234u, 0x0012u, 0x0011u, 0x000du };
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAImportedAction action;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&action, 0, sizeof(action));
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
    raw[80] = 0x82u;
    put_le16(raw, 62u, (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10));
    put_le16(raw, 90u, 0xfffeu);
    put_le16(raw, 92u, 0x012fu);
    put_le16(raw, 96u, 1u);

    action.dsa_id = 7u;
    action.state_index = 1u;
    action.column = 0u;
    action.program_words = pure_stack;
    action.program_word_count = 5u;

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[0][2] = 7u;
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1u;
    profile.csbwin_extended_dsa_state.imported_headers[7].valid = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].local_state = 2u;
    profile.csbwin_extended_dsa_state.imported_headers[7].state_slot_count = 2u;
    profile.csbwin_timer_summary_count = 2u;
    profile.csbwin_timer_summary_total = 2u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_max_timers = 2u;
    profile.csbwin_num_timer = 1u;
    profile.csbwin_first_avail_timer = 0u;
    profile.csbwin_timer_sequence = 31u;
    profile.csbwin_timer_queue[0] = 1u;
    profile.csbwin_timers[0].valid = 1;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].function = DM1_EVENT_NONE;
    profile.csbwin_timers[1].valid = 1;
    profile.csbwin_timers[1].source_index = 1u;
    profile.csbwin_timers[1].function = 10u;
    profile.csbwin_timers[1].time = 0u;

    /* ProcessTT_DOOR turns the consumed TIMER into TT_1 and hands it to
     * Timer.cpp SetTimer, so the successor takes the pool's first free slot
     * while the dispatched slot is released. */
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              raw[80] == 0x82u && profile.csbwin_timers[0].function == 1u &&
              profile.csbwin_timers[0].source_index == 0u &&
              profile.csbwin_timers[0].sequence == 31u &&
              profile.csbwin_timers[0].time + 1u == profile.game_time &&
              profile.csbwin_timers[1].function == DM1_EVENT_NONE &&
              profile.csbwin_timer_queue[0] == 0u &&
              profile.csbwin_first_avail_timer == 1u &&
              profile.csbwin_timer_sequence == 32u &&
              profile.timeline_queue.eventCount == 1,
          "one authenticated door DSA action preserves the source TT_1 handoff");

    /* The TT_1 continuation requeues through the same pool ownership: the
     * dispatch releases its slot to the avail chain and SetTimer claims that
     * same slot back, so the live TIMER keeps queue slot zero. */
    check(csb_v1_runtime_tick_v1(&profile) == 1 && raw[80] == 0x81u &&
              profile.csbwin_timers[0].function == 1u &&
              profile.csbwin_timers[0].source_index == 0u &&
              profile.csbwin_timers[0].sequence == 32u &&
              profile.csbwin_timers[0].time == profile.game_time &&
              profile.csbwin_timers[1].function == DM1_EVENT_NONE &&
              profile.csbwin_timer_queue[0] == 0u &&
              profile.csbwin_num_timer == 1u &&
              profile.csbwin_first_avail_timer == 1u &&
              profile.csbwin_timer_sequence == 33u &&
              profile.timeline_queue.eventCount == 1,
          "the source-owned TT_1 successor advances the same door");

    return failures == 0 ? 0 : 1;
}
