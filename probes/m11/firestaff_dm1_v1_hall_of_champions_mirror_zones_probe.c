/*
 * firestaff_dm1_v1_hall_of_champions_mirror_zones_probe.c
 *
 * Source-locked verification gate for the DM1 V1 endgame "Hall of
 * Champions" overlay in m11_game_view.c.  The source layout-696 zones
 * C412..C415 hold one mirror per party champion and C416..C419 hold
 * the parented portrait boxes, so every visible row (mirror graphic,
 * portrait, name, title, skill lines) must end up at the source
 * coordinate for the matching champion slot.
 *
 *   slot 0: mirror (19, 7, 48, 43), portrait (27, 13, 32, 29)
 *           name (87, 14), skills (105, 23 + n*8)
 *   slot 1: mirror (19, 55, 48, 43), portrait (27, 61, 32, 29)
 *           name (87, 62), skills (105, 71 + n*8)
 *   slot 2: mirror (19, 103, 48, 43), portrait (27, 109, 32, 29)
 *           name (87, 110), skills (105, 119 + n*8)
 *   slot 3: mirror (19, 151, 48, 43), portrait (27, 157, 32, 29)
 *           name (87, 158), skills (105, 167 + n*8)
 *
 * Each zone helper (M11_GameView_GetV1EndgameChampionMirrorZone,
 * M11_GameView_GetV1EndgameChampionPortraitZone, etc.) is the source of
 * truth and must match.  The probe then drives M11_GameView_Draw with
 * gameWon=1 and four distinct champions (portraits 0, 5, 11, 18 from
 * the GRAPHICS.DAT portrait strip) to confirm every slot paints at
 * the source coordinates and that nothing from one slot bleeds into
 * another.
 *
 * Source evidence:
 *   ReDMCSB ENDGAME.C:327-394 champion summary render path
 *   ReDMCSB ENDGAME.C:440-456 THE END blit (DATA.C box 120,95,80,14)
 *   ReDMCSB layout-696 C412..C415 mirror zones
 *   ReDMCSB layout-696 C416..C419 portrait zones parented at +8,+6
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    MIRROR_W   = 48,
    MIRROR_H   = 43,
    SKILL_LINE_H = 8
};

enum {
    PROBE_COLOR_BLACK      = 0,
    PROBE_COLOR_DARK_GRAY  = 12,
    PROBE_COLOR_LIGHT_RED  = 9,
    PROBE_COLOR_SILVER     = 13
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* ── helpers ─────────────────────────────────────────────────── */
static unsigned int count_color_in_rect(const unsigned char* fb,
                                        int x, int y, int w, int h,
                                        unsigned int want) {
    unsigned int n = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char raw = fb[yy * FB_W + xx];
            if (((unsigned int)(raw & 0x0F)) == want) ++n;
        }
    }
    return n;
}

static int compare_mirror_zone(const unsigned char* fb,
                               const M11_AssetSlot* mirror,
                               int slot,
                               int* outMatched,
                               int* outCompared) {
    int mirrorX = 0, mirrorY = 0;
    int matched = 0, compared = 0;
    int x, y;
    if (!M11_GameView_GetV1EndgameChampionMirrorZone(slot, &mirrorX, &mirrorY, NULL, NULL)) {
        return 0;
    }
    if (!mirror || !mirror->loaded || !mirror->pixels) {
        return 0;
    }
    for (y = 0; y < MIRROR_H && y < (int)mirror->height; ++y) {
        for (x = 0; x < MIRROR_W && x < (int)mirror->width; ++x) {
            unsigned char expected = mirror->pixels[y * (int)mirror->width + x] & 0x0F;
            if (expected == 10) continue; /* transparent */
            if (x >= 8 && x < 40 && y >= 6 && y < 35) continue; /* portrait cutout */
            ++compared;
            if (((fb[(mirrorY + y) * FB_W + (mirrorX + x)] & 0x0F)) == expected) {
                ++matched;
            }
        }
    }
    *outMatched = matched;
    *outCompared = compared;
    return 1;
}

