#include "csb_v1_viewport_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#define CSB_FIRST_OBJECT_NATIVE 498
#define CSB_LAST_OBJECT_NATIVE  583
#define C10_COLOR_FLESH 10

static const int weapon_graphics[46] = {
    537, 537, 534, 536, 510, 511, 511, 538, 516, 511, 511, 511,
    511, 511, 511, 511, 541, 511, 512, 512, 520, 520, 532, 542,
    543, 513, 544, 515, 545, 510, 546, 547, 548, 549, 510, 530,
    530, 510, 510, 510, 550, 531, 529, 564, 544, 581
};

static const int armour_graphics[58] = {
    522, 522, 522, 554, 507, 523, 523, 523, 523, 568, 523, 523,
    568, 506, 506, 556, 522, 522, 528, 568, 568, 523, 523, 552,
    552, 508, 508, 508, 553, 553, 509, 553, 518, 518, 518, 518,
    508, 518, 551, 519, 521, 555, 509, 551, 519, 521, 555, 509,
    551, 519, 521, 555, 509, 551, 518, 521, 580, 583
};

static const int junk_graphics[52] = {
    533, 505, 514, 514, 539, 540, 503, 582, 503, 517, 517, 517,
    517, 517, 517, 517, 517, 561, 561, 561, 561, 561, 561, 561,
    561, 502, 559, 560, 526, 527, 524, 525, 570, 569, 504, 565,
    514, 514, 557, 558, 558, 578, 562, 563, 571, 572, 573, 574,
    576, 577, 573, 540
};

static const int potion_graphics[21] = {
    565, 565, 565, 565, 565, 565,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    566, 566, 566, 566,
    578
};

int csb_v1_viewport_f0115_object_native_graphic_pc34(
    int thingType, int subtype, int alcoveSlot)
{
    if (subtype < 0) return -1;
    switch (thingType) {
        case THING_TYPE_CONTAINER:
            if (subtype != 0 || alcoveSlot < 0 || alcoveSlot > 1) return -1;
            return CSB_FIRST_OBJECT_NATIVE + alcoveSlot;
        case THING_TYPE_SCROLL:
            if (subtype != 0) return -1;
            return 500;
        case THING_TYPE_POTION:
            if (subtype >= 21) return -1;
            return potion_graphics[subtype];
        case THING_TYPE_WEAPON:
            if (subtype >= 46) return -1;
            return weapon_graphics[subtype];
        case THING_TYPE_ARMOUR:
            if (subtype >= 58) return -1;
            return armour_graphics[subtype];
        case THING_TYPE_JUNK:
            if (subtype >= 52) return -1;
            return junk_graphics[subtype];
        default:
            return -1;
    }
}

int csb_v1_viewport_f0115_first_object_native_graphic_pc34(
    int thingType, int subtype)
{
    return csb_v1_viewport_f0115_object_native_graphic_pc34(
        thingType, subtype, 0);
}

static int is_flippable_graphic(int nativeGraphic)
{
    return nativeGraphic == 498 ||
           nativeGraphic == 563 ||
           nativeGraphic == 578 ||
           nativeGraphic == 580;
}

int csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
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
    int mirror)
{
    int drawn = 0;
    int flip;
    int y, x;

    (void)framebufferWidth;
    (void)depthOrdinal;

    if (nativeGraphic < CSB_FIRST_OBJECT_NATIVE ||
        nativeGraphic > CSB_LAST_OBJECT_NATIVE)
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

    flip = mirror && is_flippable_graphic(nativeGraphic);

    for (y = 0; y < sourceHeight; y++) {
        for (x = 0; x < sourceWidth; x++) {
            uint8_t pixel = sourcePixels[y * sourceWidth + x];
            int destX;
            if (pixel == C10_COLOR_FLESH)
                continue;
            if (flip)
                destX = drawX + (sourceWidth - 1 - x);
            else
                destX = drawX + x;
            framebuffer[(drawY + y) * framebufferStride + destX] = pixel;
            drawn++;
        }
    }
    return drawn;
}
