#include "dm1_v1_fmtowns_dynamenu.h"
#include <assert.h>
#include <stdio.h>

static void test_layout_constants(void) {
    /* Byte-verified from DRAW_DMENU disassembly at EDM.EXP 0x4620. */
    assert(DM1_V1_FMTOWNS_DYNAMENU_BYTES == 8u);
    assert(DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON0 == 1u);
    assert(DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON1 == 2u);
    assert(DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON2 == 3u);
    assert(DM1_V1_FMTOWNS_DYNAMENU_BUTTON_COUNT == 3u);
    assert(DM1_V1_FMTOWNS_DYNAMENU_SLOT_DISABLED == 0xFFu);
    assert(DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_DEFAULT == 0x0Bu);
    assert(DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_A   == 0x4Du);
    assert(DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_B   == 0x4Fu);
}

static void test_slot_lookup_valid(void) {
    /* Record with three real label indices. */
    static const uint8_t r[DM1_V1_FMTOWNS_DYNAMENU_BYTES] = {
        0x00, 0x01, 0x02, 0x14, 0x00, 0x00, 0x00, 0x00,
    };
    assert(dm1_v1_fmtowns_dynamenu_slot_label_pc34(r, 0) == 0x01);
    assert(dm1_v1_fmtowns_dynamenu_slot_label_pc34(r, 1) == 0x02);
    assert(dm1_v1_fmtowns_dynamenu_slot_label_pc34(r, 2) == 0x14);
}

static void test_slot_lookup_invalid(void) {
    static const uint8_t r[DM1_V1_FMTOWNS_DYNAMENU_BYTES] = {0,0,0,0,0,0,0,0};
    assert(dm1_v1_fmtowns_dynamenu_slot_label_pc34(r, 3) == 0xFFu);
    assert(dm1_v1_fmtowns_dynamenu_slot_label_pc34(r, 99) == 0xFFu);
    assert(dm1_v1_fmtowns_dynamenu_slot_label_pc34(NULL, 0) == 0xFFu);
}

static void test_panel_colour_default(void) {
    /* Byte 2 and byte 3 both hold real labels (not 0xFF). */
    static const uint8_t r[DM1_V1_FMTOWNS_DYNAMENU_BYTES] = {
        0x00, 0x01, 0x02, 0x14, 0x00, 0x00, 0x00, 0x00,
    };
    assert(dm1_v1_fmtowns_dynamenu_panel_colour_pc34(r) ==
           DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_DEFAULT);
}

static void test_panel_colour_button2_disabled(void) {
    /* Only byte 3 (button-2 label) is 0xFF => ALT_A. */
    static const uint8_t r[DM1_V1_FMTOWNS_DYNAMENU_BYTES] = {
        0x00, 0x01, 0x02, 0xFF, 0x00, 0x00, 0x00, 0x00,
    };
    assert(dm1_v1_fmtowns_dynamenu_panel_colour_pc34(r) ==
           DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_A);
}

static void test_panel_colour_button1_disabled(void) {
    /* Only byte 2 (button-1 label) is 0xFF => ALT_B. */
    static const uint8_t r[DM1_V1_FMTOWNS_DYNAMENU_BYTES] = {
        0x00, 0x01, 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00,
    };
    assert(dm1_v1_fmtowns_dynamenu_panel_colour_pc34(r) ==
           DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_B);
}

static void test_panel_colour_both_disabled(void) {
    /* Both flag bytes are 0xFF. Byte 2 is compared LAST in DRAW_DMENU,
     * so ALT_B wins. */
    static const uint8_t r[DM1_V1_FMTOWNS_DYNAMENU_BYTES] = {
        0x00, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    };
    assert(dm1_v1_fmtowns_dynamenu_panel_colour_pc34(r) ==
           DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_B);
}

static void test_panel_colour_null_record(void) {
    assert(dm1_v1_fmtowns_dynamenu_panel_colour_pc34(NULL) ==
           DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_DEFAULT);
}

int main(void) {
    test_layout_constants();
    test_slot_lookup_valid();
    test_slot_lookup_invalid();
    test_panel_colour_default();
    test_panel_colour_button2_disabled();
    test_panel_colour_button1_disabled();
    test_panel_colour_both_disabled();
    test_panel_colour_null_record();
    printf("All dm1_v1_fmtowns_dynamenu tests passed.\n");
    return 0;
}
