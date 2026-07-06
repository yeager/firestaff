#include "nexus_v1_startup_layout.h"

static int nexus_v1_rect_contains(const Nexus_V1_StartupRect *rect,
                                  int x,
                                  int y)
{
    if (!rect || rect->w <= 0 || rect->h <= 0) {
        return 0;
    }
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

static void nexus_v1_startup_hit_clear(Nexus_V1_StartupHit *hit)
{
    if (!hit) {
        return;
    }
    hit->kind = NEXUS_V1_STARTUP_HIT_NONE;
    hit->row = -1;
}

int nexus_v1_startup_save_row_rect(int row, Nexus_V1_StartupRect *out_rect)
{
    if (!out_rect || row < 0) {
        return 0;
    }
    out_rect->x = 18;
    out_rect->y = 42 + row * 13;
    out_rect->w = 284;
    out_rect->h = 12;
    return 1;
}

int nexus_v1_startup_save_panel_rect(int row_count,
                                     Nexus_V1_StartupRect *out_rect)
{
    if (!out_rect || row_count < 0) {
        return 0;
    }
    out_rect->x = 18;
    out_rect->y = 14;
    out_rect->w = 284;
    out_rect->h = 42 + row_count * 13;
    return 1;
}

int nexus_v1_startup_save_hit(int row_count,
                              int x,
                              int y,
                              Nexus_V1_StartupHit *out_hit)
{
    Nexus_V1_StartupRect rect;
    int row;

    nexus_v1_startup_hit_clear(out_hit);
    if (row_count < 0) {
        return 0;
    }
    for (row = 0; row < row_count; ++row) {
        if (!nexus_v1_startup_save_row_rect(row, &rect)) {
            continue;
        }
        if (nexus_v1_rect_contains(&rect, x, y)) {
            if (out_hit) {
                out_hit->kind = NEXUS_V1_STARTUP_HIT_SAVE_ROW;
                out_hit->row = row;
            }
            return 1;
        }
    }
    if (nexus_v1_startup_save_panel_rect(row_count, &rect) &&
        nexus_v1_rect_contains(&rect, x, y)) {
        if (out_hit) {
            out_hit->kind = NEXUS_V1_STARTUP_HIT_SAVE_PANEL;
        }
        return 1;
    }
    return 0;
}

int nexus_v1_startup_champion_row_rect(int row,
                                       Nexus_V1_StartupRect *out_rect)
{
    if (!out_rect || row < 0) {
        return 0;
    }
    out_rect->x = 18;
    out_rect->y = 37 + row * 11;
    out_rect->w = 284;
    out_rect->h = 11;
    return 1;
}

int nexus_v1_startup_champion_footer_rect(Nexus_V1_StartupRect *out_rect)
{
    if (!out_rect) {
        return 0;
    }
    out_rect->x = 18;
    out_rect->y = 180;
    out_rect->w = 284;
    out_rect->h = 18;
    return 1;
}

int nexus_v1_startup_champion_panel_rect(Nexus_V1_StartupRect *out_rect)
{
    if (!out_rect) {
        return 0;
    }
    out_rect->x = 18;
    out_rect->y = 20;
    out_rect->w = 284;
    out_rect->h = 178;
    return 1;
}

int nexus_v1_startup_champion_hit(int champion_count,
                                  int x,
                                  int y,
                                  Nexus_V1_StartupHit *out_hit)
{
    Nexus_V1_StartupRect rect;
    int row;
    int visible_count = champion_count < 12 ? champion_count : 12;

    nexus_v1_startup_hit_clear(out_hit);
    if (champion_count < 0) {
        return 0;
    }
    if (nexus_v1_startup_champion_footer_rect(&rect) &&
        nexus_v1_rect_contains(&rect, x, y)) {
        if (out_hit) {
            out_hit->kind = NEXUS_V1_STARTUP_HIT_CHAMPION_FOOTER;
        }
        return 1;
    }
    for (row = 0; row < visible_count; ++row) {
        if (!nexus_v1_startup_champion_row_rect(row, &rect)) {
            continue;
        }
        if (nexus_v1_rect_contains(&rect, x, y)) {
            if (out_hit) {
                out_hit->kind = NEXUS_V1_STARTUP_HIT_CHAMPION_ROW;
                out_hit->row = row;
            }
            return 1;
        }
    }
    if (nexus_v1_startup_champion_panel_rect(&rect) &&
        nexus_v1_rect_contains(&rect, x, y)) {
        if (out_hit) {
            out_hit->kind = NEXUS_V1_STARTUP_HIT_CHAMPION_PANEL;
        }
        return 1;
    }
    return 0;
}
