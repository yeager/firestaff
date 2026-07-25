#include "csb_v1_viewport_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#define CSB_GRAPHIC_FIRST_CREATURE 584
#define CSB_CREATURE_ZONE_BASE     3200
#define CSB_CREATURE_TYPE_MAX_CSB  9

static const int s_first_native_offset[CSB_CREATURE_TYPE_MAX_CSB] = {
    0, 4, 6, 10, 12, 16, 19, 21, 23
};

static const uint8_t s_transparent_color[CSB_CREATURE_TYPE_MAX_CSB] = {
    13, 11, 11, 4, 4, 4, 13, 4, 4
};

static const uint8_t s_palette_d3[16] = {
    0, 12, 1, 3, 4, 3, 0, 6, 3, 0, 0, 11, 0, 2, 0, 13
};

static const uint8_t s_palette_d2[16] = {
    0, 1, 2, 3, 4, 3, 6, 7, 5, 0, 0, 11, 12, 13, 14, 15
};

int csb_v1_viewport_f0115_native_group_front_graphic_pc34(int creatureType)
{
    if (creatureType < 0 || creatureType >= CSB_CREATURE_TYPE_MAX_CSB)
        return -1;
    return CSB_GRAPHIC_FIRST_CREATURE + s_first_native_offset[creatureType];
}

int csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
    int creatureType,
    int nativeGraphic,
    const uint8_t *sourcePixels,
    int sourceWidth,
    int sourceHeight,
    uint8_t *framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int framebufferStride,
    int drawX,
    int drawY,
    int drawW,
    int drawH,
    int depthOrdinal,
    int zone,
    int mirror)
{
    int drawn = 0;
    int expectedGraphic;
    uint8_t transColor;
    const uint8_t *palette;
    int y, x;

    (void)framebufferWidth;
    (void)mirror;

    if (creatureType < 0 || creatureType >= CSB_CREATURE_TYPE_MAX_CSB)
        return 0;

    expectedGraphic = CSB_GRAPHIC_FIRST_CREATURE +
                      s_first_native_offset[creatureType];
    if (nativeGraphic != expectedGraphic)
        return 0;

    if ((zone & ~0x8000) < CSB_CREATURE_ZONE_BASE)
        return 0;

    if (!sourcePixels || !framebuffer)
        return 0;
    if (drawW < sourceWidth || drawH < sourceHeight)
        return 0;
    if (drawX < 0 || drawY < 0)
        return 0;
    if (drawX + sourceWidth > framebufferStride ||
        drawY + sourceHeight > framebufferHeight)
        return 0;

    transColor = s_transparent_color[creatureType];
    palette = (depthOrdinal >= 3) ? s_palette_d3 : s_palette_d2;

    for (y = 0; y < sourceHeight; y++) {
        for (x = 0; x < sourceWidth; x++) {
            uint8_t pixel = sourcePixels[y * sourceWidth + x];
            if (pixel == transColor)
                continue;
            if (pixel < 16)
                pixel = palette[pixel];
            framebuffer[(drawY + y) * framebufferStride + (drawX + x)] = pixel;
            drawn++;
        }
    }
    return drawn;
}

int csb_v1_viewport_f0115_blit_f0093_group_front_family_pc34(
    int creatureType,
    const struct DungeonMapDesc_Compat *map,
    const uint8_t *sourcePixels,
    int sourceWidth,
    int sourceHeight,
    uint8_t *framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int framebufferStride,
    int drawX,
    int drawY,
    int drawW,
    int drawH,
    int depthOrdinal,
    int zone,
    int mirror)
{
    int drawn = 0;
    uint8_t transColor;
    int y, x;

    (void)framebufferWidth;
    (void)depthOrdinal;
    (void)zone;
    (void)mirror;

    if (creatureType < 0 || creatureType >= CSB_CREATURE_TYPE_MAX_CSB)
        return 0;

    if (!map || map->creatureTypeCount > sizeof(map->allowedCreatureTypes))
        return 0;

    if (!sourcePixels || !framebuffer)
        return 0;
    if (drawW < sourceWidth || drawH < sourceHeight)
        return 0;
    if (drawX < 0 || drawY < 0)
        return 0;
    if (drawX + sourceWidth > framebufferStride ||
        drawY + sourceHeight > framebufferHeight)
        return 0;

    transColor = s_transparent_color[creatureType];

    for (y = 0; y < sourceHeight; y++) {
        for (x = 0; x < sourceWidth; x++) {
            uint8_t pixel = sourcePixels[y * sourceWidth + x];
            if (pixel == transColor)
                continue;
            framebuffer[(drawY + y) * framebufferStride + (drawX + x)] = pixel;
            drawn++;
        }
    }
    return drawn;
}
