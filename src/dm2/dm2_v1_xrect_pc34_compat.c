/*
 * dm2_v1_xrect_pc34_compat.c -- DM2 extended rectangle operations.
 *
 * Ports c_xrect.cpp from skproject to callback-based pure C.
 *
 * Source: skproject/SKULLWIN/c_xrect.cpp
 */

#include "dm2_v1_xrect_pc34_compat.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────── */

/* Source: c_rect::pt_in_rect */
static bool rect_pt_in(const DM2_V1_Rect *r, int16_t px, int16_t py)
{
    return px >= r->x && px < r->x + r->w &&
           py >= r->y && py < r->y + r->h;
}

/* Source: c_rect::unify -- intersect two rects, return NULL if empty.
 * Adjusts ox/oy to the clipping offset. */
static DM2_V1_Rect *rect_unify(DM2_V1_Rect *self, const DM2_V1_Rect *rs,
                                int16_t *ox, int16_t *oy)
{
    int16_t ax = self->x < rs->x ? rs->x : self->x;
    int16_t ay = self->y < rs->y ? rs->y : self->y;
    int16_t bx = (self->x + self->w) < (rs->x + rs->w)
                 ? (self->x + self->w) : (rs->x + rs->w);
    int16_t by = (self->y + self->h) < (rs->y + rs->h)
                 ? (self->y + self->h) : (rs->y + rs->h);
    if (bx <= ax || by <= ay)
        return NULL;
    *ox = ax - self->x;
    *oy = ay - self->y;
    self->x = ax;
    self->y = ay;
    self->w = bx - ax;
    self->h = by - ay;
    return self;
}

/* Source: crdecode -- decode blit position from mode */
static void crdecode(int16_t mode, int16_t x0, int16_t y0,
                     int16_t x1, int16_t y1,
                     int16_t *xout, int16_t *yout)
{
    switch (mode) {
    case 0:
        *xout = x0 - (x1 + 1) / 2;
        *yout = y0 - (y1 + 1) / 2;
        break;
    case 1:
        *xout = x0;
        *yout = y0;
        break;
    case 2:
        *xout = x0 - x1 + 1;
        *yout = y0;
        break;
    case 3:
        *xout = x0 - x1 + 1;
        *yout = y0 - y1 + 1;
        break;
    case 4:
        *xout = x0;
        *yout = y0 - y1 + 1;
        break;
    case 5:
        *xout = x0 - (x1 + 1) / 2;
        *yout = y0;
        break;
    case 6:
        *xout = x0 - x1 + 1;
        *yout = y0 - (y1 + 1) / 2;
        break;
    case 7:
        *xout = x0 - (x1 + 1) / 2;
        *yout = y0 - y1 + 1;
        break;
    case 8:
        *xout = x0;
        *yout = y0 - (y1 + 1) / 2;
        break;
    default:
        *xout = 0;
        *yout = 0;
        break;
    }
}

/* ── Init ───────────────────────────────────────────────────────────── */

/* Source: c_xrectdat::init */
void dm2_v1_xrect_init(DM2_V1_XrectState *st)
{
    st->queryrectsindex = 0;
    memset(st->queryrects, 0, sizeof(st->queryrects));
    st->rnodep_rectanglelist = NULL;
}

/* ── DM2_CALC_SIZE_OF_COMPRESSED_RECT ───────────────────────────────── */

/* Source: c_xrect.cpp DM2_CALC_SIZE_OF_COMPRESSED_RECT */
int16_t dm2_v1_calc_size_of_compressed_rect(uint8_t mask)
{
    int16_t wret = 8;

    if (mask & 0x4)
        wret = 6;
    else {
        if (mask & 0x2)
            wret = 6;
        if (mask & 0x1)
            wret -= 2;
    }

    if (mask & 0x18)
        wret -= 2;

    return wret;
}

/* ── DM2_QUERY_RECT ─────────────────────────────────────────────────── */

