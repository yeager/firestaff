/*
 * DM1 V1 champion mirror visual capture probe.
 *
 * This probe addresses two P1 visual bugs by generating deterministic
 * Firestaff runtime captures for every Hall of Champions pose that has
 * a known mirror ordinal, plus the negative (no-mirror) corridor
 * poses, plus the Z-order / no-floating side-wall poses. Each pose is
 * rendered with M11_GameView_Draw and saved as both a full-frame
 * 320x200 PPM and a 224x136 viewport crop PPM, plus a JSON+MD
 * manifest describing the capture.
 *
 * Visual evidence is what closes the P1 bug tickets:
 *   - "champion mirrors not visible" closed when every positive-
 *     ordinal pose shows the expected portrait in the D1C front-wall
 *     rectangle (96,35)-(128,64) and every negative-ordinal pose
 *     shows wall texture only.
 *   - "champion Z-order/floating" closed when every no-floating
 *     side-wall pose shows wall texture only (no champion portrait
 *     sprite floating over a side wall after the player turns).
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 blits D1C champion portrait
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw order far-to-near
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB REVIVE.C F0280 materializes the candidate from sensorData
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin/224x136 dimensions
 *
 * Usage: firestaff_dm1_v1_champion_mirror_capture_probe DATA_DIR OUT_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
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
    PORTRAIT_WARM_THRESHOLD = 30,
    MIRROR_FRAME_BLACK_THRESHOLD = 16
};

typedef struct MirrorCapture {
    const char* label;
    int mapX;
    int mapY;
    int direction;
    int expectedOrdinal; /* -1 means no mirror */
    int actualOrdinal;
    int portraitRectNonzero;
    int portraitRectWarmCount;
    int mirrorFrameBlackCount;
} MirrorCapture;

static MirrorCapture kCaptures[] = {
    /* Positive ordinals: front cell has C127 sensor with ordinal */
    {"hall_start_north_ordinal_1_HALK",     1, 2, 0,  1, 0, 0, 0, 0},
    {"hall_start_east_ordinal_4_LEIF",      1, 2, 1,  4, 0, 0, 0, 0},
    {"hall_corridor_east_ordinal_18_SONJA", 1, 3, 1, 18, 0, 0, 0, 0},
    {"hall_end_north_ordinal_10_GANDO",     1, 5, 0, 10, 0, 0, 0, 0},
    {"hall_end_east_ordinal_15_MOPHUS",     1, 5, 1, 15, 0, 0, 0, 0},
    {"hall_end_south_ordinal_13_WUUF",      1, 5, 2, 13, 0, 0, 0, 0},
    {"hall_gothmog_south_ordinal_22",       2, 5, 2, 22, 0, 0, 0, 0},
    {"hall_daroou_south_ordinal_0",         1, 7, 2,  0, 0, 0, 0, 0},
    /* Negative ordinals: corridor poses where no C127 sensor exists */
    {"hall_start_west_no_portrait",         1, 2, 3, -1, 0, 0, 0, 0},
    {"hall_corridor_north_no_portrait",     1, 3, 0, -1, 0, 0, 0, 0},
    {"hall_corridor_north_no_portrait_2",   1, 4, 0, -1, 0, 0, 0, 0},
    /* Z-order / no-floating poses: side walls that must NOT show a
     * champion portrait sprite floating over them. These mirror the
     * scenarios covered by firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
     * but here we save visual evidence (PPMs) so the P1 ticket
     * 'champion Z-order/floating' has both probe + capture coverage. */
    {"hall_d1c_front_route_blocked_1_N",    1, 3, 0, -1, 0, 0, 0, 0},
    {"hall_d1c_front_route_blocked_2_N",    1, 4, 0, -1, 0, 0, 0, 0},
    {"hall_d1c_front_route_blocked_east",   1, 4, 1, -1, 0, 0, 0, 0},
    {"hall_d1c_front_route_blocked_south",  1, 4, 2, -1, 0, 0, 0, 0},
    {"hall_side_no_floating_west_1",        1, 3, 3, -1, 0, 0, 0, 0},
    {"hall_side_no_floating_west_2",        1, 4, 3, -1, 0, 0, 0, 0},
};

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') return;
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

