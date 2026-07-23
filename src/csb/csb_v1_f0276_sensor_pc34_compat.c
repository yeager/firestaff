#include "csb_v1_f0276_sensor_pc34_compat.h"

#include "csb_v1_f0272_f0274_sensor_pc34_compat.h"

#include <string.h>

#define CSB_V1_F0276_PARTY_THING 0xffffu
#define CSB_V1_F0276_END_OF_LIST 0xfffeu

static uint16_t read_u16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static int is_object_thing(int thing_type)
{
    return thing_type > CSB_V1_THING_TYPE_GROUP && thing_type < 14;
}

static int object_type(const CSB_V1_DungeonData *dungeon, uint16_t thing)
{
    const uint8_t *record;
    int thing_type;
    int record_size;

    record = csb_v1_dungeon_get_thing_record(dungeon, thing, &thing_type,
                                              NULL, &record_size);
    if (!record || !is_object_thing(thing_type) || record_size < 4) return -1;
    return (int)(read_u16(record + 2) & 0x007fu);
}

static int possession_matches(const CSB_V1_RuntimeProfile *profile,
                              int object_type_value)
{
    CSB_V1_F0272F0274ReceiptPc34 possession;

    if (!csb_v1_f0274_party_possession_receipt_pc34(profile,
                                                     object_type_value,
                                                     &possession)) {
        return -1;
    }
    return possession.possession_found != 0;
}

