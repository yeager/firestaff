#include "m11_game_view.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int verify_pc34_d2_pixels(const unsigned char *framebuffer,
                                 const unsigned char *source,
                                 const M11_Dm1FloorItemHostPresentationReceipt *r)
{
    int x;
    int y;
    if (!framebuffer || !source || !r || r->sourceScaleIndex != 1 ||
        r->destinationW != 20 || r->destinationH != 20 ||
        r->assetWidth != 32 || r->assetHeight != 32) return 0;
    for (y = 0; y < r->destinationH; ++y) {
        uint64_t stepY = ((uint64_t)r->assetHeight << 16) /
                         (unsigned)r->destinationH;
        unsigned sy = (unsigned)((stepY / 2u + 32767u +
                                  (uint64_t)y * stepY) >> 16);
        for (x = 0; x < r->destinationW; ++x) {
            uint64_t stepX = ((uint64_t)r->assetWidth << 16) /
                             (unsigned)r->destinationW;
            int sampleX = r->sourceMirrored ? r->destinationW - 1 - x : x;
            unsigned sx = (unsigned)((stepX / 2u + 32767u +
                                      (uint64_t)sampleX * stepX) >> 16);
            unsigned char pixel = source[sy * (unsigned)r->assetWidth + sx];
            unsigned char expected = DM1_FloorOrnPalette_D2[pixel & 0x0fu];
            unsigned char actual = framebuffer[(r->destinationY + y) * 320 +
                                                r->destinationX + x];
            if (expected == r->transparentColor) expected = 0u;
            if (actual != expected) {
                fprintf(stderr, "pixel mismatch %d,%d src=%u expected=%u actual=%u plan=%d,%d %dx%d mirror=%d\n",
                        x, y, pixel, expected, actual, r->destinationX,
                        r->destinationY, r->destinationW, r->destinationH,
                        r->sourceMirrored);
                return 0;
            }
        }
    }
    return 1;
}

int main(void)
{
    M11_GameViewState state;
    M11_Dm1FloorItemHostPresentationReceipt receipt;
    M11_Dm1FloorItemHostPresentationReceipt alcoveReceipt;
    unsigned char framebuffer[320 * 200];
    unsigned char sourcePixels[32 * 32];
    int i;

    M11_GameView_Init(&state);
    memset(framebuffer, 0, sizeof(framebuffer));
    for (i = 0; i < (int)sizeof(sourcePixels); ++i)
        sourcePixels[i] = (unsigned char)(1 + ((i * 7 + i / 32) % 15));
    sourcePixels[0] = 10; /* F0791 C10 transparency must survive scaling. */
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (receipt.valid || receipt.graphicsId != 0 || receipt.destinationW != 0 ||
        receipt.destinationH != 0 || receipt.assetWidth != 0 ||
        receipt.assetHeight != 0) {
        return 1;
    }

    /* The test supplies only an asset-loader slot. The exercised calls use
     * the production M11 material path and do not install a fallback image. */
    state.assetsAvailable = 1;
    state.assetLoader.initialized = 1;
    state.assetLoader.cacheUsed = M11_ASSET_CACHE_SLOTS;
    for (i = 0; i < M11_ASSET_CACHE_SLOTS; ++i) {
        state.assetLoader.cache[i].loaded = 1;
        state.assetLoader.cache[i].graphicIndex = (unsigned int)i;
        state.assetLoader.cache[i].width = 32;
        state.assetLoader.cache[i].height = 32;
        state.assetLoader.cache[i].pixels = sourcePixels;
    }
    if (!M11_GameView_ProbeDrawDm1FloorItemHostReceipt(
            &state, framebuffer, 320, 200)) {
        return 1;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (!receipt.valid || !receipt.floorItemLane ||
        !receipt.usesF0791Blit || receipt.transparentColor != 10 ||
        !M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1) ||
        !verify_pc34_d2_pixels(framebuffer, sourcePixels, &receipt)) {
        fprintf(stderr, "receipt scale=%d size=%dx%d asset=%dx%d\n",
                receipt.sourceScaleIndex, receipt.destinationW,
                receipt.destinationH, receipt.assetWidth, receipt.assetHeight);
        fprintf(stderr, "PC34 D2 material output differs from F0129 sampling/palette\n");
        return 1;
    }

    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (!M11_GameView_ProbeDrawDm1AlcoveItemForFloorItemReceipt(
            &state, framebuffer, 320, 200)) {
        return 1;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (receipt.valid || receipt.floorItemLane ||
        M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1)) {
        return 1;
    }
    M11_GameView_GetDm1AlcoveItemHostPresentationReceipt(&alcoveReceipt);
    if (!alcoveReceipt.valid || alcoveReceipt.floorItemLane ||
        !alcoveReceipt.usesF0791Blit || alcoveReceipt.transparentColor != 10 ||
        alcoveReceipt.sourceZone < 2548 || alcoveReceipt.destinationW <= 0 ||
        alcoveReceipt.destinationH <= 0) {
        return 1;
    }

    state.assetLoader.cacheUsed = 0;
    state.assetLoader.initialized = 0;
    state.assetsAvailable = 0;
    M11_GameView_Shutdown(&state);
    puts("ok: M11 keeps F0115 floor and alcove host receipts separate");
    return 0;
}
