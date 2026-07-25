#include "csb_v1_viewport_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#define CSB_PROJECTILE_NATIVE_MIN 454
#define CSB_PROJECTILE_NATIVE_MAX 464
#define CSB_C10_COLOR_FLESH 10

static int is_proven_projectile_graphic(int graphicIndex)
{
    if (graphicIndex < CSB_PROJECTILE_NATIVE_MIN ||
        graphicIndex > CSB_PROJECTILE_NATIVE_MAX)
        return 0;
    if (graphicIndex == 456 || graphicIndex == 459)
        return 0;
    return 1;
}

int csb_v1_viewport_f0115_blit_m715_m716_m717_m718_projectile_family_pc34(
    int graphicIndex,
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
    int y, x;

    (void)framebufferWidth;
    (void)depthOrdinal;
    (void)mirror;

    if (!is_proven_projectile_graphic(graphicIndex))
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

    for (y = 0; y < sourceHeight; y++) {
        for (x = 0; x < sourceWidth; x++) {
            uint8_t pixel = sourcePixels[y * sourceWidth + x];
            if (pixel == CSB_C10_COLOR_FLESH)
                continue;
            framebuffer[(drawY + y) * framebufferStride + (drawX + x)] = pixel;
            drawn++;
        }
    }
    return drawn;
}
