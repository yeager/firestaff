#include "nexus_v1_title.h"
#include "nexus_v1_title_sequence.h"
#include "nexus_v1_ui_surfaces.h"

#include <stdlib.h>
#include <string.h>

static uint16_t nexus_title_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

/* DMWeb, Dungeon Master Nexus Data File Decoder: MAPD contains five
 * 64x28 tilemaps; TITLE.CG supplies the 5249 contiguous 8x8 4bpp tiles. */
int nexus_v1_title_decode_mapd(const uint8_t *mapd,
                               size_t mapd_size,
                               const uint8_t *title_cg,
                               size_t title_cg_bytes,
                               Nexus_TitleScreen *title)
{
    int map;
    if (!mapd || mapd_size < 0x8c70U || !title_cg || !title ||
        title_cg_bytes < (size_t)5249U * 32U ||
        memcmp(mapd, "MAPD", 4) != 0 ||
        memcmp(mapd + 8U, "TIBG", 4) != 0) {
        return 0;
    }
    for (map = 0; map < NEXUS_V1_TITLE_MAP_COUNT; ++map) {
        size_t map_offset = 0x40U + (size_t)map * 0x1c04U;
        uint8_t *out = (uint8_t *)calloc(
            (size_t)NEXUS_V1_TITLE_MAP_WIDTH * NEXUS_V1_TITLE_MAP_HEIGHT,
            1U);
        int cell;
        if (!out || map_offset + 4U + 0x1c00U > mapd_size) {
            free(out);
            return 0;
        }
        if (nexus_title_be16(mapd + map_offset) != 0x0040U ||
            nexus_title_be16(mapd + map_offset + 2U) != 0x001cU) {
            free(out);
            return 0;
        }
        for (cell = 0; cell < 64 * 28; ++cell) {
            size_t cell_offset = map_offset + 4U + (size_t)cell * 4U;
            int tile_index = (int)(nexus_title_be16(mapd + cell_offset + 2U) &
                                   0x7fffU) - 4608;
            int x = (cell % 64) * 8;
            int y = (cell / 64) * 8;
            int py;
            if (tile_index < 0 || tile_index >= 5249 ||
                (size_t)tile_index * 32U + 32U > title_cg_bytes) {
                free(out);
                return 0;
            }
            for (py = 0; py < 8; ++py) {
                int px;
                for (px = 0; px < 4; ++px) {
                    uint8_t packed = title_cg[(size_t)tile_index * 32U +
                                              (size_t)py * 4U + px];
                    out[(size_t)(y + py) * NEXUS_V1_TITLE_MAP_WIDTH + x +
                        px * 2] = (uint8_t)(packed >> 4);
                    out[(size_t)(y + py) * NEXUS_V1_TITLE_MAP_WIDTH + x +
                        px * 2 + 1] = (uint8_t)(packed & 0x0fU);
                }
            }
        }
        title->decoded_map_pixels[map] = out;
    }
    for (map = 0; map < 16; ++map) {
        title->decoded_map_palette[map] = nexus_title_be16(
            mapd + 0x8c54U + (size_t)map * 2U);
    }
    title->decoded_map_count = NEXUS_V1_TITLE_MAP_COUNT;
    title->decoded_map_source_bound = 1;
    return 1;
}

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

static void nexus_title_copy_warning_palette(Nexus_TitleScreen *title,
                                             const Nexus_UI_Surface *surface)
{
    if (!title || !surface || !surface->dgt2_palette_loaded) {
        return;
    }
    memcpy(title->warning_palette_rgba, surface->dgt2_palette_rgba,
           sizeof(title->warning_palette_rgba));
    title->warning_palette_loaded = 1;
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
            nexus_title_copy_warning_palette(title, surface);
        }
    }
    nexus_ui_manager_free(&mgr);
    free(data);
}

