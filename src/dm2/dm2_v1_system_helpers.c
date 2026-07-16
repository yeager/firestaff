#include "dm2_v1_system_helpers.h"

#include <limits.h>

enum {
    DM2_V1_TILE_TELEPORTER = 5,
    DM2_V1_TILE_MAP_EXIT = 7,
};

static void dm2_v1_system_receipt_set(
    DM2_V1_SystemReceipt* receipt,
    const char* symbol,
    int valid,
    int result,
    int blocked)
{
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->valid = valid;
    receipt->result = result;
    receipt->blocked = blocked;
    receipt->symbol = symbol;
    receipt->source_path = dm2_v1_system_helpers_source_evidence();
}

void dm2_v1_system_receipt_clear(DM2_V1_SystemReceipt* receipt)
{
    if (!receipt) {
        return;
    }
    receipt->handled = 0;
    receipt->source_locked = 0;
    receipt->valid = 0;
    receipt->result = 0;
    receipt->blocked = 0;
    receipt->symbol = 0;
    receipt->source_path = 0;
}

uint16_t dm2_v1_RETURN_1(void* ref, DM2_V1_SystemReceipt* out_receipt)
{
    (void)ref;
    dm2_v1_system_receipt_clear(out_receipt);
    dm2_v1_system_receipt_set(out_receipt, "RETURN_1", 1, 1, 0);
    return 1;
}

uint16_t dm2_v1_IS_GAME_ENDED(
    uint8_t ref_b1,
    uint8_t game_has_ended,
    DM2_V1_SystemReceipt* out_receipt)
{
    const uint16_t result = (ref_b1 == game_has_ended) ? 1u : 0u;
    dm2_v1_system_receipt_clear(out_receipt);
    dm2_v1_system_receipt_set(out_receipt, "IS_GAME_ENDED", 1, (int)result, 0);
    return result;
}

static int dm2_v1_candidate_blocks_level_change(
    const uint8_t* candidate_tile_types,
    const uint8_t* candidate_active_teleporters,
    size_t map_index)
{
    if (!candidate_tile_types) {
        return 0;
    }
    if (candidate_tile_types[map_index] == DM2_V1_TILE_MAP_EXIT) {
        return 1;
    }
    return candidate_tile_types[map_index] == DM2_V1_TILE_TELEPORTER &&
        candidate_active_teleporters &&
        candidate_active_teleporters[map_index] != 0u;
}

static int dm2_v1_map_contains_global_position(
    const DM2_V1_MapDefinitionCompat* map,
    int32_t global_x,
    int32_t global_y)
{
    const int32_t min_x = (int32_t)map->offset_x - 1;
    const int32_t max_x = (int32_t)map->offset_x + map->raw_columns + 1;
    const int32_t min_y = (int32_t)map->offset_y - 1;
    const int32_t max_y = (int32_t)map->offset_y + map->raw_rows;
    return global_x >= min_x && global_x <= max_x &&
        global_y >= min_y && global_y <= max_y;
}

