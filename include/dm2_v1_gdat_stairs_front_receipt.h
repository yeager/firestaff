#ifndef FIRESTAFF_DM2_V1_GDAT_STAIRS_FRONT_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_STAIRS_FRONT_RECEIPT_H

#include "dm2_v1_asset_loader.h"

/*
 * DM2_DRAW_STAIRS_FRONT receipt (SKULLWIN/c_gui_vp.cpp:468 SKW_32cb_4e1c).
 *
 * Translates the register-level SKProject algorithm into a pure-data receipt:
 *   1. index = eaxl (view cell index)
 *   2. offset = index * 18  (8*index + index then *2)
 *   3. has_detail = (ptr1e1044[offset+8] != 0) ? 1 : 0
 *   4. rect = table1d6f9c[has_detail + 2*index]; if < 0 → no_draw
 *   5. field = table1d6f5c[has_detail + 2*index]
 *   6. If GDAT loadable(0x08, graphicsset, 1, field):
 *        → DRAW_DUNGEON_GRAPHIC(8, gs, field, rect, light, 0)
 *   7. Else: alt_field = table1d6f7c[has_detail + 2*index]
 *        → QUERY_TEMP_PICST + DRAW_TEMP_PICST fallback
 */

typedef struct {
    int valid;
    int no_draw;
    uint8_t view_cell;
    uint8_t graphicsset;
    uint16_t light_parameter;
    int has_detail;
    int16_t rect_number;
    uint8_t field;
    uint8_t alt_field;
    int loadable;
    uint32_t identity_hash;
} DM2_V1_GdatStairsFrontSourceReceipt;

typedef struct {
    int (*query_gdat_entry_if_loadable)(void *ctx, uint8_t cls, uint8_t sub, uint8_t idx, uint8_t field);
    const int16_t *table_1d6f9c;
    const uint8_t *table_1d6f5c;
    const uint8_t *table_1d6f7c;
    const uint8_t *detail_ptr;
} DM2_V1_StairsFrontCallbacks;

int dm2_v1_gdat_stairs_front_source_receipt(
    uint8_t cell, uint8_t graphicsset, uint16_t light,
    const DM2_V1_StairsFrontCallbacks *cb, void *ctx,
    DM2_V1_GdatStairsFrontSourceReceipt *out);

#endif
