#ifndef CSB_V1_FMTOWNS_DYNA_BUTTONS_H
#define CSB_V1_FMTOWNS_DYNA_BUTTONS_H

#include <stdint.h>
#include "dm1_v1_fmtowns_dyna_buttons.h"  /* the byte-verified label pool */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked CSB CHTWE.EXP DYNA_BUTTONS accessor.
 *
 * Byte-verified 2026-08-07: CSB CHTWE.EXP embeds the IDENTICAL
 * DM1 DYNA_BUTTONS label pool at CSB vaddr 0x29d50. The first
 * 500 bytes (covering every English action label BLOCK, CHOP,
 * FIREBALL, FUSE, STAB, ...) are byte-identical between DM1
 * EDM.EXP (@ 0x24194) and CSB CHTWE.EXP (@ 0x29d50).
 *
 * Consumers reuse the DM1 dyna_buttons label lookup verbatim; no
 * duplicate storage. Only the CSB-specific vaddr differs.
 */

#define CSB_V1_FMTOWNS_DYNA_BUTTONS_VADDR   0x29d50u

/* Reuse DM1's byte-verified label lookup. */
#define csb_v1_fmtowns_dyna_button_label_pc34 \
    dm1_v1_fmtowns_dyna_button_label_pc34

/* Return the CSB-specific load-image vaddr where the DYNA_BUTTONS
 * pool lives inside CHTWE.EXP. */
uint32_t csb_v1_fmtowns_dyna_buttons_vaddr_in_chtwe_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_DYNA_BUTTONS_H */