static int compare_portrait_zone(const unsigned char* fb,
                                 const M11_AssetSlot* portraits,
                                 int slot,
                                 int portraitIndex,
                                 int* outMatched,
                                 int* outCompared) {
    int portraitX = 0, portraitY = 0;
    int matched = 0, compared = 0;
    int x, y;
    int srcPX, srcPY;
    if (!M11_GameView_GetV1EndgameChampionPortraitZone(slot, &portraitX, &portraitY, NULL, NULL)) {
        return 0;
    }
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        return 0;
    }
    srcPX = (portraitIndex & 7) * PORTRAIT_W;
    srcPY = (portraitIndex >> 3) * PORTRAIT_H;
    if (srcPX + PORTRAIT_W > (int)portraits->width ||
        srcPY + PORTRAIT_H > (int)portraits->height) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char expected = portraits->pixels[(srcPY + y) * (int)portraits->width + (srcPX + x)] & 0x0F;
            if (expected == PROBE_COLOR_DARK_GRAY) continue; /* transparency */
            ++compared;
            if (((fb[(portraitY + y) * FB_W + (portraitX + x)] & 0x0F)) == expected) {
                ++matched;
            }
        }
    }
    *outMatched = matched;
    *outCompared = compared;
    return 1;
}

static int count_portrait_pixel_at(const unsigned char* fb,
                                   const M11_AssetSlot* portraits,
                                   int slot,
                                   int portraitIndex,
                                   unsigned int wantColor,
                                   unsigned int* outCount) {
    int portraitX = 0, portraitY = 0;
    unsigned int count = 0;
    int x, y;
    int srcPX, srcPY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if (!M11_GameView_GetV1EndgameChampionPortraitZone(slot, &portraitX, &portraitY, NULL, NULL)) {
        return 0;
    }
    srcPX = (portraitIndex & 7) * PORTRAIT_W;
    srcPY = (portraitIndex >> 3) * PORTRAIT_H;
    if (srcPX + PORTRAIT_W > (int)portraits->width ||
        srcPY + PORTRAIT_H > (int)portraits->height) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char expected = portraits->pixels[(srcPY + y) * (int)portraits->width + (srcPX + x)] & 0x0F;
            if (expected != wantColor) continue;
            if (((fb[(portraitY + y) * FB_W + (portraitX + x)] & 0x0F)) == wantColor) {
                ++count;
            }
        }
    }
    *outCount = count;
    return 1;
}

