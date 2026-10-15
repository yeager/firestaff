#include "dm1_v1_fmtowns_entrance_viewport.h"

#include "dm1_v1_fmtowns_layout_crop.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <string.h>

enum {
    DM1_C255_SIDE = 5,
    DM1_C255_CENTER_X = 2,
    DM1_C255_CENTER_Y = 0,
    DM1_C255_DIRECTION_SOUTH = 2,
    DM1_C255_MAP_INDEX = 255,
    DM1_C255_ELEMENT_CORRIDOR = 1,
    DM1_C255_UNSUPPORTED_GRAPHIC_BIT = 31
};

static int dm1_c255_aperture_y(DM1_V1_FmtownsLang language)
{
    /* ReDMCSB COORD.C G2068: MEDIA562_F20E English initializes y=33;
     * MEDIA571_F20J Japanese initializes y=31. G2067 is x=0 in both. */
    if (language == DM1_FMTOWNS_LANG_EN) return 33;
    if (language == DM1_FMTOWNS_LANG_JP) return 31;
    return -1;
}

typedef struct DM1_C255ProviderContext {
    DM1_V1_FmtownsEntranceGraphicProvider provider;
    void *user_data;
    DM1_V1_FmtownsEntranceViewportReceipt *receipt;
    unsigned int loaded_wall_surfaces;
    uint8_t *towns_viewport;
} DM1_C255ProviderContext;

static const uint8_t g_dm1_c255_pc_frame_safety_pixels[
    DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH *
    DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT] = {0};

/* ReDMCSB COORD.C G3019 and DUNVIEW.C F0095/F0128 use the same wall
 * source/zone identities for the C255 entrance and the loaded dungeon. */
static int dm1_c255_graphic_route(int graphic,
                                  unsigned int *out_graphic,
                                  unsigned int *out_zone)
{
    DM1_V1_FmtownsWallPanelRoute route;
    if (!out_graphic || !out_zone) return 0;
    if (graphic == -1 || graphic == -2) {
        *out_graphic = graphic == -1
            ? DM1_V1_FMTOWNS_ENTRANCE_SOURCE_FLOOR
            : DM1_V1_FMTOWNS_ENTRANCE_SOURCE_CEILING;
        *out_zone = graphic == -1 ? 701u : 700u;
        return 1;
    }
    if (graphic < 0 ||
        !dm1_v1_fmtowns_wall_panel_route((unsigned int)graphic, 0,
                                          &route)) return 0;
    *out_graphic = route.source_graphic_base;
    *out_zone = route.layout_zone;
    return 1;
}

static int dm1_c255_known_native_dimensions(unsigned int graphic,
                                            int width, int height)
{
    static const struct { unsigned short graphic, width, height; } expected[] = {
        {75, 224, 70}, {76, 224, 29}, {84, 32, 136},
        {85, 32, 136}, {86, 248, 111}, {87, 136, 71},
        {88, 117, 51}, {89, 16, 49}
    };
    size_t i;
    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        if (expected[i].graphic == graphic) {
            return width == expected[i].width && height == expected[i].height;
        }
    }
    return 1;
}

static int dm1_c255_renderer_graphic_provider(
    void *user_data, int renderer_graphic, const uint8_t **out_pixels,
    int *out_width, int *out_height)
{
    DM1_C255ProviderContext *context = (DM1_C255ProviderContext *)user_data;
    unsigned int source_graphic;
    unsigned int zone_index;
    uint32_t bit;

    if (!context || !context->receipt || !context->provider ||
        !out_pixels || !out_width || !out_height) {
        return 0;
    }
    if (!dm1_c255_graphic_route(renderer_graphic, &source_graphic,
                                  &zone_index)) {
        context->receipt->requested_source_graphics |=
            UINT32_C(1) << DM1_C255_UNSUPPORTED_GRAPHIC_BIT;
        context->receipt->missing_source_graphics |=
            UINT32_C(1) << DM1_C255_UNSUPPORTED_GRAPHIC_BIT;
        return 0;
    }
    if (renderer_graphic == -1) bit = UINT32_C(1);
    else if (renderer_graphic == -2) bit = UINT32_C(2);
    else bit = UINT32_C(1) << (2u + (unsigned int)(renderer_graphic - 93));
    context->receipt->requested_source_graphics |= bit;
    *out_pixels = NULL;
    *out_width = 0;
    *out_height = 0;
    if (!context->provider(context->user_data, source_graphic,
                           out_pixels, out_width, out_height) ||
        !*out_pixels || *out_width <= 0 || *out_height <= 0) {
        context->receipt->missing_source_graphics |= bit;
        return 0;
    }
    if (!dm1_c255_known_native_dimensions(source_graphic, *out_width,
                                           *out_height) ||
        !context->towns_viewport ||
        (size_t)*out_width > SIZE_MAX / (size_t)*out_height ||
        !dm1_v1_fmtowns_layout_blit(context->towns_viewport,
            DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH *
                DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT,
            DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH, *out_pixels,
            (size_t)*out_width * (size_t)*out_height, *out_width,
            *out_width, *out_height, zone_index,
            DM1_V1_FMTOWNS_LAYOUT_TRANSPARENT_INDEX)) {
        context->receipt->missing_source_graphics |= bit;
        *out_pixels = NULL;
        *out_width = 0;
        *out_height = 0;
        return 0;
    }
    context->receipt->loaded_source_graphics |= bit;
    context->receipt->source_graphic_record_mask |=
        UINT32_C(1) << (source_graphic - 75u);
    if (renderer_graphic >= 93) ++context->loaded_wall_surfaces;
    /* Keep the existing geometry walk as an ordering oracle only. Its
     * renderer sees a bounded inert page; all pixels come from the native
     * Towns image cropped through the authenticated G2174/F0635 layout. */
    *out_pixels = g_dm1_c255_pc_frame_safety_pixels;
    *out_width = DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH;
    *out_height = DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT;
    return 1;
}

