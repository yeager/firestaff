#include "nexus_v1_title.h"
#include "nexus_v1_ui_surfaces.h"

#include <stdlib.h>
#include <string.h>

static void nexus_title_draw_rect(Nexus_Framebuffer *fb,
                                  int x, int y, int w, int h,
                                  uint8_t color) {
    int yy;
    if (!fb || w <= 0 || h <= 0) {
        return;
    }
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > NEXUS_FB_W) {
        w = NEXUS_FB_W - x;
    }
    if (y + h > NEXUS_FB_H) {
        h = NEXUS_FB_H - y;
    }
    if (w <= 0 || h <= 0) {
        return;
    }
    for (yy = y; yy < y + h; ++yy) {
        memset(&fb->color_buffer[yy * NEXUS_FB_W + x], color, (size_t)w);
    }
}

int nexus_title_load(Nexus_TitleScreen *title, Nexus_V1_Engine *engine) {
    uint8_t *data;
    int size = 0;
    Nexus_UI_Manager mgr;
    const Nexus_UI_Surface *surface;
    size_t pixels;

    if (!title || !engine) {
        return -1;
    }
    memset(title, 0, sizeof(*title));
    data = nexus_v1_read_file(engine, "TITLE.CG", &size);
    if (!data || size <= 0) {
        free(data);
        return -1;
    }

    nexus_ui_manager_init(&mgr);
    if (nexus_ui_load_title(&mgr, data, size, NULL) != 0) {
        nexus_ui_manager_free(&mgr);
        free(data);
        return -1;
    }
    surface = &mgr.surfaces[NEXUS_SURFACE_TITLE];
    if (!surface->data || surface->w <= 0 || surface->h <= 0) {
        nexus_ui_manager_free(&mgr);
        free(data);
        return -1;
    }

    pixels = (size_t)surface->w * (size_t)surface->h;
    title->pixels = (uint8_t *)malloc(pixels);
    if (!title->pixels) {
        nexus_ui_manager_free(&mgr);
        free(data);
        return -1;
    }
    memcpy(title->pixels, surface->data, pixels);
    title->width = surface->w;
    title->height = surface->h;
    title->loaded = 1;

    nexus_ui_manager_free(&mgr);
    free(data);
    return 0;
}

void nexus_title_free(Nexus_TitleScreen *title) {
    if (!title) {
        return;
    }
    free(title->pixels);
    memset(title, 0, sizeof(*title));
}

void nexus_render_title(const Nexus_TitleScreen *title,
                        Nexus_Framebuffer *fb, int frame) {
    int y;
    (void)frame;
    if (!fb) {
        return;
    }
    nexus_fb_clear(fb);
    if (!title || !title->loaded || !title->pixels ||
        title->width <= 0 || title->height <= 0) {
        nexus_render_title_fallback(fb, frame);
        return;
    }
    for (y = 0; y < title->height && y < NEXUS_FB_H; ++y) {
        int copy_w = title->width < NEXUS_FB_W ? title->width : NEXUS_FB_W;
        memcpy(&fb->color_buffer[y * NEXUS_FB_W],
               &title->pixels[y * title->width],
               (size_t)copy_w);
    }
}

void nexus_render_title_fallback(Nexus_Framebuffer *fb, int frame) {
    int y;
    int pulse;
    if (!fb) {
        return;
    }
    nexus_fb_clear(fb);
    pulse = (frame / 8) & 3;
    for (y = 0; y < NEXUS_FB_H; ++y) {
        uint8_t shade = (uint8_t)(2 + (y / 28));
        memset(&fb->color_buffer[y * NEXUS_FB_W], shade, NEXUS_FB_W);
    }
    nexus_title_draw_rect(fb, 54, 28, 212, 18, (uint8_t)(10 + pulse));
    nexus_title_draw_rect(fb, 70, 56, 180, 8, 14);
    nexus_title_draw_rect(fb, 90, 74, 140, 6, 12);
    nexus_title_draw_rect(fb, 32, 104, 256, 3, 18);
    nexus_title_draw_rect(fb, 48, 124, 224, 3, 16);
    nexus_title_draw_rect(fb, 68, 144, 184, 3, 14);
    nexus_title_draw_rect(fb, 96, 164, 128, 3, 12);
    nexus_title_draw_rect(fb, 20 + pulse * 2, 190, 280 - pulse * 4, 2, 22);
}
