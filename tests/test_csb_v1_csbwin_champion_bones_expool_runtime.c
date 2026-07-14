/* CSBWin Vi Altar ChampionBones EXPOOL runtime regression.
 * Source: CSBWin Timer.cpp ProcessTT_ViAltar:2692-2741;
 * data.cpp EXPOOL::GetChampionBonesRecord:1660-1669. */

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
    const uint16_t bones_thing = (uint16_t)(10u << 10);
    const uint32_t record_id = (9u << 24) | bones_thing;
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 80;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 62;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[10] = 100;
    dungeon.thing_type_counts[10] = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    raw[80] = 0x10u;
    put_le16(raw, 62u, bones_thing);
    put_le16(raw, 100u, 0xfffeu);
    put_le16(raw, 102u, 0x0005u);

    /* One three-word DB11 node: link, key, one fingerprint word. */
    put_le16(tail, 2u, 3u);
    put_le32(tail, (size_t)bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, record_id);
    put_le32(tail, 3u * 4u, 0x0000beefu);

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.party_state_valid = 1;
    profile.champion_count = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.Champions[0].Fingerprint = 0xbeefu;
    profile.party_state.Champions[0].MaximumHealth = 100;
    profile.party_state.Champions[0].CurrentHealth = 0;
    profile.party_state.Champions[0].Attributes =
        CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    profile.csbwin_timer_summary_count = 1u;
    profile.csbwin_timer_summary_total = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_timer_queue[0] = 0u;
    profile.csbwin_timers[0].valid = 1;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].function = 13u;
    profile.csbwin_timers[0].time = 0u;
    profile.csbwin_timers[0].ubyte5 = 1u;
    profile.csbwin_timers[0].ubyte8 = (uint8_t)bones_thing;
    profile.csbwin_timers[0].ubyte9 = (uint8_t)(bones_thing >> 8);

    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              raw[62u] == 0xfeu && raw[63u] == 0xffu &&
              raw[100u] == 0xfeu && raw[101u] == 0xffu &&
              raw[102u] == 0u && raw[103u] == 0u &&
              profile.csbwin_timers[0].ubyte5 == 0u &&
              profile.timeline_queue.eventCount == 1 &&
              csb_v1_runtime_locate_csbwin_appended_expool_record(
                  &profile, record_id, &payload, &payload_size) == 0,
          "CSBWin Vi Altar consumes saved ChampionBones identity and queues C13");

    profile.game_time = profile.csbwin_timers[0].time;
    check(csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.party_state.Champions[0].MaximumHealth == 98 &&
              profile.party_state.Champions[0].CurrentHealth == 49 &&
              (profile.party_state.Champions[0].Attributes &
               CSB_V1_CHAMPION_ATTRIBUTE_DEAD) == 0u,
          "CSBWin Vi Altar fingerprint successor resurrects the matched champion");

    profile.dungeon_handle = NULL;
    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
