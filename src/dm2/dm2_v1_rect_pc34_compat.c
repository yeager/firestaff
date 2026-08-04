#include "dm2_v1_rect_pc34_compat.h"

void dm2_v1_rect_init(DM2_V1_Rect *r)
{
    r->x = r->y = r->w = r->h = 0;
}

void dm2_v1_rect_set(DM2_V1_Rect *r, int16_t x, int16_t y, int16_t w, int16_t h)
{
    r->x = x; r->y = y; r->w = w; r->h = h;
}

void dm2_v1_rect_set_origin(DM2_V1_Rect *r, int16_t w, int16_t h)
{
    r->x = 0; r->y = 0; r->w = w; r->h = h;
}

void dm2_v1_rect_inflate(DM2_V1_Rect *r, int16_t dw, int16_t dh)
{
    r->x -= dw;
    r->y -= dh;
    r->w += 2 * dw;
    r->h += 2 * dh;
}

int dm2_v1_rect_intersect(DM2_V1_Rect *r, const DM2_V1_Rect *clip,
                          int16_t *ox, int16_t *oy)
{
    int16_t dx0 = clip->x - r->x;
    if (dx0 <= 0)
        *ox = 0;
    else {
        *ox = dx0;
        r->x += dx0;
        r->w -= dx0;
    }

    int16_t dy0 = clip->y - r->y;
    if (dy0 <= 0)
        *oy = 0;
    else {
        *oy = dy0;
        r->y += dy0;
        r->h -= dy0;
    }

    int16_t dx1 = (r->x + r->w) - (clip->x + clip->w);
    if (dx1 > 0)
        r->w -= dx1;

    int16_t dy1 = (r->y + r->h) - (clip->y + clip->h);
    if (dy1 > 0)
        r->h -= dy1;

    if (r->w <= 0 || r->h <= 0)
        return 0;

    return 1;
}

void dm2_v1_rect_center_in(DM2_V1_Rect *r, const DM2_V1_Rect *container,
                           int16_t nw, int16_t nh)
{
    r->x = container->x + (container->w - nw + 1) / 2;
    r->y = container->y + (container->h - nh + 1) / 2;
    r->w = nw;
    r->h = nh;
}

int dm2_v1_rect_contains(const DM2_V1_Rect *r, int16_t px, int16_t py)
{
    return (r->x <= px) && (px <= (r->x + r->w - 1))
        && (r->y <= py) && (py <= (r->y + r->h - 1));
}

void dm2_v1_tmprects_init(DM2_V1_TempRects *t)
{
    t->index = 0;
    for (int i = 0; i < DM2_V1_NUM_TMPRECTS; i++)
        dm2_v1_rect_init(&t->rects[i]);
}

DM2_V1_Rect *dm2_v1_tmprects_alloc(DM2_V1_TempRects *t,
                                    int16_t x, int16_t y,
                                    int16_t w, int16_t h)
{
    DM2_V1_Rect *r = &t->rects[t->index];
    if (++t->index >= DM2_V1_NUM_TMPRECTS)
        t->index = 0;
    dm2_v1_rect_set(r, x, y, w, h);
    return r;
}

DM2_V1_Rect *dm2_v1_tmprects_alloc_origin(DM2_V1_TempRects *t,
                                           int16_t w, int16_t h)
{
    return dm2_v1_tmprects_alloc(t, 0, 0, w, h);
}
