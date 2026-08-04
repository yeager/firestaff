/*
 * dm2_v1_buttons_pc34_compat.c — DM2 button group management.
 *
 * Source: skproject c_buttons.cpp
 * Reference: c_buttons.h (c_button, c_buttongroup)
 *
 * Functions:
 *   button_init            — zero a single button (dbidx = -1)
 *   buttongroup_init       — zero a button group (button + 5 rects)
 *   init_global_buttongroups — init two global button groups
 *   offset_rect            — compute rect offset relative to button origin
 *   adjust_buttongroup_rects — containment/insertion/clipping of sub-rects
 */

#include "dm2_v1_buttons_pc34_compat.h"
#include <string.h>

/* skproject NODATA = -1 */
#define NODATA (-1)

void dm2_v1_button_init(DM2_V1_Button *b)
{
    if (!b) return;
    b->dbidx = NODATA;
    b->r.x = 0; b->r.y = 0; b->r.w = 0; b->r.h = 0;
    b->groupsize = 0;
}

void dm2_v1_buttongroup_init(DM2_V1_ButtonGroup *bg)
{
    if (!bg) return;
    dm2_v1_button_init(&bg->button);
    for (int i = 0; i < 5; i++) {
        bg->rects[i].x = 0;
        bg->rects[i].y = 0;
        bg->rects[i].w = 0;
        bg->rects[i].h = 0;
    }
}

void dm2_v1_init_global_buttongroups(DM2_V1_ButtonGroup *bg1,
                                      DM2_V1_ButtonGroup *bg2)
{
    dm2_v1_buttongroup_init(bg1);
    dm2_v1_buttongroup_init(bg2);
}

DM2_V1_OffsetRectReceipt dm2_v1_offset_rect(
    const DM2_V1_ButtonGroup *bg,
    const DM2_V1_ButtonRect *src)
{
    DM2_V1_OffsetRectReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));
    if (!bg || !src) return receipt;

    receipt.result.x = src->x - bg->button.r.x;
    receipt.result.y = src->y - bg->button.r.y;
    receipt.result.w = src->w;
    receipt.result.h = src->h;
    return receipt;
}

DM2_V1_AdjustButtonGroupReceipt dm2_v1_adjust_buttongroup_rects(
    DM2_V1_ButtonGroup *bg,
    const DM2_V1_ButtonRect *rect)
{
    DM2_V1_AdjustButtonGroupReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    if (!bg || !rect) return receipt;

    int16_t cnt = 0;
    bool found = false;

    for (;;) {
        if (cnt < bg->button.groupsize) {
            DM2_V1_ButtonRect *r = &bg->rects[cnt];

            /* Check if existing rect fully contains input */
            if (r->x <= rect->x
                && (r->x + r->w - 1) >= (rect->x + rect->w - 1)
                && r->y <= rect->y
                && (r->y + r->h - 1) >= (rect->y + rect->h - 1))
            {
                receipt.already_contained = true;
                return receipt;
            }

            /* Check if input fully contains existing rect */
            if (r->x >= rect->x
                && (r->x + r->w - 1) <= (rect->x + rect->w - 1)
                && r->y >= rect->y
                && (r->y + r->h - 1) <= (rect->y + rect->h - 1))
            {
                found = true;
                break;
            }
        } else {
            if (cnt >= 5) {
                receipt.overflow = true;
                return receipt;
            }
            cnt = bg->button.groupsize++;
            found = true;
            break;
        }
        cnt++;
    }

    if (!found) return receipt;

    DM2_V1_ButtonRect *r = &bg->rects[cnt];
    *r = *rect;

    /* Clip left/top */
    int16_t deltax = r->x - bg->button.r.x;
    if (deltax < 0) {
        r->w += deltax;
        if (r->w <= 0) {
            bg->button.groupsize--;
            receipt.removed = true;
            return receipt;
        }
        r->x -= deltax;
    }

    int16_t deltay = r->y - bg->button.r.y;
    if (deltay < 0) {
        r->h += deltay;
        if (r->h <= 0) {
            bg->button.groupsize--;
            receipt.removed = true;
            return receipt;
        }
        r->y -= deltay;
    }

    /* Clip right/bottom */
    deltax = (bg->button.r.x + bg->button.r.w - 1) - (r->x + r->w - 1);
    if (deltax < 0)
        r->w += deltax;

    deltay = (bg->button.r.y + bg->button.r.h - 1) - (r->y + r->h - 1);
    if (deltay < 0)
        r->h += deltay;

    receipt.adjusted = true;
    receipt.slot_used = cnt;
    receipt.clipped = (r->x != rect->x || r->y != rect->y
                       || r->w != rect->w || r->h != rect->h);
    return receipt;
}
