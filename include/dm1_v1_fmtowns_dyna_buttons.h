#ifndef DM1_V1_FMTOWNS_DYNA_BUTTONS_H
#define DM1_V1_FMTOWNS_DYNA_BUTTONS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked English DYNA_BUTTONS label pool for the FM Towns
 * DM1 menu. Every string here is a byte-verified extraction from
 * the hash-verified HMA-240 English EDM.EXP DYNA_BUTTONS symbol at
 * vaddr 0x24194 (300-byte span up to symbol SPELLS at 0x242c0).
 *
 * The pool is NUL-separated; GET_LABEL (EDM.EXP 0x43e4) walks it
 * by index, treating 0xFF as a sentinel that returns the fixed
 * blank pointer at 0x21d9c. Indices 44..54 are empty padding
 * strings emitted by the linker between DYNA_BUTTONS and SPELLS
 * and are intentionally omitted from this API — GET_LABEL still
 * finds them via NUL walking, but the FM Towns menu never
 * dereferences them (DYNAMENU carries only three button indices
 * whose valid range is 0..43).
 *
 * Evidence:
 *   parity-evidence/dm1_fmtowns_menu_p3_disassembly.md (GET_LABEL,
 *     DYNAMENU byte layout, panel/label draw)
 *   parity-evidence/dm1_fmtowns_region_table.md (region 10/11
 *     that the labels are drawn inside)
 *
 * Do not modify without matching EDM.EXP evidence. No host font
 * pixels ship in this module; only the source-lifted glyph-index
 * strings that a future EGB shim will hand to DO_FDRAW_CTEXT.
 */

/* GET_LABEL's 0xFF sentinel: the FM Towns menu marks a disabled
 * button slot with 0xFF, which the label lookup translates into a
 * fixed empty-string pointer (EDM.EXP 0x21d9c). */
#define DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL 0xFFu

/* Number of real (non-padding) DYNA_BUTTONS entries in EDM.EXP.
 * Byte-verified: 44 NUL-terminated strings from index 0 through 43
 * before the 11 empty padding entries. */
#define DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT 44u

/* Look up the DYNA_BUTTONS label at `index`. Returns a pointer to a
 * NUL-terminated string owned by this module. Returns NULL if the
 * index is out of range. If `index == DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL`,
 * returns the empty string, matching GET_LABEL's 0xFF sentinel path
 * (EDM.EXP 0x21d9c is an empty-string pointer). */
const char *dm1_v1_fmtowns_dyna_button_label_pc34(unsigned int index);

/* Total byte count of the source-owned pool (including NULs) as it
 * appears in EDM.EXP up to DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT-1. */
size_t dm1_v1_fmtowns_dyna_buttons_source_span_bytes_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_DYNA_BUTTONS_H */
