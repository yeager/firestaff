#include "dm1_v1_click_routing_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    assert(state.zone_count == 0);
    assert(state.mouse_x == 0);
    assert(state.mouse_y == 0);
    assert(state.mouse_visible == false);
    assert(state.left_pressed == false);
    assert(state.right_pressed == false);
}

static void test_add_zone_and_hit(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);

    bool ok = DM1_V1_ClickRouting_AddZonePc34Compat(&state, DM1_V1_CK_ZONE_VIEWPORT,
                                                     10, 20, 100, 80, 42);
    assert(ok);
    assert(state.zone_count == 1);

    /* Hit inside zone */
    DM1_V1_ClickResultPc34 r = DM1_V1_ClickRouting_TestClickPc34Compat(&state, 50, 50);
    assert(r.hit == true);
    assert(r.zone_type == DM1_V1_CK_ZONE_VIEWPORT);
    assert(r.zone_data == 42);
    assert(r.local_x == 40);  /* 50 - 10 */
    assert(r.local_y == 30);  /* 50 - 20 */
    (void)r;
    (void)ok;
}

static void test_miss(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    DM1_V1_ClickRouting_AddZonePc34Compat(&state, DM1_V1_CK_ZONE_VIEWPORT,
                                           10, 20, 100, 80, 0);

    DM1_V1_ClickResultPc34 r = DM1_V1_ClickRouting_TestClickPc34Compat(&state, 5, 5);
    assert(r.hit == false);
    assert(r.zone_type == DM1_V1_CK_ZONE_NONE);
    (void)r;
}

static void test_clear_zones(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    DM1_V1_ClickRouting_AddZonePc34Compat(&state, DM1_V1_CK_ZONE_VIEWPORT,
                                           0, 0, 100, 100, 0);
    assert(state.zone_count == 1);
    DM1_V1_ClickRouting_ClearZonesPc34Compat(&state);
    assert(state.zone_count == 0);
}

static void test_update_mouse(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    DM1_V1_ClickRouting_UpdateMousePc34Compat(&state, 123, 456, true, false);
    assert(state.mouse_x == 123);
    assert(state.mouse_y == 456);
    assert(state.left_pressed == true);
    assert(state.right_pressed == false);
    assert(state.mouse_visible == true);
}

static void test_zone_capacity(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    for (int i = 0; i < DM1_CK_MAX_ZONES; i++) {
        bool ok = DM1_V1_ClickRouting_AddZonePc34Compat(&state, DM1_V1_CK_ZONE_INVENTORY,
                                                         0, 0, 10, 10, (uint16_t)i);
        assert(ok);
        (void)ok;
    }
    /* Should fail when full */
    bool full = DM1_V1_ClickRouting_AddZonePc34Compat(&state, DM1_V1_CK_ZONE_MENU,
                                                       0, 0, 10, 10, 99);
    assert(!full);
    (void)full;
}

static void test_setup_dungeon_zones(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    DM1_V1_ClickRouting_SetupDungeonZonesPc34Compat(&state);
    /* Viewport + 6 movement + 4 champion + spell + hand = 13 zones */
    assert(state.zone_count == 13);

    /* Click in viewport area */
    DM1_V1_ClickResultPc34 r = DM1_V1_ClickRouting_TestClickPc34Compat(&state, 100, 50);
    assert(r.hit == true);
    assert(r.zone_type == DM1_V1_CK_ZONE_VIEWPORT);
    (void)r;
}

static void test_setup_inventory_zones(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    DM1_V1_ClickRouting_SetupInventoryZonesPc34Compat(&state, 4);
    /* 30 inventory slots + 4 champion tabs + 1 menu = 35, but capped at 32 */
    assert(state.zone_count <= DM1_CK_MAX_ZONES);
    assert(state.zone_count > 0);
}

static void test_edge_of_zone(void) {
    DM1_V1_ClickRoutingStatePc34 state;
    DM1_V1_ClickRouting_InitPc34Compat(&state);
    DM1_V1_ClickRouting_AddZonePc34Compat(&state, DM1_V1_CK_ZONE_CHAMPION,
                                           10, 10, 20, 20, 7);

    /* Top-left corner: exactly at origin, should hit */
    DM1_V1_ClickResultPc34 r = DM1_V1_ClickRouting_TestClickPc34Compat(&state, 10, 10);
    assert(r.hit == true);
    assert(r.local_x == 0);
    assert(r.local_y == 0);

    /* Just past right edge: x=30 should miss (10+20=30, test is x < x+w) */
    r = DM1_V1_ClickRouting_TestClickPc34Compat(&state, 30, 15);
    assert(r.hit == false);

    /* Just past bottom edge */
    r = DM1_V1_ClickRouting_TestClickPc34Compat(&state, 15, 30);
    assert(r.hit == false);
    (void)r;
}

static void test_enum_values(void) {
    assert(DM1_V1_CK_ZONE_NONE == 0);
    assert(DM1_V1_CK_ZONE_VIEWPORT == 1);
    assert(DM1_V1_CK_ZONE_CHAMPION == 2);
    assert(DM1_V1_CK_ZONE_INVENTORY == 3);
}

int main(void) {
    test_init();
    test_add_zone_and_hit();
    test_miss();
    test_clear_zones();
    test_update_mouse();
    test_zone_capacity();
    test_setup_dungeon_zones();
    test_setup_inventory_zones();
    test_edge_of_zone();
    test_enum_values();
    puts("ok: dm1_v1_click_routing_pc34_compat 10 tests passed");
    return 0;
}
