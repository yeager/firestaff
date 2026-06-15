#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_PIT = 2,
    DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D3C_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC = 108,
    DM1_V1_D3C_STAIRS_PIT_PC34_WALLSET_GRAPHIC_COUNT = 40,
    DM1_V1_D3C_STAIRS_PIT_PC34_STAIRS_UP_SLOT_D3C = 1,
    DM1_V1_D3C_STAIRS_PIT_PC34_STAIRS_DOWN_SLOT_D3C = 8,
    DM1_V1_D3C_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D3C = 51,
    DM1_V1_D3C_STAIRS_PIT_PC34_ZONE_STAIRS_UP_D3C = 803,
    DM1_V1_D3C_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_D3C = 816,
    DM1_V1_D3C_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3C = 853,
    DM1_V1_D3C_STAIRS_PIT_PC34_VIEW_SQUARE_D3C = 11,
    DM1_V1_D3C_STAIRS_PIT_PC34_CELL_ORDER_OPEN = 0x3421,
    DM1_V1_D3C_STAIRS_PIT_PC34_TRANSPARENT_COLOR = 10
};

typedef enum {
    DM1_V1_D3C_STAIRS_PIT_ROLE_UP_FRONT_PC34 = 0,
    DM1_V1_D3C_STAIRS_PIT_ROLE_DOWN_FRONT_PC34 = 1,
    DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34 = 2
} DM1_V1_D3CStairsPitRolePc34;

typedef struct {
    DM1_V1_D3CStairsPitRolePc34 role;
    const char *role_name;
    const char *dispatch_anchor;
    const char *draw_anchor;
    const char *defs_anchor;
    int native_bitmap_slot_or_graphic;
    int zone_index;
    int view_square_index;
    int cell_order;
    int transparent_color;
    bool uses_f0104;
    bool uses_f0107;
    bool uses_f0108_metadata;
    bool uses_f0111;
    bool uses_f0115_thing_pass;
    bool uses_f0128_wall_followup_writes;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D3CStairsPitEvidencePc34;

typedef struct {
    int element;
    bool stairs_up;
    bool pit_or_teleporter_visible;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D3CStairsPitDispatchInputPc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
    bool unsupported_element;
    DM1_V1_D3CStairsPitRolePc34 role;
    int native_bitmap_slot_or_graphic;
    int native_bitmap_index;
    int first_stairs_graphic_index;
    int zone_index;
    int view_square_index;
    int cell_order;
    bool used_f0104;
    bool used_f0107;
    bool used_f0108_metadata;
    bool used_f0111;
    bool used_f0115_thing_pass;
    bool used_f0128_wall_followup_writes;
    const DM1_V1_D3CStairsPitEvidencePc34 *evidence;
} DM1_V1_D3CStairsPitDispatchResultPc34;

typedef struct {
    DM1_V1_D3CStairsPitRolePc34 role;
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    size_t destination_stride;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D3CStairsPitBlitInputPc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
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
    const DM1_V1_D3CStairsPitEvidencePc34 *evidence;
} DM1_V1_D3CStairsPitBlitResultPc34;

const DM1_V1_D3CStairsPitEvidencePc34 *
dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_pc34(size_t *count);

const DM1_V1_D3CStairsPitEvidencePc34 *
dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
    DM1_V1_D3CStairsPitRolePc34 role);

bool dm1_v1_viewport_d3c_stairs_pit_dispatch_probe_pc34(
    const DM1_V1_D3CStairsPitDispatchInputPc34 *input,
    DM1_V1_D3CStairsPitDispatchResultPc34 *out);

bool dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(
    const DM1_V1_D3CStairsPitBlitInputPc34 *input,
    DM1_V1_D3CStairsPitBlitResultPc34 *out);

const char *dm1_v1_viewport_d3c_stairs_pit_dispatch_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
