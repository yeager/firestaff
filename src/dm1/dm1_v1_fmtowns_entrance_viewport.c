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

static int dm1_c255_zone_for_graphic(unsigned int graphic,
                                     unsigned int *out_zone)
{
    static const unsigned short wall_zones[10] = {
        717, 716, 714, 713, 712, 708, 707, 711, 710, 709
    };
    if (!out_zone) return 0;
    if (graphic == DM1_V1_FMTOWNS_ENTRANCE_SOURCE_FLOOR) {
        *out_zone = 701u;
        return 1;
    }
    if (graphic == DM1_V1_FMTOWNS_ENTRANCE_SOURCE_CEILING) {
        *out_zone = 700u;
        return 1;
    }
    if (graphic >= 93u && graphic <= 107u) {
        if (graphic <= 102u) {
            *out_zone = wall_zones[graphic - 93u];
        } else {
            static const unsigned short near_zones[5] = {
                703, 702, 706, 705, 704
            };
            *out_zone = near_zones[graphic - 103u];
        }
        return *out_zone >= 700u && *out_zone <= 717u;
    }
    return 0;
}

static int dm1_c255_known_native_dimensions(unsigned int graphic,
                                            int width, int height)
{
    static const struct { unsigned short graphic, width, height; } expected[] = {
        {78, 25, 94}, {79, 21, 65}, {95, 158, 100},
        {96, 32, 44}, {101, 32, 91}, {105, 20, 43}, {106, 20, 39}
    };
    size_t i;
    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        if (expected[i].graphic == graphic) {
            return width == expected[i].width && height == expected[i].height;
        }
    }
    return 1;
}

static int dm1_c255_graphic_bit(unsigned int graphic, uint32_t *out_bit)
{
    unsigned int bit;

    if (!out_bit) return 0;
    if (graphic == DM1_V1_FMTOWNS_ENTRANCE_SOURCE_FLOOR) {
        bit = 0u;
    } else if (graphic == DM1_V1_FMTOWNS_ENTRANCE_SOURCE_CEILING) {
        bit = 1u;
    } else if (graphic >= DM1_V1_FMTOWNS_ENTRANCE_SOURCE_WALL_SET_ZERO_FIRST &&
               graphic <= DM1_V1_FMTOWNS_ENTRANCE_SOURCE_WALL_SET_ZERO_LAST) {
        bit = 2u + graphic -
            DM1_V1_FMTOWNS_ENTRANCE_SOURCE_WALL_SET_ZERO_FIRST;
    } else {
        return 0;
    }
    *out_bit = UINT32_C(1) << bit;
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
    if (renderer_graphic == -1) {
        source_graphic = DM1_V1_FMTOWNS_ENTRANCE_SOURCE_FLOOR;
    } else if (renderer_graphic == -2) {
        source_graphic = DM1_V1_FMTOWNS_ENTRANCE_SOURCE_CEILING;
    } else if (renderer_graphic >= 86 && renderer_graphic <= 107) {
        /* The transient startup cache is wall-set zero. ReDMCSB DEFS.H's
         * MEDIA720 records 86..107 are its Towns door-frame and wall art. */
        source_graphic = (unsigned int)renderer_graphic;
    } else {
        context->receipt->requested_source_graphics |=
            UINT32_C(1) << DM1_C255_UNSUPPORTED_GRAPHIC_BIT;
        context->receipt->missing_source_graphics |=
            UINT32_C(1) << DM1_C255_UNSUPPORTED_GRAPHIC_BIT;
        return 0;
    }
    if (!dm1_c255_graphic_bit(source_graphic, &bit)) return 0;
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
        !dm1_c255_zone_for_graphic(source_graphic, &zone_index) ||
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
    if (source_graphic >= 93u) ++context->loaded_wall_surfaces;
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
    memset(&provider_context, 0, sizeof(provider_context));
    provider_context.provider = provider;
    provider_context.user_data = provider_user_data;
    provider_context.receipt = &receipt;
    provider_context.towns_viewport = staged;

    dm1_viewport_3d_init(&renderer, staged,
                         DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH);
    renderer.party_map_index = DM1_C255_MAP_INDEX;
    renderer.dungeon_grid = grid;
    renderer.dungeon_width = DM1_C255_SIDE;
    renderer.dungeon_height = DM1_C255_SIDE;
    renderer.graphic_provider_callback = dm1_c255_renderer_graphic_provider;
    renderer.graphic_provider_user_data = &provider_context;
    renderer.source_graphics_required = true;
    /* F0797's transient C255 has no MAP descriptor. STARTUP2's cache is
     * wall-set zero; map/floor selection must not leak in from gameplay. */
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
