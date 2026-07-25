#include "dm1_v1_cedt006_champion_editor_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_capacity_constants(void)
{
    int v;
    v = DM1_V1_CEDT006_STATUS_NAME_CAPACITY_PC34;
    assert(v == 32);
    v = DM1_V1_CEDT006_STATUS_VALUE_CAPACITY_PC34;
    assert(v == 16);
    v = DM1_V1_CEDT006_CHAMPION_COUNT_PC34;
    assert(v == 4);
    v = DM1_V1_CEDT006_CHAMPION_NAME_CAPACITY_PC34;
    assert(v == 32);
    v = DM1_V1_CEDT006_CHAMPION_TITLE_CAPACITY_PC34;
    assert(v == 32);
    v = DM1_V1_CEDT006_BUTTON_TEXT_CAPACITY_PC34;
    assert(v == 32);
    v = DM1_V1_CEDT006_COLOR_COUNT_PC34;
    assert(v == 16);
    (void)v;
}

static void test_draw_button(void)
{
    DM1_V1_CEDT006_BoxPc34 box = {10, 20, 100, 30, 1};
    DM1_V1_CEDT006_ButtonReceiptPc34 out;
    memset(&out, 0, sizeof(out));
    F7034_DrawButton(&box, "OK", 5, &out);
    assert(out.valid == 1);
}

static void test_set_selected_color_box(void)
{
    DM1_V1_CEDT006_BoxPc34 box = {0, 0, 16, 16, 1};
    DM1_V1_CEDT006_SelectedColorBoxReceiptPc34 out;
    memset(&out, 0, sizeof(out));
    F7035_SetSelectedColorBox(3, &box, &out);
    assert(out.valid == 1);
}

static void test_draw_status_line(void)
{
    DM1_V1_CEDT006_StatusLineReceiptPc34 out;
    memset(&out, 0, sizeof(out));
    F7039_DrawHealthOrStaminaOrMana("Health", 100, 50, &out);
    assert(out.valid == 1);
}

static void test_keyboard_input_null_keys(void)
{
    char text[32] = "Test";
    size_t cursor = 4;
    DM1_V1_CEDT006_KeyboardInputReceiptPc34 out;
    memset(&out, 0, sizeof(out));
    F7041_ProcessKeyboardInput(text, 32, 31, &cursor, NULL, 0, &out);
    assert(out.valid == 1);
    assert(out.insertedCount == 0);
}

static void test_source_evidence_f7039(void)
{
    const char *ev = F7039_F7041_CEDT006_SourceEvidencePc34();
    assert(ev != NULL);
    assert(ev[0] != '\0');
    (void)ev;
}

static void test_source_evidence_f7032(void)
{
    const char *ev = F7032_F7033_F7038_F7040_CEDT006_SourceEvidencePc34();
    assert(ev != NULL);
    assert(ev[0] != '\0');
    (void)ev;
}

static void test_source_evidence_f7034(void)
{
    const char *ev = F7034_F7035_F7036_F7037_CEDT006_SourceEvidencePc34();
    assert(ev != NULL);
    assert(ev[0] != '\0');
    (void)ev;
}

int main(void)
{
    test_capacity_constants();
    test_draw_button();
    test_set_selected_color_box();
    test_draw_status_line();
    test_keyboard_input_null_keys();
    test_source_evidence_f7039();
    test_source_evidence_f7032();
    test_source_evidence_f7034();
    puts("ok: DM1 CEDT006 champion editor (Q-DM1-08) 8 tests passed");
    return 0;
}
