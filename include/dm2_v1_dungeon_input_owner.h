#ifndef FIRESTAFF_DM2_V1_DUNGEON_INPUT_OWNER_H
#define FIRESTAFF_DM2_V1_DUNGEON_INPUT_OWNER_H

/*
 * Source-owned PC DM2 dungeon pointer route.
 *
 * The route table is the relocated MOUSE_INPUT inventory from SKWIN's
 * skval1.h, decoded against the PC-English GRAPHICS.DAT rectangle pool.
 * The caller must supply the scanner's immutable GRAPHICS.DAT MD5; the
 * route is deliberately unavailable for a different edition instead of
 * borrowing a geometry table from DM1, FM Towns, or Amiga.
 *
 * Source: SKWIN skval1.h:92/109/110/113/114; SkWinCore.cpp:12131-12242,
 * 15239-15303, 55037-55120 (DM2_1031_07d6 / _030a / _0a88).
 */

#include <stdint.h>

#include "dm2_touch_click_zone_matrix_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_DUNGEON_INPUT_PC_EN_GRAPHICS_MD5 \
    "25247ede4dabb6a71e5dabdfbcd5907d"

typedef struct {
    int active;
    char graphics_md5[33];
    uint32_t source_table_hash;
} DM2_V1_DungeonInputOwner;

typedef struct {
    int accepted;
    int blocked_unverified_graphics;
    int blocked_no_source_zone;
    uint16_t event_index;
    uint16_t source_zone_index;
    int16_t source_x;
    int16_t source_y;
    int16_t source_w;
    int16_t source_h;
    uint32_t source_table_hash;
} DM2_V1_DungeonInputReceipt;

typedef void (*DM2_V1_DungeonInputEventSink)(void *ctx,
                                              int16_t event_index,
                                              int16_t x,
                                              int16_t y);

/* Activates the PC-English table only after asset discovery has supplied
 * the exact GRAPHICS.DAT MD5.  It does not read, extract, or manufacture
 * game data. */
int dm2_v1_dungeon_input_owner_init(DM2_V1_DungeonInputOwner *owner,
                                    const char *graphics_md5);

/* Routes one physical mouse/touch button through the source dungeon view.
 * On a hit, exactly the source event number and source-space pointer
 * coordinates are passed to c_input's eventual event sink. */
int dm2_v1_dungeon_input_owner_route(
    const DM2_V1_DungeonInputOwner *owner,
    int16_t screen_x,
    int16_t screen_y,
    unsigned int button_mask,
    DM2_V1_DungeonInputEventSink sink,
    void *sink_ctx,
    DM2_V1_DungeonInputReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
