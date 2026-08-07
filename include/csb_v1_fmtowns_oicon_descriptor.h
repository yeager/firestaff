#ifndef CSB_V1_FMTOWNS_OICON_DESCRIPTOR_H
#define CSB_V1_FMTOWNS_OICON_DESCRIPTOR_H

#include <stdint.h>
#include "dm1_v1_fmtowns_oicon_descriptor.h"  /* the 224 x 6 data */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked CSB CHTWE.EXP OICON descriptor accessor.
 *
 * Byte-verified 2026-08-07: CSB CHTWE.EXP embeds the IDENTICAL
 * 1344-byte OICON descriptor table (224 records * 6 bytes) as DM1
 * EDM.EXP, just at a different vaddr. Full 1344/1344 byte match
 * confirmed by SHA-comparable memcmp.
 *
 *   DM1 EDM.EXP: OICON @ 0x224db
 *   CSB CHTWE.EXP: OICON @ 0x27f77
 *
 * This module aliases the DM1 OICON constant array so CSB consumers
 * can access the same 224 kind bytes and 6-byte records without
 * duplicating storage. Only the CSB-specific vaddr differs.
 */

#define CSB_V1_FMTOWNS_OICON_KIND_COUNT   DM1_V1_FMTOWNS_OICON_KIND_COUNT
#define CSB_V1_FMTOWNS_OICON_VADDR        0x27f77u

/* Reuse DM1's byte-verified tables. */
#define csb_v1_fmtowns_oicon_kind          dm1_v1_fmtowns_oicon_kind
#define csb_v1_fmtowns_oicon_descriptor    dm1_v1_fmtowns_oicon_descriptor

/* Same accessors as DM1 — thin aliases. */
#define csb_v1_fmtowns_oicon_kind_at_pc34  dm1_v1_fmtowns_oicon_kind_at_pc34
#define csb_v1_fmtowns_oicon_is_thing_pc34 dm1_v1_fmtowns_oicon_is_thing_pc34
#define csb_v1_fmtowns_oicon_descriptor_at_pc34 \
    dm1_v1_fmtowns_oicon_descriptor_at_pc34

/* Return the CSB-specific load-image vaddr where the OICON
 * descriptor lives inside CHTWE.EXP. */
uint32_t csb_v1_fmtowns_oicon_vaddr_in_chtwe_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_OICON_DESCRIPTOR_H */
