#ifndef DM1_V1_FMTOWNS_TEXT_GEOMETRY_H
#define DM1_V1_FMTOWNS_TEXT_GEOMETRY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked font and screen geometry constants for the FM Towns
 * DM1 runtime. Every value below is a byte-verified read of the
 * initialised data section of the hash-verified HMA-240 English
 * EDM.EXP at the SYM1-declared vaddr shown. The values are static
 * initialisers; no runtime write path modifies them (verified by
 * pattern search for `66 c7 05 <ea>` writes over the code segment).
 *
 * Evidence:
 *   parity-evidence/dm1_fmtowns_text_rasteriser.md
 *   parity-evidence/dm1_fmtowns_menu_p3_disassembly.md
 *
 * These constants let the M11 menu shim compute exact FM Towns
 * label geometry without opening EDM.EXP at runtime.
 */

/* Vaddr and value of every CHAR_* / ICON_* / SCR_X_SIZE field the
 * FM Towns menu draw chain reads. */

#define DM1_V1_FMTOWNS_CHAR_X_SIZE      5   /* glyph body width  (px)   */
#define DM1_V1_FMTOWNS_CHAR_Y_SIZE      6   /* glyph body height (px)   */
#define DM1_V1_FMTOWNS_CHAR_X_SPC       1   /* horizontal spacing (px)  */
#define DM1_V1_FMTOWNS_CHAR_Y_SPC       1   /* vertical spacing   (px)  */
#define DM1_V1_FMTOWNS_CHAR_DESCENDER   1   /* baseline descender (px)  */
#define DM1_V1_FMTOWNS_CHAR_X_WID       6   /* per-char advance   (px)  */
#define DM1_V1_FMTOWNS_CHAR_Y_HYT       7   /* per-line height    (px)  */

#define DM1_V1_FMTOWNS_SCR_X_SIZE       320 /* screen stride            */
#define DM1_V1_FMTOWNS_ICON_SIZE        256 /* icon bitmap byte size    */
#define DM1_V1_FMTOWNS_ICON_X_SIZE      16  /* icon width  (px)         */
#define DM1_V1_FMTOWNS_ICON_Y_SIZE      16  /* icon height (px)         */

/* Total horizontal pixel run of `char_count` characters, including
 * the CHAR_X_WID advance (which already folds in CHAR_X_SIZE +
 * CHAR_X_SPC). Matches text_measure (EDM.EXP 0x1a710) under a
 * static CHAR_X_WID load. */
int dm1_v1_fmtowns_text_pixel_width_pc34(int char_count);

/* Total vertical pixel run of `line_count` lines using CHAR_Y_HYT
 * (which folds in CHAR_Y_SIZE + CHAR_Y_SPC). */
int dm1_v1_fmtowns_text_pixel_height_pc34(int line_count);

/* Return the SYM1-declared vaddr of the given constant, for
 * cross-checking against EDM.EXP. Returns 0 if `name` is NULL or
 * unknown. Safe to call at any time; does not touch external files. */
uint32_t dm1_v1_fmtowns_text_geometry_vaddr_pc34(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_TEXT_GEOMETRY_H */
