#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"

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

/* The archive is original A20E media; production startup supplies both the
 * dungeon map/floor-set and its admitted graphics loader. No fixture map or
 * reconstructed raster is used. */
static int check_original_amiga_background(void)
{
    const char* archive = getenv("FIRESTAFF_DM1_AMIGA_BACKGROUND_ARCHIVE");
    M11_GameViewState state;
    unsigned char framebuffer[320 * 200];
    const M11_AssetSlot *floor, *ceiling;
    int parity, x, y, ok = 0, index;
    M12_AssetStatus* scan;
    const M12_AssetVersionStatus* version;
    M11_GameLaunchSpec spec = {0};
    char runtime[M12_ASSET_DATA_DIR_CAPACITY];
    if (!archive || !archive[0]) return 1;
    scan = calloc(1, sizeof(*scan));
    if (!scan) return 0;
    M11_GameView_Init(&state);
    M12_AssetStatus_ScanGame(scan, archive, "dm1");
    index = M12_AssetStatus_FindVersionIndex("dm1", "amiga20-en");
    version = index < 0 ? NULL : M12_AssetStatus_GetVersion(scan, "dm1", (size_t)index);
    if (!version || !version->matched ||
        strcmp(version->matchedMd5, "6a2f135b53c2220f0251fa103e2a6e7e") ||
        !M12_AssetStatus_PrepareDM1RuntimeVersion(scan, "amiga20-en", runtime, sizeof(runtime))) goto done;
    spec.title = "Dungeon Master";
    spec.gameId = spec.sourceId = "dm1";
    spec.dataDir = runtime;
    spec.verifiedAssetPath = version->matchedPath;
    spec.verifiedAssetMd5 = version->matchedMd5;
    if (!M11_GameView_Start(&state, &spec) || !state.assetsAvailable ||
        !state.assetLoader.legacyDm1 || !state.assetLoader.legacyBigEndian ||
        !state.world.dungeon || !state.world.dungeon->maps ||
        state.world.dungeon->maps[state.world.party.mapIndex].floorSet != 0 ||
        !asset_file_matches_md5(state.assetLoader.graphicsDatPath,
                               "6a2f135b53c2220f0251fa103e2a6e7e")) goto done;
    floor = M11_AssetLoader_Load(&state.assetLoader, 75u);
    ceiling = M11_AssetLoader_Load(&state.assetLoader, 76u);
    if (!floor || !ceiling || floor->width != 224 || floor->height != 70 ||
        ceiling->width != 224 || ceiling->height != 29) goto done;
    /* The original78/79 are door frames, not an alternate floor pair.
     * Their real dimensions cannot satisfy the complete75/76 raster oracle. */
    for (x = 78; x <= 79; ++x) {
        const M11_AssetSlot* wrong = M11_AssetLoader_Load(&state.assetLoader, (unsigned)x);
        if (!wrong || !wrong->pixels ||
            (wrong->width == 224 && (wrong->height == 70 || wrong->height == 29))) goto done;
    }
    for (parity = 0; parity < 2; ++parity) {
        state.world.party.mapX = parity;
        state.world.party.mapY = state.world.party.direction = 0;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_ProbeDrawDm1ViewportBackground(&state, framebuffer, 320, 200);
        for (y = 0; y < 200; ++y) for (x = 0; x < 320; ++x) {
            unsigned char expected = 0;
            if (x < 224 && y >= 33 && y < 33 + 29) {
                const int sx = parity ? x : 223 - x;
                expected = ceiling->pixels[(y - 33) * 224 + sx];
            } else if (x < 224 && y >= 33 + 66 && y < 33 + 136) {
                const int sx = parity ? 223 - x : x;
                expected = floor->pixels[(y - 33 - 66) * 224 + sx];
            }
            if (framebuffer[y * 320 + x] != expected) {
                fprintf(stderr, "Amiga F0098 parity%d pixel%d,%d got%u expected%u\n",
                    parity, x, y, framebuffer[y * 320 + x], expected);
                goto done;
            }
        }
        printf("PASS: original A20E F0098 parity%d floor75=224x70 ceiling76=224x29\n", parity);
    }
    ok = 1;
done:
    M11_GameView_Shutdown(&state);
    free(scan);
    if (!ok) fprintf(stderr, "original A20E background oracle failed\n");
    return ok;
}

/* Original MEDIA009 C013, not a generated arrow raster. Optional so the
 * ordinary PC-only test keeps its existing corpus requirement. */
static unsigned int source_pixel_hash(const M11_AssetSlot* slot)
{
    unsigned int hash=2166136261u;
    for (size_t i=0; i<(size_t)slot->width*slot->height; ++i) hash=(hash^slot->pixels[i])*16777619u;
    return hash;
}

