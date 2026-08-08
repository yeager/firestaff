#include "dm1_v1_fmtowns_text_geometry.h"
#include "dm1_v1_fmtowns_menu_regions.h"
#include "dm1_v1_fmtowns_dynamenu.h"

#include <assert.h>
#include <stdio.h>

static int fmtowns_menu_hit_test(int x, int y, int actionCount) {
    int fmx1 = 232, fmy1 = 77, fmx2 = 318;
    if (x >= fmx1 && x <= fmx2 && y >= fmy1 &&
        y < fmy1 + actionCount * DM1_V1_FMTOWNS_CHAR_Y_HYT) {
        return (y - fmy1) / DM1_V1_FMTOWNS_CHAR_Y_HYT;
    }
    return -1;
}

int main(void) {
    DM1_V1_FmtownsRegionRecord panel_size, panel_anchor;

    assert(dm1_v1_fmtowns_region_menu_panel_pc34(&panel_size));
    assert(dm1_v1_fmtowns_region_menu_clear_area_pc34(&panel_anchor));
    assert(panel_size.a == 87);
    assert(panel_size.b == 45);
    assert(panel_anchor.a == 319);
    assert(panel_anchor.b == 77);
    assert(panel_anchor.a - panel_size.a == 232);

    assert(fmtowns_menu_hit_test(232, 77, 3) == 0);
    assert(fmtowns_menu_hit_test(270, 77, 3) == 0);
    assert(fmtowns_menu_hit_test(318, 83, 3) == 0);
    assert(fmtowns_menu_hit_test(250, 84, 3) == 1);
    assert(fmtowns_menu_hit_test(250, 90, 3) == 1);
    assert(fmtowns_menu_hit_test(250, 91, 3) == 2);
    assert(fmtowns_menu_hit_test(250, 97, 3) == 2);
    assert(fmtowns_menu_hit_test(250, 98, 3) == -1);
    assert(fmtowns_menu_hit_test(231, 80, 3) == -1);
    assert(fmtowns_menu_hit_test(319, 80, 3) == -1);
    assert(fmtowns_menu_hit_test(250, 76, 3) == -1);

    assert(fmtowns_menu_hit_test(250, 84, 1) == -1);
    assert(fmtowns_menu_hit_test(250, 77, 1) == 0);
    assert(fmtowns_menu_hit_test(250, 83, 1) == 0);

    assert(fmtowns_menu_hit_test(250, 84, 2) == 1);
    assert(fmtowns_menu_hit_test(250, 91, 2) == -1);

    puts("PASS dm1_v1_fmtowns_menu_click_geometry");
    return 0;
}
