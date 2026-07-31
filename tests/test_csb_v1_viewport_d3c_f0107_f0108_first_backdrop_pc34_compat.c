#include "csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

static void check(const char *label, int value)
{
    ++checks;
    if (!value) { ++failures; printf("FAIL %s\n", label); }
}

int main(void)
{
    const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract *c =
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_contract_pc34();
    const char *e = csb_v1_viewport_d3c_f0107_f0108_first_backdrop_source_evidence_pc34();
    check("contract", c && c->contract_only && c->no_game_data_dependency);
    check("dimensions", c && c->viewport_width == 224 && c->viewport_height == 136);
    check("views", c && c->view_square_d3c == 11 && c->view_wall_d3c_front == 5 && c->view_floor_d3c == 3);
    check("zones", c && c->wall_ornament_zone_base == 1004 && c->floor_ornament_zone_base == 1500);
    check("strides", c && c->wall_ornament_coordinate_set_stride == 15 && c->floor_ornament_coordinate_set_stride == 11);
    check("transparency", c && c->transparent_color == 10 && c->f0108_transparent_mask_preserves_destination);
    check("order", c && c->first_backdrop_is_before_cell_routes && c->f0107_before_f0108);
    check("d3c.window", c && c->d3c_window.x1 == 74 && c->d3c_window.y1 == 25 && c->d3c_window.x2 == 149 && c->d3c_window.y2 == 75);
    check("evidence", e && strstr(e, "F0097") && strstr(e, "F0107") && strstr(e, "F0108") && strstr(e, "F0118") && strstr(e, "CSBWin"));
    printf("CSB D3C backdrop metadata: checks=%d failures=%d\n", checks, failures);
    return failures != 0;
}
