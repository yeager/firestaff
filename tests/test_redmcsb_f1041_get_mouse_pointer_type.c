#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1041_get_mouse_pointer_type.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int click_count;
    int16_t click_x;
    int16_t click_y;
    int16_t click_buttons;
    unsigned int bound_count;
    const int16_t *bound_xyz;
    int16_t bound_buttons;
} test_context;

static void process_click(void *context, int16_t x, int16_t y,
                          int16_t buttons_status)
{
    test_context *test = context;

    test->click_count++;
    test->click_x = x;
    test->click_y = y;
    test->click_buttons = buttons_status;
}

static void set_mouse_bound_event(void *context, const int16_t xyz[4],
                                  int16_t buttons_status)
{
    test_context *test = context;

    test->bound_count++;
    test->bound_xyz = xyz;
    test->bound_buttons = buttons_status;
}

static redmcsb_f1041_get_mouse_pointer_type_state make_state(
    redmcsb_f1041_mouse_pointer_screen_region regions[16],
    const int16_t health[4], test_context *test)
{
    redmcsb_f1041_get_mouse_pointer_type_state state;

    memset(&state, 0, sizeof(state));
    state.use_hand_as_mouse_pointer_bitmap = true;
    state.regions = regions;
    state.screen_pixel_width = 320;
    state.screen_pixel_height = 200;
    state.party_champion_count = 4;
    state.champion_current_health = health;
    state.retained_pointer_type = REDMCSB_F1041_POINTER_HAND;
    state.process_click = process_click;
    state.set_mouse_bound_event = set_mouse_bound_event;
    state.context = test;
    return state;
}

int main(void)
{
    redmcsb_f1041_mouse_pointer_screen_region regions[16];
    const int16_t health[4] = {100, 0, 0, 0};
    redmcsb_f1041_get_mouse_pointer_type_state state;
    test_context test;
    const char *evidence;
    (void)evidence;

    memset(regions, 0, sizeof(regions));
    memset(&test, 0, sizeof(test));
    regions[0].xyz[0] = 10;
    regions[0].xyz[1] = 20;
    regions[0].xyz[2] = 10;
    regions[0].xyz[3] = 20;
    regions[0].pointer_type = REDMCSB_F1041_POINTER_HAND;
    state = make_state(regions, health, &test);
    state.refresh_regions = true;
    state.use_object_as_mouse_pointer_bitmap = true;
    assert(redmcsb_f1041_get_mouse_pointer_type(&state, 15, 15) ==
           REDMCSB_F1041_POINTER_OBJECT_ICON);
    assert(test.bound_count == 1U);
    assert(test.bound_xyz == regions[0].xyz);

    memset(&test, 0, sizeof(test));
    state = make_state(regions, health, &test);
    state.hide_mouse_pointer_request_count = 1;
    state.use_champion_icon_ordinal_as_mouse_pointer_bitmap = 1;
    assert(redmcsb_f1041_get_mouse_pointer_type(&state, 15, 15) ==
           REDMCSB_F1041_POINTER_NONE);
    state.hide_mouse_pointer_request_count = 0;
    assert(redmcsb_f1041_get_mouse_pointer_type(&state, 15, 15) ==
           REDMCSB_F1041_POINTER_CHAMPION_ICON);

    memset(&test, 0, sizeof(test));
    state = make_state(regions, health, &test);
    state.mouse_bounded = true;
    state.mouse_bound_xyz[0] = 0;
    state.mouse_bound_xyz[1] = 5;
    state.mouse_bound_xyz[2] = 0;
    state.mouse_bound_xyz[3] = 5;
    state.mouse_bound_buttons_status = 7;
    state.refresh_regions = true;
    regions[0].champion_ordinal = 1;
    state.inventory_champion_ordinal = 1;
    assert(redmcsb_f1041_get_mouse_pointer_type(&state, 15, 15) ==
           REDMCSB_F1041_POINTER_ARROW);
    assert(test.click_count == 1U);
    assert(test.click_x == 15 && test.click_y == 15 &&
           test.click_buttons == 7);
    assert(state.mouse_bound_buttons_status == 0);
    assert(test.bound_count == 1U);

    regions[0].champion_ordinal = 0;
    state.refresh_regions = false;
    assert(redmcsb_f1041_get_mouse_pointer_type(&state, -1, 15) ==
           REDMCSB_F1041_POINTER_ARROW);
    assert(state.was_outside_screen);
    assert(redmcsb_f1041_get_mouse_pointer_type(&state, 15, 15) ==
           REDMCSB_F1041_POINTER_HAND);
    assert(!state.was_outside_screen);

    evidence = redmcsb_f1041_get_mouse_pointer_type_source_evidence();
    assert(strstr(evidence, "IO.C:3318-3394") != NULL);
    assert(strstr(evidence, "COMPILE.H:1038-1039") != NULL);
    puts("ok: ReDMCSB F1041 mouse-pointer resolver");
    return 0;
}
