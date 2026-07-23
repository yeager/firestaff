#include "csb_v1_f0272_f0274_sensor_pc34_compat.h"

#include <string.h>

static uint16_t read_u16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static int event_type_for_square(int square_type)
{
    static const uint8_t event_type[7] = {
        DM1_EVENT_WALL, DM1_EVENT_CORRIDOR, DM1_EVENT_PIT, DM1_EVENT_NONE,
        DM1_EVENT_DOOR, DM1_EVENT_TELEPORTER, DM1_EVENT_FAKEWALL
    };

    return square_type >= 0 && square_type < 7 ? event_type[square_type] : -1;
}

static int sensor_is_linked_to_source_square(const CSB_V1_DungeonData *dungeon,
                                             int level, int source_x,
                                             int source_y, uint16_t sensor)
{
    int thing;
    int guard;

    thing = csb_v1_dungeon_get_first_thing(dungeon, level, source_x, source_y);
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff; ++guard) {
        const uint8_t *record;
        int size;

        if ((uint16_t)thing == sensor) return 1;
        record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                  NULL, NULL, &size);
        if (!record || size < 2) return 0;
        thing = (int)read_u16(record);
    }
    return 0;
}

int csb_v1_f0272_trigger_remote_effect_pc34(
    CSB_V1_RuntimeProfile *profile, uint16_t sensor_thing, int effect,
    int source_x, int source_y, CSB_V1_F0272F0274ReceiptPc34 *out_receipt)
{
    CSB_V1_F0272F0274ReceiptPc34 receipt;
    const uint8_t *record;
    int type;
    int size;
    uint16_t flags;
    uint16_t target;
    int target_square;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.event_receipt.event_index = -1;
    *out_receipt = receipt;
    if (!profile || !profile->dungeon_handle ||
        !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 ||
        profile->current_level < 0 || effect < DM1_EFFECT_SET ||
        effect > DM1_EFFECT_TOGGLE ||
        csb_v1_dungeon_get_raw_square(profile->dungeon_handle,
                                       profile->current_level,
                                       source_x, source_y) < 0) {
        return 0;
    }
    record = csb_v1_dungeon_get_thing_record(profile->dungeon_handle,
                                              sensor_thing, &type, NULL, &size);
    if (!record || type != 3 || size < 8 ||
        !sensor_is_linked_to_source_square(profile->dungeon_handle,
                                           profile->current_level,
                                           source_x, source_y,
                                           sensor_thing)) {
        return 0;
    }
    flags = read_u16(record + 4);
    target = read_u16(record + 6);
    /* LocalEffect is F0270/F0271-owned and intentionally not crossed here. */
    if ((flags & 0x0800u) != 0) return 0;
    receipt.target_cell = (int)((target >> 4) & 0x03u);
    receipt.target_x = (int)((target >> 6) & 0x1fu);
    receipt.target_y = (int)((target >> 11) & 0x1fu);
    target_square = csb_v1_dungeon_get_raw_square(profile->dungeon_handle,
                                                    profile->current_level,
                                                    receipt.target_x,
                                                    receipt.target_y);
    if (target_square < 0) return 0;
    receipt.event_type = (uint8_t)event_type_for_square((target_square >> 5) & 7);
    if (receipt.event_type == DM1_EVENT_NONE) return 0;
    if (((target_square >> 5) & 7) != 0) receipt.target_cell = 0;
    receipt.event_time = profile->game_time + ((flags >> 7) & 0x0fu);
    if (!csb_v1_f0268_add_event_pc34(profile, receipt.event_type,
                                     receipt.target_x, receipt.target_y,
                                     receipt.target_cell, effect,
                                     receipt.event_time,
                                     &receipt.event_receipt)) {
        return 0;
    }
    receipt.valid = 1;
    receipt.sensor_thing = sensor_thing;
    receipt.source_x = source_x;
    receipt.source_y = source_y;
    receipt.effect = effect;
    receipt.once_only = (flags & 0x0004u) != 0;
    receipt.source_evidence =
        "ReDMCSB SENSOR.C F0272 raw C03 remote target -> F0268";
    *out_receipt = receipt;
    return 1;
}

int csb_v1_f0274_party_possession_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int object_type,
    CSB_V1_F0272F0274ReceiptPc34 *out_receipt)
{
    CSB_V1_F0272F0274ReceiptPc34 receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out_receipt = receipt;
    if (!profile || !profile->dungeon_handle ||
        !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 || object_type < 0) {
        return 0;
    }
    receipt.possession_found =
        csb_v1_runtime_f0274_is_object_in_party_possession_pc34(profile,
                                                                  object_type);
    receipt.valid = 1;
    receipt.source_evidence =
        "ReDMCSB SENSOR.C F0274 loaded CHARDESC and leader-hand receipt";
    *out_receipt = receipt;
    return 1;
}
