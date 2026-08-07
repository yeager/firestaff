#ifndef CSB_V1_FMTOWNS_DM1_SHARED_RANGES_H
#define CSB_V1_FMTOWNS_DM1_SHARED_RANGES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Byte-verified 2026-08-07: contiguous ranges of CSB CHTWE.EXP that
 * are byte-identical to DM1 EDM.EXP. Discovered by an extension
 * scan from known-shared seed points (OICON descriptor confirmed
 * first; then bidirectional byte-match walk to find the maximum
 * contiguous identity length).
 *
 * These map is authoritative for cross-game alias recovery: any
 * DM1 table whose file offset falls inside one of these DM1 ranges
 * is guaranteed byte-identical in CSB at the corresponding CSB
 * file offset (delta applies uniformly per range, not globally —
 * the four ranges use different deltas because the two binaries
 * have different layouts).
 *
 * Phar Lap P3 load offset is 0x200; subtract from file offset to
 * get the runtime vaddr each binary references.
 */

typedef struct csb_v1_fmtowns_dm1_shared_range {
    uint32_t dm1_file_offset;
    uint32_t csb_file_offset;
    uint32_t length_bytes;
    uint32_t dm1_vaddr;
    uint32_t csb_vaddr;
} csb_v1_fmtowns_dm1_shared_range_t;

#define CSB_V1_FMTOWNS_DM1_SHARED_RANGE_COUNT 4

extern const csb_v1_fmtowns_dm1_shared_range_t
    csb_v1_fmtowns_dm1_shared_ranges[CSB_V1_FMTOWNS_DM1_SHARED_RANGE_COUNT];

/* Translate a DM1 file offset into the corresponding CSB file
 * offset if it lies inside one of the byte-verified shared ranges.
 * Returns 1 on success (out set), 0 if no shared range covers the
 * requested DM1 offset. Length is not validated — the caller is
 * responsible for staying within `length_bytes` from the range's
 * DM1 base. */
int csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(
    uint32_t dm1_file_offset, uint32_t *csb_file_offset_out);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_DM1_SHARED_RANGES_H */
