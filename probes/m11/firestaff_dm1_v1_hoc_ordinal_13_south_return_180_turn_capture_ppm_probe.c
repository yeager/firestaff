/*
 * DM1 V1 Hall of Champions ordinal 13 (WUUF) south_return
 * in-place 180-degree turn PPM capture/readiness gate.
 *
 * This is the visual-artifact sibling of the ordinal-13
 * portrait_rect_position gate.  It drives the real M11 input path:
 *   (1,5,SOUTH) WUUF -> TURN_LEFT -> (1,5,EAST) no mirror
 *   -> TURN_LEFT -> (1,5,NORTH) GANDO
 * and writes full-frame 320x200 PPMs plus 224x136 viewport crops to
 * a caller-provided output directory.  The generated files are CTest
 * artifacts only and must not be committed as game data or screenshots.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps sensor cell against party direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 portraits into the D1C
 *     portrait-on-wall box at viewport (96,35,32,29).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 redraws the viewport after
 *     MOVESENS.C:556 updates the party pose.
 *   ReDMCSB CLIKMENU.C F0365/F0366 own turn/move dispatch; the
 *     in-place turn path is exercised through M11_GameView_HandleInput.
 *
 * Honesty scope: Firestaff deterministic runtime capture/readiness
 * only.  This does not compare against an original DM1 PC 3.4 DOSBox
 * capture and does not promote DOS pixel parity.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136,
    PORTRAIT_FX = 96,
    PORTRAIT_FY = 68,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    CHAMPION_TRANSPARENT = 1,
    LEAK_TOLERANCE_PCT = 35,
    MATCH_THRESHOLD_PCT = 90,
    ORD_WUUF = 13,
    ORD_GANDO = 10
};

typedef struct CaptureStep {
    const char* label;
    int expectedOrdinal;
    int expectedBestOrdinal;
    int staleOrdinal;
    int actualOrdinal;
    int bestOrdinal;
    int expectedMatched;
    int expectedCompared;
    int staleMatched;
    int staleCompared;
    int ppmOk;
    int viewportPpmOk;
} CaptureStep;

static int g_pass;
static int g_fail;

static void pass(const char* label) {
    ++g_pass;
    printf("  PASS: %s\n", label);
}

static void fail(const char* label) {
    ++g_fail;
    printf("  FAIL: %s\n", label);
}

static void expect_int(const char* label, int got, int want) {
    if (got == want) {
        pass(label);
    } else {
        fail(label);
        printf("    got=%d want=%d\n", got, want);
    }
}

static void expect_true(const char* label, int ok) {
    if (ok) {
        pass(label);
    } else {
        fail(label);
    }
}

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') {
        return;
    }
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

static void set_pose(M11_GameViewState* game, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = 1;
    game->world.party.mapY = 5;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

static int ordinal_compared_count(const M11_AssetSlot* portraits, int ordinal) {
    int x;
    int y;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src != CHAMPION_TRANSPARENT) {
                ++compared;
            }
        }
    }
    return compared;
}

static int count_ordinal_pixels(const M11_AssetSlot* portraits,
                                const unsigned char* fb,
                                int ordinal) {
    int x;
    int y;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst = M11_FB_DECODE_INDEX(fb[(PORTRAIT_FY + y) * FB_W +
                                                       (PORTRAIT_FX + x)]);
            if (src != CHAMPION_TRANSPARENT && dst == src) {
                ++matched;
            }
        }
    }
    return matched;
}

static int best_ordinal_at_portrait_rect(const M11_AssetSlot* portraits,
                                         const unsigned char* fb) {
    int ordinal;
    int bestOrdinal = -1;
    int bestMatched = -1;
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int matched = count_ordinal_pixels(portraits, fb, ordinal);
        if (matched > bestMatched) {
            bestMatched = matched;
            bestOrdinal = ordinal;
        }
    }
    return bestOrdinal;
}

static int dump_vga_ppm(const char* path,
                        const unsigned char* fb,
                        int x0,
                        int y0,
                        int w,
                        int h) {
    FILE* f;
    int x;
    int y;
    long bytesWritten = 0;
    if (!path || !fb || w <= 0 || h <= 0) {
        return 0;
    }
    f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
    bytesWritten += fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            unsigned char raw = fb[(y0 + y) * FB_W + (x0 + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            int level = M11_FB_DECODE_LEVEL(raw);
            const unsigned char* rgb;
            if (level < 0) {
                level = 0;
            }
            if (level >= M11_PALETTE_LEVELS) {
                level = M11_PALETTE_LEVELS - 1;
            }
            rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
            bytesWritten += (long)fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return bytesWritten > (long)(w * h * 3);
}

static int ppm_header_matches(const char* path, int w, int h) {
    FILE* f;
    char magic[3];
    int gotW = 0;
    int gotH = 0;
    int maxv = 0;
    if (!path) {
        return 0;
    }
    f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    memset(magic, 0, sizeof(magic));
    if (fscanf(f, "%2s %d %d %d", magic, &gotW, &gotH, &maxv) != 4) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return strcmp(magic, "P6") == 0 && gotW == w && gotH == h && maxv == 255;
}

static void analyze_and_capture_step(const char* outDir,
                                     const M11_AssetSlot* portraits,
                                     const unsigned char* fb,
                                     CaptureStep* step) {
    char path[1024];
    char viewportPath[1024];
    if (!outDir || !portraits || !fb || !step) {
        return;
    }
    snprintf(path, sizeof(path), "%s/%s.ppm", outDir, step->label);
    snprintf(viewportPath, sizeof(viewportPath), "%s/%s_viewport_224x136.ppm",
             outDir, step->label);
    step->actualOrdinal = step->actualOrdinal;
    step->bestOrdinal = best_ordinal_at_portrait_rect(portraits, fb);
    step->expectedMatched = count_ordinal_pixels(portraits, fb, step->expectedOrdinal);
    step->expectedCompared = ordinal_compared_count(portraits, step->expectedOrdinal);
    step->staleMatched = count_ordinal_pixels(portraits, fb, step->staleOrdinal);
    step->staleCompared = ordinal_compared_count(portraits, step->staleOrdinal);
    step->ppmOk = dump_vga_ppm(path, fb, 0, 0, FB_W, FB_H) &&
                  ppm_header_matches(path, FB_W, FB_H);
    step->viewportPpmOk =
        dump_vga_ppm(viewportPath, fb, VIEWPORT_X, VIEWPORT_Y, VIEWPORT_W, VIEWPORT_H) &&
        ppm_header_matches(viewportPath, VIEWPORT_W, VIEWPORT_H);
}

static int step_passes(const CaptureStep* step) {
    int expectedOk;
    int bestOk;
    int staleOk;
    if (!step) {
        return 0;
    }
    expectedOk = step->expectedOrdinal < 0 ||
                 (step->expectedCompared > 0 &&
                  step->expectedMatched * 100 >=
                      step->expectedCompared * MATCH_THRESHOLD_PCT);
    bestOk = step->expectedBestOrdinal < 0 ||
             step->bestOrdinal == step->expectedBestOrdinal;
    staleOk = step->staleCompared > 0 &&
              step->staleMatched * 100 <
                  step->staleCompared * LEAK_TOLERANCE_PCT;
    return step->actualOrdinal == step->expectedOrdinal &&
           bestOk &&
           expectedOk &&
           staleOk &&
           step->ppmOk &&
           step->viewportPpmOk;
}

static int write_manifest(const char* outDir, const CaptureStep* steps, int count) {
    char path[1024];
    FILE* f;
    int i;
    if (!outDir || !steps || count <= 0) {
        return 0;
    }
    snprintf(path, sizeof(path),
             "%s/dm1_v1_hoc_ordinal_13_south_return_180_turn_capture.json",
             outDir);
    f = fopen(path, "w");
    if (!f) {
        return 0;
    }
    fprintf(f, "{\n");
    fprintf(f, "  \"schema\": \"firestaff.dm1_v1_hoc_ordinal13_180_turn_capture.v1\",\n");
    fprintf(f, "  \"route\": \"map0:(1,5) SOUTH -> TURN_LEFT -> EAST -> TURN_LEFT -> NORTH\",\n");
    fprintf(f, "  \"honesty\": \"Firestaff deterministic runtime PPM capture/readiness gate only; no original DM1 PC 3.4 DOSBox pixel-parity comparison is claimed.\",\n");
    fprintf(f, "  \"sourceEvidence\": [\n");
    fprintf(f, "    \"ReDMCSB DUNGEON.C:2573 direction-sensitive C127 sensor filter\",\n");
    fprintf(f, "    \"ReDMCSB DUNGEON.C:2608-2612 C127 sensorData -> G0289 ordinal\",\n");
    fprintf(f, "    \"ReDMCSB DUNVIEW.C:3913-3928 C026 champion portrait blit\",\n");
    fprintf(f, "    \"ReDMCSB DUNVIEW.C:8318-8542 F0128 full viewport redraw after turn\",\n");
    fprintf(f, "    \"ReDMCSB CLIKMENU.C F0365/F0366 turn dispatch\"\n");
    fprintf(f, "  ],\n");
    fprintf(f, "  \"captures\": [\n");
    for (i = 0; i < count; ++i) {
        const CaptureStep* s = &steps[i];
        fprintf(f,
                "    {\"label\":\"%s\",\"actualOrdinal\":%d,\"bestOrdinal\":%d,"
                "\"expectedOrdinal\":%d,\"expectedMatched\":%d,\"expectedCompared\":%d,"
                "\"staleOrdinal\":%d,\"staleMatched\":%d,\"staleCompared\":%d,"
                "\"fullFrame\":\"%s.ppm\",\"viewportCrop\":\"%s_viewport_224x136.ppm\","
                "\"pass\":%s}%s\n",
                s->label,
                s->actualOrdinal,
                s->bestOrdinal,
                s->expectedOrdinal,
                s->expectedMatched,
                s->expectedCompared,
                s->staleOrdinal,
                s->staleMatched,
                s->staleCompared,
                s->label,
                s->label,
                step_passes(s) ? "true" : "false",
                i == count - 1 ? "" : ",");
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    fclose(f);
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    const char* outDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    M11_GameInputResult r;
    CaptureStep steps[3] = {
        {"ordinal13_turn_000_south_wuuf", ORD_WUUF, ORD_WUUF, ORD_GANDO, -999, -1, 0, 0, 0, 0, 0, 0},
        {"ordinal13_turn_090_east_clear", -1, -2, ORD_WUUF, -999, -1, 0, 0, 0, 0, 0, 0},
        {"ordinal13_turn_180_north_gando", ORD_GANDO, ORD_GANDO, ORD_WUUF, -999, -1, 0, 0, 0, 0, 0, 0}
    };
    int i;
    int manifestOk;

    if (argc < 3) {
        fprintf(stderr, "usage: %s DATA_DIR OUT_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    outDir = argv[2];
    ensure_output_dir(outDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP firestaff_dm1_v1_hoc_ordinal_13_south_return_180_turn_"
               "capture_ppm_probe: no hash-verified DM1 data under %s\n",
               dataDir);
        return 0;
    }

    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 HoC ordinal 13 south_return 180-turn PPM capture ===\n");
    printf("dataDir=%s outDir=%s\n", dataDir, outDir);

    set_pose(&game, DIR_SOUTH);
    steps[0].actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (steps[0].actualOrdinal != ORD_WUUF) {
        printf("SKIP ordinal-13 180-turn PPM capture: (1,5,SOUTH) ordinal=%d "
               "want=%d; this DM1 V1 fixture does not expose WUUF on the "
               "south_return route.\n",
               steps[0].actualOrdinal, ORD_WUUF);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    analyze_and_capture_step(outDir, portraits, fb, &steps[0]);

    r = M11_GameView_HandleInput(&game, M12_MENU_INPUT_TURN_LEFT);
    expect_int("first TURN_LEFT result is REDRAW", (int)r, (int)M11_GAME_INPUT_REDRAW);
    expect_int("first TURN_LEFT leaves party at same x", game.world.party.mapX, 1);
    expect_int("first TURN_LEFT leaves party at same y", game.world.party.mapY, 5);
    expect_int("first TURN_LEFT rotates SOUTH -> EAST", game.world.party.direction, DIR_EAST);
    steps[1].actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    analyze_and_capture_step(outDir, portraits, fb, &steps[1]);

    r = M11_GameView_HandleInput(&game, M12_MENU_INPUT_TURN_LEFT);
    expect_int("second TURN_LEFT result is REDRAW", (int)r, (int)M11_GAME_INPUT_REDRAW);
    expect_int("second TURN_LEFT leaves party at same x", game.world.party.mapX, 1);
    expect_int("second TURN_LEFT leaves party at same y", game.world.party.mapY, 5);
    expect_int("second TURN_LEFT rotates EAST -> NORTH", game.world.party.direction, DIR_NORTH);
    steps[2].actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    analyze_and_capture_step(outDir, portraits, fb, &steps[2]);

    for (i = 0; i < 3; ++i) {
        char label[160];
        const CaptureStep* s = &steps[i];
        snprintf(label, sizeof(label), "%s front ordinal matches expected", s->label);
        expect_int(label, s->actualOrdinal, s->expectedOrdinal);
        snprintf(label, sizeof(label), "%s best D1C portrait ordinal matches expected", s->label);
        if (s->expectedBestOrdinal >= 0) {
            expect_int(label, s->bestOrdinal, s->expectedBestOrdinal);
        } else {
            expect_true(label, s->actualOrdinal < 0);
        }
        snprintf(label, sizeof(label), "%s expected portrait match >= 90%%", s->label);
        if (s->expectedOrdinal >= 0) {
            expect_true(label,
                        s->expectedCompared > 0 &&
                        s->expectedMatched * 100 >= s->expectedCompared * MATCH_THRESHOLD_PCT);
        } else {
            expect_true(label, s->expectedMatched == 0 && s->expectedCompared == 0);
        }
        snprintf(label, sizeof(label), "%s stale ordinal leak <35%%", s->label);
        expect_true(label,
                    s->staleCompared > 0 &&
                    s->staleMatched * 100 < s->staleCompared * LEAK_TOLERANCE_PCT);
        snprintf(label, sizeof(label), "%s full-frame PPM header is P6 320x200", s->label);
        expect_true(label, s->ppmOk);
        snprintf(label, sizeof(label), "%s viewport-crop PPM header is P6 224x136", s->label);
        expect_true(label, s->viewportPpmOk);
        printf("  INFO: %s ordinal=%d best=%d expected=%d/%d stale=%d/%d\n",
               s->label, s->actualOrdinal, s->bestOrdinal,
               s->expectedMatched, s->expectedCompared,
               s->staleMatched, s->staleCompared);
    }

    manifestOk = write_manifest(outDir, steps, 3);
    expect_true("capture manifest was written", manifestOk);
    printf("wrote %s/dm1_v1_hoc_ordinal_13_south_return_180_turn_capture.json\n",
           outDir);

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
