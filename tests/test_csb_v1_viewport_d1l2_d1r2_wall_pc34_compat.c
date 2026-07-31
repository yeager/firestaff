#include "csb/csb_v1_viewport_d1l2_d1r2_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;
static void check(const char *label, int value) { ++checks; if (!value) { ++failures; printf("FAIL %s\n", label); } }

int main(void)
{
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *left = csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *right = csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(2);
    int sx = -1;
    check("two.routes", csb_v1_viewport_d1l2_d1r2_wall_route_spec_count_pc34() == 2 && left && right);
    check("dispatch", left && right && left->redmcsb_function_number == 122 && right->redmcsb_function_number == 123 && left->f0128_draw_order_index == 0 && right->f0128_draw_order_index == 1);
    check("source.lock", left && right && left->source_locked_contract_only && right->source_locked_contract_only && left->no_real_asset_bitmap_parity && right->no_real_asset_bitmap_parity);
    check("geometry", left && right && left->wall_frame_x1 == 0 && left->wall_frame_x2 == 63 && right->wall_frame_x1 == 160 && right->wall_frame_x2 == 223);
    check("c10", left && right && left->transparent_color == 10 && right->transparent_color == 10 && left->f0105_flipped_route && right->f0105_flipped_route);
    check("source.map", csb_v1_viewport_d1l2_d1r2_wall_map_viewport_x_to_source_pc34(left, 0, 0, &sx) == 0 && sx == 192);
    check("flipped.map", csb_v1_viewport_d1l2_d1r2_wall_map_viewport_x_to_source_pc34(left, 0, 1, &sx) == 0 && sx == 255);
    check("thing.keepout", left && right && left->f0115_thing_pass_keepout && right->f0115_thing_pass_keepout && csb_v1_viewport_d1l2_d1r2_wall_preserve_first_thing_pc34(left, 0x2345u) == 0x2345u);
    check("evidence", strstr(csb_v1_viewport_d1l2_d1r2_wall_source_evidence_pc34(), "F0122") && strstr(csb_v1_viewport_d1l2_d1r2_wall_source_evidence_pc34(), "F0123"));
    printf("CSB D1L2/D1R2 wall metadata: checks=%d failures=%d\n", checks, failures);
    return failures != 0;
}
