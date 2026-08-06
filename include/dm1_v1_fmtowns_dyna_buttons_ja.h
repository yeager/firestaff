#ifndef DM1_V1_FMTOWNS_DYNA_BUTTONS_JA_H
#define DM1_V1_FMTOWNS_DYNA_BUTTONS_JA_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked Japanese DYNA_BUTTONS-equivalent label pool for
 * the FM Towns DM1 menu. Every byte here is a byte-verified
 * extraction from the hash-verified HMA-240 Japanese JDM.EXP at
 * vaddr 0x243bc (EDM.EXP's DYNA_BUTTONS at 0x24194 + 0x228 shift
 * documented in the JDM structural map).
 *
 * The pool is NUL-separated; the FM Towns menu walks it index-by-
 * index the same way English GET_LABEL does. Indices 0..43 are real
 * verbs (44 entries, same count as English); indices 44..54 are
 * empty linker padding, same shape as the English pool.
 *
 * Label bytes are raw Shift-JIS. The FM Towns runtime renders them
 * through the TownsOS text primitives; Firestaff must preserve the
 * exact byte sequence when handing them back to any glyph pipeline.
 *
 * Index-for-index alignment with the English pool is verified:
 * every English verb at index N maps to a Japanese verb at index N
 * (e.g. [1] BLOCK / さえぎる, [2] CHOP / 叩き切る, [4] BLOW HORN /
 * 角笛を吹く, [20] FIREBALL / 火炎弾, [33] SPELLSHIELD / 呪文防御).
 *
 * Evidence:
 *   parity-evidence/dm1_fmtowns_jdm_structural_map.md
 *   parity-evidence/dm1_fmtowns_menu_p3_disassembly.md (English
 *     side of the same layout).
 *
 * Do not modify without matching JDM.EXP evidence. No host font
 * pixels ship here; only the source-lifted Shift-JIS strings that
 * a future EGB text shim will hand to DO_FDRAW_CTEXT.
 */

/* Sentinel that maps to the empty string, matching the English
 * GET_LABEL 0xFF path (JDM.EXP mirrors this behaviour byte-for-byte
 * per the structural map). */
#define DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL_JA 0xFFu

/* Number of real (non-padding) Japanese label entries. Byte-verified:
 * 44 NUL-terminated Shift-JIS strings before the 11 empty padding
 * entries. Matches the English pool count. */
#define DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT_JA 44u

/* Look up the Japanese label at `index`. Returns a pointer to a
 * NUL-terminated Shift-JIS byte string owned by this module.
 * Returns NULL if the index is out of range. If `index ==
 * DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL_JA`, returns the empty string,
 * matching the English 0xFF sentinel semantics. */
const char *dm1_v1_fmtowns_dyna_button_label_ja_pc34(unsigned int index);

/* Return the raw byte length (excluding NUL) of the label at
 * `index`. Returns 0 for the sentinel, for out-of-range indices,
 * and for the empty string. This is the source-owned Shift-JIS
 * byte count -- do not treat it as a character count. */
size_t dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(unsigned int index);

/* Total byte count of the source-owned pool (including NULs) for
 * the 44 real Japanese labels. Byte-verified 336 against JDM.EXP
 * (versus 289 for the English pool). */
size_t dm1_v1_fmtowns_dyna_buttons_source_span_bytes_ja_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_DYNA_BUTTONS_JA_H */
