#include "csb_v1_f0268_f0269_sensor_pc34_compat.h"

#include <string.h>

static int csb_v1_f0268_f0269_has_pc34_source(const CSB_V1_RuntimeProfile *profile)
{
    return profile && profile->dungeon_handle && profile->dungeon_handle->raw_data &&
        profile->dungeon_handle->raw_size > 0 &&
        profile->dungeon_handle->square_bytes == 1;
}

int csb_v1_f0268_add_event_pc34(
    CSB_V1_RuntimeProfile *profile, uint8_t event_type, int map_x, int map_y,
    int cell, int effect, uint32_t event_time,
    CSB_V1_F0268F0269ReceiptPc34 *out_receipt)
{
    CSB_V1_F0268F0269ReceiptPc34 receipt;
    struct DM1_Event_V1 event;
    int raw_square;
    int event_index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.event_index = -1;
    *out_receipt = receipt;
    if (!csb_v1_f0268_f0269_has_pc34_source(profile) ||
        profile->current_level < 0 || profile->current_level >=
            profile->dungeon_handle->level_count || cell < 0 || cell > 3 ||
        event_time < profile->game_time) return 0;
    raw_square = csb_v1_dungeon_get_raw_square(
        profile->dungeon_handle, profile->current_level, map_x, map_y);
    if (raw_square < 0) return 0;
    memset(&event, 0, sizeof(event));
    event.map_time = ((uint32_t)profile->current_level << 24) |
        (event_time & 0x00FFFFFFu);
    event.type = event_type;
    event.priority = 0;
    event.b_mapX = (uint8_t)map_x;
    event.b_mapY = (uint8_t)map_y;
    event.c_cell = (uint8_t)cell;
    event.c_effect = (uint8_t)effect;
    event_index = csb_v1_runtime_add_timeline_event(profile, &event);
    if (event_index < 0) return 0;
    receipt.valid = 1;
    receipt.event_index = event_index;
    receipt.map_index = profile->current_level;
    receipt.map_x = map_x;
    receipt.map_y = map_y;
    receipt.cell = cell;
    receipt.effect = effect;
    receipt.event_type = event_type;
    receipt.event_time = event_time;
    receipt.source_evidence = "ReDMCSB SENSOR.C F0268 -> raw PC34 square -> F0238";
    *out_receipt = receipt;
    return 1;
}

int csb_v1_f0269_skill_experience_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int skill_index, int experience,
    int leader_only, CSB_V1_F0268F0269ReceiptPc34 *out_receipt)
{
    CSB_V1_F0268F0269ReceiptPc34 receipt;
    int i;
    int party_count;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.event_index = -1;
    *out_receipt = receipt;
    if (!csb_v1_f0268_f0269_has_pc34_source(profile) ||
        !profile->party_state_valid || skill_index < 0 ||
        skill_index >= CSB_V1_FULL_SKILL_COUNT || experience <= 0) return 0;
    party_count = profile->party_state.ChampionCount;
    if (party_count <= 0 || party_count > CSB_V1_MAX_CHAMPIONS) return 0;
    receipt.leader_only = leader_only ? 1 : 0;
    if (leader_only) {
        if (profile->leader_index < 0 || profile->leader_index >= party_count ||
            profile->party_state.Champions[profile->leader_index].CurrentHealth <= 0) return 0;
        receipt.recipient_count = 1;
        receipt.experience_per_recipient = experience;
    } else {
        receipt.experience_per_recipient = experience / party_count;
        for (i = 0; i < party_count; ++i) {
            if (profile->party_state.Champions[i].CurrentHealth > 0) {
                ++receipt.recipient_count;
            }
        }
        if (receipt.recipient_count == 0) return 0;
    }
    receipt.valid = 1;
    receipt.effect = skill_index;
    receipt.source_evidence = "ReDMCSB SENSOR.C F0269 -> loaded PC34 party state";
    *out_receipt = receipt;
    return 1;
}
