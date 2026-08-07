#include "dm1_v1_fmtowns_egb_rect.h"
#include <string.h>

#define WALK_MAX 32

/* GET_COORD anchor types observed in the shipping FM Towns region
 * chain (from parity-evidence/dm1_fmtowns_region_table_full.md):
 *
 *   type 1  -> origin at (a, b) inside parent's rectangle
 *   type 2  -> bottom-right anchor: parent's bottom-right at (a, b)
 *   type 3  -> mid-right anchor:    parent's right edge at (a, b)
 *   type 4  -> bottom-left anchor:  parent's bottom-left at (a, b)
 *   type 9  -> SIZE (a=width, b=height)
 *   type 10 -> scale anchor tied to parent (identity in the shipping
 *              chain because no non-1:1 scale record is present)
 *
 * We reproduce the geometry exactly for these types. Unknown types
 * cause fail-closed rejection.
 */

static dm1_v1_fmtowns_egb_rect_status_t
lookup(uint16_t id, DM1_V1_FmtownsRegionRecord *out) {
    if (!dm1_v1_fmtowns_region_lookup_pc34(id, out)) {
        return DM1_V1_FMTOWNS_EGB_RECT_ERR_UNKNOWN_ID;
    }
    return DM1_V1_FMTOWNS_EGB_RECT_OK;
}

/* Compose the rectangle for `id` by walking up the parent chain.
 * Fills out->{x1,y1,x2,y2}; width/height are set by the caller. */
static dm1_v1_fmtowns_egb_rect_status_t
compose(uint16_t id, int depth, int16_t *ox1, int16_t *oy1,
        int16_t *ox2, int16_t *oy2) {
    DM1_V1_FmtownsRegionRecord rec;
    dm1_v1_fmtowns_egb_rect_status_t st;
    if (depth <= 0) return DM1_V1_FMTOWNS_EGB_RECT_ERR_CYCLE;
    st = lookup(id, &rec);
    if (st != DM1_V1_FMTOWNS_EGB_RECT_OK) return st;

    if (rec.type == 9) {
        /* SIZE: origin (0,0), extent (a-1, b-1). */
        *ox1 = 0;
        *oy1 = 0;
        *ox2 = (int16_t)(rec.a - 1);
        *oy2 = (int16_t)(rec.b - 1);
        return DM1_V1_FMTOWNS_EGB_RECT_OK;
    }
    if (rec.type == 10) {
        /* Scale anchor: identity in the shipping chain (no non-1:1
         * scale record present). Compose parent directly. */
        if (rec.parent == 0)
            return DM1_V1_FMTOWNS_EGB_RECT_ERR_BAD_PARENT;
        return compose(rec.parent, depth - 1, ox1, oy1, ox2, oy2);
    }
    if (rec.type == 1 || rec.type == 2 ||
        rec.type == 3 || rec.type == 4) {
        int16_t px1, py1, px2, py2;
        int16_t pw, ph;
        if (rec.parent == 0)
            return DM1_V1_FMTOWNS_EGB_RECT_ERR_BAD_PARENT;
        st = compose(rec.parent, depth - 1, &px1, &py1, &px2, &py2);
        if (st != DM1_V1_FMTOWNS_EGB_RECT_OK) return st;
        pw = (int16_t)(px2 - px1 + 1);
        ph = (int16_t)(py2 - py1 + 1);
        if (rec.type == 1) {
            /* origin at (a,b) inside parent */
            *ox1 = (int16_t)(px1 + rec.a);
            *oy1 = (int16_t)(py1 + rec.b);
            *ox2 = (int16_t)(*ox1 + pw - 1);
            *oy2 = (int16_t)(*oy1 + ph - 1);
        } else if (rec.type == 2) {
            /* bottom-right at (a,b) inside parent */
            *ox2 = (int16_t)(px1 + rec.a);
            *oy2 = (int16_t)(py1 + rec.b);
            *ox1 = (int16_t)(*ox2 - pw + 1);
            *oy1 = (int16_t)(*oy2 - ph + 1);
        } else if (rec.type == 3) {
            /* mid-right at (a,b): rect extends left+down from anchor */
            *ox2 = (int16_t)(px1 + rec.a);
            *oy1 = (int16_t)(py1 + rec.b);
            *ox1 = (int16_t)(*ox2 - pw + 1);
            *oy2 = (int16_t)(*oy1 + ph - 1);
        } else { /* type 4 */
            /* bottom-left at (a,b) inside parent */
            *ox1 = (int16_t)(px1 + rec.a);
            *oy2 = (int16_t)(py1 + rec.b);
            *ox2 = (int16_t)(*ox1 + pw - 1);
            *oy1 = (int16_t)(*oy2 - ph + 1);
        }
        return DM1_V1_FMTOWNS_EGB_RECT_OK;
    }
    return DM1_V1_FMTOWNS_EGB_RECT_ERR_UNSUPPORTED;
}

dm1_v1_fmtowns_egb_rect_status_t
dm1_v1_fmtowns_egb_rect_resolve_pc34(
        uint16_t region_id, dm1_v1_fmtowns_egb_rect_t *out) {
    dm1_v1_fmtowns_egb_rect_status_t st;
    if (!out) return DM1_V1_FMTOWNS_EGB_RECT_ERR_NULL;
    memset(out, 0, sizeof(*out));
    st = compose(region_id, WALK_MAX, &out->x1, &out->y1,
                 &out->x2, &out->y2);
    if (st != DM1_V1_FMTOWNS_EGB_RECT_OK) return st;
    out->width  = (int16_t)(out->x2 - out->x1 + 1);
    out->height = (int16_t)(out->y2 - out->y1 + 1);
    return DM1_V1_FMTOWNS_EGB_RECT_OK;
}
