#include "nexus_v1_title.h"
#include "nexus_v1_title_sequence.h"
#include "nexus_v1_ui_surfaces.h"

#include <stdlib.h>
#include <string.h>

static uint16_t nexus_title_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static uint64_t nexus_title_fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0U; i < size; ++i) {
        value ^= bytes[i];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

/* DMWeb MAPD/TIBG layout: five 0x1c04-byte 64x28 maps begin at 0x40
 * (ending at 0x8c54), followed by sixteen BE16 palette words at 0x8c54.
 * The final palette word therefore ends at 0x8c74. */
#define NEXUS_TITLE_MAPD_MIN_BYTES 0x8c74U

static void nexus_title_clear_decoded_maps(Nexus_TitleScreen *title)
{
    int map;
    if (!title) {
        return;
    }
    for (map = 0; map < NEXUS_V1_TITLE_MAP_COUNT; ++map) {
        free(title->decoded_map_pixels[map]);
        title->decoded_map_pixels[map] = NULL;
    }
    memset(title->decoded_map_source_offsets, 0,
           sizeof(title->decoded_map_source_offsets));
    memset(title->decoded_map_cell_bytes, 0,
           sizeof(title->decoded_map_cell_bytes));
    memset(title->decoded_map_tile_min, 0,
           sizeof(title->decoded_map_tile_min));
    memset(title->decoded_map_tile_max, 0,
           sizeof(title->decoded_map_tile_max));
    memset(title->decoded_map_word0_or, 0,
           sizeof(title->decoded_map_word0_or));
    memset(title->decoded_map_word1_or, 0,
           sizeof(title->decoded_map_word1_or));
    memset(title->decoded_map_word1_attribute_or, 0,
           sizeof(title->decoded_map_word1_attribute_or));
    memset(title->decoded_map_cell_fnv1a64, 0,
           sizeof(title->decoded_map_cell_fnv1a64));
    memset(title->decoded_map_palette, 0, sizeof(title->decoded_map_palette));
    title->decoded_map_count = 0;
    title->decoded_map_source_bound = 0;
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
    if (!title) {
        return 0;
    }
    nexus_title_clear_decoded_maps(title);
    if (!mapd || mapd_size < NEXUS_TITLE_MAPD_MIN_BYTES || !title_cg ||
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
        uint16_t tile_min = UINT16_MAX;
        uint16_t tile_max = 0U;
        uint16_t word0_or = 0U;
        uint16_t word1_or = 0U;
        uint16_t word1_attribute_or = 0U;
        if (!out || map_offset + 4U + 0x1c00U > mapd_size) {
            free(out);
            nexus_title_clear_decoded_maps(title);
            return 0;
        }
        if (nexus_title_be16(mapd + map_offset) != 0x0040U ||
            nexus_title_be16(mapd + map_offset + 2U) != 0x001cU) {
            free(out);
            nexus_title_clear_decoded_maps(title);
            return 0;
        }
        for (cell = 0; cell < 64 * 28; ++cell) {
            size_t cell_offset = map_offset + 4U + (size_t)cell * 4U;
            uint16_t word0 = nexus_title_be16(mapd + cell_offset);
            uint16_t word1 = nexus_title_be16(mapd + cell_offset + 2U);
            int tile_index = (int)(nexus_title_be16(mapd + cell_offset + 2U) &
                                   0x7fffU) - 4608;
            int x = (cell % 64) * 8;
            int y = (cell / 64) * 8;
            int py;
            if (tile_index < 0 || tile_index >= 5249 ||
                (size_t)tile_index * 32U + 32U > title_cg_bytes) {
                free(out);
                nexus_title_clear_decoded_maps(title);
                return 0;
            }
            if ((uint16_t)tile_index < tile_min)
                tile_min = (uint16_t)tile_index;
            if ((uint16_t)tile_index > tile_max)
                tile_max = (uint16_t)tile_index;
            word0_or = (uint16_t)(word0_or | word0);
            word1_or = (uint16_t)(word1_or | word1);
            word1_attribute_or =
                (uint16_t)(word1_attribute_or | (word1 & 0xfe00U));
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
        title->decoded_map_source_offsets[map] = (uint32_t)map_offset;
        title->decoded_map_cell_bytes[map] = NEXUS_V1_TITLE_MAP_CELL_BYTES;
        title->decoded_map_tile_min[map] = tile_min;
        title->decoded_map_tile_max[map] = tile_max;
        title->decoded_map_word0_or[map] = word0_or;
        title->decoded_map_word1_or[map] = word1_or;
        title->decoded_map_word1_attribute_or[map] = word1_attribute_or;
        title->decoded_map_cell_fnv1a64[map] = nexus_title_fnv1a64(
            mapd + map_offset + 4U, NEXUS_V1_TITLE_MAP_CELL_BYTES);
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
        /* A cached title surface does not make a truncated TITLE.BIN safe.
         * The MAPD/TIBG block begins at 0x0e278 (DMWeb layout); reject the
         * incomplete source before forming the offset or subtracting it. */
        if (data && size > (int)0x0e278U) {
            int cg_size = 0;
            uint8_t *cg = nexus_v1_read_file(engine, "TITLE.CG", &cg_size);
            (void)nexus_v1_title_decode_mapd(
                                          data + 0x0e278U,
                                          (size_t)size - 0x0e278U, cg,
                                          cg_size > 0 ? (size_t)cg_size : 0U,
                                          title);
            free(cg);
        }
        free(data);
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
        /* TITLE.BIN's length needs its own local: reading it into `size`
         * clobbered TITLE.CG's length, and `data` (the TITLE.CG buffer) was
         * then passed with TITLE.BIN's length. With retail assets that made
         * the decoder's entry guard trip so all five title maps silently
         * failed, and it is a heap over-read whenever TITLE.BIN is the larger
         * file. The cached-surface branch above already uses a separate
         * cg_size for the same reason. */
        int mapd_size = 0;
        uint8_t *mapd = nexus_v1_read_file(engine, "TITLE.BIN", &mapd_size);
        (void)nexus_v1_title_decode_mapd(
                                      mapd && mapd_size > (int)0x0e278U
                                          ? mapd + 0x0e278U : NULL,
                                      mapd_size > (int)0x0e278U
                                          ? (size_t)mapd_size - 0x0e278U : 0U,
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
    (void)title;
    /* TITLE.CG is a character-generator atlas, not a framebuffer. Copying
     * it as a full-screen title invents the missing Saturn VDP1/VDP2 tile-map
     * selection and placement. TITLE.BIN MAPD/TIBG decoding remains a
     * receipt until an original title capture supplies that handoff. */
    return 0;
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

void nexus_render_title(const Nexus_TitleScreen *title,
                        Nexus_Framebuffer *fb, int frame) {
    /* WARNING.BIN and TITLE.CG are authentic source assets, but this public
     * host renderer has no Saturn VDP1/VDP2 capture binding for their palette,
     * tile-map, destination, or command order. Keep the production framebuffer
     * blank until that source-owned handoff is admitted. The render-plan
     * helper remains available to isolated format/timing diagnostics. */
    (void)title;
    (void)frame;
    if (!fb) {
        return;
    }
    nexus_fb_clear(fb);
}
