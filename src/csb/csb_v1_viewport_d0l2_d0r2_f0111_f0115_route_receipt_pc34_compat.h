#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0111_F0115_ROUTE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0111_F0115_ROUTE_RECEIPT_PC34_COMPAT_H

#include "csb_v1_viewport_d0l2_d0r2_f0111_door_front_pc34_compat.h"
#include "csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    int side;
    int view_square_index;
    unsigned int thing_pass_cell_order;
    unsigned int door_rear_cell_order;
    unsigned int door_front_cell_order;
    int thing_pass_disabled_items_and_projectiles;
    int door_draw_does_not_mutate_lists;
    int c10_transparency_preserved;
    int no_game_data_load;
    int no_real_asset_bitmap_parity;
    int no_synthetic_viewport_pixels;
    int no_legacy_viewport_wrapper;
    const char *source_evidence;
} CSB_V1_D0L2D0R2F0111F0115RouteReceiptPc34;

int csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_pc34(
    int side,
    CSB_V1_D0L2D0R2F0111F0115RouteReceiptPc34 *out_receipt);

const char *
csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
