#include "dm2_v1_startup_layout.h"

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
    /* The former 78x50 host panel had no SHOW_MENU_SCREEN/GDAT owner.
     * Original input is decoded through the RAW4 click matrix in
     * dm2_v1_boot_startup_menu_pointer_hit(); never expose guessed geometry
     * as a production menu surface. */
    *out_rect = (DM2_V1_StartupRect){0, 0, 0, 0};
    return 0;
}

int dm2_v1_startup_row_rect(int row, DM2_V1_StartupRect *out_rect)
{
    if (!out_rect || row < 0) {
        return 0;
    }
    (void)row;
    *out_rect = (DM2_V1_StartupRect){0, 0, 0, 0};
    return 0;
}

int dm2_v1_startup_row_highlight_rect(int row,
                                      DM2_V1_StartupRect *out_rect)
{
    if (!out_rect || row < 0) {
        return 0;
    }
    (void)row;
    *out_rect = (DM2_V1_StartupRect){0, 0, 0, 0};
    return 0;
}

int dm2_v1_startup_hit(int row_count,
                       int x,
                       int y,
                       DM2_V1_StartupHit *out_hit)
{
    dm2_v1_startup_hit_clear(out_hit);
    (void)row_count;
    (void)x;
    (void)y;
    return 0;
}
