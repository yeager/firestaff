#include "dm2_v1_mouse_cursor.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char* label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void test_state_init_and_first_hide(void)
{
    DM2_V1_MouseCursorState state;
    DM2_V1_MouseCursorReceipt receipt;

    memset(&state, 0xcc, sizeof(state));
    dm2_v1_mouse_cursor_state_init(&state);
    expect_true(state.mouse_visibility == 1 &&
                    state.ibmio_hide_depth == 0 &&
                    state.queued_count == 0,
                "cursor init starts visible with empty queue");

    expect_true(dm2_v1_FIRE_HIDE_MOUSE_CURSOR(&state, &receipt),
                "FIRE_HIDE_MOUSE_CURSOR admits initialized state");
    expect_true(receipt.handled && receipt.source_locked && receipt.admitted &&
                    !receipt.blocked,
                "hide receipt is handled and source locked");
    expect_true(receipt.symbol &&
                    strcmp(receipt.symbol, "FIRE_HIDE_MOUSE_CURSOR") == 0,
                "hide receipt names skproject symbol");
    expect_true(receipt.source_path &&
                    strstr(receipt.source_path, "SkWinCore.cpp:4547") != 0,
                "hide receipt cites skproject source line");
    expect_true(state.mouse_visibility == 0 && state.ibmio_hide_depth == 1,
                "first hide clears visibility and increments depth");
}

static void test_nested_hide_preserves_queue_state(void)
{
    DM2_V1_MouseCursorState state;
    DM2_V1_MouseCursorReceipt receipt;

    dm2_v1_mouse_cursor_state_init(&state);
    state.queued_count = 3;
    expect_true(dm2_v1_FIRE_HIDE_MOUSE_CURSOR(&state, &receipt),
                "first hide succeeds");
    expect_true(dm2_v1_FIRE_HIDE_MOUSE_CURSOR(&state, &receipt),
                "nested hide succeeds");
    expect_true(state.mouse_visibility == 0 && state.ibmio_hide_depth == 2,
                "nested hide increments depth while hidden");
    expect_true(receipt.queue_count == 3,
                "hide receipt preserves current queue count evidence");
}

static void test_fail_closed_inputs(void)
{
    DM2_V1_MouseCursorState state;
    DM2_V1_MouseCursorReceipt receipt;

    dm2_v1_mouse_cursor_state_init(&state);
    state.ibmio_hide_depth = INT_MAX;
    expect_true(!dm2_v1_FIRE_HIDE_MOUSE_CURSOR(&state, &receipt),
                "saturated hide depth is rejected");
    expect_true(receipt.handled && receipt.source_locked && receipt.blocked,
                "saturated hide returns blocked receipt");
    expect_true(state.ibmio_hide_depth == INT_MAX &&
                    state.mouse_visibility == 1,
                "saturated hide preserves state");

    expect_true(!dm2_v1_FIRE_HIDE_MOUSE_CURSOR(NULL, &receipt),
                "NULL state is rejected");
    expect_true(receipt.handled && receipt.blocked,
                "NULL state returns bounded blocked receipt");

    expect_true(!dm2_v1_FIRE_HIDE_MOUSE_CURSOR(&state, NULL),
                "NULL receipt is rejected");
}

