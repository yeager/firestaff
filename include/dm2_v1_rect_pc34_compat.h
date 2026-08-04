#ifndef FIRESTAFF_DM2_V1_RECT_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_RECT_PC34_COMPAT_H

/*
 * dm2_v1_rect_pc34_compat.h — DM2 rectangle operations.
 *
 * Ports rectangle primitives from skproject/SKWINSPX/src/v5/skrect.cpp.
 * Used by the viewport renderer, HUD layout, and blit clipping.
 *
 * Source: skproject/SKWINSPX/src/v5/skrect.{h,cpp}
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t x, y, w, h;
} DM2_V1_Rect;

#define DM2_V1_NUM_TMPRECTS 4

typedef struct {
    DM2_V1_Rect rects[DM2_V1_NUM_TMPRECTS];
    int16_t index;
} DM2_V1_TempRects;

/* Initialize rect to zero. */
void dm2_v1_rect_init(DM2_V1_Rect *r);

/* Set all fields. Source: skrect.cpp c_rect::set */
void dm2_v1_rect_set(DM2_V1_Rect *r, int16_t x, int16_t y, int16_t w, int16_t h);

/* Set origin rect (x=0, y=0). Source: skrect.cpp c_rect::set_origin */
void dm2_v1_rect_set_origin(DM2_V1_Rect *r, int16_t w, int16_t h);

/* Inflate rect by dw,dh on each side. Source: skrect.cpp c_rect::inflate */
void dm2_v1_rect_inflate(DM2_V1_Rect *r, int16_t dw, int16_t dh);

/* Intersect r with clip. Returns 1 if intersection is non-empty, 0 if empty.
 * On success, r is clipped and *ox/*oy give the offset into the original rect.
 * Source: skrect.cpp c_rect::unify (DM2_UNION_RECT) */
int dm2_v1_rect_intersect(DM2_V1_Rect *r, const DM2_V1_Rect *clip,
                          int16_t *ox, int16_t *oy);

/* Center a rect of size nw x nh inside container.
 * Source: skrect.cpp c_rect::calc_centered_rect_in_rect */
void dm2_v1_rect_center_in(DM2_V1_Rect *r, const DM2_V1_Rect *container,
                           int16_t nw, int16_t nh);

/* Point-in-rect test (inclusive bounds).
 * Source: skrect.cpp c_rect::pt_in_rect */
int dm2_v1_rect_contains(const DM2_V1_Rect *r, int16_t px, int16_t py);

/* Initialize temp rect ring buffer. */
void dm2_v1_tmprects_init(DM2_V1_TempRects *t);

/* Allocate a temp rect from the ring buffer.
 * Source: skrect.cpp c_tmprects::alloc_tmprect */
DM2_V1_Rect *dm2_v1_tmprects_alloc(DM2_V1_TempRects *t,
                                    int16_t x, int16_t y,
                                    int16_t w, int16_t h);

/* Allocate a temp origin rect (x=0, y=0).
 * Source: skrect.cpp c_tmprects::alloc_origin_tmprect */
DM2_V1_Rect *dm2_v1_tmprects_alloc_origin(DM2_V1_TempRects *t,
                                           int16_t w, int16_t h);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RECT_PC34_COMPAT_H */
