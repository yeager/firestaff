#include "nexus_v1_title.h"
#include "nexus_v1_title_sequence.h"
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

static int nexus_title_copy_surface(uint8_t **out_pixels,
                                    int *out_w,
                                    int *out_h,
                                    const Nexus_UI_Surface *surface)
{
    size_t pixels;
    if (!out_pixels || !out_w || !out_h || !surface ||
        !surface->data || surface->w <= 0 || surface->h <= 0) {
        return -1;
    }
    pixels = (size_t)surface->w * (size_t)surface->h;
    *out_pixels = (uint8_t *)malloc(pixels);
    if (!*out_pixels) {
        return -1;
    }
    memcpy(*out_pixels, surface->data, pixels);
    *out_w = surface->w;
    *out_h = surface->h;
    return 0;
}

static void nexus_title_load_warning_if_available(Nexus_TitleScreen *title,
                                                  Nexus_V1_Engine *engine)
{
    uint8_t *data;
    int size = 0;
    Nexus_UI_Manager mgr;
    const Nexus_UI_Surface *surface;

    if (!title || !engine) {
        return;
    }
    data = nexus_v1_read_file(engine, "WARNING.BIN", &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }
    nexus_ui_manager_init(&mgr);
    if (nexus_ui_load_warning(&mgr, data, size, NULL) == 0 ||
        mgr.surfaces[NEXUS_SURFACE_WARNING].data) {
        surface = &mgr.surfaces[NEXUS_SURFACE_WARNING];
        if (nexus_title_copy_surface(&title->warning_pixels,
                                     &title->warning_width,
                                     &title->warning_height,
                                     surface) == 0) {
            title->warning_loaded = 1;
        }
    }
    nexus_ui_manager_free(&mgr);
    free(data);
}

int nexus_title_load(Nexus_TitleScreen *title, Nexus_V1_Engine *engine) {
    uint8_t *data;
    int size = 0;
    Nexus_UI_Manager mgr;
    const Nexus_UI_Surface *surface;

    if (!title || !engine) {
        return -1;
    }
    memset(title, 0, sizeof(*title));
    nexus_title_load_warning_if_available(title, engine);
    data = nexus_v1_read_file(engine, "TITLE.CG", &size);
    if (!data || size <= 0) {
        nexus_title_free(title);
        free(data);
        return -1;
    }

    nexus_ui_manager_init(&mgr);
    if (nexus_ui_load_title(&mgr, data, size, NULL) < 0) {
        nexus_ui_manager_free(&mgr);
        free(data);
        return -1;
    }
    surface = &mgr.surfaces[NEXUS_SURFACE_TITLE];
    if (!surface->data || surface->w <= 0 || surface->h <= 0) {
        nexus_ui_manager_free(&mgr);
        nexus_title_free(title);
        free(data);
        return -1;
    }

    if (nexus_title_copy_surface(&title->pixels,
                                 &title->width,
                                 &title->height,
                                 surface) != 0) {
        nexus_ui_manager_free(&mgr);
        nexus_title_free(title);
        free(data);
        return -1;
    }
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
    free(title->warning_pixels);
    memset(title, 0, sizeof(*title));
}

int nexus_title_min_boot_frames(void) {
    return nexus_v1_title_min_boot_frames();
}

int nexus_title_start_ready_frames(void) {
    return nexus_v1_title_start_ready_frames();
}

int nexus_title_boot_warning_frames(void) {
    return nexus_v1_boot_warning_frames();
}

int nexus_title_boot_start_ready_frames(void) {
    return nexus_v1_boot_start_ready_frames();
}

int nexus_title_boot_reveal_complete(int frame) {
    Nexus_V1_TitleFrame title_frame;
    if (!nexus_v1_title_frame(frame, NEXUS_FB_H, &title_frame)) {
        return 0;
    }
    return title_frame.boot_reveal_complete;
}

int nexus_title_full_boot_start_ready(int frame) {
    Nexus_V1_BootFrame boot_frame;
    if (!nexus_v1_boot_frame(frame, NEXUS_FB_H, &boot_frame)) {
        return 0;
    }
    return boot_frame.start_ready;
}

int nexus_title_start_ready(int frame) {
    Nexus_V1_TitleFrame title_frame;
    if (!nexus_v1_title_frame(frame, NEXUS_FB_H, &title_frame)) {
        return 0;
    }
    return title_frame.start_ready;
}

static void nexus_title_draw_prompt(Nexus_Framebuffer *fb,
                                    const Nexus_V1_TitleFrame *title_frame)
{
    int x;
    int y;
    uint8_t base;
    if (!fb || !title_frame || !title_frame->prompt_visible) {
        return;
    }
    base = (uint8_t)(18 + ((title_frame->hold_frame / 6) & 3));
    y = NEXUS_FB_H - 18;
    for (x = 86; x < 234; x += 18) {
        nexus_title_draw_rect(fb, x, y, 12, 2, base);
        nexus_title_draw_rect(fb, x, y + 4, 10, 2, (uint8_t)(base - 2));
        nexus_title_draw_rect(fb, x, y + 8, 12, 2, base);
    }
    nexus_title_draw_rect(fb, 74, y - 6, 172, 1, (uint8_t)(base - 4));
    nexus_title_draw_rect(fb, 74, y + 14, 172, 1, (uint8_t)(base - 4));
}

