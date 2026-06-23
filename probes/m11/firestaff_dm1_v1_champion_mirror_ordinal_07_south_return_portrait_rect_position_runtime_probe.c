/*
 * firestaff_dm1_v1_champion_mirror_ordinal_07_south_return_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for the DM1 V1 Hall of Champions
 * champion portrait slice:
 *
 *   ordinal           = 7  (TIGGY / TAMAL)
 *   route             = south_return   (party facing SOUTH at the
 *                                       return-leg of the Hall corridor)
 *   aspect            = portrait_rect_position
 *
 * The probe targets the only Hall-of-Champions pose in real DM1 V1
 * DUNGEON.DAT (PC 3.4) where the C127 sensor on the front square
 * carries sensorData = 7:
 *
 *   (2, 17) facing SOUTH  ->  M11_GameView_GetFrontMirrorOrdinal() == 7
 *
 * The "(2, 17) SOUTH" cell is the south-facing mirror on the return
 * path of the Hall corridor.  The existing
 * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe covers
 * ordinals 1, 4, 10, 13, 15, 18 and the zorder reblt probe uses
 * {1, 4, 10, 13, 15, 18, 19}, so ordinal 7 + south_return was the
 * last unproven slice of the front-mirror lattice.
 *
 * This probe is complementary to the existing
 * firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
 * (which uses the same (2, 17, SOUTH) cell but frames it as the
 * "front_north_entry" route and adds the catalog identity, side-wall
 * no-floating check, and resurrect-round-trip).  The slice here
 * focuses on the south_return route framing and adds:
 *
 *   - A D1C wall-ornament zone constant assertion (DUNVIEW.C G0205
 *     coordSet 5 / index 12: (80, 29, 64, 43) viewport-relative).
 *   - A strict best-ordinal sweep that picks the dominant ordinal in
 *     the (96, 35, 32, 29) portrait cutout and asserts it equals 7.
 *   - A re-blt invariant test that turns from the south-facing
 *     ordinal-7 pose to a west-facing no-portrait pose and verifies
 *     the portrait is cleared from the D1C rect (symmetric with the
 *     existing zorder reblt probe's 35% leak envelope).
 *
 * The probe drives the public M11 path and pixel-proves four
 * contracts:
 *
 *   (A) Front-cell ordinal contract.
 *       M11_GameView_GetFrontMirrorOrdinal at (2, 17, SOUTH) == 7.
 *       M11_GameView_GetFrontMirrorOrdinal at (2, 17, N/E/W) == -1
 *       (no-portrait side poses must NOT expose ordinal 7 through the
 *       wrong-wall side of the same cell).
 *
 *   (B) D1C wall-ornament box contract.
 *       M11_GameView_GetD1CWallOrnamentZone returns the source-locked
 *       D1C front-wall destination box at (80, 29, 64, 43) viewport
 *       coords (DUNVIEW.C G0205 coordSet 5 / index 12).
 *
 *   (C) Portrait_rect_position contract.
 *       After M11_GameView_Draw, the (96, 35, 32, 29) viewport-relative
 *       rectangle (DUNVIEW.C:3913-3928 cutout) is dominated by ordinal
 *       7 pixels from the C026 portrait strip (>= 90% pixel match).
 *
 *   (D) No-floating side-wall contract.
 *       After M11_GameView_Draw with the same cell but NORTH / EAST /
 *       WEST direction, the D1C portrait rectangle is NOT dominated by
 *       ordinal 7 pixels.  The probe tolerates a <= 35% leak (same
 *       tolerance the existing zorder reblt probe locks) so the
 *       assertion is symmetric with the corridor-pose coverage.
 *
 *   (E) Re-blt invariant on south->west transition.
 *       A in-place turn from (2, 17, SOUTH, ord=7) to (2, 17, WEST,
 *       ord=-1) clears ordinal 7 pixels from the D1C rect within the
 *       same 35% leak tolerance the reblt probe locks.
 *
 * The probe is honest about runtime vs pixel parity:
 *   - M11_GameView_GetFrontMirrorOrdinal and the C127 sensorData
 *     mapping are deterministic Firestaff runtime.
 *   - The pixel-match threshold (>= 90%) is a deterministic
 *     Firestaff runtime heuristic, NOT a DOS PC 3.4 pixel-parity
 *     comparison; the latter would require a paired original-artifact
 *     capture which is out of scope here.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2608-2612  stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928  blits the D1C C026 champion portrait
 *                                at (96, 35, 32, 29) viewport coords
 *   ReDMCSB DUNVIEW.C:8522-8533  restricts champion-portrait
 *                                interaction evidence to the D1C
 *                                front-wall route (side walls do not
 *                                own the portrait).
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB REVIVE.C F0280       materializes the candidate from
 *                                sensorData
 *   ReDMCSB ENDGAME.C:327-394    champion summary render path
 *   ReDMCSB DUNVIEW.C G0205      wall-ornament coordinate sets
 *
 * Usage: firestaff_dm1_v1_champion_mirror_ordinal_07_south_return_portrait_rect_position_runtime_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

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
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* DUNVIEW.C:3913-3928 C026 champion portrait cutout in
     * viewport-relative coords: x=96, y=35, w=32, h=29. */
    PROBE_PORTRAIT_X = VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* C026 portrait strip dimensions: 8 columns x 3 rows of 32x29. */
    PROBE_STRIP_COLS = 8,
    PROBE_STRIP_ROWS = 3,
    PROBE_ORDINAL_MAX = 24,
    /* DUNVIEW.C C01_COLOR_DARK_GRAY (DUNVIEW.C:3916 transparency). */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Match threshold used by the existing zorder reblt probe. */
    PROBE_MATCH_PCT = 90,
    /* Stale-ordinal leak tolerance used by the existing zorder reblt
     * probe; the side poses here must fall inside this envelope. */
    PROBE_LEAK_PCT = 35
};