static void nexus_title_copy_cached_warning_if_available(
    Nexus_TitleScreen *title,
    Nexus_V1_Engine *engine)
{
    const Nexus_UI_Surface *surface;

    if (!title || !engine) {
        return;
    }
    surface = &engine->ui.surfaces[NEXUS_SURFACE_WARNING];
    if (surface->data && surface->w > 0 && surface->h > 0 &&
        nexus_title_copy_surface(&title->warning_pixels,
                                 &title->warning_width,
                                 &title->warning_height,
                                 surface) == 0) {
        title->warning_loaded = 1;
        nexus_title_copy_warning_palette(title, surface);
    }
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
    nexus_title_copy_cached_warning_if_available(title, engine);
    if (!title->warning_loaded) {
        nexus_title_load_warning_if_available(title, engine);
    }

    surface = &engine->ui.surfaces[NEXUS_SURFACE_TITLE];
    if (surface->data && surface->w > 0 && surface->h > 0 &&
        nexus_title_copy_surface(&title->pixels,
                                 &title->width,
                                 &title->height,
                                 surface) == 0) {
        title->loaded = 1;
        data = nexus_v1_read_file(engine, "TITLE.BIN", &size);
        if (data && size > 0) {
            int cg_size = 0;
            uint8_t *cg = nexus_v1_read_file(engine, "TITLE.CG", &cg_size);
            (void)nexus_v1_title_decode_mapd(
                                          data + 0x0e278U,
                                          (size_t)size - 0x0e278U, cg,
                                          cg_size > 0 ? (size_t)cg_size : 0U,
                                          title);
            free(cg);
            free(data);
        }
        return 0;
    }

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

    {
        uint8_t *mapd = nexus_v1_read_file(engine, "TITLE.BIN", &size);
        (void)nexus_v1_title_decode_mapd(
                                      mapd && size > (int)0x0e278U
                                          ? mapd + 0x0e278U : NULL,
                                      size > (int)0x0e278U
                                          ? (size_t)size - 0x0e278U : 0U,
                                      data, (size_t)size,
                                      title);
        free(mapd);
    }

    nexus_ui_manager_free(&mgr);
    free(data);
    return 0;
}