static void test_queue_mouse_event(void)
{
    DM2_V1_MouseCursorState state;
    DM2_V1_MouseCursorReceipt receipt;
    int i;

    dm2_v1_mouse_cursor_state_init(&state);
    expect_true(dm2_v1_FIRE_QUEUE_MOUSE_EVENT(&state, 10, 20, 1, &receipt),
                "FIRE_QUEUE_MOUSE_EVENT accepts first event");
    expect_true(state.queued_count == 1 && state.write_index == 1,
                "mouse queue advances ring index");
    expect_true(state.events[1].x == 10 && state.events[1].y == 20 &&
                    state.events[1].button == 1,
                "mouse queue stores x/y/button");

    state.queue_locked = 1;
    expect_true(dm2_v1_FIRE_QUEUE_MOUSE_EVENT(&state, 30, 40, 4, &receipt),
                "locked mouse queue defers event");
    expect_true(state.deferred_pending && receipt.deferred &&
                    state.deferred.x == 30 && state.deferred.button == 4,
                "deferred event preserves payload");
    state.queue_locked = 0;

    for (i = 0; i < 10; ++i) {
        dm2_v1_FIRE_QUEUE_MOUSE_EVENT(&state, (uint16_t)i, 0, 1, &receipt);
    }
    expect_true(state.queued_count == 7,
                "normal mouse queue uses seven-entry threshold");
    dm2_v1_FIRE_QUEUE_MOUSE_EVENT(&state, 99, 0, 2, &receipt);
    expect_true(state.right_button_latched == 1,
                "button 2 latches when normal threshold is full");
}

static void test_cursor_pattern_4bpp_and_8bpp(void)
{
    static const unsigned char src4[] = {0x12, 0x30, 0x45, 0x60};
    static const unsigned char pal[16] = {
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf
    };
    static const unsigned char src8[] = {7, 8, 9, 10};
    DM2_V1_MouseCursorState state;
    DM2_V1_MouseCursorReceipt receipt;

    dm2_v1_mouse_cursor_state_init(&state);
    expect_true(dm2_v1_IBMIO_SET_CURSOR_PATTERN(
                    &state, 2, src4, sizeof(src4), 3, 4, 3, 2, 4,
                    pal, sizeof(pal), 3, &receipt),
                "IBMIO_SET_CURSOR_PATTERN accepts 4bpp source");
    expect_true(state.patterns[2].valid && state.patterns[2].hot_x == 3 &&
                    state.patterns[2].hot_y == 4 &&
                    state.patterns[2].transparent_color == 0xa3,
                "cursor pattern stores metadata and transparent color");
    expect_true(state.patterns[2].pixels[0] == 0xa1 &&
                    state.patterns[2].pixels[1] == 0xa2 &&
                    state.patterns[2].pixels[2] == 0xa3 &&
                    state.patterns[2].pixels[3] == 0xa4 &&
                    state.patterns[2].pixels[4] == 0xa5 &&
                    state.patterns[2].pixels[5] == 0xa6,
                "cursor pattern expands 4bpp rows with even source width");
    expect_true(receipt.copied_pixels == 6 &&
                    strcmp(receipt.symbol, "IBMIO_SET_CURSOR_PATTERN") == 0,
                "cursor pattern receipt names source symbol");

    expect_true(!dm2_v1_IBMIO_SET_CURSOR_PATTERN(
                    &state, 0, src4, sizeof(src4), 0, 0, 3, 2, 4,
                    NULL, 0, 3, &receipt) && receipt.blocked,
                "4bpp cursor rejects a fabricated identity palette");

    expect_true(dm2_v1_IBMIO_SET_CURSOR_PATTERN(
                    &state, 1, src8, sizeof(src8), 0, 0, 2, 2, 8,
                    NULL, 0, 8, &receipt),
                "IBMIO_SET_CURSOR_PATTERN accepts 8bpp source");
    expect_true(memcmp(state.patterns[1].pixels, src8, sizeof(src8)) == 0 &&
                    state.patterns[1].transparent_color == 8,
                "cursor pattern copies 8bpp source directly");
}

int main(void)
{
    test_state_init_and_first_hide();
    test_nested_hide_preserves_queue_state();
    test_fail_closed_inputs();
    test_queue_mouse_event();
    test_cursor_pattern_4bpp_and_8bpp();
    expect_true(strstr(dm2_v1_mouse_cursor_source_evidence(),
                       "FIRE_HIDE_MOUSE_CURSOR") != 0,
                "source evidence names FIRE_HIDE_MOUSE_CURSOR");
    if (failures) {
        return 1;
    }
    puts("DM2 FIRE_HIDE_MOUSE_CURSOR: ok");
    return 0;
}
