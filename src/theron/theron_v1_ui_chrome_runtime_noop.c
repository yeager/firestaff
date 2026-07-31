/*
 * Production seam for Theron V1 UI chrome.
 *
 * The former implementation contained inferred bars, labels and slot
 * geometry.  No original Track 02 chrome bank has been decoded, so the
 * production API must preserve the framebuffer and report no source pixels.
 * The former implementation remains available to focused fixture targets.
 */

#include "theron_v1_ui_chrome.h"

void tr_ui_draw_bar(TQR_PlanarFramebuffer *fb, int x, int y, int w, int h,
                    int current, int max, uint8_t pal_index, uint8_t bg_index)
{
    (void)fb; (void)x; (void)y; (void)w; (void)h;
    (void)current; (void)max; (void)pal_index; (void)bg_index;
}

void tr_ui_draw_champion_slot(TQR_PlanarFramebuffer *fb, int slot_idx,
                              int x, int y,
                              const Theron_V1_Champion *champion)
{
    (void)fb; (void)slot_idx; (void)x; (void)y; (void)champion;
}

void tr_ui_render_topbar(TQR_PlanarFramebuffer *fb,
                         const Theron_V1_World *world, int y_offset)
{
    (void)fb; (void)world; (void)y_offset;
}

void tr_ui_render_right_panel(TQR_PlanarFramebuffer *fb,
                              const Theron_V1_World *world, int x_offset)
{
    (void)fb; (void)world; (void)x_offset;
}

void tr_ui_render(TQR_PlanarFramebuffer *fb, const Theron_V1_World *world,
                  uint32_t ui_flags)
{
    (void)fb; (void)world; (void)ui_flags;
}

const char *tr_ui_source_evidence(void)
{
    return "Track 02 original UI chrome bank not decoded; production UI route blocked";
}