/* Source: c_xrect.cpp SKW_QUERY_RECT */
DM2_V1_Rect *dm2_v1_query_rect(DM2_V1_XrectState *st, int16_t query)
{
    if (query == 0)
        return NULL;

    DM2_V1_RNode *rnodep = st->rnodep_rectanglelist;
    for (;;) {
        if (rnodep == NULL)
            return NULL;
        if (rnodep->min <= query && query <= rnodep->max)
            break;
        rnodep = rnodep->next;
    }

    query -= rnodep->min;

    if (++st->queryrectsindex >= DM2_V1_NUM_QUERYRECTS)
        st->queryrectsindex = 0;

    DM2_V1_Rect *r = &st->queryrects[st->queryrectsindex];

    uint8_t mask = rnodep->mask;

    if (mask & 0x2)
        r->x = (uint16_t)(uint8_t)rnodep->b_x;

    /* Variable data starts immediately after the fixed node fields */
    int8_t *base = (int8_t *)rnodep + sizeof(DM2_V1_RNode);
    int8_t *ptr = base;

    if (mask & 0x1) {
        r->y = *(int16_t *)ptr;
        ptr += 2;
    }

    ptr += query * dm2_v1_calc_size_of_compressed_rect(mask);

    if (mask & 0x4) {
        r->x = (uint16_t)(uint8_t)*ptr++;
        r->y = (uint16_t)(uint8_t)*ptr++;
    } else {
        if ((mask & 0x2) == 0) {
            r->x = *(int16_t *)ptr;
            ptr += 2;
        }
        if ((mask & 0x1) == 0) {
            r->y = *(int16_t *)ptr;
            ptr += 2;
        }
    }

    /* width and height: two bytes (signed or unsigned) or two words */
    if (mask & 0x8) {
        r->w = (int16_t)(int8_t)ptr[0];
        r->h = (int16_t)(int8_t)ptr[1];
    } else if (mask & 0x10) {
        r->w = (uint16_t)(uint8_t)ptr[0];
        r->h = (uint16_t)(uint8_t)ptr[1];
    } else {
        r->w = ((int16_t *)ptr)[0];
        r->h = ((int16_t *)ptr)[1];
    }

    return r;
}

/* ── DM2_PT_IN_EXPANDED_RECT ───────────────────────────────────────── */

/* Source: c_xrect.cpp SKW_098d_02a2 */
bool dm2_v1_pt_in_expanded_rect(DM2_V1_XrectState *st,
                                const DM2_V1_XrectCallbacks *cb,
                                int16_t query, int16_t x, int16_t y)
{
    DM2_V1_Rect rc;
    if (dm2_v1_query_expanded_rect(st, cb, query, &rc) == NULL)
        return false;
    return rect_pt_in(&rc, x, y);
}

/* ── DM2_rect_098d_04c7 ────────────────────────────────────────────── */

/* Source: c_xrect.cpp DM2_rect_098d_04c7 */
DM2_V1_Rect098d04c7Receipt dm2_v1_rect_098d_04c7(
    DM2_V1_XrectState *st,
    const DM2_V1_XrectCallbacks *cb,
    int16_t wa, int16_t wd, int16_t wb)
{
    DM2_V1_Rect098d04c7Receipt receipt = {0, 0};

    DM2_V1_Rect *rect1 = dm2_v1_query_rect(st, wa);
    if (rect1 == NULL)
        return receipt;

    DM2_V1_Rect *rect2 = dm2_v1_query_rect(st, wd);
    if (rect2 == NULL)
        return receipt;

    wb = cb->between_value(cb->ctx, 0, 100, wb);
    if (wb != 0) {
        receipt.wc = (int16_t)((rect2->w - rect1->w) * wb / 100);
        receipt.we = (int16_t)((rect2->h - rect1->h) * wb / 100);
    }
    return receipt;
}

/* ── DM2_QUERY_BLIT_RECT ───────────────────────────────────────────── */

/* rinfo overlay: reinterpret a queried rect as mode/link/datax/datay.
 * Source: c_rinfo class in c_xrect.cpp */
typedef struct {
    int16_t mode1;  /* x */
    int16_t mode2;  /* y */
    int16_t datax;  /* w */
    int16_t datay;  /* h */
} DM2_V1_RInfo;

static const DM2_V1_RInfo *as_rinfo(const DM2_V1_Rect *r)
{
    return (const DM2_V1_RInfo *)r;
}

