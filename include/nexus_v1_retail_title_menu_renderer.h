#ifndef NEXUS_V1_RETAIL_TITLE_MENU_RENDERER_H
#define NEXUS_V1_RETAIL_TITLE_MENU_RENDERER_H

#include "nexus_v1_retail_title_menu.h"
#include "nexus_v1_rasterizer.h"
#include "nexus_v1_title.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Draw only the Japanese and English VDP1 sprite layouts authenticated
 * against retail TITLE.BIN, captured CRAM, placement and screenshot pixels.
 * This deliberately does not draw the unbound NBG0/background. Returns 1
 * only when a retained layout was consumed. */
int nexus_v1_retail_title_menu_render(const Nexus_TitleScreen *title,
                                     const Nexus_V1_RetailTitleMenuState *state,
                                     Nexus_Framebuffer *framebuffer,
                                     int animation_frame);

#ifdef __cplusplus
}
#endif

#endif
