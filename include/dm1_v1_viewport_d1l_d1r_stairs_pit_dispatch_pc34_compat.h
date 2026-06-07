#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_CORRIDOR = 1,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_PIT = 2,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_SIDE = 18,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D1LR_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC = 108,
    DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D1L = 4,
    DM1_V1_D1LR_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D1L = 11,
    DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D1L = 54,
    DM1_V1_D1LR_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D1L = 60,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D1L = 808,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D1R = 810,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D1L = 821,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D1R = 823,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1L = 858,
    DM1_V1_D1LR_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D1R = 860,
    DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1L = 4,
    DM1_V1_D1LR_STAIRS_PIT_PC34_VIEW_SQUARE_D1R = 5,
    DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1L_OPEN = 0x0032,
    DM1_V1_D1LR_STAIRS_PIT_PC34_CELL_ORDER_D1R_OPEN = 0x0041,
    DM1_V1_D1LR_STAIRS_PIT_PC34_TRANSPARENT_COLOR = 10
};

typedef enum {
    DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34 = 0,
    DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34 = 1
} DM1_V1_D1LD1RStairsPitSidePc34;

typedef enum {
    DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34 = 0,
    DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34 = 1,
    DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34 = 2,
    DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34 = 3
} DM1_V1_D1LD1RStairsPitRoutePc34;

typedef struct {
    DM1_V1_D1LD1RStairsPitSidePc34 side;
    DM1_V1_D1LD1RStairsPitRoutePc34 route;
    const char *role_name;
    const char *draw_square_function;
    const char *dispatch_anchor;
    const char *draw_anchor;
    const char *defs_anchor;
    const char *dungeon_anchor;
    int element_class;
    int native_bitmap_slot_or_graphic;
    int native_bitmap_index;
    int zone_index;
    int view_square_index;
    int cell_order;
    int transparent_color;
    bool uses_f0104;
    bool uses_f0105_flipped;
    bool uses_f0111;
    bool uses_f0115_thing_pass;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D1LD1RStairsPitEvidencePc34;

typedef struct {
    DM1_V1_D1LD1RStairsPitSidePc34 side;
    int direction;
    int map_x;
    int map_y;
    int element_class;
    bool stairs_up;
    bool pit_or_teleporter_visible;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D1LD1RStairsPitDispatchContextPc34;

typedef struct {
    bool ok;
    bool unsupported_element;
    bool contract_only;
    bool real_asset_claim;
    DM1_V1_D1LD1RStairsPitSidePc34 side;
    DM1_V1_D1LD1RStairsPitRoutePc34 route;
    int direction;
    int map_x;
    int map_y;
    int element_class;
    int native_bitmap_slot_or_graphic;
    int native_bitmap_index;
    int first_stairs_graphic_index;
    int zone_index;
    int view_square_index;
    int cell_order;
    bool used_f0104;
    bool used_f0105_flipped;
    bool used_f0111;
    bool used_f0115_thing_pass;
    const DM1_V1_D1LD1RStairsPitEvidencePc34 *evidence;
} DM1_V1_D1LD1RStairsPitDispatchResultPc34;

typedef struct {
    DM1_V1_D1LD1RStairsPitSidePc34 side;
    DM1_V1_D1LD1RStairsPitRoutePc34 route;
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    size_t destination_stride;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D1LD1RStairsPitBlitInputPc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
    bool flipped_horizontally;
    bool wrote_any;
    bool transparent_skip_seen;
    size_t row_width;
    size_t height;
    size_t byte_count;
    size_t destination_stride;
    size_t writes;
    size_t transparent_skips;
    uint8_t first_source_byte;
    uint8_t last_source_byte;
    uint8_t first_destination_byte;
    uint8_t last_destination_byte;
    const DM1_V1_D1LD1RStairsPitEvidencePc34 *evidence;
} DM1_V1_D1LD1RStairsPitBlitResultPc34;

typedef struct {
    int expected_assertions;
    int failures;
    bool evidence_table_complete;
    bool d1l_uses_f0104;
    bool d1r_uses_f0105;
    bool no_f0111;
    bool no_f0115_thing_pass;
} DM1_V1_D1LD1RStairsPitAssertResultPc34;

void M11_GameView_ViewportD1LD1RStairsPitDispatch_InitContextPc34(
    DM1_V1_D1LD1RStairsPitDispatchContextPc34 *context,
    DM1_V1_D1LD1RStairsPitSidePc34 side);

const DM1_V1_D1LD1RStairsPitEvidencePc34 *
M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidencePc34(size_t *count);

const DM1_V1_D1LD1RStairsPitEvidencePc34 *
M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
    DM1_V1_D1LD1RStairsPitSidePc34 side,
    DM1_V1_D1LD1RStairsPitRoutePc34 route);

bool M11_GameView_ViewportD1LD1RStairsPitDispatch_RenderPc34(
    const DM1_V1_D1LD1RStairsPitDispatchContextPc34 *context,
    DM1_V1_D1LD1RStairsPitDispatchResultPc34 *out);

bool M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(
    const DM1_V1_D1LD1RStairsPitBlitInputPc34 *input,
    DM1_V1_D1LD1RStairsPitBlitResultPc34 *out);

bool M11_GameView_ViewportD1LD1RStairsPitDispatch_AssertPc34(
    DM1_V1_D1LD1RStairsPitAssertResultPc34 *out);

const char *M11_GameView_ViewportD1LD1RStairsPitDispatch_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