void nexus_title_free(Nexus_TitleScreen *title) {
    int map;
    if (!title) {
        return;
    }
    for (map = 0; map < NEXUS_V1_TITLE_MAP_COUNT; ++map) {
        free(title->decoded_map_pixels[map]);
        title->decoded_map_pixels[map] = NULL;
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

static void nexus_title_plan_reset(Nexus_V1_TitleRenderPlan *plan)
{
    if (plan) {
        memset(plan, 0, sizeof(*plan));
    }
}

static int nexus_title_screen_surface_ready(const Nexus_TitleScreen *title)
{
    /* DMWeb documents TITLE.CG plus TITLE.BIN MAPD tilemaps, not a ready
     * screen. The current 328x1024 atlas has no verified host presentation
     * binding to the 320x200 Firestaff framebuffer. */
    return title && title->loaded && title->pixels &&
           title->width == NEXUS_FB_W && title->height == NEXUS_FB_H;
}

int nexus_v1_title_build_render_plan(const Nexus_TitleScreen *title,
                                     int frame,
                                     Nexus_V1_TitleRenderPlan *out_plan)
{
    Nexus_V1_BootFrame boot_frame;
    if (!out_plan) {
        return 0;
    }
    if (!nexus_v1_boot_frame(frame, NEXUS_FB_H, &boot_frame)) {
        nexus_title_plan_reset(out_plan);
        return 0;
    }
    if (boot_frame.phase == NEXUS_V1_BOOT_PHASE_WARNING) {
        if (!title || !title->warning_loaded || !title->warning_pixels ||
            title->warning_width <= 0 || title->warning_height <= 0) {
            /* This direct renderer may be used outside the launcher gate.
             * Preserve real title art rather than drawing a fake warning. */
            if (!nexus_title_screen_surface_ready(title)) {
                nexus_title_plan_reset(out_plan);
                return 0;
            }
            nexus_title_plan_reset(out_plan);
            if (!nexus_v1_title_frame(boot_frame.frame_in_phase,
                                      NEXUS_FB_H,
                                      &boot_frame.title)) {
                return 0;
            }
            out_plan->kind = NEXUS_V1_TITLE_RENDER_PLAN_TITLE_ART;
            out_plan->boot_phase = NEXUS_V1_BOOT_PHASE_TITLE;
            out_plan->input_frame = frame;
            out_plan->title_frame = boot_frame.frame_in_phase;
            out_plan->reveal_y0 = boot_frame.title.reveal_y0;
            out_plan->reveal_y1 = boot_frame.title.reveal_y1;
            out_plan->edge_color = boot_frame.title.edge_color;
            out_plan->copy_width = title->width < NEXUS_FB_W
                                       ? title->width : NEXUS_FB_W;
            out_plan->copy_height = title->height < NEXUS_FB_H
                                        ? title->height : NEXUS_FB_H;
            return 1;
        }
        nexus_title_plan_reset(out_plan);
        out_plan->kind = NEXUS_V1_TITLE_RENDER_PLAN_WARNING_ART;
        out_plan->boot_phase = NEXUS_V1_BOOT_PHASE_WARNING;
        out_plan->input_frame = frame;
        out_plan->frame_in_phase = boot_frame.frame_in_phase;
        out_plan->copy_width = title->warning_width < NEXUS_FB_W
                                   ? title->warning_width
                                   : NEXUS_FB_W;
        out_plan->copy_height = title->warning_height < NEXUS_FB_H
                                    ? title->warning_height
                                    : NEXUS_FB_H;
        return 1;
    }
    if (!nexus_title_screen_surface_ready(title)) {
        nexus_title_plan_reset(out_plan);
        return 0;
    }
    nexus_title_plan_reset(out_plan);
    out_plan->kind = NEXUS_V1_TITLE_RENDER_PLAN_TITLE_ART;
    out_plan->boot_phase = NEXUS_V1_BOOT_PHASE_TITLE;
    out_plan->input_frame = frame;
    out_plan->frame_in_phase = boot_frame.frame_in_phase;
    out_plan->title_frame = boot_frame.title_frame;
    out_plan->reveal_y0 = boot_frame.title.reveal_y0;
    out_plan->reveal_y1 = boot_frame.title.reveal_y1;
    out_plan->edge_color = boot_frame.title.edge_color;
    out_plan->copy_width = title->width < NEXUS_FB_W
                               ? title->width
                               : NEXUS_FB_W;
    out_plan->copy_height = title->height < NEXUS_FB_H
                                ? title->height
                                : NEXUS_FB_H;
    /* The real TITLE.CG pixels are the only startup surface. Do not add
     * code-built edge lines, prompts, or other substitute artwork. */
    return 1;
}

static void nexus_render_title_plan(const Nexus_TitleScreen *title,
                                    Nexus_Framebuffer *fb,
                                    const Nexus_V1_TitleRenderPlan *plan)
{
    int y;
    int i;
    if (!fb) {
        return;
    }
    nexus_fb_clear(fb);
    if (!plan) {
        return;
    }
    if (plan->kind == NEXUS_V1_TITLE_RENDER_PLAN_TITLE_ART) {
        /* Nexus boot presentation: keep the real TITLE.CG artwork, but make
         * startup title presentation frame-owned by Nexus title code until the
         * original VDP1/VDP2 title program is separately authenticated. The
         * supplied corpus has no NEXUS.BIN, so TITLE.CG remains the only
         * admitted title pixel source. */
        if (title && title->pixels && title->width > 0 && title->height > 0) {
            for (y = plan->reveal_y0;
                 y < plan->reveal_y1 && y < plan->copy_height &&
                     y < NEXUS_FB_H;
                 ++y) {
                memcpy(&fb->color_buffer[y * NEXUS_FB_W],
                       &title->pixels[y * title->width],
                       (size_t)plan->copy_width);
            }
        }
    } else if (plan->kind == NEXUS_V1_TITLE_RENDER_PLAN_WARNING_ART) {
        if (title && title->warning_pixels && title->warning_width > 0) {
            if (title->warning_palette_loaded) {
                nexus_fb_set_palette(fb, title->warning_palette_rgba);
            }
            for (y = 0; y < plan->copy_height && y < NEXUS_FB_H; ++y) {
                memcpy(&fb->color_buffer[y * NEXUS_FB_W],
                       &title->warning_pixels[y * title->warning_width],
                       (size_t)plan->copy_width);
            }
        }
    } else {
        /* Invalid/unknown plans remain blank. Never synthesize a gradient
         * when the authenticated title or warning surface is unavailable. */
    }
    for (i = 0; i < plan->rect_count; ++i) {
        const Nexus_V1_TitleRenderRect *rect = &plan->rects[i];
        nexus_title_draw_rect(fb,
                              rect->x,
                              rect->y,
                              rect->w,
                              rect->h,
                              rect->color);
    }
}

void nexus_render_title(const Nexus_TitleScreen *title,
                        Nexus_Framebuffer *fb, int frame) {
    Nexus_V1_TitleRenderPlan plan;
    if (!fb) {
        return;
    }
    if (!nexus_v1_title_build_render_plan(title, frame, &plan)) {
        nexus_fb_clear(fb);
        return;
    }
    nexus_render_title_plan(title, fb, &plan);
}
