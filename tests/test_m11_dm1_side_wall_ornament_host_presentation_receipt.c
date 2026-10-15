#include "m11_game_view.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* pc34_graphics_path(void)
{
    static char path[2048];
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    if (!root || !root[0]) return NULL;
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    return path;
}

static int check_original_d2_pixels(M11_GameViewState* state)
{
    unsigned char framebuffer[320 * 200];
    int right, flip;
    DM1_ViewportLaneVisibilityReceiptPc34 visibility =
        dm1_viewport_3d_lane_visibility_from_cells_pc34(NULL, NULL, NULL, NULL, NULL);
    for (right = 0; right < 2; ++right) {
        for (flip = 0; flip < 2; ++flip) {
            DM1_ViewportSideWallHostReceiptPc34 receipt;
            const M11_AssetSlot* source;
            int x, y, opaque = 0, transparent = 0;
            const int left = right ? 146 : 0;
            if (!dm1_viewport_3d_build_side_wall_host_receipt_pc34(
                    right ? DM1_VIEW_SQUARE_D2R : DM1_VIEW_SQUARE_D2L,
                    0, flip != 0, true, false, 3, &visibility, &receipt)) return 0;
            source = M11_AssetLoader_Load(&state->assetLoader,
                                          (unsigned int)receipt.material.graphic_index);
            if (!source || source->width != 78 || source->height != 74) return 0;
            memset(framebuffer, 0xe7, sizeof(framebuffer));
            if (!M11_GameView_ProbeDrawDm1SideWallHostReceipt(
                    state, &receipt, framebuffer, 320, 200)) return 0;
            /* Independent original C696 oracle: C710 left0/y19, C711
             * right223/y19, native78x74. Check the entire destination,
             * including all four edges and C10 holes, against decoded
             * original pixels. The old75x71 crop cannot satisfy this. */
            for (y = 0; y < 200; ++y) {
                for (x = 0; x < 320; ++x) {
                    unsigned char expected = 0xe7;
                    if (x >= left && x < left + 78 && y >= 33 + 19 && y < 33 + 19 + 74) {
                        const int sx = flip ? 77 - (x - left) : x - left;
                        unsigned char pixel = source->pixels[(y - 33 - 19) * 78 + sx];
                        if (pixel != 10) { expected = pixel; ++opaque; }
                        else ++transparent;
                    }
                    if (framebuffer[y * 320 + x] != expected) {
                        fprintf(stderr, "D2%s flip%d pixel%d,%d got%u expected%u\n",
                            right ? "R" : "L", flip, x, y,
                            framebuffer[y * 320 + x], expected);
                        return 0;
                    }
                }
            }
            if (!opaque || !transparent) return 0;
            printf("PASS: original D2%s flip=%d rect=(%d,19,78,74) opaque=%d C10holes=%d\n",
                right ? "R" : "L", flip, left, opaque, transparent);
        }
    }
    return 1;
}

int main(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    M11_Dm1WallOrnamentHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!graphicsPath) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    if (!asset_file_matches_md5(graphicsPath, "fa6b1aa29e191418713bf2cda93d962e")) {
        fprintf(stderr, "original I34E graphics identity mismatch\n");
        return 1;
    }
    M11_GameView_Init(&state);
    if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        M11_GameView_Shutdown(&state);
        fprintf(stderr, "configured PC34 GRAPHICS.DAT failed to load\n");
        return 1;
    }
    state.assetsAvailable = 1;
    state.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    strcpy(state.sourceId, "dm1");
    if (!check_original_d2_pixels(&state)) {
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    if (!M11_GameView_ProbeDrawDm1SideWallOrnamentHostReceipt(
            &state, framebuffer, 320, 200)) {
        fprintf(stderr, "real PC34 side-wall ornament material draw failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(&receipt);
    if (!receipt.valid || receipt.globalOrnamentIndex != 1 ||
        receipt.viewWallIndex != 1 || receipt.graphicIndex != 261 ||
        receipt.transparentColor != 10 || !receipt.flipHorizontal ||
        !receipt.paletteMapValid || receipt.paletteMap[2] != 12 ||
        receipt.width <= 0 || receipt.height <= 0) {
        fprintf(stderr, "real PC34 side-wall ornament receipt incomplete\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 F0107 side-wall ornament host material receipt");
    return 0;
}
