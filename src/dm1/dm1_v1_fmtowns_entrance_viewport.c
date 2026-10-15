#include "dm1_v1_fmtowns_entrance_viewport.h"
#include "dm1_v1_fmtowns_layout_crop.h"

#include <string.h>

/* ENTRANCE.C F0797:57-80 authors a column-major 5x5 map, corridors at
 * 10..14 and 7, viewed south from (2,0). No loaded dungeon enters C255. */
static int c255_wall(int forward, int side)
{
    int x = 2 + side;
    int y = forward;
    return x != 2 && !(x == 1 && y == 2);
}

static int c255_native_size(unsigned int graphic, int width, int height)
{
    static const unsigned short sizes[][3] = {
        {75,224,70}, {76,224,29}, {84,32,136}, {85,32,136},
        {86,248,111}, {87,136,71}, {88,117,51}, {89,16,49}
    };
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); ++i)
        if (sizes[i][0] == graphic)
            return width == sizes[i][1] && height == sizes[i][2];
    return 0;
}

static int c255_blit(uint8_t *page,
    DM1_V1_FmtownsEntranceGraphicProvider provider, void *user_data,
    int semantic, DM1_V1_FmtownsEntranceViewportReceipt *receipt)
{
    DM1_V1_FmtownsWallPanelRoute route;
    const uint8_t *pixels = NULL;
    int width = 0, height = 0, ok;
    unsigned int graphic;
    uint32_t bit;
    if (semantic < 0) {
        graphic = semantic == -1 ? 75u : 76u;
        bit = semantic == -1 ? 1u : 2u;
    } else {
        if (!dm1_v1_fmtowns_wall_panel_route((unsigned)semantic, 0, &route))
            return 0;
        graphic = route.source_graphic_base;
        bit = UINT32_C(1) << (2u + (unsigned)(semantic - 93));
    }
    receipt->requested_source_graphics |= bit;
    if (!provider(user_data, graphic, &pixels, &width, &height) || !pixels ||
        !c255_native_size(graphic, width, height)) {
        receipt->missing_source_graphics |= bit;
        return 0;
    }
    if (semantic == -2) {
        /* DUNVIEW.C F0128:8432-8433 (MEDIA544) mirrors the ceiling
         * after F0098 on even parity. C255's (2+0+South) is even. */
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                page[y * 224 + x] = pixels[y * width + width - 1 - x];
        ok = 1;
    } else if (semantic < 0) {
        /* F0098/F0128 copy the floor without C10 holes. */
        ok = dm1_v1_fmtowns_layout_blit(page, 224u*136u, 224, pixels,
            (size_t)width*height, width, width, height,
            semantic == -1 ? 701u : 700u, -1);
    } else {
        ok = dm1_v1_fmtowns_wall_blit(page, 224u*136u, 224, pixels,
            (size_t)width*height, width, width, height, &route, 10);
    }
    if (!ok) {
        receipt->missing_source_graphics |= bit;
        return 0;
    }
    receipt->loaded_source_graphics |= bit;
    receipt->source_graphic_record_mask |= UINT32_C(1) << (graphic - 75u);
    return 1;
}

int dm1_v1_fmtowns_entrance_render_c255(
    uint8_t *viewport_pixels, size_t viewport_pixel_count,
    int viewport_stride, DM1_V1_FmtownsEntranceGraphicProvider provider,
    void *provider_user_data, DM1_V1_FmtownsLang language,
    DM1_V1_FmtownsEntranceViewportReceipt *out_receipt)
{
    /* DUNVIEW.C F0128:8444-8542: F20 has D3 outer walls but no F0678/79
     * D2 outer calls. Each empty C255 Thing chain has no content pass. */
    static const struct { int graphic, forward, side; } calls[] = {
        {104,3,2}, {103,3,-2}, {106,3,1}, {105,3,-1}, {107,3,0},
        {101,2,1}, {100,2,-1}, {102,2,0},
        {96,1,1}, {95,1,-1}, {97,1,0}, {94,0,1}, {93,0,-1}
    };
    uint8_t page[224*136] = {0};
    DM1_V1_FmtownsEntranceViewportReceipt receipt = {0};
    uint32_t hash = UINT32_C(2166136261);
    size_t required;
    if (out_receipt) *out_receipt = receipt;
    if (!viewport_pixels || !provider || !out_receipt || viewport_stride < 224 ||
        (language != DM1_FMTOWNS_LANG_EN && language != DM1_FMTOWNS_LANG_JP) ||
        (size_t)viewport_stride > (SIZE_MAX - 224u)/135u) return 0;
    required = (size_t)viewport_stride * 135u + 224u;
    if (viewport_pixel_count < required) return 0;
    /* COORD.C G2068: F20E y33, F20J y31. C255 parity is even (2+0+2). */
    receipt.aperture_y = language == DM1_FMTOWNS_LANG_EN ? 33 : 31;
    if (!c255_blit(page, provider, provider_user_data, -2, &receipt) ||
        !c255_blit(page, provider, provider_user_data, -1, &receipt)) {
        *out_receipt = receipt;
        return 0;
    }
    for (size_t i = 0; i < sizeof(calls)/sizeof(calls[0]); ++i) {
        if (c255_wall(calls[i].forward, calls[i].side) &&
            !c255_blit(page, provider, provider_user_data,
                       calls[i].graphic, &receipt)) {
            *out_receipt = receipt;
            return 0;
        }
    }
    for (size_t i = 0; i < sizeof(page); ++i) {
        hash ^= page[i]; hash *= UINT32_C(16777619);
    }
    for (int y = 0; y < 136; ++y)
        memcpy(viewport_pixels + (size_t)y*viewport_stride, page + y*224, 224);
    receipt.valid = 1;
    receipt.pixel_hash_fnv1a = hash;
    *out_receipt = receipt;
    return 1;
}