/* Source: c_xrect.cpp DM2_QUERY_BLIT_RECT */
DM2_V1_Rect *dm2_v1_query_blit_rect(DM2_V1_XrectState *st,
                                     const DM2_V1_XrectCallbacks *cb,
                                     void *bmp, DM2_V1_Rect *blitrect,
                                     int16_t query1,
                                     int16_t *xout, int16_t *yout,
                                     int16_t query2)
{
    int16_t x0, y0;
    int16_t blitrectmode;
    DM2_V1_Rect rc;
    DM2_V1_RInfo ri_04;

    if (query1 == -1)
        return NULL;

    bool sign = query1 < 0;
    if (sign)
        query1 &= 0x7fff;

    const DM2_V1_RInfo *rinfop2 = as_rinfo(
        dm2_v1_query_rect(st, query1));
    if (rinfop2 == NULL)
        return NULL;

    rc.x = -10000; rc.y = -10000; rc.w = 20000; rc.h = 20000;

    if (query2 != -1) {
        ri_04 = *rinfop2;
        ri_04.mode1 = query2;
        blitrectmode = query2;
        rinfop2 = &ri_04;
    } else {
        blitrectmode = rinfop2->mode1;
    }

    if (blitrectmode > 8) {
        if (blitrectmode == 9)
            return NULL;
        x0 = 0;
        y0 = 0;
        blitrectmode -= 10;
    } else {
        x0 = rinfop2->datax;
        y0 = rinfop2->datay;
    }

    if (sign) {
        x0 += *xout;
        y0 += *yout;
        *xout = 0;
        *yout = 0;
    }

    if (bmp == NULL && (*xout <= 0 || *yout <= 0))
        return NULL;

    int16_t deltax = 0;
    int16_t deltay = 0;
    bool flag = false;
    int16_t query_w2 = 0;
    const DM2_V1_RInfo *rinfop1 = NULL;

    while (rinfop2->mode2 != 0) {
        if (rinfop2->mode1 < 10 || rinfop2->mode1 > 18) {
            rinfop1 = as_rinfo(
                dm2_v1_query_rect(st, rinfop2->mode2));
            if (rinfop1 == NULL)
                break;

            query_w2 = rinfop2->mode2;
            deltax = rinfop1->datax;
            deltay = rinfop1->datay;
            if (rinfop1->mode1 != 1) {
                if (rinfop1->mode1 != 9) {
                    if (rinfop1->mode1 <= 8)
                        flag = true;
                } else {
                    if (rinfop2->mode1 <= 8)
                        crdecode(rinfop2->mode1, rinfop2->datax,
                                 rinfop2->datay, rinfop1->datax,
                                 rinfop1->datay, &deltax, &deltay);

                    if (flag) {
                        flag = false;
                        x0 += deltax;
                        y0 += deltay;
                        rc.x += deltax;
                        rc.y += deltay;
                    }

                    if (deltax > rc.x)
                        rc.x = deltax;
                    if (rc.w + rc.x - 1 >= deltax + rinfop1->datax)
                        rc.w = rinfop1->datax - rc.x + deltax;
                    if (rc.y < deltay)
                        rc.y = deltay;
                    deltax = rinfop1->datay + deltay;
                    if (rc.y + rc.h - 1 >= deltax)
                        rc.h = deltay + rinfop1->datay - rc.y;
                }
            } else {
                x0 += deltax;
                y0 += deltay;
                rc.x += deltax;
                rc.y += deltay;
            }
        } else {
            /* mode1 in [10..18] -- two-query decode path */
            const DM2_V1_RInfo *tmprect = as_rinfo(
                dm2_v1_query_rect(st, rinfop2->mode2));
            if (tmprect == NULL)
                break;
            query_w2 = tmprect->mode2;
            deltax = tmprect->datax;
            deltay = tmprect->datay;

            rinfop1 = as_rinfo(
                dm2_v1_query_rect(st, query_w2));
            if (rinfop1 == NULL)
                break;

            switch (tmprect->mode1) {
            case 0:
                deltay -= (rinfop1->datay + 1) / 2;
                /* fallthrough */
            case 5:
                deltax -= (rinfop1->datax + 1) / 2;
                /* fallthrough */
            case 1:
                break;
            case 3:
                deltay -= rinfop1->datay - 1;
                /* fallthrough */
            case 2:
                deltax -= rinfop1->datax - 1;
                break;
            case 6:
                deltax -= rinfop1->datax - 1;
                /* fallthrough */
            case 8:
                deltay -= (rinfop1->datay + 1) / 2;
                break;
            case 7:
                deltax -= (rinfop1->datax + 1) / 2;
                /* fallthrough */
            case 4:
                deltay -= rinfop1->datay - 1;
                break;
            default:
                return NULL;
            }

            rc.x += deltax;
            if (deltax > rc.x)
                rc.x = deltax;

            if (rinfop1->datax + deltax <= rc.x + rc.w - 1)
                rc.w = rinfop1->datax - rc.x + deltax;
            else
                rc.w = rinfop1->datax + deltax;

            rc.y += deltay;
            if (rc.y < deltay)
                rc.y = deltay;
            if (deltay + rinfop1->datay <= rc.y + rc.h - 1)
                rc.h = deltay + rinfop1->datay - rc.y;

            switch (rinfop2->mode1 - 10) {
            case 0:
                deltay += (rinfop1->datay + 1) / 2;
                /* fallthrough */
            case 5:
                deltax += (rinfop1->datax + 1) / 2;
                /* fallthrough */
            case 1:
                break;
            case 3:
                deltay += rinfop1->datay - 1;
                /* fallthrough */
            case 2:
                deltax += rinfop1->datax - 1;
                break;
            case 6:
                deltax += rinfop1->datax - 1;
                /* fallthrough */
            case 8:
                deltay += (rinfop1->datay + 1) / 2;
                break;
            case 7:
                deltax += (rinfop1->datax + 1) / 2;
                /* fallthrough */
            case 4:
                deltay += rinfop1->datay - 1;
                break;
            default:
                return NULL;
            }

            deltax += rinfop2->datax;
            x0 += deltax;
            y0 += deltay + rinfop2->datay;
        }
        rinfop2 = rinfop1;
    }

    int16_t basex = *xout;
    if (basex == 0 && bmp != NULL)
        basex = cb->get_bmp_width(cb->ctx, bmp);

    int16_t basey = *yout;
    if (basey == 0 && bmp != NULL)
        basey = cb->get_bmp_height(cb->ctx, bmp);

    crdecode(blitrectmode, x0, y0, basex, basey,
             &blitrect->x, &blitrect->y);

    /* Source: ddat.v1e01d0 check */
    if (cb->get_v1e01d0 && cb->get_v1e01d0(cb->ctx)) {
        DM2_V1_Rect *gr1 = cb->get_glblrect1(cb->ctx);
        if (gr1)
            rc = *gr1;
    }

    /* Source: ddat.v1e01d8 && ddat.v1e025c != 0 && query_w2 == 3 */
    if (cb->get_v1e01d8 && cb->get_v1e01d8(cb->ctx) &&
        cb->get_v1e025c && cb->get_v1e025c(cb->ctx) != 0 &&
        query_w2 == 3) {
        DM2_V1_Rect *gr2 = cb->get_glblrect2(cb->ctx);
        if (gr2) {
            if (rect_unify(&rc, gr2, xout, yout) == NULL)
                return NULL;
        }
    }

    int16_t dx = rc.x - blitrect->x;
    if (dx > 0) {
        *xout = dx;
        blitrect->x = rc.x;
        int16_t a = basex - dx;
        blitrect->w = a < rc.w ? a : rc.w;
    } else {
        *xout = 0;
        int16_t a = dx + rc.w;
        blitrect->w = basex < a ? basex : a;
    }

    int16_t dy = rc.y - blitrect->y;
    if (dy > 0) {
        *yout = dy;
        blitrect->y = rc.y;
        int16_t a = basey - dy;
        blitrect->h = a < rc.h ? a : rc.h;
    } else {
        *yout = 0;
        int16_t a = dy + rc.h;
        blitrect->h = basey < a ? basey : a;
    }

    if (blitrect->w <= 0 || blitrect->h <= 0)
        return NULL;

    return blitrect;
}

