#include "dm2_v1_startup_layout.h"

static int dm2_v1_startup_rect_contains(const DM2_V1_StartupRect *rect,
                                        int x,
                                        int y)
{
    if (!rect || rect->w <= 0 || rect->h <= 0) {
        return 0;
    }
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

static void dm2_v1_startup_hit_clear(DM2_V1_StartupHit *hit)
{
    if (!hit) {
        return;
    }
    hit->kind = DM2_V1_STARTUP_HIT_NONE;
    hit->row = -1;
}

int dm2_v1_startup_panel_rect(DM2_V1_StartupRect *out_rect)
{
    if (!out_rect) {
        return 0;
    }
    out_rect->x = 78;
    out_rect->y = 50;
    out_rect->w = 164;
    out_rect->h = 122;
    return 1;
}

int dm2_v1_startup_row_rect(int row, DM2_V1_StartupRect *out_rect)
{
    if (!out_rect || row < 0) {
        return 0;
    }
    out_rect->x = 92;
    out_rect->y = 76 + row * 14;
    out_rect->w = 136;
    out_rect->h = 12;
    return 1;
}

int dm2_v1_startup_row_highlight_rect(int row,
                                      DM2_V1_StartupRect *out_rect)
{
    if (!out_rect || row < 0) {
        return 0;
    }
    out_rect->x = 90;
    out_rect->y = 76 + row * 14;
    out_rect->w = 140;
    out_rect->h = 12;
    return 1;
}

int dm2_v1_startup_hit(int row_count,
                       int x,
                       int y,
                       DM2_V1_StartupHit *out_hit)
{
    DM2_V1_StartupRect rect;
    int row;

    dm2_v1_startup_hit_clear(out_hit);
    if (row_count < 0) {
        return 0;
    }
    for (row = 0; row < row_count; ++row) {
        if (!dm2_v1_startup_row_rect(row, &rect)) {
            continue;
        }
        if (dm2_v1_startup_rect_contains(&rect, x, y)) {
            if (out_hit) {
                out_hit->kind = DM2_V1_STARTUP_HIT_ROW;
                out_hit->row = row;
            }
            return 1;
        }
    }
    if (dm2_v1_startup_panel_rect(&rect) &&
        dm2_v1_startup_rect_contains(&rect, x, y)) {
        if (out_hit) {
            out_hit->kind = DM2_V1_STARTUP_HIT_PANEL;
        }
        return 1;
    }
    return 0;
}
