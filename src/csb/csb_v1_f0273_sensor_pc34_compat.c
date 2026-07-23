#include "csb_v1_f0273_sensor_pc34_compat.h"

#include <string.h>

static uint16_t read_u16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

int csb_v1_f0273_get_object_of_type_in_cell_pc34(
    const CSB_V1_RuntimeProfile *profile, int map_x, int map_y, int cell,
    int object_type, CSB_V1_F0273ReceiptPc34 *out_receipt)
{
    CSB_V1_F0273ReceiptPc34 receipt;
    const CSB_V1_DungeonData *dungeon;
    int thing;
    int guard;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.matched_thing = 0xffffu;
    receipt.matched_cell = -1;
    *out_receipt = receipt;
    if (!profile || !profile->dungeon_handle ||
        !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 ||
        profile->current_level < 0 || cell < -1 || cell > 3 ||
        object_type < 0 || object_type > 0x7f) {
        return 0;
    }
    dungeon = profile->dungeon_handle;
    if (csb_v1_dungeon_get_raw_square(dungeon, profile->current_level,
                                      map_x, map_y) < 0) {
        return 0;
    }
    thing = csb_v1_dungeon_get_first_thing(dungeon, profile->current_level,
                                           map_x, map_y);
    if (thing < 0) return 0;
    receipt.valid = 1;
    receipt.map_index = profile->current_level;
    receipt.map_x = map_x;
    receipt.map_y = map_y;
    receipt.requested_cell = cell;
    receipt.object_type = object_type;
    receipt.source_evidence =
        "ReDMCSB SENSOR.C F0273; CSBWin Mouse.cpp FindObjectOfTypeAtPosition";
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff; ++guard) {
        const uint8_t *record;
        int thing_type;
        int record_size;
        int thing_cell;

        record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                  &thing_type, NULL, &record_size);
        if (!record || record_size < 2) return 0;
        thing_cell = ((uint16_t)thing >> 14) & 3;
        if (thing_type > CSB_V1_THING_TYPE_GROUP && thing_type < 14 &&
            record_size >= 4 && (int)(read_u16(record + 2) & 0x007fu) == object_type &&
            (cell == -1 || thing_cell == cell)) {
            receipt.matched_thing = (uint16_t)thing;
            receipt.matched_cell = thing_cell;
            *out_receipt = receipt;
            return 1;
        }
        thing = (int)read_u16(record);
    }
    if (guard >= 128) return 0;
    *out_receipt = receipt;
    return 1;
}
