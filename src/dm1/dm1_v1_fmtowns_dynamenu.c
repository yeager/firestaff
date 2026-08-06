#include "dm1_v1_fmtowns_dynamenu.h"

/* Source-locked DYNAMENU record layout for the FM Towns DM1 menu.
 * Every offset and colour selector is byte-verified from the
 * DRAW_DMENU disassembly at EDM.EXP 0x4620.
 * Evidence: parity-evidence/dm1_fmtowns_menu_p3_disassembly.md */

uint8_t dm1_v1_fmtowns_dynamenu_slot_label_pc34(
        const uint8_t *record, unsigned int slot) {
    if (!record || slot >= DM1_V1_FMTOWNS_DYNAMENU_BUTTON_COUNT) {
        return DM1_V1_FMTOWNS_DYNAMENU_SLOT_DISABLED;
    }
    /* Slots 0/1/2 map to DYNAMENU+1 / +2 / +3 per the DRAW_DMENU
     * `mov al, byte [edi + 0x2418d]` at 0x46cf, with edi walking
     * 0..2 across the three-button loop. */
    return record[DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON0 + slot];
}

uint8_t dm1_v1_fmtowns_dynamenu_panel_colour_pc34(const uint8_t *record) {
    uint8_t colour;
    if (!record) return DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_DEFAULT;

    /* Exact DRAW_DMENU sequence at EDM.EXP 0x4667..0x4685:
     *   mov di, 0x0b
     *   cmp byte [0x2418f (DYNAMENU+3)], 0xff
     *   jne .keep
     *   mov di, 0x4d
     * .keep:
     *   cmp byte [0x2418e (DYNAMENU+2)], 0xff
     *   jne .done
     *   mov di, 0x4f
     * .done:
     * Byte 2 is checked LAST, so a record where both flag bytes are
     * 0xFF picks ALT_B. */
    colour = DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_DEFAULT;
    if (record[DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON2] ==
        DM1_V1_FMTOWNS_DYNAMENU_SLOT_DISABLED) {
        colour = DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_A;
    }
    if (record[DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON1] ==
        DM1_V1_FMTOWNS_DYNAMENU_SLOT_DISABLED) {
        colour = DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_B;
    }
    return colour;
}