static void nexus_render_title_art(const Nexus_TitleScreen *title,
                                   Nexus_Framebuffer *fb, int title_frame_no) {
    int y;
    Nexus_V1_TitleFrame title_frame;
    if (!fb) {
        return;
    }
    nexus_fb_clear(fb);
    if (!title || !title->loaded || !title->pixels ||
        title->width <= 0 || title->height <= 0) {
        nexus_render_title_fallback(fb, title_frame_no);
        return;
    }
    /* Nexus boot presentation: keep the real TITLE.CG artwork, but make the
     * startup title phase frame-dependent instead of a static blit. This is a
     * bounded Saturn-style reveal until the original VDP1/VDP2 title program
     * is decoded from NEXUS.BIN. */
    if (title_frame_no < 0) {
        title_frame_no = 0;
    }
    if (!nexus_v1_title_frame(title_frame_no, NEXUS_FB_H, &title_frame)) {
        return;
    }
    for (y = title_frame.reveal_y0;
         y < title_frame.reveal_y1 && y < title->height &&
                 y < NEXUS_FB_H; ++y) {
        int copy_w = title->width < NEXUS_FB_W ? title->width : NEXUS_FB_W;
        memcpy(&fb->color_buffer[y * NEXUS_FB_W],
               &title->pixels[y * title->width],
               (size_t)copy_w);
    }
    nexus_title_draw_rect(fb, 0, title_frame.reveal_y0 - 1,
                          NEXUS_FB_W, 1, title_frame.edge_color);
    nexus_title_draw_rect(fb, 0, title_frame.reveal_y1,
                          NEXUS_FB_W, 1, title_frame.edge_color);
    nexus_title_draw_prompt(fb, &title_frame);
}

static void nexus_render_title_warning_art(const Nexus_TitleScreen *title,
                                           Nexus_Framebuffer *fb,
                                           int frame)
{
    int y;
    int copy_w;
    if (!fb) {
        return;
    }
    nexus_fb_clear(fb);
    if (!title || !title->warning_loaded || !title->warning_pixels ||
        title->warning_width <= 0 || title->warning_height <= 0) {
        nexus_render_title_warning_fallback(fb, frame);
        return;
    }
    copy_w = title->warning_width < NEXUS_FB_W ? title->warning_width : NEXUS_FB_W;
    for (y = 0; y < title->warning_height && y < NEXUS_FB_H; ++y) {
        memcpy(&fb->color_buffer[y * NEXUS_FB_W],
               &title->warning_pixels[y * title->warning_width],
               (size_t)copy_w);
    }
}

void nexus_render_title(const Nexus_TitleScreen *title,
                        Nexus_Framebuffer *fb, int frame) {
    Nexus_V1_BootFrame boot_frame;
    if (!fb) {
        return;
    }
    if (!nexus_v1_boot_frame(frame, NEXUS_FB_H, &boot_frame)) {
        nexus_fb_clear(fb);
        return;
    }
    if (boot_frame.phase == NEXUS_V1_BOOT_PHASE_WARNING) {
        nexus_render_title_warning_art(title, fb, boot_frame.frame_in_phase);
        return;
    }
    nexus_render_title_art(title, fb, boot_frame.title_frame);
}

void nexus_render_title_fallback(Nexus_Framebuffer *fb, int frame) {
    int y;
    int pulse;
    Nexus_V1_TitleFrame title_frame;
    if (!fb) {
        return;
    }
    if (frame < 0) {
        frame = 0;
    }
    if (!nexus_v1_title_frame(frame, NEXUS_FB_H, &title_frame)) {
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
    nexus_title_draw_prompt(fb, &title_frame);
}

void nexus_render_title_warning_fallback(Nexus_Framebuffer *fb, int frame) {
    int y;
    int pulse;
    if (!fb) {
        return;
    }
    if (frame < 0) {
        frame = 0;
    }
    nexus_fb_clear(fb);
    pulse = (frame / 10) & 3;
    for (y = 0; y < NEXUS_FB_H; ++y) {
        uint8_t shade = (uint8_t)(1 + ((y / 40) & 3));
        memset(&fb->color_buffer[y * NEXUS_FB_W], shade, NEXUS_FB_W);
    }
    nexus_title_draw_rect(fb, 36, 34, 248, 24, (uint8_t)(32 + pulse));
    nexus_title_draw_rect(fb, 54, 74, 212, 4, 38);
    nexus_title_draw_rect(fb, 54, 92, 212, 4, 36);
    nexus_title_draw_rect(fb, 54, 110, 212, 4, 34);
    nexus_title_draw_rect(fb, 68, 150, 184, 3, 40);
}