/* The DM1 V1 Hall-of-Champions return-leg cell where the front
 * C127 sensor carries sensorData=7.  Confirmed by the Firestaff
 * map-0 sensor scan (probe-time introspection over every
 * (mapX, mapY, dir) on map 0).  This is the only (mapX, mapY, dir)
 * triple on map 0 that produces front ordinal 7. */
#define SOUTH_RETURN_MAP_X   2
#define SOUTH_RETURN_MAP_Y   17
#define SOUTH_RETURN_DIR     DIR_SOUTH
#define SOUTH_RETURN_ORDINAL 7
/* The 3 adjacent wrong-wall sides must NOT expose the same ordinal. */
#define NORTH_RETURN_DIR     DIR_NORTH
#define EAST_RETURN_DIR      DIR_EAST
#define WEST_RETURN_DIR      DIR_WEST

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Source-strip pixel at ordinal O, cell (x, y).  Returns the low
 * nibble (palette index); the source uses the upper bit for level. */
static unsigned char strip_pixel(const M11_AssetSlot* portraits,
                                 int ordinal, int x, int y) {
    int srcX;
    int srcY;
    if (!portraits || !portraits->pixels || ordinal < 0 ||
        ordinal >= PROBE_ORDINAL_MAX) {
        return 0;
    }
    srcX = (ordinal & (PROBE_STRIP_COLS - 1)) * PROBE_PORTRAIT_W + x;
    srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
    if (srcX < 0 || srcX >= (int)portraits->width ||
        srcY < 0 || srcY >= (int)portraits->height) {
        return 0;
    }
    return (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
}

/* Count palette-index matches between ordinal O and the D1C portrait
 * rectangle on the framebuffer.  Transparency (C01 dark gray) on the
 * source is skipped so the underlying wall pixels do not poison the
 * match count. */
static int match_ordinal(const M11_AssetSlot* portraits,
                         const unsigned char* fb, int ordinal) {
    int matched = 0;
    int compared = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= PROBE_ORDINAL_MAX) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char src = strip_pixel(portraits, ordinal, x, y);
            unsigned char dst;
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(
                fb[(PROBE_PORTRAIT_Y + y) * FB_W + (PROBE_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) {
                ++matched;
            }
        }
    }
    /* Two-output convention: encode "compared" in the high word so a
     * single int can carry both numbers.  The caller decodes with
     * MATCH_COMPARED. */
    return (matched << 16) | (compared & 0xFFFF);
}
#define MATCH_MATCHED(v)   (((v) >> 16) & 0xFFFF)
#define MATCH_COMPARED(v)  ((v) & 0xFFFF)