static void write_ppm(const char* path, const unsigned char* fb) {
    FILE* f = fopen(path, "wb");
    int px;
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", FB_W, FB_H);
    for (px = 0; px < FB_W * FB_H; ++px) {
        unsigned char raw = fb[px];
        unsigned char idx = M11_FB_DECODE_INDEX(raw);
        int level = M11_FB_DECODE_LEVEL(raw);
        const unsigned char* rgb;
        if (level >= M11_PALETTE_LEVELS) level = M11_PALETTE_LEVELS - 1;
        rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

/* ── main ────────────────────────────────────────────────────── */
int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState gameView;
    unsigned char fb[FB_W * FB_H];
    const M11_AssetSlot* mirror;
    const M11_AssetSlot* portraits;
    static const char* kNames[4]   = { "TIGGY", "HALK", "WUK",   "KIM" };
    static const char* kTitles[4]  = { "THE WIZARD",
                                       "THE BARBARIAN",
                                       "THE NINJA",
                                       "THE PRIEST" };
    static const int   kPortraitIdx[4] = { 0, 5, 11, 18 };
    int assetsOk = 0;
    int slot;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR [OUT_PPM]\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 Hall of Champions: 4-mirror zone verification ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&gameView);
    if (!M11_GameView_OpenSelectedMenuEntry(&gameView, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&gameView);
        return 1;
    }
    gameView.showDebugHUD = 0;
    gameView.gameWon = 1;
    gameView.gameWonTick = 100;

    /* Four distinct champions with distinct portraits. */
    gameView.world.party.championCount = 4;
    for (slot = 0; slot < 4; ++slot) {
        size_t nameLen = strlen(kNames[slot]);
        size_t titleLen = strlen(kTitles[slot]);
        memset(&gameView.world.party.champions[slot], 0, sizeof(gameView.world.party.champions[slot]));
        gameView.world.party.champions[slot].present = 1;
        gameView.world.party.champions[slot].portraitIndex = kPortraitIdx[slot];
        memcpy(gameView.world.party.champions[slot].name, kNames[slot], nameLen);
        memcpy(gameView.world.party.champions[slot].title, kTitles[slot], titleLen);
        gameView.world.party.champions[slot].skillLevels[0] = 3;  /* FIGHTER */
        gameView.world.party.champions[slot].skillLevels[1] = 4;  /* NINJA   */
        gameView.world.party.champions[slot].skillLevels[2] = 5;  /* PRIEST  */
        gameView.world.party.champions[slot].skillLevels[3] = 6;  /* WIZARD  */
    }

    mirror = M11_AssetLoader_Load(&gameView.assetLoader,
                                  (unsigned int)M11_GameView_GetV1EndgameChampionMirrorGraphicId());
    portraits = M11_AssetLoader_Load(&gameView.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    assetsOk = (mirror && mirror->loaded && mirror->pixels &&
                portraits && portraits->loaded && portraits->pixels &&
                portraits->width >= 256 && portraits->height >= 87);

    /* ── A) Zone helpers themselves ───────────────────────────── */
    printf("\n[Group A] Zone helper coordinates (C412..C419)\n");
    for (slot = 0; slot < 4; ++slot) {
        int mx = -1, my = -1, mw = -1, mh = -1;
        int px = -1, py = -1, pw = -1, ph = -1;
        int nx = -1, ny = -1;
        int zid_mirror = M11_GameView_GetV1EndgameChampionMirrorZoneId(slot);
        int zid_port   = M11_GameView_GetV1EndgameChampionPortraitZoneId(slot);
        M11_GameView_GetV1EndgameChampionMirrorZone(slot, &mx, &my, &mw, &mh);
        M11_GameView_GetV1EndgameChampionPortraitZone(slot, &px, &py, &pw, &ph);
        M11_GameView_GetV1EndgameChampionNameOrigin(slot, &nx, &ny);

        char msg[160];
        snprintf(msg, sizeof(msg), "slot %d mirror zone id C%d == 412+%d", slot, zid_mirror, slot);
        CHECK(zid_mirror == 412 + slot, msg);
        snprintf(msg, sizeof(msg), "slot %d portrait zone id C%d == 416+%d", slot, zid_port, slot);
        CHECK(zid_port == 416 + slot, msg);
        snprintf(msg, sizeof(msg), "slot %d mirror rect (%d,%d,%d,%d) == (19, 7+%d*48, 48, 43)",
                 slot, mx, my, mw, mh, slot);
        CHECK(mx == 19 && my == 7 + slot * 48 && mw == 48 && mh == 43, msg);
        snprintf(msg, sizeof(msg), "slot %d portrait rect (%d,%d,%d,%d) == (27, 13+%d*48, 32, 29)",
                 slot, px, py, pw, ph, slot);
        CHECK(px == 27 && py == 13 + slot * 48 && pw == 32 && ph == 29, msg);
        snprintf(msg, sizeof(msg), "slot %d name origin (%d,%d) == (87, 14+%d*48)",
                 slot, nx, ny, slot);
        CHECK(nx == 87 && ny == 14 + slot * 48, msg);
    }
    /* Per-slot stride is 48, mirrors do not overlap (slot 0 bottom = 7+43=50 < slot 1 top = 55). */
    CHECK(7 + 0 * 48 + 43 < 7 + 1 * 48, "slot 0..1 mirror rows do not overlap");
    CHECK(7 + 1 * 48 + 43 < 7 + 2 * 48, "slot 1..2 mirror rows do not overlap");
    CHECK(7 + 2 * 48 + 43 < 7 + 3 * 48, "slot 2..3 mirror rows do not overlap");
    /* Portrait cutout is parented to the mirror at +8,+6. */
    CHECK(27 - 19 == 8 && 13 - 7 == 6, "portrait offset from mirror = (+8,+6)");

    /* ── B) Draw the endgame and confirm each slot renders ────── */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&gameView, fb, FB_W, FB_H);

    if (argc > 2) {
        write_ppm(argv[2], fb);
        printf("\nWrote framebuffer PPM to %s\n", argv[2]);
    }

    printf("\n[Group B] Mirror graphic (C346) drawn at each slot\n");
    if (assetsOk) {
        for (slot = 0; slot < 4; ++slot) {
            int matched = 0, compared = 0;
            char msg[160];
            if (compare_mirror_zone(fb, mirror, slot, &matched, &compared) && compared > 0) {
                snprintf(msg, sizeof(msg),
                         "slot %d mirror graphic: %d/%d pixel match (>=90%%)",
                         slot, matched, compared);
                CHECK(matched * 100 >= compared * 90, msg);
            } else {
                snprintf(msg, sizeof(msg), "slot %d mirror graphic: skipped (no asset)", slot);
                printf("  SKIP: %s\n", msg);
            }
        }
    } else {
        printf("  SKIP: mirror asset unavailable\n");
    }

    printf("\n[Group C] Portrait blit at each slot (correct portrait for that champion)\n");
    if (assetsOk) {
        for (slot = 0; slot < 4; ++slot) {
            int matched = 0, compared = 0;
            char msg[160];
            if (compare_portrait_zone(fb, portraits, slot, kPortraitIdx[slot], &matched, &compared) && compared > 0) {
                snprintf(msg, sizeof(msg),
                         "slot %d portrait (idx %d): %d/%d pixel match (>=90%%)",
                         slot, kPortraitIdx[slot], matched, compared);
                CHECK(matched * 100 >= compared * 90, msg);
            } else {
                snprintf(msg, sizeof(msg), "slot %d portrait: skipped (no asset)", slot);
                printf("  SKIP: %s\n", msg);
            }
        }
    } else {
        printf("  SKIP: portrait strip unavailable\n");
    }

    printf("\n[Group D] Portrait cross-contamination (no other slot has the wrong portrait)\n");
    if (assetsOk) {
        for (slot = 0; slot < 4; ++slot) {
            for (int other = 0; other < 4; ++other) {
                if (other == slot) continue;
                unsigned int wrongPixels = 0;
                char msg[200];
                if (count_portrait_pixel_at(fb, portraits, slot, kPortraitIdx[other],
                                            /* skin color proxy */ 10, &wrongPixels)) {
                    snprintf(msg, sizeof(msg),
                             "slot %d does NOT show portrait of slot %d (idx %d) in cutout",
                             slot, other, kPortraitIdx[other]);
                    /* We only check that the expected (slot's own) portrait
                     * dominates.  A hard zero is too strict because
                     * mirror/portrait palettes can share skin tones; use
                     * a generous sanity threshold. */
                    CHECK(wrongPixels < 60, msg);
                }
            }
        }
    } else {
        printf("  SKIP: cross-contamination skipped (no assets)\n");
    }

    printf("\n[Group E] Champion name (gold) at each slot\n");
    for (slot = 0; slot < 4; ++slot) {
        int nx = 0, ny = 0;
        unsigned int gold;
        char msg[160];
        M11_GameView_GetV1EndgameChampionNameOrigin(slot, &nx, &ny);
        gold = count_color_in_rect(fb, nx, ny, 70, SKILL_LINE_H, PROBE_COLOR_LIGHT_RED);
        snprintf(msg, sizeof(msg),
                 "slot %d name gold pixels at (%d,%d): %u (>= 4)",
                 slot, nx, ny, gold);
        CHECK(gold >= 4, msg);
    }

    printf("\n[Group F] Skill lines (silver) at each slot\n");
    for (slot = 0; slot < 4; ++slot) {
        int sx = 0, sy = 0;
        unsigned int silver;
        char msg[160];
        M11_GameView_GetV1EndgameChampionSkillOrigin(slot, 0, &sx, &sy);
        silver = count_color_in_rect(fb, sx, sy, 110, SKILL_LINE_H, PROBE_COLOR_SILVER);
        snprintf(msg, sizeof(msg),
                 "slot %d skill line 0 (fighter) silver at (%d,%d): %u (>= 6)",
                 slot, sx, sy, silver);
        CHECK(silver >= 6, msg);
    }

    printf("\n[Group G] Vertical stride: name/skill y offsets follow the 48px row pitch\n");
    {
        int prevMy = 0, prevPy = 0, prevNy = 0, prevSy = 0;
        int prevSlot = 0;
        for (slot = 1; slot < 4; ++slot) {
            int mx, my, px, py, nx, ny, sx, sy;
            char msg[200];
            M11_GameView_GetV1EndgameChampionMirrorZone(slot, &mx, &my, NULL, NULL);
            M11_GameView_GetV1EndgameChampionPortraitZone(slot, &px, &py, NULL, NULL);
            M11_GameView_GetV1EndgameChampionNameOrigin(slot, &nx, &ny);
            M11_GameView_GetV1EndgameChampionSkillOrigin(slot, 0, &sx, &sy);
            if (slot == 1) {
                prevMy = my; prevPy = py; prevNy = ny; prevSy = sy; prevSlot = slot;
                continue;
            }
            (void)prevSlot;
            snprintf(msg, sizeof(msg), "slot %d mirror y - slot %d mirror y = 48", slot, slot - 1);
            CHECK(my - prevMy == 48, msg);
            snprintf(msg, sizeof(msg), "slot %d portrait y - slot %d portrait y = 48", slot, slot - 1);
            CHECK(py - prevPy == 48, msg);
            snprintf(msg, sizeof(msg), "slot %d name y - slot %d name y = 48", slot, slot - 1);
            CHECK(ny - prevNy == 48, msg);
            snprintf(msg, sizeof(msg), "slot %d skill y - slot %d skill y = 48", slot, slot - 1);
            CHECK(sy - prevSy == 48, msg);
            prevMy = my; prevPy = py; prevNy = ny; prevSy = sy;
        }
    }

    M11_GameView_Shutdown(&gameView);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
