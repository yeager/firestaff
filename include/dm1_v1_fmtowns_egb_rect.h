#ifndef DM1_V1_FMTOWNS_EGB_RECT_H
#define DM1_V1_FMTOWNS_EGB_RECT_H

#include <stdint.h>
#include "dm1_v1_fmtowns_menu_regions.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-lifted rectangle resolver for the FM Towns DM1 EGB
 * primitive path. Reproduces the subset of GET_COORD (0x18df0)
 * semantics used by every menu / HUD / viewport paint: walk the
 * region parent chain, apply SIZE (type 9) and anchor (types 1-4)
 * records exactly as EDM.EXP does, then return the resolved
 * screen-space (x1, y1, x2, y2) rectangle.
 *
 * The full GET_COORD implements 9 anchor types (1..4 + 10..18 scale
 * variants). This resolver ships the 5 types that actually appear in
 * the shipping FM Towns region blocks (verified by walking the
 * disassembled table): 9 (SIZE), 1 (origin), 2 (anchor-BR),
 * 3 (anchor-TR-mid), 4 (anchor-BL). Types 5..8 and the 10..18 scale
 * anchors have no callers in the menu path and are rejected with a
 * clear return code so any future user notices immediately.
 *
 * The resolver never invents pixels; if a region record is malformed
 * or its parent chain contains an unsupported type, resolve_pc34
 * returns 0 and callers fall through to the fail-closed "no draw"
 * path — the same policy the source engine uses.
 */

typedef enum {
    DM1_V1_FMTOWNS_EGB_RECT_OK                 = 0,
    DM1_V1_FMTOWNS_EGB_RECT_ERR_NULL           = 1,
    DM1_V1_FMTOWNS_EGB_RECT_ERR_UNKNOWN_ID     = 2,
    DM1_V1_FMTOWNS_EGB_RECT_ERR_BAD_PARENT     = 3,
    DM1_V1_FMTOWNS_EGB_RECT_ERR_UNSUPPORTED    = 4,
    DM1_V1_FMTOWNS_EGB_RECT_ERR_CYCLE          = 5
} dm1_v1_fmtowns_egb_rect_status_t;

typedef struct {
    int16_t x1, y1, x2, y2;   /* inclusive rectangle */
    int16_t width, height;    /* convenience (x2-x1+1, y2-y1+1) */
} dm1_v1_fmtowns_egb_rect_t;

/* Resolve a region_id into its screen-space rectangle. Walks up
 * the parent chain (bounded by cycle depth 32) to compose size and
 * anchor records the same way GET_COORD does. Populates *out on
 * success. */
dm1_v1_fmtowns_egb_rect_status_t
dm1_v1_fmtowns_egb_rect_resolve_pc34(
    uint16_t region_id, dm1_v1_fmtowns_egb_rect_t *out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_EGB_RECT_H */