int dm2_v1_LOCATE_OTHER_LEVEL(
    const DM2_V1_MapDefinitionCompat* maps,
    size_t map_count,
    const uint16_t* first_map_by_level,
    size_t level_count,
    const int8_t* scan_order,
    size_t scan_order_count,
    const uint8_t* candidate_tile_types,
    const uint8_t* candidate_active_teleporters,
    uint16_t current_map,
    int16_t z_delta,
    int16_t* x,
    int16_t* y,
    DM2_V1_LocateOtherLevelResult* out_result,
    DM2_V1_SystemReceipt* out_receipt)
{
    int32_t target_level;
    int32_t global_x;
    int32_t global_y;
    size_t scan_limit;

    dm2_v1_system_receipt_clear(out_receipt);
    if (out_result) {
        out_result->map_index = -1;
        out_result->x = 0;
        out_result->y = 0;
        out_result->scan_index = 0;
        out_result->valid = 0;
    }

    if (!maps || !x || !y || current_map >= map_count) {
        dm2_v1_system_receipt_set(out_receipt, "LOCATE_OTHER_LEVEL", 0, -1, 1);
        return -1;
    }

    target_level = (int32_t)maps[current_map].level + z_delta;
    if (target_level < 0 || (size_t)target_level >= level_count) {
        dm2_v1_system_receipt_set(out_receipt, "LOCATE_OTHER_LEVEL", 0, -1, 1);
        return -1;
    }
    if (first_map_by_level && first_map_by_level[target_level] == UINT16_MAX) {
        dm2_v1_system_receipt_set(out_receipt, "LOCATE_OTHER_LEVEL", 0, -1, 1);
        return -1;
    }

    global_x = (int32_t)*x + maps[current_map].offset_x;
    global_y = (int32_t)*y + maps[current_map].offset_y;
    scan_limit = scan_order ? scan_order_count : map_count;

    for (size_t scan = 0; scan < scan_limit; ++scan) {
        const int32_t ordered_map = scan_order ? (int32_t)scan_order[scan] : (int32_t)scan;
        int16_t local_x;
        int16_t local_y;
        if (ordered_map < 0 || (size_t)ordered_map >= map_count) {
            continue;
        }
        if (maps[ordered_map].level != target_level) {
            continue;
        }
        if (!dm2_v1_map_contains_global_position(&maps[ordered_map], global_x, global_y)) {
            continue;
        }
        if (dm2_v1_candidate_blocks_level_change(
                candidate_tile_types,
                candidate_active_teleporters,
                (size_t)ordered_map)) {
            continue;
        }
        local_x = (int16_t)(global_x - maps[ordered_map].offset_x);
        local_y = (int16_t)(global_y - maps[ordered_map].offset_y);
        *x = local_x;
        *y = local_y;
        if (out_result) {
            out_result->map_index = (int16_t)ordered_map;
            out_result->x = local_x;
            out_result->y = local_y;
            out_result->scan_index = scan;
            out_result->valid = 1;
        }
        dm2_v1_system_receipt_set(out_receipt, "LOCATE_OTHER_LEVEL", 1, (int)ordered_map, 0);
        return (int)ordered_map;
    }

    dm2_v1_system_receipt_set(out_receipt, "LOCATE_OTHER_LEVEL", 0, -1, 1);
    return -1;
}

int dm2_v1_GUARANTEE_FREE_CPXHEAP_SIZE(
    int32_t requested_size,
    int32_t* current_free,
    const int32_t* freeable_chunks,
    size_t freeable_chunk_count,
    size_t* inout_next_chunk,
    DM2_V1_SystemReceipt* out_receipt)
{
    dm2_v1_system_receipt_clear(out_receipt);
    if (requested_size <= 0 || !current_free || !freeable_chunks || !inout_next_chunk) {
        dm2_v1_system_receipt_set(out_receipt, "GUARANTEE_FREE_CPXHEAP_SIZE", 0, 0, 1);
        return 0;
    }
    while (*current_free < requested_size) {
        int32_t chunk;
        if (*inout_next_chunk >= freeable_chunk_count) {
            dm2_v1_system_receipt_set(
                out_receipt,
                "GUARANTEE_FREE_CPXHEAP_SIZE",
                0,
                *current_free,
                1);
            return 0;
        }
        chunk = freeable_chunks[*inout_next_chunk];
        ++(*inout_next_chunk);
        if (chunk > 0 && *current_free <= INT_MAX - chunk) {
            *current_free += chunk;
        }
    }
    dm2_v1_system_receipt_set(
        out_receipt,
        "GUARANTEE_FREE_CPXHEAP_SIZE",
        1,
        *current_free,
        0);
    return 1;
}

const char* dm2_v1_system_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp:3207,4273,11916,11926";
}
