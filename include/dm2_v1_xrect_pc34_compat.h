#ifndef FIRESTAFF_DM2_V1_XRECT_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_XRECT_PC34_COMPAT_H

/*
 * dm2_v1_xrect_pc34_compat.h -- DM2 extended rectangle operations.
 *
 * Source: skproject c_xrect.cpp (9 functions).
 * All public functions use callback-based architecture.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define DM2_V1_NUM_QUERYRECTS  4
#define DM2_V1_XRECT_SCALE    10000

/* ========================================================================
 * Rect type (reuse from gui_draw if included, else define here)
 * ======================================================================== */

#ifndef FIRESTAFF_DM2_V1_GUI_DRAW_PC34_COMPAT_H
typedef struct DM2_V1_Rect {
    int16_t x, y, w, h;
} DM2_V1_Rect;
#endif

/* ========================================================================
 * s_rnode -- compressed rectangle list node
 * Source: c_xrect.h s_rnode
 * ======================================================================== */

typedef struct DM2_V1_RNode {
    struct DM2_V1_RNode *next;  /* @00 */
    int16_t  min;               /* @04 */
    int16_t  max;               /* @06 */
    uint8_t  mask;              /* @08 */
    int8_t   b_x;               /* @09 */
    /* variable-length compressed data follows at offset 10 */
} DM2_V1_RNode;

/* ========================================================================
 * Xrect state -- replaces global c_xrectdat
 * ======================================================================== */

typedef struct DM2_V1_XrectState {
    int16_t       queryrectsindex;
    DM2_V1_Rect   queryrects[DM2_V1_NUM_QUERYRECTS];
    DM2_V1_RNode *rnodep_rectanglelist;
} DM2_V1_XrectState;

/* ========================================================================
 * Callback struct -- external dependencies
 * ======================================================================== */

typedef struct DM2_V1_XrectCallbacks {
    void *ctx;

    /* Clamp value to [minv, maxv]. Source: DM2_BETWEEN_VALUE */
    int16_t (*between_value)(void *ctx, int16_t minv, int16_t maxv,
                             int16_t val);

    /* min/max helpers */
    int16_t (*min_i16)(void *ctx, int16_t a, int16_t b);

    /* Get bitmap width/height (NULL bmp returns 0) */
    int16_t (*get_bmp_width)(void *ctx, void *bmp);
    int16_t (*get_bmp_height)(void *ctx, void *bmp);

    /* Memory allocator for COMPRESS_RECTS nodes */
    void *(*alloc_freepool)(void *ctx, int32_t size, bool clear);

    /* Global rect state flags (ddat fields) */
    bool (*get_v1e01d0)(void *ctx);
    bool (*get_v1e01d8)(void *ctx);
    int16_t (*get_v1e025c)(void *ctx);

    /* Global rects access */
    DM2_V1_Rect *(*get_glblrect1)(void *ctx);
    DM2_V1_Rect *(*get_glblrect2)(void *ctx);
} DM2_V1_XrectCallbacks;

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_QueryTopleftReceipt {
    int16_t x;
    int16_t y;
} DM2_V1_QueryTopleftReceipt;

typedef struct DM2_V1_Rect098d04c7Receipt {
    int16_t wc;
    int16_t we;
} DM2_V1_Rect098d04c7Receipt;

/* ========================================================================
 * Function declarations
 * ======================================================================== */

/* Initialize xrect state. Source: c_xrectdat::init */
void dm2_v1_xrect_init(DM2_V1_XrectState *st);

/* Source: SKW_098d_02a2 -- point in expanded rect test */
bool dm2_v1_pt_in_expanded_rect(DM2_V1_XrectState *st,
                                const DM2_V1_XrectCallbacks *cb,
                                int16_t query, int16_t x, int16_t y);

/* Source: SKW_QUERY_RECT -- decode a rect from compressed list */
DM2_V1_Rect *dm2_v1_query_rect(DM2_V1_XrectState *st,
                                int16_t query);

/* Source: DM2_rect_098d_04c7 -- interpolate two rects */
DM2_V1_Rect098d04c7Receipt dm2_v1_rect_098d_04c7(
    DM2_V1_XrectState *st,
    const DM2_V1_XrectCallbacks *cb,
    int16_t wa, int16_t wd, int16_t wb);

/* Source: DM2_QUERY_BLIT_RECT -- compute blit rectangle */
DM2_V1_Rect *dm2_v1_query_blit_rect(DM2_V1_XrectState *st,
                                     const DM2_V1_XrectCallbacks *cb,
                                     void *bmp, DM2_V1_Rect *blitrect,
                                     int16_t query1,
                                     int16_t *xout, int16_t *yout,
                                     int16_t query2);

/* Source: DM2_QUERY_TOPLEFT_OF_RECT */
DM2_V1_QueryTopleftReceipt dm2_v1_query_topleft_of_rect(
    DM2_V1_XrectState *st,
    const DM2_V1_XrectCallbacks *cb,
    int16_t wn);

/* Source: DM2_SCALE_RECT */
DM2_V1_Rect *dm2_v1_scale_rect(DM2_V1_XrectState *st,
                                const DM2_V1_XrectCallbacks *cb,
                                int16_t query, int16_t scalew,
                                int16_t scaleh, DM2_V1_Rect *r);

/* Source: DM2_QUERY_EXPANDED_RECT */
DM2_V1_Rect *dm2_v1_query_expanded_rect(DM2_V1_XrectState *st,
                                         const DM2_V1_XrectCallbacks *cb,
                                         int16_t query, DM2_V1_Rect *r);

/* Source: DM2_COMPRESS_RECTS */
void dm2_v1_compress_rects(DM2_V1_XrectState *st,
                           const DM2_V1_XrectCallbacks *cb,
                           void *buffer, DM2_V1_RNode *firstnode);

/* Source: DM2_CALC_SIZE_OF_COMPRESSED_RECT (exposed for testing) */
int16_t dm2_v1_calc_size_of_compressed_rect(uint8_t mask);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_XRECT_PC34_COMPAT_H */
