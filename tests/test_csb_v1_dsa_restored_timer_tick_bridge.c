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
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint16_t store_global[] = { 0x0686u, 0x55aau, 0x0054u };
    const uint32_t message_record_id = (1u << 24);
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t message_bucket = 32u +
        ((message_record_id * 0xbb40e62du) >> 27);
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

    /* CSBWin EXPOOL records: a source-index-0 parameter message followed
     * by the global store record used by the authenticated DSA program. */
    put_le16(tail, 2u, 3u);
    put_le32(tail, (size_t)message_bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, message_record_id);
    put_le32(tail, 3u * 4u, 0x12345678u);
    put_le16(tail, 64u * 4u + 2u, 18u);
    put_le32(tail, (size_t)global_bucket * 4u, 65u);
    put_le32(tail, 65u * 4u,
             global_bucket == message_bucket ? 1u : 0u);
    put_le32(tail, 66u * 4u, global_record_id);

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

    raw[80] = 0x10u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].function = 7u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[80] == 0x14u,
          "restored falsewall SET timer applies its source cell flag");

    raw[80] = 0x10u;
    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[80] == 0x10u,
          "stale falsewall SET timer cannot mutate a source cell flag");

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

    profile.csbwin_timers[0].function = 101u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "restored parameter message reaches authenticated EXPOOL DSA dispatch");

    profile.csbwin_global_variables[1] = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_global_variables[1] == 0u,
          "altered parameter EXPOOL receipt cannot reach the live dispatcher");

    raw[80] = 0x83u; /* Source byte-map door, partially closed. */
    profile.csbwin_timers[0].function = 2u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].source_index = 0u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              raw[80] == 0x85u,
          "restored bash-door timer reaches its exact source door receipt");

    raw[80] = 0x83u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].source_index = 1u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              raw[80] == 0x83u,
          "malformed bash-door identity cannot fall through to generic destruction");

    raw[80] = 0x03u; /* Not a source byte-map door. */
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].source_index = 0u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              raw[80] == 0x03u,
          "bash-door receipt cannot mutate a non-door square");

    profile.party_state_valid = 1;
    profile.champion_count = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.Champions[0].EnableActionEventIndex = 19;
    profile.party_state.Champions[0].Attributes = 0x0008u;
    profile.party_state.Champions[0].CsbWinWord64 = 37;
    profile.party_state.Champions[0].ActionIndex = 30u;
    profile.csbwin_timers[0].function = 11u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].EnableActionEventIndex == -1 &&
              profile.party_state.Champions[0].Attributes == 0u &&
              profile.party_state.Champions[0].CsbWinWord64 == 0 &&
              profile.party_state.Champions[0].ActionIndex == CSB_V1_ACTION_NONE,
          "restored action-enable timer clears the authenticated champion lock");

    profile.party_state.Champions[0].EnableActionEventIndex = 19;
    profile.party_state.Champions[0].Attributes = 0x0008u;
    profile.party_state.Champions[0].CsbWinWord64 = 37;
    profile.party_state.Champions[0].ActionIndex = 30u;
    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].source_index = 1u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].EnableActionEventIndex == 19 &&
              profile.party_state.Champions[0].Attributes == 0x0008u &&
              profile.party_state.Champions[0].CsbWinWord64 == 37 &&
              profile.party_state.Champions[0].ActionIndex == 30u,
          "stale action-enable timer identity cannot clear a champion lock");

    profile.csbwin_timers[0].time = profile.game_time;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].ubyte6 = 1u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].EnableActionEventIndex == 19 &&
              profile.party_state.Champions[0].Attributes == 0x0008u &&
              profile.party_state.Champions[0].CsbWinWord64 == 37 &&
              profile.party_state.Champions[0].ActionIndex == 30u,
          "action-enable rearm branch remains blocked without saved inventory handoff");

    profile.party_state.Champions[0].HideDamageReceivedEventIndex = 23;
    profile.csbwin_timers[0].function = 12u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].HideDamageReceivedEventIndex == -1,
          "restored hide-damage timer clears the authenticated source receipt");

    profile.party_state.Champions[0].HideDamageReceivedEventIndex = 23;
    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].HideDamageReceivedEventIndex == 23,
          "stale hide-damage timer identity cannot clear a champion receipt");

    raw[80] = 0x81u; /* Source byte-map door, terminal closing step. */
    profile.csbwin_timers[0].function = 1u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[80] == 0x80u,
          "restored terminal door timer closes its exact source byte-map door");

    raw[80] = 0x81u;
    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[80] == 0x81u,
          "stale terminal door identity cannot fall through to generic animation");

    raw[80] = 0x82u; /* A later closing step would need source requeue. */
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[80] == 0x81u &&
              profile.csbwin_timers[0].time == profile.game_time &&
              profile.timeline_queue.eventCount == 1,
          "nonterminal restored door timer retains source requeue");

    profile.csbwin_character_tail_invisible = 2u;
    profile.csbwin_timers[0].function = 71u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_invisible == 1u,
          "restored invisibility-expiry timer decrements its source party count");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_invisible == 1u,
          "stale invisibility-expiry identity cannot change the party count");

    profile.csbwin_character_tail_invisible = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_invisible == 0u,
          "invisibility expiry fails closed rather than underflowing a zero count");

    profile.party_state.Champions[0].ShieldStrength = 100u;
    profile.csbwin_timers[0].function = 72u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 30u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].ShieldStrength == 70u,
          "restored champion-shield timer applies its exact saved defense delta");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].ShieldStrength == 70u,
          "stale champion-shield identity cannot change saved defense");

    profile.party_state.Champions[0].ShieldStrength = 10u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].ubyte6 = 11u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].ShieldStrength == 10u,
          "underflowing champion-shield expiry remains fail-closed");

    profile.csbwin_character_tail_see_thru_walls = 2u;
    profile.csbwin_timers[0].function = 73u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_see_thru_walls == 1u,
          "restored Thieves Eye timer decrements its source party count");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_see_thru_walls == 1u,
          "stale Thieves Eye identity cannot change the party count");

    profile.csbwin_character_tail_see_thru_walls = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_see_thru_walls == 0u,
          "Thieves Eye expiry fails closed rather than underflowing a zero count");

    profile.csbwin_character_tail_party_shield = 100;
    profile.csbwin_timers[0].function = 74u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 30u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_party_shield == 70,
          "restored Party Shield timer applies its signed saved defense delta");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_party_shield == 70,
          "stale Party Shield identity cannot change saved defense");

    profile.csbwin_character_tail_party_shield = 10;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].ubyte6 = 0xffu;
    profile.csbwin_timers[0].ubyte7 = 0xffu;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_party_shield == 10,
          "nonpositive Party Shield expiry remains fail-closed");

    profile.csbwin_timers[0].ubyte6 = 11u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_party_shield == 10,
          "underflowing Party Shield expiry remains fail-closed");

    profile.csbwin_character_tail_spell_shield = 100;
    profile.csbwin_timers[0].function = 77u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 30u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_spell_shield == 70,
          "restored Spell Shield timer applies its signed saved defense delta");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_spell_shield == 70,
          "stale Spell Shield identity cannot change saved defense");

    profile.csbwin_character_tail_spell_shield = 10;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].ubyte6 = 0xffu;
    profile.csbwin_timers[0].ubyte7 = 0xffu;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_spell_shield == 10,
          "nonpositive Spell Shield expiry remains fail-closed");

    profile.csbwin_timers[0].ubyte6 = 11u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_spell_shield == 10,
          "underflowing Spell Shield expiry remains fail-closed");

    profile.csbwin_character_tail_fire_shield = 100;
    profile.csbwin_timers[0].function = 78u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 30u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_fire_shield == 70,
          "restored Fire Shield timer applies its signed saved defense delta");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_fire_shield == 70,
          "stale Fire Shield identity cannot change saved defense");

    profile.csbwin_character_tail_fire_shield = 10;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].ubyte6 = 0xffu;
    profile.csbwin_timers[0].ubyte7 = 0xffu;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_fire_shield == 10,
          "nonpositive Fire Shield expiry remains fail-closed");

    profile.csbwin_timers[0].ubyte6 = 11u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_fire_shield == 10,
          "underflowing Fire Shield expiry remains fail-closed");

    profile.csbwin_character_tail_magic_footprints_active = 2u;
    profile.csbwin_timers[0].function = 79u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_magic_footprints_active == 1u,
          "restored Magic Footprints timer expires one source effect");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_magic_footprints_active == 1u,
          "stale Magic Footprints identity cannot change saved effect count");

    profile.csbwin_character_tail_magic_footprints_active = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_character_tail_magic_footprints_active == 0u,
          "underflowing Magic Footprints expiry remains fail-closed");

    profile.party_state.Champions[0].CurrentHealth = 10;
    profile.party_state.Champions[0].PoisonDose = 0u;
    profile.party_state.Champions[0].PoisonEventCount = 1u;
    profile.csbwin_timers[0].function = 75u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 3u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].CurrentHealth == 9 &&
              profile.party_state.Champions[0].PoisonDose == 3u &&
              profile.party_state.Champions[0].PoisonEventCount == 1u,
          "restored poison timer applies its exact C75 tick and requeue");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].CurrentHealth == 9 &&
              profile.party_state.Champions[0].PoisonDose == 3u &&
              profile.party_state.Champions[0].PoisonEventCount == 1u,
          "stale poison identity cannot enter the C75 runtime chain");

    profile.party_state.Champions[0].PoisonEventCount = 1u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].CurrentHealth == 9 &&
              profile.party_state.Champions[0].PoisonDose == 3u &&
              profile.party_state.Champions[0].PoisonEventCount == 1u,
          "wide saved poison attack remains fail-closed without truncation");

    /* Rebuild the source square after prior timer cases and give TT_65 its
     * actual timerObj8 actuator handle (type 3, index 0). */
    raw[60u] = 0u;
    raw[61u] = 0u;
    raw[80u] = 0x10u;
    put_le16(raw, 62u, (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10));
    put_le16(raw, 90u, 0xfffeu);
    put_le16(raw, 92u, 0u);
    profile.csbwin_timers[0].function = 65u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 =
        (uint8_t)(CSB_V1_THING_TYPE_ACTUATOR << 2);
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              (raw[92u] & 0x7fu) == 6u,
          "restored generator timer reactivates its exact saved actuator");

    put_le16(raw, 92u, 0u);
    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[92u] == 0u,
          "stale generator timer cannot enter generic C65 mutation");

    profile.party_dir = 3;
    profile.party_state.Champions[0].MaximumHealth = 100;
    profile.party_state.Champions[0].CurrentHealth = 0;
    profile.party_state.Champions[0].Attributes =
        CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    profile.csbwin_timers[0].function = 13u;
    profile.csbwin_timers[0].ubyte5 = 0u; /* packed position 0, state 0 */
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].level = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].MaximumHealth == 98 &&
              profile.party_state.Champions[0].CurrentHealth == 49 &&
              profile.party_state.Champions[0].Direction == 3u &&
              (profile.party_state.Champions[0].Attributes &
               CSB_V1_CHAMPION_ATTRIBUTE_DEAD) == 0u,
          "restored Vi Altar final stage applies exact source life penalty");

    profile.party_state.Champions[0].MaximumHealth = 100;
    profile.party_state.Champions[0].CurrentHealth = 0;
    profile.party_state.Champions[0].Attributes =
        CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].MaximumHealth == 100 &&
              profile.party_state.Champions[0].CurrentHealth == 0 &&
              (profile.party_state.Champions[0].Attributes &
               CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0u,
          "stale Vi Altar receipt cannot enter generic rebirth handling");

    put_le16(raw, 92u, 0u);
    profile.csbwin_timers[0].function = 65u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].ubyte8 = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              (raw[92u] & 0x7fu) == 6u,
          "old-save generator timer uses CSBWin first-disabled fallback");

    raw[80u] = 0x10u;
    put_le16(raw, 62u, (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10));
    put_le16(raw, 90u, 0xfffeu);
    put_le16(raw, 92u, 0x012fu);
    profile.csbwin_timers[0].function = 24u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 =
        (uint8_t)(CSB_V1_THING_TYPE_ACTUATOR << 2);
    profile.csbwin_timers[0].level = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[62u] == 0xfeu &&
              raw[63u] == 0xffu && raw[90u] == 0xffu && raw[91u] == 0xffu,
          "restored TT_24 removes and frees its exact saved object");

    raw[80u] = 0x10u;
    put_le16(raw, 62u, (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10));
    put_le16(raw, 90u, 0xfffeu);
    put_le16(raw, 92u, 0x012fu);
    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 && raw[62u] == 0x00u &&
              raw[63u] == 0x0cu && raw[90u] == 0xfeu && raw[91u] == 0xffu,
          "stale TT_24 receipt cannot unlink or free a saved object");

    profile.csbwin_timers[0].function = 53u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[0].level = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_timers[0].time == profile.game_time + 299u &&
              profile.timeline_queue.eventCount == 1,
          "restored watchdog timer retains its CSBWin owner through +300 requeue");

    profile.game_time = profile.csbwin_timers[0].time;
    check(csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_timers[0].time == profile.game_time + 299u &&
              profile.timeline_queue.eventCount == 1,
          "requeued watchdog remains owned by its original saved timer slot");

    profile.csbwin_timers[0].source_index = 1u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.timeline_queue.eventCount == 0 &&
              profile.csbwin_timers[0].time == profile.game_time - 1u,
          "stale watchdog identity cannot enter a generic recurring path");

    /* CSBWin Timer.cpp ProcessTimer60and61 reads timerObj8 before the shared
     * C60/C61 movement path. The party-square, non-Lord-Chaos branch has an
     * exact source +5 requeue and does not need the unretained move/RNG data. */
    put_le16(raw, 100u, 0xfffeu);
    raw[104u] = 0u; /* DB4 monsterType: Scorpion, not mon_LordChaos (0x17). */
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_GROUP] = 100;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_GROUP] = 1;
    profile.current_level = 0;
    profile.party_x = 0;
    profile.party_y = 0;
    profile.csbwin_timers[0].function = 60u;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[0].ubyte6 = 0u;
    profile.csbwin_timers[0].ubyte7 = 0u;
    profile.csbwin_timers[0].ubyte8 = 0u;
    profile.csbwin_timers[0].ubyte9 =
        (uint8_t)(CSB_V1_THING_TYPE_GROUP << 2);
    profile.csbwin_timers[0].level = 0u;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = profile.game_time;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_timers[0].time == profile.game_time + 4u &&
              profile.timeline_queue.eventCount == 1,
          "restored TT_60 preserves its CSBWin payload before M10 group mutation");

    profile.game_time = profile.csbwin_timers[0].time;
    check(csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.csbwin_timers[0].time == profile.game_time + 4u &&
              profile.timeline_queue.eventCount == 1,
          "requeued TT_60 retains its original CSBWin timer queue owner");
    return failures == 0 ? 0 : 1;
}
