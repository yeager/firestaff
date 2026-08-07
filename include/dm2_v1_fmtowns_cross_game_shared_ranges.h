#ifndef DM2_V1_FMTOWNS_CROSS_GAME_SHARED_RANGES_H
#define DM2_V1_FMTOWNS_CROSS_GAME_SHARED_RANGES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Byte-verified 2026-08-07: contiguous ranges of DM2 SKULL.EXP that
 * are byte-identical to DM1 EDM.EXP or CSB CHTWE.EXP.
 *
 * All three FM Towns titles link the same Phar Lap 386|DOS-Extender
 * + Watcom C runtime and share a common TownsOS glue blob at DM2
 * SKULL.EXP file offset ~0x0016b7..0x00e77e; the largest shared
 * range is 32 015 bytes at DM2 file 0x006928 (DM1) / 0x00693c (CSB).
 *
 * Discovered by the same bidirectional walk used for
 * `csb_v1_fmtowns_dm1_shared_ranges`: seed on a 512-byte match,
 * extend byte-by-byte until first inequality on either side.
 * Phar Lap P3 load offset is 0x200 (vaddr = file_off - 0x200).
 */

typedef struct dm2_v1_fmtowns_cross_range {
    uint32_t src_file_offset;   /* file offset in the sibling binary */
    uint32_t dm2_file_offset;
    uint32_t length_bytes;
    uint32_t src_vaddr;
    uint32_t dm2_vaddr;
} dm2_v1_fmtowns_cross_range_t;

#define DM2_V1_FMTOWNS_DM1_TO_DM2_RANGE_COUNT 9
#define DM2_V1_FMTOWNS_CSB_TO_DM2_RANGE_COUNT 10

extern const dm2_v1_fmtowns_cross_range_t
    dm2_v1_fmtowns_dm1_to_dm2_ranges[DM2_V1_FMTOWNS_DM1_TO_DM2_RANGE_COUNT];

extern const dm2_v1_fmtowns_cross_range_t
    dm2_v1_fmtowns_csb_to_dm2_ranges[DM2_V1_FMTOWNS_CSB_TO_DM2_RANGE_COUNT];

/* Translate a DM1 file offset into DM2 file offset if covered by a
 * byte-verified shared range. Returns 1 on success, 0 otherwise. */
int dm2_v1_fmtowns_dm1_to_dm2_file_offset_pc34(
    uint32_t dm1_file_offset, uint32_t *dm2_file_offset_out);

/* Same for CSB -> DM2. */
int dm2_v1_fmtowns_csb_to_dm2_file_offset_pc34(
    uint32_t csb_file_offset, uint32_t *dm2_file_offset_out);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_FMTOWNS_CROSS_GAME_SHARED_RANGES_H */