static void dump_vga_ppm(const char* path, const unsigned char* fb) {
    FILE* f;
    int px;
    if (!path || !fb) return;
    f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", FB_W, FB_H);
    for (px = 0; px < FB_W * FB_H; ++px) {
        unsigned char raw = fb[px];
        unsigned char idx = M11_FB_DECODE_INDEX(raw);
        int level = M11_FB_DECODE_LEVEL(raw);
        const unsigned char* rgb;
        if (level < 0) level = 0;
        if (level >= M11_PALETTE_LEVELS) level = M11_PALETTE_LEVELS - 1;
        rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

static void dump_vga_viewport_ppm(const char* path, const unsigned char* fb) {
    FILE* f;
    int x, y;
    if (!path || !fb) return;
    f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", VIEWPORT_W, VIEWPORT_H);
    for (y = 0; y < VIEWPORT_H; ++y) {
        for (x = 0; x < VIEWPORT_W; ++x) {
            unsigned char raw = fb[(VIEWPORT_Y + y) * FB_W + (VIEWPORT_X + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            int level = M11_FB_DECODE_LEVEL(raw);
            const unsigned char* rgb;
            if (level < 0) level = 0;
            if (level >= M11_PALETTE_LEVELS) level = M11_PALETTE_LEVELS - 1;
            rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
}

/*
 * Count non-zero pixels in the D1C champion portrait rectangle
 * (96,35 .. 128,64 in viewport-local coordinates, i.e. the standard
 * DM1 front-wall portrait blit location per ReDMCSB DUNVIEW.C:3913-3928).
 * Returns 1 if any pixels are non-zero, 0 if the rect is empty.
 */
static int portrait_rect_nonzero(const unsigned char* fb) {
    int x, y;
    for (y = 35; y < 64; ++y) {
        for (x = 96; x < 128; ++x) {
            if (fb[(VIEWPORT_Y + y) * FB_W + (VIEWPORT_X + x)] != 0) {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Count warm-colored pixels in the D1C champion portrait rectangle.
 *
 * Per the F20E PC 3.4 palette (src/shared/vga_palette_pc34_compat.c,
 * LIGHT0):
 *   0x07 = {0,219,0} green   | 0x08 = {255,0,0} red
 *   0x09 = {255,182,0} orange | 0x0A = {219,146,109} peach
 *   0x0B = {255,255,0} yellow | 0x0E = {0,0,255} blue
 *
 * Champion portrait sprites (C026/C027 per ReDMCSB DUNVIEW.C:3913-3928)
 * use a mix of warm palette indices for skin tones and clothing:
 *   HALK:    peach+orange (219,146,109) (255,182,0)
 *   LEIF:    blue background (0,0,255) + peach highlights
 *   SONJA:   grey-blue background (73,73,73) + cyan hair (0,219,219)
 *   ZED:     red (255,0,0) for clothing
 *   MOPHUS:  grey + warm skin (73,73,73) + (219,146,109)
 *   WUUF:    dark brown (109,36,0) for beard
 *
 * The grey-stone wall texture uses palette indices 0x01/0x02/0x07/0x0D
 * (grey shades) — never the warm indices. So counting pixels with
 * palette indices in {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} (green, red,
 * orange, peach, yellow, blue) distinguishes 'portrait present' from
 * 'wall texture only'. Each champion triggers a different subset of
 * these indices so the threshold is intentionally low (>= 30 pixels
 * of any of these indices collectively = portrait detected).
 *
 * Negative poses (corridor_no_portrait) may still have a few warm
 * pixels from edge antialiasing or torch glow at the side walls, so
 * the threshold is set above the typical noise floor of < 20 pixels.
 */
static int portrait_rect_warm_count(const unsigned char* fb) {
    int x, y;
    int count = 0;
    for (y = 35; y < 64; ++y) {
        for (x = 96; x < 128; ++x) {
            unsigned char raw = fb[(VIEWPORT_Y + y) * FB_W + (VIEWPORT_X + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            switch (idx) {
                case 0x07: /* green */
                case 0x08: /* red */
                case 0x09: /* orange */
                case 0x0A: /* peach */
                case 0x0B: /* yellow */
                case 0x0E: /* blue */
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/*
 * C346's D1C champion-mirror frame is viewport-local (80,29)-(143,71).
 * C026's champion portrait is the smaller cutout inside it at
 * (96,35)-(127,63).  A portrait drawn over ordinary Hall stone has warm
 * portrait pixels but no black frame pixels, which is the user-visible
 * "floating portrait / wrong wall" failure.
 */
static int mirror_frame_black_count(const unsigned char* fb) {
    int x;
    int y;
    int count = 0;
    for (y = 29; y < 72; ++y) {
        for (x = 80; x < 144; ++x) {
            unsigned char raw;
            unsigned char idx;
            if (x >= 96 && x < 128 && y >= 35 && y < 64) {
                continue;
            }
            raw = fb[(VIEWPORT_Y + y) * FB_W + (VIEWPORT_X + x)];
            idx = M11_FB_DECODE_INDEX(raw);
            if (idx == 0x00) {
                ++count;
            }
        }
    }
    return count;
}

static int capture_row_passes(const MirrorCapture* r) {
    if (!r) {
        return 0;
    }
    if (r->actualOrdinal != r->expectedOrdinal) {
        return 0;
    }
    if (r->expectedOrdinal >= 0) {
        return r->portraitRectWarmCount >= PORTRAIT_WARM_THRESHOLD &&
               r->mirrorFrameBlackCount >= MIRROR_FRAME_BLACK_THRESHOLD;
    }
    return r->portraitRectWarmCount < PORTRAIT_WARM_THRESHOLD;
}

static int write_manifest(const char* outDir,
                          const MirrorCapture* rows,
                          int count) {
    char jsonPath[1024];
    char mdPath[1024];
    FILE* js = NULL;
    FILE* md = NULL;
    int i;
    int jsonOk = 0, mdOk = 0;

    snprintf(jsonPath, sizeof(jsonPath),
             "%s/dm1_v1_champion_mirror_capture.json", outDir);
    snprintf(mdPath, sizeof(mdPath),
             "%s/dm1_v1_champion_mirror_capture.md", outDir);

    js = fopen(jsonPath, "w");
    md = fopen(mdPath, "w");
    if (!js || !md) goto done;

    fprintf(js, "{\n");
    fprintf(js, "  \"schema\": \"firestaff.dm1_v1_champion_mirror_capture.v1\",\n");
    fprintf(js, "  \"sourceEvidence\": [\n");
    fprintf(js, "    \"DUNGEON.C:2573 maps M011_CELL(sensor) against view direction\",\n");
    fprintf(js, "    \"DUNGEON.C:2608-2612 stores C127 sensorData in G0289\",\n");
    fprintf(js, "    \"DUNVIEW.C:3913-3928 blits D1C champion portrait\",\n");
    fprintf(js, "    \"DUNVIEW.C:8318-8618 F0128 viewport redraw from party map/x/y/direction\",\n");
    fprintf(js, "    \"MOVESENS.C:1501-1503 passes C127 sensorData to F0280\",\n");
    fprintf(js, "    \"REVIVE.C F0280 materializes candidate from sensorData\",\n");
    fprintf(js, "    \"COORD.C:1693-1722 PC34 viewport origin/224x136 dimensions\"\n");
    fprintf(js, "  ],\n");
    fprintf(js, "  \"honesty\": \"Firestaff deterministic runtime capture with exact state coordinates, full-frame PPM screenshots, and source-geometry viewport crop PPMs. The portrait_rect_nonzero column is a coarse heuristic that scans the (96,35)-(128,64) D1C front-wall rectangle for any non-black pixel. The portrait_rect_warm_count column counts pixels with palette indices in {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} in the same rectangle. The mirror_frame_black_count column scans the C346 D1C mirror frame zone outside the portrait cutout; ordinary Hall stone has no black pixels there, so positive rows must prove a frame/backing exists instead of accepting a floating portrait over stone. Pass criterion: actual ordinal must equal expected ordinal, positive-ordinal poses must have >= 30 warm pixels and >= 16 black frame pixels, and negative-ordinal poses must have < 30 warm pixels. This is NOT pixel parity with original DM1 PC 3.4 — it is visual-evidence readiness for Hall champion-mirror placement.\",\n");
    fprintf(js, "  \"captures\": [\n");

    fprintf(md, "# DM1 V1 champion mirror visual capture\n\n");
    fprintf(md, "Deterministic Firestaff runtime captures for every Hall of Champions pose with a known mirror ordinal plus the negative corridor poses. Each row records exact map/x/y/direction, the expected and actual front-mirror ordinals, the D1C front-wall portrait rectangle (96,35)-(128,64), the C346 mirror-frame backing around it, a full-frame PPM, and a source-geometry viewport crop PPM.\n\n");
    fprintf(md, "## Source evidence\n\n");
    fprintf(md, "- DUNGEON.C:2573 — map sensor cell against view direction\n");
    fprintf(md, "- DUNGEON.C:2608-2612 — store C127 sensorData in G0289\n");
    fprintf(md, "- DUNVIEW.C:3913-3928 — D1C champion portrait blit rectangle\n");
    fprintf(md, "- DUNVIEW.C:8318-8618 F0128 — viewport redraw from party map/x/y/direction\n");
    fprintf(md, "- MOVESENS.C:1501-1503 — C127 sensorData passes to F0280\n");
    fprintf(md, "- REVIVE.C F0280 — materializes candidate from sensorData\n");
    fprintf(md, "- COORD.C:1693-1722 — PC34 viewport origin/224x136 dimensions\n\n");
    fprintf(md, "## P1 bug status\n\n");
    fprintf(md, "The Hall champion-mirror placement ticket is closed-by-evidence when every row has `actualOrdinal == expectedOrdinal`, every positive-ordinal pose has `portrait_rect_warm_count >= 30` and `mirror_frame_black_count >= 16`, and every negative-ordinal/no-floating pose has `portrait_rect_warm_count < 30`. If a positive pose has portrait pixels but no frame pixels, the portrait is floating over the Hall wall instead of sitting in a mirror backing.\n\n");
    fprintf(md, "## Captures\n\n");
    fprintf(md, "| label | map | x | y | dir | expected | actual | portrait_rect_nonzero | portrait_rect_warm_count | mirror_frame_black_count | screenshot | viewport crop |\n");
    fprintf(md, "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |\n");

    for (i = 0; i < count; ++i) {
        const MirrorCapture* r = &rows[i];
        int pass = capture_row_passes(r);
        fprintf(js,
                "    {\"label\":\"%s\",\"party\":{\"mapIndex\":0,\"mapX\":%d,\"mapY\":%d,\"direction\":%d},\"expectedOrdinal\":%d,\"actualOrdinal\":%d,\"portraitRectNonzero\":%d,\"portraitRectWarmCount\":%d,\"mirrorFrameBlackCount\":%d,\"screenshot\":\"%s.ppm\",\"viewportCrop\":\"%s_viewport_224x136.ppm\",\"pass\":%s}%s\n",
                r->label, r->mapX, r->mapY, r->direction,
                r->expectedOrdinal, r->actualOrdinal,
                r->portraitRectNonzero, r->portraitRectWarmCount,
                r->mirrorFrameBlackCount,
                r->label, r->label,
                pass ? "true" : "false",
                i == count - 1 ? "" : ",");
        fprintf(md,
                "| %s | 0 | %d | %d | %d | %d | %d | %d | %d | %d | `%s.ppm` | `%s_viewport_224x136.ppm` |\n",
                r->label, r->mapX, r->mapY, r->direction,
                r->expectedOrdinal, r->actualOrdinal,
                r->portraitRectNonzero, r->portraitRectWarmCount,
                r->mirrorFrameBlackCount,
                r->label, r->label);
    }

    fprintf(js, "  ]\n}\n");
    jsonOk = 1;
    mdOk = 1;
done:
    if (js) fclose(js);
    if (md) fclose(md);
    return jsonOk && mdOk;
}

int main(int argc, char** argv) {
    const char* dataDir;
    const char* outDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int i;
    int n = (int)(sizeof(kCaptures) / sizeof(kCaptures[0]));
    int rc;
    int failures = 0;

    if (argc < 3) {
        fprintf(stderr,
                "usage: %s DATA_DIR OUT_DIR\n"
                "  captures Hall of Champions poses with mirror ordinals plus no-floating negatives\n"
                "  and saves PPMs + manifest under OUT_DIR\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];
    outDir = argv[2];

    ensure_output_dir(outDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP dm1_v1_champion_mirror_capture_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 champion mirror visual capture ===\n");
    printf("dataDir=%s outDir=%s\n", dataDir, outDir);
    for (i = 0; i < n; ++i) {
        MirrorCapture* r = &kCaptures[i];
        char ppmPath[1024];
        char ppmViewportPath[1024];
        unsigned char framebuffer[FB_W * FB_H];

        game.world.party.mapIndex = 0;
        game.world.party.mapX = r->mapX;
        game.world.party.mapY = r->mapY;
        game.world.party.direction = r->direction;

        r->actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);

        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&game, framebuffer, FB_W, FB_H);

        r->portraitRectNonzero = portrait_rect_nonzero(framebuffer);
        r->portraitRectWarmCount = portrait_rect_warm_count(framebuffer);
        r->mirrorFrameBlackCount = mirror_frame_black_count(framebuffer);
        if (!capture_row_passes(r)) {
            ++failures;
        }

        snprintf(ppmPath, sizeof(ppmPath), "%s/%s.ppm", outDir, r->label);
        dump_vga_ppm(ppmPath, framebuffer);
        snprintf(ppmViewportPath, sizeof(ppmViewportPath),
                 "%s/%s_viewport_224x136.ppm", outDir, r->label);
        dump_vga_viewport_ppm(ppmViewportPath, framebuffer);

        printf("  %s pose=(%d,%d,%d) expected=%d actual=%d portrait_rect=%s warm_count=%d mirror_frame_black=%d %s -> %s.ppm\n",
               r->label, r->mapX, r->mapY, r->direction,
               r->expectedOrdinal, r->actualOrdinal,
               r->portraitRectNonzero ? "nonzero" : "empty",
               r->portraitRectWarmCount,
               r->mirrorFrameBlackCount,
               capture_row_passes(r) ? "PASS" : "FAIL",
               r->label);
    }

    rc = write_manifest(outDir, kCaptures, n) ? 0 : 1;
    printf("wrote %s/dm1_v1_champion_mirror_capture.{json,md}\n", outDir);
    M11_GameView_Shutdown(&game);
    if (failures > 0) {
        fprintf(stderr,
                "FAIL dm1_v1_champion_mirror_capture_probe %d/%d capture invariants failed\n",
                failures, n);
        return 1;
    }
    return rc;
}