/* Independent G0163/F0096 oracle, using only pixels decoded from originals.
 * DUNVIEW.C:581-594 and2389-2408 specify padded source strides and crops. */
static int check_original_legacy_walls(M11_GameViewState* state, const char* edition)
{
    static const struct {
        DM1_ViewSquareIndex square; int record, stride, height, left, top, width, sourceX; int center;
    } cases[] = {
        {DM1_VIEW_SQUARE_D3L,88,128,51,0,25,84,32,0},
        {DM1_VIEW_SQUARE_D3C,88,128,51,74,25,76,18,1},
        {DM1_VIEW_SQUARE_D3R,88,128,51,139,25,85,0,0},
        {DM1_VIEW_SQUARE_D2L,87,144,71,0,20,75,61,0},
        {DM1_VIEW_SQUARE_D2C,87,144,71,60,20,104,16,1},
        {DM1_VIEW_SQUARE_D2R,87,144,71,149,20,75,0,0},
        {DM1_VIEW_SQUARE_D1L,86,256,111,0,9,64,192,0},
        {DM1_VIEW_SQUARE_D1C,86,256,111,32,9,160,48,1},
        {DM1_VIEW_SQUARE_D1R,86,256,111,160,9,64,0,0},
        {DM1_VIEW_SQUARE_D0L,85,32,136,0,0,32,0,0},
        {DM1_VIEW_SQUARE_D0R,84,32,136,192,0,32,0,0}
    };
    const M11_AssetSlot* originals[5];
    unsigned int hashes[5];
    unsigned char framebuffer[320*200];
    int mapX=state->world.party.mapX, mapY=state->world.party.mapY;
    int direction=state->world.party.direction;
    int wallSet=state->world.dungeon->maps[state->world.party.mapIndex].wallSet;
    for (int i=0;i<5;++i) {
        originals[i]=M11_AssetLoader_Load(&state->assetLoader,(unsigned int)(84+i+wallSet*13));
        if (!originals[i] || !originals[i]->pixels) return 0;
        hashes[i]=source_pixel_hash(originals[i]);
    }
    /* The preceding full Draw may already own the real cache. Release it
     * before deliberately exercising a failed-allocation capability. */
    free(state->dm1LegacyWallCache);
    state->dm1LegacyWallCache = NULL;
    for (int capability=0;capability<2;++capability) {
        /* Deliberately exercise the allocation-failure/native capability
         * state. This changes ownership policy, never original media bytes. */
        state->dm1LegacyWallCacheCapability=capability ? 0 : -1;
        for (int parity=0;parity<2;++parity) for (size_t c=0;c<sizeof(cases)/sizeof(cases[0]);++c) {
            const int flipped=capability && parity;
            int record=cases[c].record;
            const M11_AssetSlot* source;
            state->world.party.mapX=parity;
            state->world.party.mapY=state->world.party.direction=0;
            if (flipped && record==84) record=85;
            else if (flipped && record==85) record=84;
            source=originals[record-84];
            if (source->width!=cases[c].stride || source->height!=cases[c].height) return 0;
            memset(framebuffer,0xe7,sizeof(framebuffer));
            if (!M11_GameView_ProbeDrawDm1LegacyWall(state,cases[c].square,framebuffer,320,200) ||
                state->dm1LegacyWallCacheCapability!=(capability ? 1 : -1)) return 0;
            for (int y=0;y<200;++y) for (int x=0;x<320;++x) {
                unsigned char expected=0xe7;
                if (x>=cases[c].left && x<cases[c].left+cases[c].width &&
                    y>=33+cases[c].top && y<33+cases[c].top+cases[c].height) {
                    int sx=cases[c].sourceX+x-cases[c].left;
                    const int sy=y-33-cases[c].top;
                    unsigned char pixel;
                    if (flipped) {
                        int content=record==88 ? 116 : record==87 ? 136 : cases[c].stride;
                        int shift=record==88 ? 11 : record==87 ? 8 : 0;
                        pixel=sx>=content ? 10 : source->pixels[sy*cases[c].stride+cases[c].stride-1-sx-shift];
                    } else pixel=source->pixels[sy*cases[c].stride+sx];
                    if (cases[c].center || pixel!=10) expected=pixel;
                }
                if (framebuffer[y*320+x]!=expected) {
                    fprintf(stderr,"%s legacy wall capability%d parity%d case%zu pixel%d,%d got%u expected%u\n",
                        edition,capability,parity,c,x,y,framebuffer[y*320+x],expected);
                    return 0;
                }
            }
        }
        printf("PASS: original %s MEDIA009 capability%d eleven full wall rasters both parities\n",edition,capability);
    }
    for (int i=0;i<5;++i) if (source_pixel_hash(originals[i])!=hashes[i]) return 0;
    state->world.party.mapX=mapX;
    state->world.party.mapY=mapY;
    state->world.party.direction=direction;
    return 1;
}

