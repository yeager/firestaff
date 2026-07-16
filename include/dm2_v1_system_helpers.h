#ifndef DM2_V1_SYSTEM_HELPERS_H
#define DM2_V1_SYSTEM_HELPERS_H

#include <stddef.h>
#include <stdint.h>

typedef struct DM2_V1_SystemReceipt {
    int handled;
    int source_locked;
    int valid;
    int result;
    int blocked;
    const char* symbol;
    const char* source_path;
} DM2_V1_SystemReceipt;

typedef struct DM2_V1_MapDefinitionCompat {
    int16_t level;
    int16_t offset_x;
    int16_t offset_y;
    int16_t raw_columns;
    int16_t raw_rows;
} DM2_V1_MapDefinitionCompat;

typedef struct DM2_V1_LocateOtherLevelResult {
    int16_t map_index;
    int16_t x;
    int16_t y;
    size_t scan_index;
    int valid;
} DM2_V1_LocateOtherLevelResult;

void dm2_v1_system_receipt_clear(DM2_V1_SystemReceipt* receipt);

uint16_t dm2_v1_RETURN_1(void* ref, DM2_V1_SystemReceipt* out_receipt);

uint16_t dm2_v1_IS_GAME_ENDED(
    uint8_t ref_b1,
    uint8_t game_has_ended,
    DM2_V1_SystemReceipt* out_receipt);

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
    DM2_V1_SystemReceipt* out_receipt);

int dm2_v1_GUARANTEE_FREE_CPXHEAP_SIZE(
    int32_t requested_size,
    int32_t* current_free,
    const int32_t* freeable_chunks,
    size_t freeable_chunk_count,
    size_t* inout_next_chunk,
    DM2_V1_SystemReceipt* out_receipt);

const char* dm2_v1_system_helpers_source_evidence(void);

#endif