/* ── DM2_QUERY_TOPLEFT_OF_RECT ──────────────────────────────────────── */

/* Source: c_xrect.cpp DM2_QUERY_TOPLEFT_OF_RECT */
DM2_V1_QueryTopleftReceipt dm2_v1_query_topleft_of_rect(
    DM2_V1_XrectState *st,
    const DM2_V1_XrectCallbacks *cb,
    int16_t wn)
{
    DM2_V1_Rect rc;
    int16_t vw_0c = 1;
    int16_t vw_08 = 1;

    dm2_v1_query_blit_rect(st, cb, NULL, &rc, wn, &vw_0c, &vw_08, -1);

    DM2_V1_QueryTopleftReceipt receipt;
    receipt.x = rc.x;
    receipt.y = rc.y;
    return receipt;
}

/* ── DM2_SCALE_RECT ─────────────────────────────────────────────────── */

/* Source: c_xrect.cpp DM2_SCALE_RECT */
DM2_V1_Rect *dm2_v1_scale_rect(DM2_V1_XrectState *st,
                                const DM2_V1_XrectCallbacks *cb,
                                int16_t query, int16_t scalew,
                                int16_t scaleh, DM2_V1_Rect *r)
{
    int16_t w, h;

    DM2_V1_Rect *qrect1 = dm2_v1_query_rect(st, query);
    if (qrect1 == NULL || qrect1->y == 0)
        return NULL;

    DM2_V1_Rect *qrect2 = dm2_v1_query_rect(st, qrect1->y);
    if (qrect2 == NULL || qrect2->x != 9)
        return NULL;

    if (scalew != DM2_V1_XRECT_SCALE)
        w = (int16_t)(scalew * qrect2->w / DM2_V1_XRECT_SCALE);
    else
        w = qrect2->w;

    if (scaleh != DM2_V1_XRECT_SCALE)
        h = (int16_t)(scaleh * qrect2->h / DM2_V1_XRECT_SCALE);
    else
        h = qrect2->h;

    if (w == 0 && scalew != 0)
        w = 1;
    if (h == 0 && scaleh != 0)
        h = 1;

    if (w <= 0 || h <= 0)
        return NULL;

    return dm2_v1_query_blit_rect(st, cb, NULL, r, query, &w, &h, -1);
}

