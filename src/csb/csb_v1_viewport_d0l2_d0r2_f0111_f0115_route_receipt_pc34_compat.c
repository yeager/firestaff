#include "csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CSB V1 D0L2/D0R2 viewport route receipt over ReDMCSB DUNVIEW.C "
    "F0125/F0126/F0128 side dispatch, F0115 thing-pass order, and "
    "F0111 door-front draw. It requires caller-visible source contracts for "
    "the D0 side lane, the F0115 cell order, the F0111 rear/door/front split, "
    "C10 transparency, and the non-mutating draw boundary; it performs no "
    "DUNGEON.DAT/GRAPHICS.DAT load and creates no synthetic viewport pixels.";

static int same_side(int side)
{
    return side == CSB_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34 ||
           side == CSB_V1_D0L2_D0R2_F0115_SIDE_D0R2_PC34;
}

int csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_pc34(
    int side,
    CSB_V1_D0L2D0R2F0111F0115RouteReceiptPc34 *out_receipt)
{
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *thing_pass;
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *door_front;
    CSB_V1_D0L2D0R2F0111F0115RouteReceiptPc34 receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!same_side(side)) return 0;

    thing_pass =
        csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(side);
    door_front =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(side);
    if (!thing_pass || !door_front) return 0;

    memset(&receipt, 0, sizeof(receipt));
    if (!thing_pass->source_locked_contract_only ||
        !door_front->source_locked_contract_only ||
        !thing_pass->no_game_data_load ||
        !door_front->no_game_data_load ||
        !thing_pass->no_real_asset_bitmap_parity ||
        !door_front->no_real_asset_bitmap_parity ||
        !thing_pass->item_projectile_disabled_by_g2028 ||
        csb_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
            thing_pass, thing_pass->f0115_first_cell) != -1 ||
        csb_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
            thing_pass, thing_pass->f0115_first_cell) != -1 ||
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_is_draw_mutating_pc34(
            door_front) != 0 ||
        thing_pass->transparent_color != door_front->transparent_color ||
        !thing_pass->no_write_on_transparent ||
        !door_front->f0111_final_blit_uses_c10 ||
        !door_front->rear_pass_before_door ||
        !door_front->door_before_front_pass ||
        !door_front->distinct_from_d0l2_d0r2_wall_gate ||
        !door_front->distinct_from_f0115_thing_pass_gate) {
        return 0;
    }

    receipt.valid = 1;
    receipt.side = side;
    receipt.view_square_index = thing_pass->view_square_index;
    receipt.thing_pass_cell_order = thing_pass->f0115_cell_order;
    receipt.door_rear_cell_order = door_front->f0115_rear_cell_order;
    receipt.door_front_cell_order = door_front->f0115_front_cell_order;
    receipt.thing_pass_disabled_items_and_projectiles = 1;
    receipt.door_draw_does_not_mutate_lists = 1;
    receipt.c10_transparency_preserved = 1;
    receipt.no_game_data_load = 1;
    receipt.no_real_asset_bitmap_parity = 1;
    receipt.no_synthetic_viewport_pixels = 1;
    receipt.no_legacy_viewport_wrapper = 1;
    receipt.source_evidence = s_source_evidence;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

const char *
csb_v1_viewport_d0l2_d0r2_f0111_f0115_route_receipt_source_evidence_pc34(void)
{
    return s_source_evidence;
}