static int check_original_legacy_arrows(const char *env_name,
                                        const char *edition,
                                        const char *graphics_md5,
                                        int atari)
{
    const char *path = getenv(env_name);
    const M12_AssetVersionStatus *version;
    M12_AssetStatus *scan;
    M11_GameViewState *state;
    M11_GameLaunchSpec spec = {0};
    const M11_AssetSlot *arrows;
    unsigned char framebuffer[320 * 200];
    char runtime[M12_ASSET_DATA_DIR_CAPACITY];
    int index, result = 0, nonblack = 0;
    if (!path || !path[0]) return 1;
    scan = calloc(1, sizeof(*scan));
    state = calloc(1, sizeof(*state));
    if (!scan || !state) { free(scan); free(state); return 0; }
    M11_GameView_Init(state);
    M12_AssetStatus_ScanGame(scan, path, "dm1");
    index = M12_AssetStatus_FindVersionIndex("dm1", edition);
    version = index < 0 ? NULL :
        M12_AssetStatus_GetVersion(scan, "dm1", (size_t)index);
    if (!version || !version->matched ||
        strcmp(version->matchedMd5, graphics_md5) ||
        !M12_AssetStatus_PrepareDM1RuntimeVersion(scan, edition,
                                               runtime, sizeof(runtime))) goto done;
    spec.title = "Dungeon Master";
    spec.gameId = "dm1";
    spec.sourceId = "dm1";
    spec.dataDir = runtime;
    spec.verifiedAssetPath = version->matchedPath;
    spec.verifiedAssetMd5 = version->matchedMd5;
    if (!M11_GameView_Start(state, &spec) ||
        (atari ? !state->assetLoader.atariStDm1 :
         (!state->assetLoader.legacyDm1 || !state->assetLoader.legacyBigEndian)))
        goto done;
    arrows = M11_AssetLoader_Load(&state->assetLoader, 13);
    if (!arrows || !arrows->loaded || !arrows->pixels ||
        arrows->width != 96 || arrows->height != 45) goto done;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(state, framebuffer, 320, 200);
    for (int y = 0; y < 45; ++y) {
        for (int x = 0; x < 96; ++x) {
            unsigned char source = arrows->pixels[y * 96 + x] & 15;
            if ((framebuffer[(124 + y) * 320 + 224 + x] & 15) != source)
                goto done;
            nonblack += source != 0;
        }
    }
    if (!nonblack) goto done;
    if (!check_original_legacy_walls(state,edition)) goto done;
    printf("PASS: original %s C013 matches all 4320 presented indices\n", edition);
    {
        static const int turn_x[4] = { 257, 243, 314, 300 };
        for (int click = 0; click < 4; ++click) {
            int direction, expected;
            M11_GameView_Shutdown(state);
            M11_GameView_Init(state);
            if (!M11_GameView_Start(state, &spec)) goto done;
            direction = state->world.party.direction;
            expected = (direction + (click < 2 ? 3 : 1)) & 3;
            if (M11_GameView_HandlePointer(state, turn_x[click], 135, 1) !=
                    M11_GAME_INPUT_REDRAW ||
                state->world.party.direction != expected) {
                fprintf(stderr, "FAIL: %s turn click at %d,135 direction %d expected %d\n",
                    edition, turn_x[click], state->world.party.direction, expected);
                goto done;
            }
        }
        printf("PASS: original %s native turn edges and centers change direction\n", edition);
    }
    result = 1;
done:
    if (!result) fprintf(stderr, "FAIL: original %s native movement arrows\n", edition);
    M11_GameView_Shutdown(state);
    free(state);
    free(scan);
    return result;
}

int main(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    M11_Dm1WallOrnamentHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!check_original_amiga_background()) return 1;
    if (!check_original_legacy_arrows("FIRESTAFF_DM1_AMIGA_DEMO_ARROWS_ARCHIVE",
            "amiga-demo-en", "491ca939f9abb33ceeb26619b841fe91", 0) ||
        !check_original_legacy_arrows("FIRESTAFF_DM1_ATARI_ARROWS_ARCHIVE",
            "st12-en", "9ce2eaf7a9e78620e3f17594437caffa", 1)) return 1;
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
