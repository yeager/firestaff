#ifndef NEXUS_V1_TITLE_H
#define NEXUS_V1_TITLE_H

#include "nexus_v1_engine.h"
#include "nexus_v1_rasterizer.h"
#include <stdint.h>

typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int loaded;
} Nexus_TitleScreen;

int nexus_title_load(Nexus_TitleScreen *title, Nexus_V1_Engine *engine);
void nexus_title_free(Nexus_TitleScreen *title);
int nexus_title_min_boot_frames(void);
int nexus_title_start_ready_frames(void);
int nexus_title_boot_reveal_complete(int frame);
int nexus_title_start_ready(int frame);
void nexus_render_title(const Nexus_TitleScreen *title,
                        Nexus_Framebuffer *fb,
                        int frame);
void nexus_render_title_fallback(Nexus_Framebuffer *fb, int frame);

#endif
