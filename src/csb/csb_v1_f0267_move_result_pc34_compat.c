#include "csb_v1_f0267_move_result_pc34_compat.h"

#include <string.h>

#define CSB_V1_F0267_THING_NONE 0xFFFFu
#define CSB_V1_F0267_THING_END  0xFFFEu

static uint32_t csb_v1_f0267_fnv1a32(const uint8_t *bytes, int size)
{
    uint32_t value = 2166136261u;
    int i;

    if (!bytes || size <= 0) return 0u;
    for (i = 0; i < size; ++i) {
        value ^= bytes[i];
        value *= 16777619u;
    }
    return value;
}

static int csb_v1_f0267_square_contains_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t wanted,
    int map_index,
    int map_x,
    int map_y)
{
    int thing;
    int guard;

    thing = csb_v1_dungeon_get_first_thing(
        dungeon, map_index, map_x, map_y);
    if (thing < 0) return 0;
    for (guard = 0;
         guard < 128 && thing != CSB_V1_F0267_THING_NONE &&
             thing != CSB_V1_F0267_THING_END;
         ++guard) {
        const uint8_t *record;
        int size = 0;

        record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, NULL, NULL, &size);
        if (!record || size < 2) return 0;
        if ((uint16_t)thing == wanted) return 1;
        thing = (int)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
    }
    return 0;
}

int csb_v1_f0267_move_result_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    CSB_V1_F0267VariantPc34 variant,
    uint16_t thing,
    int source_map_index,
    int source_map_x,
    int source_map_y,
    int destination_map_index,
    int destination_map_x,
    int destination_map_y,
    CSB_V1_F0267MoveResultReceiptPc34 *out_receipt)
{
    CSB_V1_F0267MoveResultReceiptPc34 local_receipt;
    const uint8_t *record;
    int type = -1;
    int size = 0;
    int source_on_square;
    int destination_on_square;

    if (!out_receipt) return 0;
    memset(&local_receipt, 0, sizeof(local_receipt));
    local_receipt.variant = variant;
    local_receipt.thing = CSB_V1_F0267_THING_NONE;
    local_receipt.thing_type = -1;
    local_receipt.thing_record_offset = -1;
    local_receipt.source_map_index = -1;
    local_receipt.source_map_x = -1;
    local_receipt.source_map_y = -1;
    local_receipt.destination_map_index = -1;
    local_receipt.destination_map_x = -1;
    local_receipt.destination_map_y = -1;
    *out_receipt = local_receipt;

    /* IIGS.H supplies only a distinct declaration name in the audited corpus;
     * there is no source body or PC34 call contract to emulate. */
    if (variant != CSB_V1_F0267_VARIANT_CPSCE_GET_MOVE_RESULT_PC34) {
        return 0;
    }
    if (!dungeon || !dungeon->raw_data || dungeon->raw_size <= 0 ||
        dungeon->square_bytes != 1 || thing == CSB_V1_F0267_THING_NONE ||
        thing == CSB_V1_F0267_THING_END) {
        return 0;
    }
    record = csb_v1_dungeon_get_thing_record(
        dungeon, thing, &type, NULL, &size);
    if (!record || type < 0 || type > 15 || size < 2 ||
        record < dungeon->raw_data || record + size >
            dungeon->raw_data + dungeon->raw_size) {
        return 0;
    }

    source_on_square = source_map_x >= 0;
    destination_on_square = destination_map_x >= 0;
    if (source_on_square) {
        if (source_map_index < 0 || source_map_index >= dungeon->level_count ||
            csb_v1_dungeon_get_raw_square(
                dungeon, source_map_index, source_map_x, source_map_y) < 0 ||
            !csb_v1_f0267_square_contains_thing(
                dungeon, thing, source_map_index, source_map_x, source_map_y)) {
            return 0;
        }
        local_receipt.source_mode = CSB_V1_F0267_SOURCE_ON_SQUARE_PC34;
    } else if (source_map_x == -1 || source_map_x == -2) {
        if (!destination_on_square) return 0;
        local_receipt.source_mode = source_map_x == -2
            ? CSB_V1_F0267_SOURCE_PROJECTILE_ASSOCIATED_PC34
            : CSB_V1_F0267_SOURCE_NOT_ON_SQUARE_PC34;
    } else {
        return 0;
    }
    if (destination_on_square) {
        if (destination_map_index < 0 ||
            destination_map_index >= dungeon->level_count ||
            csb_v1_dungeon_get_raw_square(
                dungeon, destination_map_index, destination_map_x,
                destination_map_y) < 0) {
            return 0;
        }
        local_receipt.destination_mode =
            CSB_V1_F0267_DESTINATION_ON_SQUARE_PC34;
    } else if (destination_map_x == -1) {
        if (!source_on_square) return 0;
        local_receipt.destination_mode = CSB_V1_F0267_DESTINATION_REMOVE_PC34;
    } else {
        return 0;
    }

    local_receipt.thing = thing;
    local_receipt.thing_type = type;
    local_receipt.thing_record_offset = (int)(record - dungeon->raw_data);
    local_receipt.thing_record_size = size;
    local_receipt.thing_record_fnv1a = csb_v1_f0267_fnv1a32(record, size);
    local_receipt.source_map_index = source_map_index;
    local_receipt.source_map_x = source_map_x;
    local_receipt.source_map_y = source_map_y;
    local_receipt.destination_map_index = destination_map_index;
    local_receipt.destination_map_x = destination_map_x;
    local_receipt.destination_map_y = destination_map_y;
    local_receipt.source_evidence =
        "ReDMCSB MOVE.C F0267 CPSCE raw Thing/source/destination admission";
    local_receipt.valid = 1;
    *out_receipt = local_receipt;
    return 1;
}
