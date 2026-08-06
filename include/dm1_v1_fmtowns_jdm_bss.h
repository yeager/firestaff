#ifndef DM1_V1_FMTOWNS_JDM_BSS_H
#define DM1_V1_FMTOWNS_JDM_BSS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked JDM.EXP BSS-scalar vaddr map for the FM Towns DM1
 * menu chain. Every address below is recovered from JDM.EXP by one
 * of two byte-verified techniques against the hash-verified
 * executables:
 *
 *   (a) XREF triangulation - locate every EDM.EXP code site that
 *       loads the scalar's vaddr as an operand, mask the address
 *       bytes, search JDM.EXP for the same masked fingerprint, and
 *       read the JDM address from the un-masked bytes of the match.
 *       Accepted only when at least one voting fingerprint returns
 *       a unique JDM candidate; multiple non-conflicting votes
 *       accepted with confidence proportional to the vote count.
 *
 *   (b) Neighbor-delta derivation - when a scalar sits at
 *       EDM.OFFSET_OF(peer) + N bytes and its peer is recovered by
 *       (a), place the scalar at JDM.OFFSET_OF(peer) + N. Only
 *       used when the peer is inside the same tightly-packed BSS
 *       block (2 to 8 byte stride) with all peers sharing an
 *       identical shift.
 *
 * Neither technique invents an address. If a scalar cannot be
 * recovered by (a) or provably placed by (b), the JDM vaddr is
 * simply omitted from this table.
 *
 * Companion module: `dm1_v1_fmtowns_jdm_symbols` (code + EGB
 * trampoline vaddrs). This file only lists the BSS scalars that
 * the code-fingerprint pass couldn't recover.
 *
 * Evidence:
 *   parity-evidence/dm1_fmtowns_jdm_structural_map.md
 *   parity-evidence/dm1_fmtowns_jdm_symbol_recovery.md
 *   parity-evidence/dm1_fmtowns_jdm_bss_triangulation.md (this
 *     module's evidence).
 */

/* Menu-owner block (JDM shift = +0x228 vs EDM). Recovered as: */
/*   MENU_OWNER, MENU_ICONS by triangulation (8 and 1 votes).    */
/*   NUM_DYNABTNS, REDRAW_MENU by neighbor-delta (block is       */
/*   contiguous 6-byte header + 2-byte flag then MENU_ICONS).    */
#define DM1_V1_FMTOWNS_JDM_MENU_OWNER_VADDR       0x2437eu
#define DM1_V1_FMTOWNS_JDM_NUM_DYNABTNS_VADDR     0x24380u  /* +2 */
#define DM1_V1_FMTOWNS_JDM_REDRAW_MENU_VADDR      0x24382u  /* +4 */
#define DM1_V1_FMTOWNS_JDM_MENU_ICONS_VADDR       0x24384u  /* +6 */

/* DYNAMENU 8-byte record and its DYNA_BUTTONS neighbour.
 * DYNAMENU inferred by shift (+0x228) confirmed on MENU_OWNER
 * / MENU_ICONS; DYNA_BUTTONS independently confirmed by the
 * JDM structural-map string search. */
#define DM1_V1_FMTOWNS_JDM_DYNAMENU_VADDR         0x243b4u
#define DM1_V1_FMTOWNS_JDM_DYNA_BUTTONS_VADDR     0x243bcu

/* Screen / icon block (JDM shift = +0x264 vs EDM).              */
/*   SCR_X_SIZE, ICON_X_SIZE, ICON_Y_SIZE, CHAR_X_WID recovered  */
/*   by triangulation. ICON_SIZE by neighbor-delta.              */
#define DM1_V1_FMTOWNS_JDM_SCR_X_SIZE_VADDR       0x26eccu
#define DM1_V1_FMTOWNS_JDM_ICON_SIZE_VADDR        0x26edau  /* ICON_X - 2 */
#define DM1_V1_FMTOWNS_JDM_ICON_X_SIZE_VADDR      0x26edcu
#define DM1_V1_FMTOWNS_JDM_ICON_Y_SIZE_VADDR      0x26edeu

/* Character-metrics block (JDM shift = +0x276 vs EDM).          */
/*   CHAR_Y_SIZE, CHAR_Y_SPC, CHAR_Y_HYT recovered by            */
/*   triangulation. CHAR_X_SIZE, CHAR_X_SPC, CHAR_DESCENDER,     */
/*   CHAR_X_WID (JDM) derived by neighbor-delta within the       */
/*   contiguous 12-byte block.                                   */
#define DM1_V1_FMTOWNS_JDM_CHAR_X_SIZE_VADDR      0x26f00u  /* CHAR_Y - 2 */
#define DM1_V1_FMTOWNS_JDM_CHAR_Y_SIZE_VADDR      0x26f02u
#define DM1_V1_FMTOWNS_JDM_CHAR_X_SPC_VADDR       0x26f04u
#define DM1_V1_FMTOWNS_JDM_CHAR_Y_SPC_VADDR       0x26f06u
#define DM1_V1_FMTOWNS_JDM_CHAR_DESCENDER_VADDR   0x26f08u
#define DM1_V1_FMTOWNS_JDM_CHAR_X_WID_VADDR       0x26f0au  /* neighbor  */
#define DM1_V1_FMTOWNS_JDM_CHAR_Y_HYT_VADDR       0x26f0cu

/* Party/game state (JDM shift = +0x26c vs EDM).
 * PARTY_SIZE recovered by triangulation with 24 concurring votes. */
#define DM1_V1_FMTOWNS_JDM_PARTY_SIZE_VADDR       0x29690u

/* Look up a JDM.EXP BSS vaddr by symbol name. Returns 0 if the
 * name is NULL or not one of the recovered scalars. */
uint32_t dm1_v1_fmtowns_jdm_bss_vaddr_pc34(const char *name);

/* Number of recovered scalars. */
size_t   dm1_v1_fmtowns_jdm_bss_count_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_JDM_BSS_H */