int csb_v1_f0276_sensor_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int map_x, int map_y,
    uint16_t moving_thing, int party_square, int add_thing,
    CSB_V1_F0276ReceiptPc34 *out_receipt)
{
    CSB_V1_F0276ReceiptPc34 receipt;
    const CSB_V1_DungeonData *dungeon;
    int square;
    int square_type;
    int moving_type;
    int moving_object_type = -1;
    int triggered_cell;
    int thing;
    int guard;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.moving_thing = moving_thing;
    receipt.moving_object_type = -1;
    receipt.sensor_thing = CSB_V1_F0276_PARTY_THING;
    receipt.sensor_cell = -1;
    *out_receipt = receipt;
    if (!profile || !profile->dungeon_handle ||
        !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 ||
        profile->current_level < 0 || (party_square != 0 && party_square != 1) ||
        (add_thing != 0 && add_thing != 1)) return 0;
    dungeon = profile->dungeon_handle;
    square = csb_v1_dungeon_get_raw_square(dungeon, profile->current_level,
                                            map_x, map_y);
    if (square < 0) return 0;
    square_type = (square >> 5) & 7;
    if (moving_thing == CSB_V1_F0276_PARTY_THING) {
        moving_type = -1;
    } else {
        const uint8_t *record;
        int record_size;

        record = csb_v1_dungeon_get_thing_record(dungeon, moving_thing,
                                                  &moving_type, NULL, &record_size);
        if (!record || record_size < 2 || !is_object_thing(moving_type)) return 0;
        moving_object_type = object_type(dungeon, moving_thing);
        if (moving_object_type < 0) return 0;
    }
    triggered_cell = square_type == 0 ? ((moving_thing >> 14) & 3) : -1;
    thing = csb_v1_dungeon_get_first_thing(dungeon, profile->current_level,
                                           map_x, map_y);
    if (thing < 0) return 0;

    /* This receipt reads only the loaded chain; it deliberately does not run
     * F0276's source unlink/link phase. */
    for (guard = 0; guard < 128 && thing != CSB_V1_F0276_END_OF_LIST &&
         thing != CSB_V1_F0276_PARTY_THING; ++guard) {
        const uint8_t *record;
        int thing_type;
        int record_size;
        int include_object;

        record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                  &thing_type, NULL, &record_size);
        if (!record || record_size < 2) return 0;
        include_object = is_object_thing(thing_type) &&
            (triggered_cell < 0 || (((uint16_t)thing >> 14) & 3) == triggered_cell);
        if (thing_type == CSB_V1_THING_TYPE_GROUP && triggered_cell < 0) {
            receipt.square_contains_group = 1;
        } else if (include_object) {
            int candidate_object_type = object_type(dungeon, (uint16_t)thing);
            if (candidate_object_type < 0) return 0;
            receipt.square_contains_object = 1;
            if (candidate_object_type == moving_object_type) receipt.square_contains_same_type = 1;
            else receipt.square_contains_different_type = 1;
        }
        thing = (int)read_u16(record);
    }
    if (guard >= 128) return 0;

    thing = csb_v1_dungeon_get_first_thing(dungeon, profile->current_level,
                                           map_x, map_y);
    for (guard = 0; guard < 128 && thing != CSB_V1_F0276_END_OF_LIST &&
         thing != CSB_V1_F0276_PARTY_THING; ++guard) {
        const uint8_t *record;
        int thing_type;
        int record_size;
        uint16_t type_data;
        uint16_t flags;
        int sensor_type;
        int sensor_data;
        int sensor_cell;
        int trigger = add_thing;
        int effect;

        record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                  &thing_type, NULL, &record_size);
        if (!record || record_size < 2) return 0;
        if (thing_type >= CSB_V1_THING_TYPE_GROUP) break;
        if (thing_type != CSB_V1_THING_TYPE_ACTUATOR || record_size < 8) {
            thing = (int)read_u16(record);
            continue;
        }
        type_data = read_u16(record + 2);
        flags = read_u16(record + 4);
        sensor_type = type_data & 0x007f;
        sensor_data = type_data >> 7;
        sensor_cell = ((uint16_t)thing >> 14) & 3;
        if (sensor_type == 0) goto next_thing;
        if (triggered_cell < 0) {
            switch (sensor_type) {
            case 1:
                if (party_square || receipt.square_contains_object || receipt.square_contains_group) goto next_thing;
                break;
            case 2:
                if (moving_type > CSB_V1_THING_TYPE_GROUP || party_square || receipt.square_contains_group) goto next_thing;
                break;
            case 3:
                if (moving_type != -1 || profile->champion_count == 0) goto next_thing;
                if (sensor_data == 0) { if (party_square) goto next_thing; }
                else if (!add_thing || sensor_data != ((profile->party_dir & 3) + 1)) trigger = 0;
                break;
            case 4:
                if (moving_object_type < 0 || sensor_data != moving_object_type || receipt.square_contains_same_type) goto next_thing;
                break;
            case 5:
                if (moving_type != -1 || square_type != 3) goto next_thing;
                break;
            case 6:
                goto next_thing;
            case 7:
                if (moving_type > CSB_V1_THING_TYPE_GROUP || moving_type == -1 || receipt.square_contains_group) goto next_thing;
                break;
            case 8: {
                int matches;
                if (moving_type != -1) goto next_thing;
                matches = possession_matches(profile, sensor_data);
                if (matches < 0) return 0;
                trigger = matches;
                break;
            }
            case 9:
                if (moving_type != -1 || !add_thing || party_square) goto next_thing;
                trigger = sensor_data <= 34;
                break;
            default:
                goto next_thing;
            }
        } else {
            if (sensor_cell != triggered_cell) goto next_thing;
            switch (sensor_type) {
            case 1:
                if (receipt.square_contains_object) goto next_thing;
                break;
            case 2:
                if (moving_object_type < 0 || receipt.square_contains_same_type || sensor_data != moving_object_type) goto next_thing;
                break;
            case 3:
                if (moving_object_type < 0 || receipt.square_contains_different_type || sensor_data == moving_object_type) goto next_thing;
                break;
            default:
                goto next_thing;
            }
        }
        trigger ^= (flags >> 5) & 1;
        effect = (flags >> 3) & 3;
        if (effect != 3 && !trigger) goto next_thing;
        receipt.valid = 1;
        receipt.map_index = profile->current_level;
        receipt.map_x = map_x;
        receipt.map_y = map_y;
        receipt.moving_thing_type = moving_type;
        receipt.moving_object_type = moving_object_type;
        receipt.party_square = party_square;
        receipt.add_thing = add_thing;
        receipt.triggered_cell = triggered_cell;
        receipt.sensor_thing = (uint16_t)thing;
        receipt.sensor_type = sensor_type;
        receipt.sensor_data = sensor_data;
        receipt.sensor_cell = sensor_cell;
        receipt.effect = effect == 3 ? (trigger ? 0 : 1) : effect;
        receipt.would_trigger = trigger;
        receipt.source_evidence = "ReDMCSB MOVESENS.C F0276 raw C03 admission; no F0272/F0271 side effect";
        *out_receipt = receipt;
        return 1;
next_thing:
        thing = (int)read_u16(record);
    }
    if (guard >= 128) return 0;
    receipt.valid = 1;
    receipt.map_index = profile->current_level;
    receipt.map_x = map_x;
    receipt.map_y = map_y;
    receipt.moving_thing_type = moving_type;
    receipt.moving_object_type = moving_object_type;
    receipt.party_square = party_square;
    receipt.add_thing = add_thing;
    receipt.triggered_cell = triggered_cell;
    receipt.source_evidence = "ReDMCSB MOVESENS.C F0276 raw C03 admission; no source candidate";
    *out_receipt = receipt;
    return 1;
}
