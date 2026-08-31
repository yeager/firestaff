#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static int open_game(const char* dataPath, M11_GameViewState* game)
{
    M11_GameView_Init(game);
    /* This test is specifically PC 3.4 F0107/C127 material coverage.  A
     * generic menu scan ranks any valid DM1 package and can therefore select
     * an Atari ST archive before the explicit PC 3.4 path.  Launch the
     * selected original archive directly; startup-menu selection is covered
     * by its separate real-media tests. */
    return M11_GameView_StartDm1(game, dataPath);
}

static int expected_backing_graphic_for_f0107_view(int viewWallIndex)
{
    /* DUNVIEW.C:805-819 G0190_auc_Graphic558_WallOrnamentDerivedBitmapIndexIncrement
     * for MEDIA720 (PC34/I34E) is a 14-entry table indexed by the F0107
     * view-wall index:
     *   D3L2/D3R2/D3L/D3R = 0
     *   D3L/D3R front     = 0
     *   D2L right / D2R left = 1
     *   D2L/D2R front     = 1 / 1 / 2  (view 5..9 = 1,1,1,2,2)
     *   D1L/D1R depth     = 3
     *   D1L/D1R front     = 3, 4, 4
     * For the C346 champion-mirror route the source blit selects
     *   backingGraphicIndex = 345 + G0190[view]
     * The prior 345/346-only formula was a stale two-value approximation
     * that flagged every viewIndex above the D3 side pair as wrong. */
    static const int increments[14] = {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4
    };
    if (viewWallIndex < 0 || viewWallIndex >= 14) return 345;
    return 345 + increments[viewWallIndex];
}

int main(int argc, char** argv)
{
    M11_GameViewState game;
    const struct DungeonMapDesc_Compat* map;
    unsigned char framebuffer[320 * 200];
    int sideCount = 0;
    int depthBackingCount = 0;
    int clearedAfterMaterial = 0;
    int previousHadMaterial = 0;
    int ok = 1;
    int x;
    int y;
    int direction;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    if (!open_game(argv[1], &game) || !game.world.dungeon ||
        game.world.dungeon->header.mapCount < 1 || !game.assetsAvailable) {
        fprintf(stderr, "FAIL could not open real DM1 PC34 data\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    map = &game.world.dungeon->maps[0];
    for (x = 0; x < (int)map->width; ++x) {
        for (y = 0; y < (int)map->height; ++y) {
            for (direction = 0; direction < 4; ++direction) {
                M11_Dm1HoCMirrorViewportMaterialFrameReceipt frame;
                int frameHadMaterial = 0;
                int i;

                game.world.party.mapIndex = 0;
                game.world.party.mapX = x;
                game.world.party.mapY = y;
                game.world.party.direction = direction;
                memset(framebuffer, 0, sizeof(framebuffer));
                M11_GameView_Draw(&game, framebuffer, 320, 200);
                memset(&frame, 0, sizeof(frame));
                M11_GameView_GetDm1HoCMirrorViewportMaterialFrameReceipt(&frame);

                for (i = 0; i < frame.count; ++i) {
                    const M11_Dm1HoCMirrorViewportMaterialReceipt* entry =
                        &frame.entries[i];
                    if (!entry->valid || entry->renderIndex < 0 ||
                        entry->renderIndex >= game.mirrorCatalog.count ||
                        !entry->suppressHostFallbackVisuals) {
                        fprintf(stderr, "FAIL invalid C127 viewport material receipt\n");
                        ok = 0;
                        continue;
                    }
                    if (entry->relativeForward == 1 &&
                        entry->relativeSide == 0) {
                        /* D1C's C346/C026 decision depends on the separate
                         * live F0115 route. Its dedicated all-ordinal test
                         * owns that contract; this test covers F0107's
                         * side/depth C345/C346 materialization. */
                        continue;
                    } else if (entry->relativeForward >= 1) {
                        if (!entry->materialized ||
                            entry->backingGraphicIndex !=
                                expected_backing_graphic_for_f0107_view(
                                    entry->viewWallIndex) ||
                            entry->portraitGraphicIndex != -1 ||
                            !entry->suppressChampionPortrait) {
                            fprintf(stderr,
                                    "FAIL D%d C127 %d view=%d material=%d backing=%d portrait=%d suppress=%d\n",
                                    entry->relativeForward, entry->renderIndex,
                                    entry->viewWallIndex,
                                    entry->materialized,
                                    entry->backingGraphicIndex,
                                    entry->portraitGraphicIndex,
                                    entry->suppressChampionPortrait);
                            ok = 0;
                        }
                        if (entry->relativeForward == 1) ++sideCount;
                        else ++depthBackingCount;
                        frameHadMaterial = 1;
                    }
                }
                if (previousHadMaterial && frame.count == 0) {
                    clearedAfterMaterial = 1;
                }
                previousHadMaterial = frameHadMaterial;
            }
        }
    }

    if (sideCount <= 0 || depthBackingCount <= 0 || !clearedAfterMaterial) {
        fprintf(stderr,
                "FAIL C127 viewport coverage side=%d depth=%d clear=%d\n",
                sideCount, depthBackingCount, clearedAfterMaterial);
        ok = 0;
    }

    M11_GameView_Shutdown(&game);
    if (ok) {
        puts("ok: real HoC C127 F0107 C345/C346 side/depth material, no C026 fallback");
    }
    return ok ? 0 : 1;
}
