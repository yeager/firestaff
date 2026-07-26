#include "dm1_v1_cursor_icon_swap_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_CursorIconSwapStatePc34 s;
    dm1_v1_cursor_icon_swap_init_pc34(&s);
    assert(s.activePointerType == DM1_V1_CURSOR_ARROW_PC34);
    assert(s.previousPointerType == DM1_V1_CURSOR_ARROW_PC34);
    assert(s.swapOccurred == 0);
}

static void test_arrow_to_hand_on_action_menu(void)
{
    DM1_V1_CursorIconSwapStatePc34 s;
    DM1_V1_CursorIconSwapInputPc34 inp;
    DM1_V1_CursorIconSwapReceiptPc34 r;

    dm1_v1_cursor_icon_swap_init_pc34(&s);
    memset(&inp, 0, sizeof(inp));
    inp.actionMenuOpen = 1;
    assert(dm1_v1_cursor_icon_swap_update_pc34(&s, &inp, &r));
    assert(r.valid);
    assert(r.resolvedPointerType == DM1_V1_CURSOR_HAND_PC34);
    assert(r.swapTriggered == 1);
    assert(s.activePointerType == DM1_V1_CURSOR_HAND_PC34);
}

static void test_hand_back_to_arrow(void)
{
    DM1_V1_CursorIconSwapStatePc34 s;
    DM1_V1_CursorIconSwapInputPc34 inp;
    DM1_V1_CursorIconSwapReceiptPc34 r;

    dm1_v1_cursor_icon_swap_init_pc34(&s);
    memset(&inp, 0, sizeof(inp));
    inp.actionMenuOpen = 1;
    dm1_v1_cursor_icon_swap_update_pc34(&s, &inp, &r);
    inp.actionMenuOpen = 0;
    assert(dm1_v1_cursor_icon_swap_update_pc34(&s, &inp, &r));
    assert(r.resolvedPointerType == DM1_V1_CURSOR_ARROW_PC34);
    assert(r.swapTriggered == 1);
}

static void test_object_in_hand_overrides_action(void)
{
    DM1_V1_CursorIconSwapStatePc34 s;
    DM1_V1_CursorIconSwapInputPc34 inp;
    DM1_V1_CursorIconSwapReceiptPc34 r;

    dm1_v1_cursor_icon_swap_init_pc34(&s);
    memset(&inp, 0, sizeof(inp));
    inp.actionMenuOpen = 1;
    inp.objectInHand = 1;
    assert(dm1_v1_cursor_icon_swap_update_pc34(&s, &inp, &r));
    assert(r.resolvedPointerType == DM1_V1_CURSOR_OBJECT_PC34);
}

static void test_no_swap_when_stable(void)
{
    DM1_V1_CursorIconSwapStatePc34 s;
    DM1_V1_CursorIconSwapInputPc34 inp;
    DM1_V1_CursorIconSwapReceiptPc34 r;

    dm1_v1_cursor_icon_swap_init_pc34(&s);
    memset(&inp, 0, sizeof(inp));
    assert(dm1_v1_cursor_icon_swap_update_pc34(&s, &inp, &r));
    assert(r.swapTriggered == 0);
}

static void test_champion_icon(void)
{
    DM1_V1_CursorIconSwapStatePc34 s;
    DM1_V1_CursorIconSwapInputPc34 inp;
    DM1_V1_CursorIconSwapReceiptPc34 r;

    dm1_v1_cursor_icon_swap_init_pc34(&s);
    s.championIconOrdinal = 1;
    memset(&inp, 0, sizeof(inp));
    assert(dm1_v1_cursor_icon_swap_update_pc34(&s, &inp, &r));
    assert(r.resolvedPointerType == DM1_V1_CURSOR_CHAMPION_PC34);
    assert(r.swapTriggered == 1);
}

int main(void)
{
    test_init();
    test_arrow_to_hand_on_action_menu();
    test_hand_back_to_arrow();
    test_object_in_hand_overrides_action();
    test_no_swap_when_stable();
    test_champion_icon();

    puts("ok: DM1 cursor icon swap (Q-DM1-07) 6 tests passed");
    return 0;
}
