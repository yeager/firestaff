#include "dm1_v1_fmtowns_text_geometry.h"
#include "dm1_v1_fmtowns_menu_regions.h"
#include "dm1_v1_fmtowns_dynamenu.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    DM1_V1_FmtownsRegionRecord panel_size, panel_anchor;

    assert(dm1_v1_fmtowns_region_menu_panel_pc34(&panel_size));
    assert(dm1_v1_fmtowns_region_menu_clear_area_pc34(&panel_anchor));
    assert(panel_size.a == 87);
    assert(panel_size.b == 45);
    assert(panel_anchor.a == 319);
    assert(panel_anchor.b == 77);
    assert(panel_anchor.a - panel_size.a == 232);

    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(234, 86, 3, 0) == 0);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(318, 96, 3, 0) == 0);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 97, 3, 0) == -1);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 98, 3, 0) == 1);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 109, 3, 0) == -1);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 110, 3, 0) == 2);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(233, 86, 3, 0) == -1);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(319, 86, 3, 0) == -1);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 98, 1, 0) == -1);

    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(234, 94, 3, 1) == 0);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(318, 113, 3, 1) == 0);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 114, 3, 1) == -1);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 115, 3, 1) == 1);
    assert(dm1_v1_fmtowns_dynamenu_action_row_at_pc34(250, 136, 3, 1) == 2);

    puts("PASS dm1_v1_fmtowns_menu_click_geometry");
    return 0;
}
