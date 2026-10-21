#include "m11_game_view.h"
#include "main_loop_m11.h"
#include "menu_startup_m12.h"
#include "dm1_v1_floor_pit_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "asset_loader_m11.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum { FB_W = 320, FB_H = 200, VIEWPORT_Y = 33, TRANSPARENT = 10 };

static int start_real_dm1(M11_GameViewState* state, const char* path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
        M12_StartupMenuState menu;
        int opened;
        M12_StartupMenu_InitWithDataDir(&menu, path, "dm1");
        if (!M11_PrepareDirectLaunchForGame(&menu, "dm1")) {
            M12_StartupMenu_Destroy(&menu);
            return 0;
        }
        opened = M11_GameView_OpenSelectedMenuEntry(state, &menu);
        M12_StartupMenu_Destroy(&menu);
        return opened;
    }
    return M11_GameView_StartDm1(state, path);
}

static int find_open_pit_pose(M11_GameViewState* state,
                              unsigned char* framebuffer,
                              DM1_FloorPitRenderPlanPc34* outPlan,
                              unsigned char* outSquare,
                              const M11_AssetSlot** outAsset)
{
    const struct DungeonDatState_Compat* dungeon = state->world.dungeon;
    static const int forwardX[4] = { 0, 1, 0, -1 };
    static const int forwardY[4] = { -1, 0, 1, 0 };
    int mapIndex;
    if (!dungeon || !dungeon->maps || !dungeon->tiles || !dungeon->tilesLoaded) return 0;

    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        const struct DungeonMapTiles_Compat* tiles = &dungeon->tiles[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                unsigned char square = tiles->squareData[x * map->height + y];
                int relForward;
                if (((square & DUNGEON_SQUARE_MASK_TYPE) >> 5) != DUNGEON_ELEMENT_PIT ||
                    !(square & DM1_V1_FLOOR_PIT_OPEN_MASK_PC34)) continue;
                for (relForward = 0; relForward <= 3; ++relForward) {
                    int relSide;
                    for (relSide = -2; relSide <= 2; ++relSide) {
                        DM1_FloorPitRenderPlanPc34 plan;
                        int direction;
                        if (!dm1_v1_floor_pit_render_plan_pc34(relForward, relSide, &plan)) continue;
                        for (direction = 0; direction < 4; ++direction) {
                            const int rightX = forwardY[direction];
                            const int rightY = -forwardX[direction];
                            const int partyX = x - relForward * forwardX[direction] - relSide * rightX;
                            const int partyY = y - relForward * forwardY[direction] - relSide * rightY;
                            int sampledX, sampledY, sampledElement;
                            unsigned char sampledSquare;
                            int graphic;
                            const DM1_FloorPitBlitPc34* blit;
                            const M11_AssetSlot* asset;
                            if (partyX < 0 || partyY < 0 || partyX >= map->width || partyY >= map->height) continue;
                            state->world.partyMapIndex = mapIndex;
                            state->world.newPartyMapIndex = mapIndex;
                            state->world.party.mapIndex = mapIndex;
                            state->world.party.mapX = partyX;
                            state->world.party.mapY = partyY;
                            state->world.party.direction = direction;
                            if (!M11_GameView_ProbeViewportCellClass(state, relForward, relSide,
                                    &sampledX, &sampledY, &sampledSquare, &sampledElement,
                                    NULL, NULL, NULL) || sampledX != x || sampledY != y ||
                                sampledElement != DUNGEON_ELEMENT_PIT ||
                                !(sampledSquare & DM1_V1_FLOOR_PIT_OPEN_MASK_PC34)) continue;
                            blit = dm1_v1_floor_pit_square_uses_invisible_pc34(sampledSquare) &&
                                   plan.hasInvisibleBlit ? &plan.invisibleBlit : &plan.visibleBlit;
                            graphic = blit->graphicIndex;
                            asset = M11_AssetLoader_Load(&state->assetLoader, (unsigned int)graphic);
                            if (!asset || !asset->loaded || !asset->pixels ||
                                blit->srcX < 0 || blit->srcY < 0 ||
                                blit->srcX + blit->width > asset->width ||
                                blit->srcY + blit->height > asset->height) continue;
                            memset(framebuffer, 0, FB_W * FB_H);
                            M11_GameView_Draw(state, framebuffer, FB_W, FB_H);
                            /* M11's bounded asset cache may evict the pit
                             * slot while composing the rest of the frame. */
                            asset = M11_AssetLoader_Load(&state->assetLoader,
                                                        (unsigned int)graphic);
                            if (!asset || !asset->loaded || !asset->pixels) continue;
                            *outPlan = plan;
                            *outSquare = sampledSquare;
                            *outAsset = asset;
                            printf("real PC34 open pit: map=%d party=(%d,%d,d%d) pit=(%d,%d) relative=(%d,%d) graphic=%d dst=(%d,%d) size=%dx%d\n",
                                   mapIndex, partyX, partyY, direction, x, y,
                                   relForward, relSide, graphic, blit->dstX,
                                   blit->dstY, blit->width, blit->height);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    unsigned char framebuffer[FB_W * FB_H];
    DM1_FloorPitRenderPlanPc34 plan;
    unsigned char square;
    const M11_AssetSlot* asset = NULL;
    const DM1_FloorPitBlitPc34* blit;
    int expected = 0, exact = 0, occluded = 0, y, x;
    M11_Dm1F0128PerSquareSchedulerReceipt scheduler;

    if (!dataDir || !dataDir[0]) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!start_real_dm1(&state, dataDir) || !state.assetsAvailable) {
        M11_GameView_Shutdown(&state);
        fputs("real PC34 corpus has no renderable open floor-pit pose\n", stderr);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    if (!find_open_pit_pose(&state, framebuffer, &plan, &square, &asset)) {
        M11_GameView_Shutdown(&state);
        fputs("real PC34 corpus has no renderable open floor-pit pose\n", stderr);
        return 1;
    }
    blit = dm1_v1_floor_pit_square_uses_invisible_pc34(square) && plan.hasInvisibleBlit
        ? &plan.invisibleBlit : &plan.visibleBlit;
    for (y = 0; y < blit->height; ++y) {
        for (x = 0; x < blit->width; ++x) {
            unsigned char source = asset->pixels[(blit->srcY + y) * asset->width + blit->srcX + x];
            unsigned char actual = framebuffer[(VIEWPORT_Y + blit->dstY + y) * FB_W + blit->dstX + x];
            if (source == TRANSPARENT) continue;
            ++expected;
            if (M11_FB_DECODE_INDEX(actual) == source) ++exact;
            else ++occluded;
        }
    }
    printf("pit source pixels: %d; source pixels in final palette-indexed frame: %d; occluded: %d\n",
           expected, exact, occluded);
    memset(&scheduler, 0, sizeof(scheduler));
    M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&scheduler);
    if (!scheduler.valid || !scheduler.planReady || !scheduler.sourcePlanDispatched ||
        scheduler.d0PrimitiveCallbackStepCount == 0) {
        fputs("F0128 did not execute the real D0 pit callback\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    if (expected == 0 || exact == 0) {
        fputs("open pit's original material did not reach the final runtime framebuffer\n", stderr);
        return 1;
    }
    puts("ok: original PC34 open-pit material reaches the M11 runtime framebuffer");
    return 0;
}
