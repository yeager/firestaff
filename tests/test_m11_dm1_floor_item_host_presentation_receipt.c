#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    M11_GameViewState state;
    M11_Dm1FloorItemHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];
    unsigned char sourcePixels[32 * 32];
    int i;

    M11_GameView_Init(&state);
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(sourcePixels, 7, sizeof(sourcePixels));
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
        !M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1)) {
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

    state.assetLoader.cacheUsed = 0;
    state.assetLoader.initialized = 0;
    state.assetsAvailable = 0;
    M11_GameView_Shutdown(&state);
    puts("ok: M11 DM1 floor-item host receipt only accepts the F0115 floor lane");
    return 0;
}