/* For the 24-ordinal sweep that picks the best-matching ordinal on
 * the framebuffer (used to verify "no other ordinal wins"). */
static int best_ordinal(const M11_AssetSlot* portraits,
                        const unsigned char* fb) {
    int best = -1;
    int bestMatched = -1;
    int o;
    for (o = 0; o < PROBE_ORDINAL_MAX; ++o) {
        int r = match_ordinal(portraits, fb, o);
        int matched = MATCH_MATCHED(r);
        if (matched > bestMatched) {
            bestMatched = matched;
            best = o;
        }
    }
    return best;
}

/* Set the runtime pose to (mapX, mapY, dir) on map 0 with the
 * Hall-of-Champions panel/disabled-flag state cleared. */
static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    int ordinalSouth, ordinalNorth, ordinalEast, ordinalWest;
    int ornX, ornY, ornW, ornH;
    int result = 0;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 Hall of Champions: ordinal 7, route south_return,\n");
    printf("===          aspect portrait_rect_position (v3.0.1)\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL: GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* ── A) Front-cell ordinal contract ──────────────────────────── */
    printf("\n[Group A] Front-cell ordinal contract at (2, 17)\n");

    set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, SOUTH_RETURN_DIR);
    ordinalSouth = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 17, SOUTH) -> front ordinal = %d (want %d)",
                 ordinalSouth, SOUTH_RETURN_ORDINAL);
        CHECK(ordinalSouth == SOUTH_RETURN_ORDINAL, msg);
    }

    set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, NORTH_RETURN_DIR);
    ordinalNorth = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 17, NORTH) -> front ordinal = %d (want -1, side wall)",
                 ordinalNorth);
        CHECK(ordinalNorth == -1, msg);
    }

    set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, EAST_RETURN_DIR);
    ordinalEast = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 17, EAST)  -> front ordinal = %d (want -1, side wall)",
                 ordinalEast);
        CHECK(ordinalEast == -1, msg);
    }

    set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, WEST_RETURN_DIR);
    ordinalWest = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 17, WEST)  -> front ordinal = %d (want -1, side wall)",
                 ordinalWest);
        CHECK(ordinalWest == -1, msg);
    }

    /* ── B) D1C wall-ornament box contract ───────────────────────── */
    printf("\n[Group B] D1C wall-ornament box (DUNVIEW.C G0205 / index 12)\n");

    set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, SOUTH_RETURN_DIR);
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "D1C box X = %d (want 80, source coordSet 5 / index 12)",
                 ornX);
        CHECK(ornX == 80, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "D1C box Y = %d (want 29, source coordSet 5 / index 12)",
                 ornY);
        CHECK(ornY == 29, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "D1C box W = %d (want 64, source coordSet 5 / index 12)",
                 ornW);
        CHECK(ornW == 64, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "D1C box H = %d (want 43, source coordSet 5 / index 12)",
                 ornH);
        CHECK(ornH == 43, msg);
    }

    /* The portrait cutout must sit inside the D1C box.  The D1C box
     * is in viewport-relative coords (ornX/ornY); the portrait cutout
     * is stored here in framebuffer coords (PROBE_PORTRAIT_X =
     * VIEWPORT_X + 96, PROBE_PORTRAIT_Y = VIEWPORT_Y + 35), so we
     * convert to viewport coords before the comparison. */
    {
        int portXvp = PROBE_PORTRAIT_X - VIEWPORT_X;
        int portYvp = PROBE_PORTRAIT_Y - VIEWPORT_Y;
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "portrait cutout viewport=(%d, %d, %d, %d) sits inside D1C "
                 "viewport box (%d, %d, %d, %d)",
                 portXvp, portYvp, PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
                 ornX, ornY, ornW, ornH);
        CHECK(portXvp >= ornX &&
             portYvp >= ornY &&
             portXvp + PROBE_PORTRAIT_W <= ornX + ornW &&
             portYvp + PROBE_PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ── C) Portrait_rect_position contract ──────────────────────── */
    printf("\n[Group C] Portrait rect position at (2, 17, SOUTH): ordinal 7 drawn\n");

    set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, SOUTH_RETURN_DIR);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);

    {
        int r = match_ordinal(portraits, fb, SOUTH_RETURN_ORDINAL);
        int matched = MATCH_MATCHED(r);
        int compared = MATCH_COMPARED(r);
        int pct = (compared > 0) ? (matched * 100 / compared) : 0;
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(2, 17, SOUTH) portrait ordinal %d pixel match "
                 "%d/%d (%d%% >= %d%%)",
                 SOUTH_RETURN_ORDINAL, matched, compared, pct, PROBE_MATCH_PCT);
        CHECK(compared > 0 && pct >= PROBE_MATCH_PCT, msg);
    }

    /* Best-ordinal sanity: no other portrait ordinal should win the
     * 24-ordinal sweep on the same framebuffer. */
    {
        int b = best_ordinal(portraits, fb);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 17, SOUTH) best ordinal sweep = %d (want %d)",
                 b, SOUTH_RETURN_ORDINAL);
        CHECK(b == SOUTH_RETURN_ORDINAL, msg);
    }

    /* ── D) No-floating side-wall contract ───────────────────────── */
    printf("\n[Group D] No-floating side walls at (2, 17): N/E/W must not show ordinal 7\n");
    {
        static const int kSideDirs[3] = { DIR_NORTH, DIR_EAST, DIR_WEST };
        static const char* kSideNames[3] = { "NORTH", "EAST", "WEST" };
        int i;
        for (i = 0; i < 3; ++i) {
            set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, kSideDirs[i]);
            memset(fb, 0, sizeof(fb));
            M11_GameView_Draw(&game, fb, FB_W, FB_H);

            int r = match_ordinal(portraits, fb, SOUTH_RETURN_ORDINAL);
            int matched = MATCH_MATCHED(r);
            int compared = MATCH_COMPARED(r);
            int pct = (compared > 0) ? (matched * 100 / compared) : 0;
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(2, 17, %s) ordinal-7 leak in D1C rect = %d/%d (%d%% <= %d%%)",
                     kSideNames[i], matched, compared, pct, PROBE_LEAK_PCT);
            CHECK(compared == 0 || pct < PROBE_LEAK_PCT, msg);
        }
    }

    /* ── E) Re-blt invariant on south->west transition ──────────── */
    printf("\n[Group E] Re-blt invariant on south(7) -> west(no portrait)\n");
    {
        unsigned char fbSouth[FB_W * FB_H];
        unsigned char fbWest[FB_W * FB_H];

        set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, SOUTH_RETURN_DIR);
        memset(fbSouth, 0, sizeof(fbSouth));
        M11_GameView_Draw(&game, fbSouth, FB_W, FB_H);

        set_pose(&game, SOUTH_RETURN_MAP_X, SOUTH_RETURN_MAP_Y, WEST_RETURN_DIR);
        memset(fbWest, 0, sizeof(fbWest));
        M11_GameView_Draw(&game, fbWest, FB_W, FB_H);

        /* The west framebuffer's ordinal-7 leak must be at or below
         * the leak tolerance the existing zorder reblt probe locks. */
        int r = match_ordinal(portraits, fbWest, SOUTH_RETURN_ORDINAL);
        int matched = MATCH_MATCHED(r);
        int compared = MATCH_COMPARED(r);
        int pct = (compared > 0) ? (matched * 100 / compared) : 0;
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(2, 17, SOUTH -> WEST) ordinal-7 stale pixels = %d/%d "
                 "(%d%% <= %d%%, re-blt cleared the portrait)",
                 matched, compared, pct, PROBE_LEAK_PCT);
        CHECK(compared == 0 || pct < PROBE_LEAK_PCT, msg);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    if (g_fail != 0) {
        result = 1;
    }
    return result;
}
