#ifndef DM1_V1_FMTOWNS_MENU_BSS_H
#define DM1_V1_FMTOWNS_MENU_BSS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 menu BSS symbol addresses and record
 * layouts. Every vaddr is byte-verified against the hash-verified
 * HMA-240 English `EDM.EXP` via disassembly recovery documented in
 * `parity-evidence/dm1_fmtowns_menu_p3_disassembly.md` and the JDM
 * companion `parity-evidence/dm1_fmtowns_jdm_bss_triangulation.md`.
 *
 * All symbols named here are BSS scalars (zero-initialised at load)
 * or BSS structures whose runtime contents are set by the executable
 * — the .EXP file therefore contains no initialised data at these
 * addresses. Consumers use these constants as source-lifted layout
 * knowledge: field offsets to read PLAYER records, ID values to test
 * MENU_OWNER, and so on. Neither the vaddrs nor the structure offsets
 * may be modified without matching evidence updates in
 * parity-evidence/.
 */

/* Menu state scalars (words unless noted). */
#define DM1_V1_FMTOWNS_MENU_BSS_MENU_OWNER_VADDR          0x24156U
#define DM1_V1_FMTOWNS_MENU_BSS_NUM_DYNABTNS_VADDR        0x24158U
#define DM1_V1_FMTOWNS_MENU_BSS_REDRAW_MENU_VADDR         0x2415aU
#define DM1_V1_FMTOWNS_MENU_BSS_MENU_ICONS_VADDR          0x2415cU
#define DM1_V1_FMTOWNS_MENU_BSS_ARMR_OPTS_VADDR           0x24160U
#define DM1_V1_FMTOWNS_MENU_BSS_DYNAMENU_VADDR            0x2418cU
#define DM1_V1_FMTOWNS_MENU_BSS_DYNA_BUTTONS_VADDR        0x24194U

/* Mouse tracking. MSE_STATE is a reference-counted hide depth
 * decremented by MOUSE_OFF and re-armed by MOUSE_ON. */
#define DM1_V1_FMTOWNS_MENU_BSS_MSE_STATE_VADDR           0x25848U

/* Party / champion display state. */
#define DM1_V1_FMTOWNS_MENU_BSS_PICKING_CHARACTER_VADDR   0x29418U
#define DM1_V1_FMTOWNS_MENU_BSS_PARTY_RESTING_VADDR       0x2941aU
#define DM1_V1_FMTOWNS_MENU_BSS_PARTY_SIZE_VADDR          0x29424U

/* Screen descriptor (dword pointer to the active framebuffer
 * descriptor consumed by every EGB primitive). */
#define DM1_V1_FMTOWNS_MENU_BSS_SCREEN_VADDR              0x31290U

/* PLAYER records: 6 slots at [0x26158], stride 319 bytes each,
 * spanning through 0x26158 + 6 * 319 = 0x266ce.
 *
 * DRAW_ICN_BUTTON at 0x44f0 reads:
 *   [player_base + 0x34]  -> presence word (jne guards the draw)
 *   [player_base + 0xd5]  -> OICON_INDEX raw value
 *                            (0xffff sentinel triggers alt path)
 *
 * The 319-byte stride is derived from EDM.EXP's own address
 * arithmetic: `lea edx, [eax + eax*4]; shl edx, 6; sub edx, eax`
 * evaluates to `edx = eax * 319`. */
#define DM1_V1_FMTOWNS_MENU_BSS_PLAYER_BASE_VADDR         0x26158U
#define DM1_V1_FMTOWNS_MENU_BSS_PLAYER_STRIDE_BYTES       319U
#define DM1_V1_FMTOWNS_MENU_BSS_PLAYER_MAX_SLOTS          6U

/* Byte offset within a PLAYER record where the presence guard word
 * lives. DRAW_ICN_BUTTON tests it non-zero before proceeding. */
#define DM1_V1_FMTOWNS_MENU_BSS_PLAYER_OFFSET_PRESENT     0x34U

/* Byte offset within a PLAYER record where the OICON index word
 * lives. 0xffff means "use fallback default (0xc9)". */
#define DM1_V1_FMTOWNS_MENU_BSS_PLAYER_OFFSET_OICON       0xd5U

/* Fallback OICON index consulted when PLAYER[+0xd5] == 0xffff. */
#define DM1_V1_FMTOWNS_MENU_BSS_PLAYER_OICON_DEFAULT      0xc9U

/* GET_LABEL text address arithmetic:
 *   record_ptr = 0x26019 + MENU_OWNER * 319
 * (5 * MENU_OWNER shifted left by 6 minus MENU_OWNER). This is a
 * per-player text buffer base that sits 0x13f bytes before the
 * corresponding PLAYER record's start (0x26019 = 0x26158 - 0x13f).
 * Consumers use it to read the currently-active main label. */
#define DM1_V1_FMTOWNS_MENU_BSS_PLAYER_LABEL_BASE_VADDR   0x26019U

/* Number of PLAYER records to walk when drawing the icon strip
 * (DRAW_ICN_BUTTON iteration limit = PARTY_SIZE, but the buffer
 * itself is sized for 6 slots so callers must bound both). */
#define DM1_V1_FMTOWNS_MENU_BSS_PARTY_MAX_ICONS           4U

/* Return the vaddr for PLAYER slot `party_index`. Returns 0 when
 * `party_index >= 6`. The offset is (base + party_index * 319). */
uint32_t dm1_v1_fmtowns_menu_bss_player_vaddr_pc34(unsigned int party_index);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_MENU_BSS_H */
