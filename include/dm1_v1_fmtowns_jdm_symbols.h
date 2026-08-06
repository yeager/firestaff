#ifndef DM1_V1_FMTOWNS_JDM_SYMBOLS_H
#define DM1_V1_FMTOWNS_JDM_SYMBOLS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked JDM.EXP symbol vaddrs recovered by byte-fingerprint
 * matching against the hash-verified EDM.EXP SYM1 table. Every vaddr
 * in this header was produced by masking rel32 / absolute-data
 * operands in EDM's function body and locating the unique JDM
 * load-image offset whose remaining bytes match the EDM template.
 *
 * Evidence:
 *   parity-evidence/dm1_fmtowns_jdm_structural_map.md
 *   parity-evidence/dm1_fmtowns_jdm_symbol_recovery.md
 *
 * Only symbols recovered with a single-match, high-confidence
 * fingerprint are exposed here. Symbols we could not resolve
 * (`INIT_TEXT`, `SPC_BLOT`, BSS scalars) are intentionally omitted
 * — a JDM lookup for them returns 0, matching the runtime's
 * fail-closed contract.
 *
 * All addresses are load-image vaddrs: file offset =
 * `0x200 + vaddr`.
 */

/* Menu / drawing / text code (single-shift per adjacent object) */
#define DM1_V1_FMTOWNS_JDM_DRAW_DMENU_VADDR        0x000046e0u
#define DM1_V1_FMTOWNS_JDM_DRAW_ICN_BUTTON_VADDR   0x000045b0u
#define DM1_V1_FMTOWNS_JDM_GET_LABEL_VADDR         0x00004498u
#define DM1_V1_FMTOWNS_JDM_MOUSE_OFF_VADDR         0x0000ddb0u
#define DM1_V1_FMTOWNS_JDM_MOUSE_ON_VADDR          0x0000dd90u
#define DM1_V1_FMTOWNS_JDM_GET_SCL_COORD_VADDR     0x000194a4u
#define DM1_V1_FMTOWNS_JDM_GET_RGN_COORD_VADDR     0x00019574u
#define DM1_V1_FMTOWNS_JDM_DO_DRAW_CTEXT_VADDR     0x0001aaccu
#define DM1_V1_FMTOWNS_JDM_FILL_RECT_VADDR         0x0001febcu
#define DM1_V1_FMTOWNS_JDM_PIX_BLOT_VADDR          0x0002006cu

/* TownsOS EGB library trampolines (uniform +0x26c shift). */
#define DM1_V1_FMTOWNS_JDM_EGB_RESOLUTIONRAM_VADDR 0x000409a5u
#define DM1_V1_FMTOWNS_JDM_EGB_VIEWPORT_VADDR      0x00040a0cu
#define DM1_V1_FMTOWNS_JDM_EGB_WRITEPAGE_VADDR     0x00040a58u
#define DM1_V1_FMTOWNS_JDM_EGB_COLOR_VADDR         0x00040aa2u
#define DM1_V1_FMTOWNS_JDM_EGB_WRITEMODE_VADDR     0x00040b11u
#define DM1_V1_FMTOWNS_JDM_EGB_PAINTMODE_VADDR     0x00040b59u
#define DM1_V1_FMTOWNS_JDM_EGB_PUTBLOCK_VADDR      0x00040e58u
#define DM1_V1_FMTOWNS_JDM_EGB_RECTANGLE_VADDR     0x00041151u

/* DYNA_BUTTONS Shift-JIS label pool (from structural map §3, re-verified
 * by direct byte-content inspection in symbol-recovery evidence). */
#ifndef DM1_V1_FMTOWNS_JDM_DYNA_BUTTONS_VADDR
#define DM1_V1_FMTOWNS_JDM_DYNA_BUTTONS_VADDR      0x000243bcu
#endif

/*
 * Look up a JDM.EXP symbol by its ASCII SYM1 name (case-sensitive,
 * exactly as it appears in EDM.EXP's SYM1 table). Returns the JDM
 * load-image vaddr, or 0 if the name is not one of the recovered
 * symbols. NULL name is treated as unknown.
 */
uint32_t dm1_v1_fmtowns_jdm_symbol_vaddr_pc34(const char *name);

/* Number of symbols the lookup can resolve. */
uint32_t dm1_v1_fmtowns_jdm_symbol_count_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_JDM_SYMBOLS_H */
