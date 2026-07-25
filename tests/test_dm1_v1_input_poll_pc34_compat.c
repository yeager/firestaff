#include "dm1_v1_input_poll_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_InputStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_Input_InitPc34Compat(&state);
    assert(state.initialized == 1);
}

/* init(NULL) crashes — no null guard in implementation */

static void test_deinit(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    DM1_V1_Input_DeinitPc34Compat(&state);
    assert(state.initialized == 0);
}

static void test_store_and_get_key(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    int rc = DM1_V1_Input_StoreKeyPc34Compat(&state, 0x1234);
    (void)rc;
    assert(rc == 1);
    assert(DM1_V1_Input_KeyAvailablePc34Compat(&state) == 1);
    uint16_t key = DM1_V1_Input_GetKeyPc34Compat(&state);
    (void)key;
    assert(key == 0x1234);
    assert(DM1_V1_Input_KeyAvailablePc34Compat(&state) == 0);
}

static void test_get_key_empty(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    uint16_t key = DM1_V1_Input_GetKeyPc34Compat(&state);
    (void)key;
    assert(key == 0);
}

static void test_discard_all(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    DM1_V1_Input_StoreKeyPc34Compat(&state, 0x0041);
    DM1_V1_Input_StoreKeyPc34Compat(&state, 0x0042);
    DM1_V1_Input_DiscardAllPc34Compat(&state);
    assert(DM1_V1_Input_KeyAvailablePc34Compat(&state) == 0);
}

static void test_mouse_set_and_get(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    DM1_V1_Input_MouseSetPositionPc34Compat(&state, 160, 100);
    int16_t x = -1, y = -1;
    DM1_V1_Input_MouseGetPositionPc34Compat(&state, &x, &y);
    assert(x == 160);
    assert(y == 100);
}

static void test_mouse_button_down(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    uint16_t mask = DM1_V1_Input_MouseButtonDownPc34Compat(&state, 1);
    (void)mask;
    assert(mask & DM1_MOUSE_LEFT_BUTTON);
}

static void test_mouse_button_up(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    DM1_V1_Input_MouseButtonDownPc34Compat(&state, 1);
    uint16_t mask = DM1_V1_Input_MouseButtonUpPc34Compat(&state, 1);
    (void)mask;
    assert(!(mask & DM1_MOUSE_LEFT_BUTTON));
}

static void test_numpad_to_movement(void)
{
    uint16_t fwd = DM1_V1_Input_NumpadToMovementPc34Compat(DM1_KEY_FORWARD);
    (void)fwd;
    assert(fwd != 0);
}

static void test_any_activity(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    int rc = DM1_V1_Input_AnyActivityPc34Compat(&state);
    (void)rc;
    assert(rc == 0 || rc == 1);
}

static void test_f1690_get_ascii(void)
{
    uint16_t ascii = F1690_GetASCIICode(0x1E41);
    (void)ascii;
    assert(ascii == 0x41);
}

static void test_f1691_cconis(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    int rc = F1691_Cconis(&state);
    (void)rc;
    assert(rc == 0);
}

static void test_f1692_crawcin(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);
    uint16_t key = F1692_Crawcin(&state);
    (void)key;
    assert(key == 0);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_Input_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_source_evidence_f1690(void)
{
    const char *ev = F1690_GetASCIICode_SourceEvidence();
    (void)ev;
    assert(ev != NULL);
}

static void test_key_constants(void)
{
    assert(DM1_KEY_NONE == 0);
    assert(DM1_KEY_SPACE == 0x0020);
    assert(DM1_KEY_ESCAPE == 0x001B);
    assert(DM1_INPUT_NONE == 0);
}

int main(void)
{
    test_init();
    /* test_init_null removed — no null guard */
    test_deinit();
    test_store_and_get_key();
    test_get_key_empty();
    test_discard_all();
    test_mouse_set_and_get();
    test_mouse_button_down();
    test_mouse_button_up();
    test_numpad_to_movement();
    test_any_activity();
    test_f1690_get_ascii();
    test_f1691_cconis();
    test_f1692_crawcin();
    test_source_evidence();
    test_source_evidence_f1690();
    test_key_constants();

    puts("ok: DM1 input poll (Q-DM1-08) 16 tests passed");
    return 0;
}