static uint32_t dm1_c255_hash(const uint8_t *pixels, int stride)
{
    uint32_t hash = UINT32_C(2166136261);
    int y;
    int x;

    for (y = 0; y < DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT; ++y) {
        for (x = 0; x < DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH; ++x) {
            hash ^= pixels[(size_t)y * (size_t)stride + (size_t)x];
            hash *= UINT32_C(16777619);
        }
    }
    return hash;
}

int dm1_v1_fmtowns_entrance_render_c255(
    uint8_t *viewport_pixels, size_t viewport_pixel_count,
    int viewport_stride, DM1_V1_FmtownsEntranceGraphicProvider provider,
    void *provider_user_data, DM1_V1_FmtownsLang language,
    DM1_V1_FmtownsEntranceViewportReceipt *out_receipt)
{
    uint8_t staged[DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH *
                   DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT];
    uint8_t geometry_oracle[DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH *
                            DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT];
    uint8_t grid[DM1_C255_SIDE * DM1_C255_SIDE];
    DM1_Viewport3DState renderer;
    DM1_C255ProviderContext provider_context;
    DM1_V1_FmtownsEntranceViewportReceipt receipt;
    size_t required_bytes;
    int y;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!viewport_pixels || !provider || !out_receipt || viewport_stride <
            DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH ||
        dm1_c255_aperture_y(language) < 0) {
        return 0;
    }
    receipt.aperture_x = 0;
    receipt.aperture_y = (int16_t)dm1_c255_aperture_y(language);
    required_bytes = (size_t)(DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT - 1) *
        (size_t)viewport_stride +
        DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH;
    if (viewport_pixel_count < required_bytes) return 0;

    /* ReDMCSB ENTRANCE.C F0797 stores map columns contiguously. Convert its
     * square 7 (column 1, row 2) to this renderer's row-major grid. */
    memset(grid, 0, sizeof(grid));
    for (y = 0; y < DM1_C255_SIDE; ++y) {
        grid[y * DM1_C255_SIDE + DM1_C255_CENTER_X] =
            DM1_C255_ELEMENT_CORRIDOR << 5;
    }
    grid[2 * DM1_C255_SIDE + 1] = DM1_C255_ELEMENT_CORRIDOR << 5;

    memset(staged, 0, sizeof(staged));
    memset(geometry_oracle, 0, sizeof(geometry_oracle));
    memset(&provider_context, 0, sizeof(provider_context));
    provider_context.provider = provider;
    provider_context.user_data = provider_user_data;
    provider_context.receipt = &receipt;
    provider_context.towns_viewport = staged;

    /* The callback composes authentic FM Towns crops into `staged` and
     * returns inert PC34 pixels only to let the legacy renderer determine
     * which source records are visible. Keep that geometry pass isolated:
     * drawing it over `staged` erases the real Towns pixels with zeroes. */
    dm1_viewport_3d_init(&renderer, geometry_oracle,
                         DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH);
    renderer.party_map_index = DM1_C255_MAP_INDEX;
    renderer.dungeon_grid = grid;
    renderer.dungeon_width = DM1_C255_SIDE;
    renderer.dungeon_height = DM1_C255_SIDE;
    renderer.graphic_provider_callback = dm1_c255_renderer_graphic_provider;
    renderer.graphic_provider_user_data = &provider_context;
    renderer.source_graphics_required = true;
    /* F0797's transient C255 has no MAP descriptor. STARTUP2's cache is
     * MEDIA020 wall-set zero; map/floor selection must not leak in from gameplay. */
    dm1_viewport_3d_load_wall_set(&renderer, 0, 0);
    dm1_viewport_3d_draw_frame(&renderer, DM1_C255_DIRECTION_SOUTH,
                               DM1_C255_CENTER_X, DM1_C255_CENTER_Y);

    if (receipt.missing_source_graphics != 0u ||
        (receipt.loaded_source_graphics & UINT32_C(3)) != UINT32_C(3) ||
        provider_context.loaded_wall_surfaces == 0u) {
        *out_receipt = receipt;
        return 0;
    }
    for (y = 0; y < DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT; ++y) {
        memcpy(viewport_pixels + (size_t)y * (size_t)viewport_stride,
               staged + (size_t)y *
                   DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH,
               DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH);
    }
    receipt.valid = 1;
    receipt.pixel_hash_fnv1a = dm1_c255_hash(
        viewport_pixels, viewport_stride);
    *out_receipt = receipt;
    return 1;
}