/* ── DM2_QUERY_EXPANDED_RECT ───────────────────────────────────────── */

/* Source: c_xrect.cpp DM2_QUERY_EXPANDED_RECT */
DM2_V1_Rect *dm2_v1_query_expanded_rect(DM2_V1_XrectState *st,
                                         const DM2_V1_XrectCallbacks *cb,
                                         int16_t query, DM2_V1_Rect *r)
{
    return dm2_v1_scale_rect(st, cb, query,
                             DM2_V1_XRECT_SCALE, DM2_V1_XRECT_SCALE, r);
}

/* ── DM2_COMPRESS_RECTS ─────────────────────────────────────────────── */

/* Source: c_xrect.cpp DM2_COMPRESS_RECTS */
void dm2_v1_compress_rects(DM2_V1_XrectState *st,
                           const DM2_V1_XrectCallbacks *cb,
                           void *buffer, DM2_V1_RNode *firstnode)
{
    (void)st;
    int8_t *srcb = (int8_t *)buffer;
    int16_t *srcw = (int16_t *)buffer;

    /* Check magic 0xfc0d */
    if (*srcw != (int16_t)0xfc0d)
        return;
    srcw++;

    int16_t loop0 = *srcw++;
    int16_t *wptr = srcw;
    srcw += 2 * loop0;

    DM2_V1_RNode *np = firstnode;
    while (np->next != NULL)
        np = np->next;

    while (--loop0 >= 0) {
        int16_t min = *wptr++;
        int16_t max = *wptr++;
        int16_t loop1 = max - min + 1;

        int16_t *bupw = srcw;
        uint8_t mask = 0x1f;
        int16_t val1 = srcw[0];
        int16_t val2 = srcw[1];
        int16_t cnt = loop1;
        do {
            if (*srcw++ != val1)
                mask &= 0xfd;
            if (*srcw != val2)
                mask &= 0xfe;
            if (*srcw++ > 0xff)
                mask &= 0xfb;
            for (int16_t i = 2; i != 0; i--) {
                int16_t v = *srcw++;
                if (v < 0 || v > 0xff)
                    mask &= 0xef;
                if (v < (int16_t)0xff80 || v > 0x7f)
                    mask &= 0xf7;
            }
        } while (--cnt > 0);

        if (mask & 0x3)
            mask &= 0xfb;

        int16_t loop2 = max - min + 1;
        int32_t size = (int32_t)dm2_v1_calc_size_of_compressed_rect(mask)
                       * loop2 + (int32_t)sizeof(DM2_V1_RNode);
        if (mask & 0x1)
            size += 2;

        np->next = (DM2_V1_RNode *)cb->alloc_freepool(
            cb->ctx, size, false);
        np = np->next;
        np->next = NULL;
        np->min = min;
        np->max = max;
        np->mask = mask;
        np->b_x = (int8_t)val1;

        int8_t *dstb = (int8_t *)np + sizeof(DM2_V1_RNode);
        int16_t *dstw = (int16_t *)dstb;
        if (mask & 0x1) {
            *dstw++ = val2;
            dstb = (int8_t *)dstw;
        }

        /* Rewind source to backup position */
        srcw = bupw;
        srcb = (int8_t *)srcw;
        do {
            if (mask & 0x4) {
                *dstb++ = *srcb++;
                srcb++;
                *dstb++ = *srcb++;
                srcb++;
            } else {
                int16_t *sw = (int16_t *)srcb;
                dstw = (int16_t *)dstb;
                if (!(mask & 0x2))
                    *dstw++ = *sw;
                sw++;
                if (!(mask & 0x1))
                    *dstw++ = *sw;
                sw++;
                srcb = (int8_t *)sw;
                dstb = (int8_t *)dstw;
            }

            if (mask & 0x18) {
                *dstb++ = *srcb++;
                srcb++;
                *dstb++ = *srcb++;
                srcb++;
            } else {
                int16_t *sw = (int16_t *)srcb;
                dstw = (int16_t *)dstb;
                *dstw++ = *sw++;
                *dstw++ = *sw++;
                srcb = (int8_t *)sw;
                dstb = (int8_t *)dstw;
            }
        } while (--loop2 > 0);

        srcw = (int16_t *)srcb;
    }
}
