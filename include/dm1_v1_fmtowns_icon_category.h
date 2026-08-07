#ifndef DM1_V1_FMTOWNS_ICON_CATEGORY_H
#define DM1_V1_FMTOWNS_ICON_CATEGORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 icon-category threshold table.
 *
 * LOAD_ICON at 0xbb74 in the HMA-240 English EDM.EXP walks a 7-word
 * table at [0x2918c] to find the largest bucket ≤ the requested icon
 * index. Byte-verified table (2 * 7 = 14 bytes):
 *
 *   [0] 0x2918c = 0     (bucket 0 start)
 *   [1] 0x2918e = 32
 *   [2] 0x29190 = 64
 *   [3] 0x29192 = 96
 *   [4] 0x29194 = 128
 *   [5] 0x29196 = 160
 *   [6] 0x29198 = 192   (bucket 6 start)
 *
 * The lookup returns `bucket - 1` (the largest bucket start that is
 * still less than or equal to the requested index) — that becomes
 * the category-relative row offset the icon consumer subtracts out
 * to get the intra-category index. LOAD_ICON then adds this to
 * 0x2a (the base of the icon-atlas descriptors) and MALLOCs the
 * per-icon buffer via ICON_SIZE.
 *
 * The table encodes 7 icon categories of 32 slots each = 224
 * indexable icons in the shipping EDM.EXP atlas. Consumers must
 * fail closed for indices >= 224 unless the atlas is extended.
 */

#define DM1_V1_FMTOWNS_ICON_CATEGORY_COUNT   7
#define DM1_V1_FMTOWNS_ICON_CATEGORY_STRIDE  32
#define DM1_V1_FMTOWNS_ICON_INDEX_MAX \
    (DM1_V1_FMTOWNS_ICON_CATEGORY_COUNT * DM1_V1_FMTOWNS_ICON_CATEGORY_STRIDE)

extern const uint16_t
    dm1_v1_fmtowns_icon_category_thresholds
        [DM1_V1_FMTOWNS_ICON_CATEGORY_COUNT];

/* Classify an icon index into its category bucket (0..6) and the
 * intra-category offset (0..31). Returns 1 on success; 0 if the
 * index is outside the shipping atlas (>= 224) or `out_*` is NULL.
 *
 * Mirrors LOAD_ICON's `for (bx=0; bx<7 && edi >= table[bx]; ++bx)
 *                       ; --bx; return bx, edi - table[bx]` exactly. */
int dm1_v1_fmtowns_icon_classify_pc34(
    uint16_t icon_index, uint16_t *out_category, uint16_t *out_offset);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_ICON_CATEGORY_H */
