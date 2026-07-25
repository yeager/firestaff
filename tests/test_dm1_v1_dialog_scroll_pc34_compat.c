#include "dm1_v1_dialog_scroll_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_DialogStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_Dialog_InitPc34Compat(&state);
    assert(state.count == 0);
    assert(state.head == 0);
    assert(state.tail == 0);
    assert(state.current.active == false);
}

static void test_set_bar_position(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    DM1_V1_Dialog_SetBarPositionPc34Compat(&state, 10, 20, 300, 16);
    assert(state.bar_x == 10);
    assert(state.bar_y == 20);
    assert(state.bar_w == 300);
    assert(state.bar_h == 16);
}

static void test_set_active(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    DM1_V1_Dialog_SetActivePc34Compat(&state, DM1_V1_DIALOG_SET_INVENTORY_PC34);
    assert(state.active_set == DM1_V1_DIALOG_SET_INVENTORY_PC34);
}

static void test_push_message(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    bool ok = DM1_V1_Dialog_PushMessagePc34Compat(&state, "Hello", 7);
    (void)ok;
    assert(ok == true);
    assert(state.count == 1);
}

static void test_has_message(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    assert(DM1_V1_Dialog_HasMessagePc34Compat(&state) == false);
    DM1_V1_Dialog_PushMessagePc34Compat(&state, "Test", 3);
    DM1_V1_Dialog_TickPc34Compat(&state);
    bool has = DM1_V1_Dialog_HasMessagePc34Compat(&state);
    (void)has;
    assert(has == true);
}

static void test_get_current_text(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    DM1_V1_Dialog_PushMessagePc34Compat(&state, "Greetings", 5);
    DM1_V1_Dialog_TickPc34Compat(&state);
    const char *txt = DM1_V1_Dialog_GetCurrentTextPc34Compat(&state);
    (void)txt;
    assert(txt != NULL);
    assert(strcmp(txt, "Greetings") == 0);
}

static void test_get_current_color(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    DM1_V1_Dialog_PushMessagePc34Compat(&state, "Color", 12);
    DM1_V1_Dialog_TickPc34Compat(&state);
    uint8_t c = DM1_V1_Dialog_GetCurrentColorPc34Compat(&state);
    (void)c;
    assert(c == 12);
}

static void test_tick_expires(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    DM1_V1_Dialog_PushMessagePc34Compat(&state, "Expire", 1);
    DM1_V1_Dialog_TickPc34Compat(&state);
    for (int i = 0; i < DM1_V1_DIALOG_DISPLAY_TICKS_PC34 + 10; i++) {
        DM1_V1_Dialog_TickPc34Compat(&state);
    }
    bool has = DM1_V1_Dialog_HasMessagePc34Compat(&state);
    (void)has;
    assert(has == false);
}

static void test_queue_multiple(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    DM1_V1_Dialog_PushMessagePc34Compat(&state, "First", 1);
    DM1_V1_Dialog_PushMessagePc34Compat(&state, "Second", 2);
    DM1_V1_Dialog_PushMessagePc34Compat(&state, "Third", 3);
    assert(state.count == 3);
}

static void test_queue_full(void)
{
    DM1_V1_DialogStatePc34 state;
    DM1_V1_Dialog_InitPc34Compat(&state);
    for (int i = 0; i < DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34; i++) {
        DM1_V1_Dialog_PushMessagePc34Compat(&state, "Msg", 1);
    }
    bool ok = DM1_V1_Dialog_PushMessagePc34Compat(&state, "Overflow", 1);
    (void)ok;
    assert(ok == false);
}

static void test_dialog_set_enum(void)
{
    assert(DM1_V1_DIALOG_SET_VIEWPORT_PC34 == 0);
    assert(DM1_V1_DIALOG_SET_INVENTORY_PC34 == 1);
    assert(DM1_V1_DIALOG_SET_COUNT_PC34 == 5);
}

static void test_constants(void)
{
    assert(DM1_V1_DIALOG_MAX_MSG_LEN_PC34 == 128);
    assert(DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34 == 16);
    assert(DM1_V1_DIALOG_DISPLAY_TICKS_PC34 == 60);
}

int main(void)
{
    test_init();
    test_set_bar_position();
    test_set_active();
    test_push_message();
    test_has_message();
    test_get_current_text();
    test_get_current_color();
    test_tick_expires();
    test_queue_multiple();
    test_queue_full();
    test_dialog_set_enum();
    test_constants();

    puts("ok: DM1 dialog scroll (Q-DM1-07) 12 tests passed");
    return 0;
}
