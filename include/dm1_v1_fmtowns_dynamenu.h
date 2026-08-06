#ifndef DM1_V1_FMTOWNS_DYNAMENU_H
#define DM1_V1_FMTOWNS_DYNAMENU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked layout of the FM Towns DM1 DYNAMENU record.
 *
 * DYNAMENU is an 8-byte data structure at EDM.EXP vaddr 0x2418c
 * (immediately followed by DYNA_BUTTONS at 0x24194). It carries the
 * three-slot dynamic action menu that DRAW_DMENU draws. The
 * following byte layout is byte-verified from the DRAW_DMENU
 * disassembly (evidence:
 * parity-evidence/dm1_fmtowns_menu_p3_disassembly.md):
 *
 *   Offset 0: unused / padding
 *   Offset 1: label index of button 0  (into DYNA_BUTTONS)
 *   Offset 2: label index of button 1  (0xFF sentinel = disabled)
 *   Offset 3: label index of button 2  (0xFF sentinel = disabled)
 *   Offset 4..7: linker padding before DYNA_BUTTONS (unused)
 *
 * The two DRAW_DMENU comparisons that back the offset assignments:
 *   0x466b: cmp byte ptr [0x2418f], 0xff   ; DYNAMENU + 3
 *   0x4678: cmp byte ptr [0x2418e], 0xff   ; DYNAMENU + 2
 *   0x46cf: mov al, byte ptr [edi + 0x2418d]  ; DYNAMENU + 1 + i, i=0..2
 *
 * When byte 3 == 0xFF the panel colour is 0x4D; when byte 2 == 0xFF
 * the panel colour is 0x4F; otherwise the default panel colour is
 * 0x0B. GET_LABEL translates any 0xFF label back to the empty
 * string (EDM.EXP 0x21d9c), so a 0xFF slot draws blank.
 *
 * This module ships no runtime state — it defines the source-lifted
 * offsets and helpers so an M11 menu-shim can compose a DYNAMENU
 * record from live game state without re-lifting the layout.
 */

/* Byte offsets within the 8-byte DYNAMENU record. */
#define DM1_V1_FMTOWNS_DYNAMENU_BYTES              8u
#define DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON0     1u
#define DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON1     2u
#define DM1_V1_FMTOWNS_DYNAMENU_OFFSET_BUTTON2     3u
#define DM1_V1_FMTOWNS_DYNAMENU_BUTTON_COUNT       3u

/* Panel colour selectors driven by the button-2 / button-3 sentinels
 * (see DRAW_DMENU sequence at EDM.EXP 0x4667..0x4685). */
#define DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_DEFAULT 0x0Bu
#define DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_A   0x4Du
#define DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_ALT_B   0x4Fu

/* Sentinel for a disabled slot; GET_LABEL returns empty string. */
#define DM1_V1_FMTOWNS_DYNAMENU_SLOT_DISABLED        0xFFu

/* Return the label index at slot `slot` (0..2) from an 8-byte
 * DYNAMENU record. Returns 0xFF (disabled sentinel) if `record` is
 * NULL or `slot` is out of range. */
uint8_t dm1_v1_fmtowns_dynamenu_slot_label_pc34(
        const uint8_t *record, unsigned int slot);

/* Compute the panel colour DRAW_DMENU would select for the given
 * record. Returns one of DM1_V1_FMTOWNS_DYNAMENU_PANEL_COLOUR_*.
 * Mirrors the exact byte-3-then-byte-2 comparison order at EDM.EXP
 * 0x466b / 0x4678 (so a record where both bytes are 0xFF picks
 * ALT_B because byte 2 is checked last). */
uint8_t dm1_v1_fmtowns_dynamenu_panel_colour_pc34(const uint8_t *record);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_DYNAMENU_H */
