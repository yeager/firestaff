/*
 * firestaff_dm1_v1_hoc_champion_portrait_07_after_party_shuffle_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 7                  (mirror catalog record TIGGY, title TAMAL)
 *   route   after_party_shuffle (C040 panel select -> F0282 C162 cancel
 *                                branch -> two F0284 rotations via
 *                                M11_GameView_HandleInput(TURN_RIGHT)
 *                                -> party lands on a different
 *                                direction at the same cell -> portrait
 *                                rect is checked against the front
 *                                wall -> second two F0284 rotations
 *                                bring party back to SOUTH -> portrait
 *                                rect must again paint ordinal-7 at
 *                                (96, 35) -> C040 panel reopens)
 *   aspect  portrait_rect_position (the D1C front-wall
 *                                    destination rectangle is exactly
 *                                    at viewport (96, 35) of size
 *                                    32x29 inside the C346 wall-mirror
 *                                    frame at viewport (80, 29) of
 *                                    size 64x43 across the full
 *                                    F0284 / F0128 redraw cycle)
 *
 * The C026 champion-portrait atlas is the source-locked 256x87, 8x3
 * grid of 32x29 portraits (DEFS.H:821-826 M027_PORTRAIT_X /
 * M028_PORTRAIT_Y macro encoding; ordinals 0..23).  Ordinal 7 sits
 * at row 0, column 7 (last column of row 0):
 *
 *     srcX = (7 & 7) << 5 = 224
 *     srcY = (7 >> 3) * 29 =  0
 *
 * The D1C front-wall destination rectangle is source-locked
 * (DUNVIEW.C:3913-3928 and DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 * ChampionPortraitOnWall = {96, 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29  (viewport coords)
 *     fbX  = 96, fbY = 68, dstW = 32, dstH = 29   (320x200 framebuffer)
 *
 * Honest slice note:
 *   The only Hall-of-Champions cell in real DM1 V1 PC 3.4 DUNGEON.DAT
 *   that exposes the C127 sensor with sensorData == 7 is mapIndex=0,
 *   (mapX=2, mapY=17) with the party facing SOUTH (direction=2).
 *   No other map 0 cell / direction pair yields ordinal 7 from the
 *   shipped DUNGEON.DAT (per the firestaff_dm1_v1_hall_corridor_
 *   ordinal_scanner_probe and the firestaff_dm1_v1_champion_mirror_
 *   ordinal_07_portrait_rect_position_probe).  The party therefore
 *   starts at (2, 17) facing SOUTH and uses TURN_RIGHT rotations
 *   instead of TURN_RIGHT-from-NORTH (which is what ordinals 0/1/3
 *   use on the (1, 2) cell).  After two TURN_RIGHT inputs the
 *   party lands at NORTH at the same (2, 17) cell -- the front cell
 *   moves away from the seeded (2, 18) sensor.  After two more
 *   TURN_RIGHT inputs the party lands back at SOUTH -- the front
 *   cell returns to (2, 18) and the portrait re-paints.
 *
 *   This is the only available route for ordinal 7 in shipped DM1
 *   V1 DUNGEON.DAT and no seeding is required (the C127 sensor on
 *   the north wall of (2, 18) naturally carries sensorData=7).
 *
 * This probe is disjoint from every sibling probe:
 *
 *   firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
 *     - covers ordinal 7 at (2, 17, SOUTH) but does NOT exercise any
 *       party shuffle.  It locks the static D1C portrait_rect_
 *       position and the no-floating invariant on the side walls,
 *       plus the candidate-panel resurrect round-trip; it does not
 *       drive F0284 between select and reopen.
 *
 *   firestaff_dm1_v1_champion_mirror_ordinal_07_south_return_portrait_rect_position_runtime_probe
 *     - covers ordinal 7 at (2, 17, SOUTH) using the south_return
 *       route framing (no candidate panel, no F0284 between draws).
 *       It locks the strict 24-ordinal best-fit sweep and the
 *       south->west re-blt invariant.  It does NOT exercise the
 *       C040 panel state machine or any F0284 rotations.
 *
 *   firestaff_dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat
 *     - pass783 contract-only synthetic state model that proves the
 *       C160 Yes close click AFTER two F0284 rotations lands on the
 *       post-shuffle party (G0305-1).  Contract-only: no real
 *       M11_GameView / D1C blit, no per-pixel portrait_rect_position
 *       invariant.
 *
 *   firestaff_dm1_v1_hoc_champion_portrait_03_after_party_shuffle_portrait_rect_position_runtime_probe
 *     - covers ordinal 3 (AZIZI) on the after_party_shuffle route at
 *       (1, 2) NORTH -- the (1, 2) cell carries a HALK (ordinal 1)
 *       sensor in shipped DM1 V1, so that probe seeds the sensor
 *       data 1 -> 3.  Ordinal 7 here needs no seeding because the
 *       sensor on (2, 18) naturally carries sensorData=7.
 *
 * This probe proves these invariants for the after_party_shuffle
 * slice only:
 *
 *   (1) Mirror catalog identity: ordinal 7 maps to TIGGY / TAMAL
 *       and the catalog count is at least 8 so ordinal 7 is real.
 *   (2) D1C frame: the wall-mirror frame zone helper returns the
 *       source-locked (80, 29, 64, 43) and the portrait rect
 *       (96, 35, 32, 29) is fully contained by it.
 *   (3) Baseline portrait_rect_position at (2, 17, SOUTH): the
 *       shipped DM1 V1 C127 sensor on (2, 18) naturally carries
 *       sensorData=7, so no seeding is required and the D1C rect
 *       paints ordinal-7 pixels at >= 90% match, side walls
 *       (left x<96 row band 33..64 and right x>=128 row band
 *       33..64) stay below the warm-color threshold so the
 *       portrait does NOT float.
 *   (4) cancel-after-select: F0282 C162 cancel closes the C040
 *       panel without disabling the front-mirror route
 *       (m11_disable_front_mirror_route is NOT called from the
 *       cancel path), so the portrait rect re-appears after
 *       cancel.
 *   (5) F0284 first half: two consecutive TURN_RIGHT inputs rotate
 *       the party SOUTH -> WEST -> NORTH at the same cell (2, 17).
 *       After the two rotations, the D1C rect must NOT paint
 *       ordinal-7 pixels (front cell changed from (2, 18) to
 *       (2, 16), sensorData=7 not on the new front cell), and
 *       the side walls must stay below the warm-color threshold
 *       so the portrait does NOT float onto them.
 *   (6) F0284 second half: two more consecutive TURN_RIGHT inputs
 *       rotate the party NORTH -> EAST -> SOUTH back to (2, 17)
 *       SOUTH.  The D1C rect MUST paint ordinal-7 pixels at
 *       >= 90% match again; the front cell is back at (2, 18)
 *       with the natural sensorData=7 so the F0128 redraw must
 *       pick it up.  This is the central "after_party_shuffle"
 *       invariant: the portrait_rect_position survives four
 *       F0284 rotations (full 360-degree spin) and re-resolves
 *       correctly when the front route is back in view.
 *   (7) Reopen-after-shuffle: a fresh SelectFrontMirrorCandidate
 *       at (2, 17) SOUTH opens the C040 panel with ordinal 7
 *       again; the panel-on redraw must suppress the portrait
 *       rect (BUG-120/121 panel guard).
 *   (8) Cancel + F0284 + side-wall invariant at (2, 17, WEST):
 *       after a cancel and one F0284 rotation to WEST, the
 *       D1C rect must NOT paint ordinal-7 pixels and the side
 *       walls (left x<96 and right x>=128 in the row band) must
 *       stay below the warm-color threshold so the portrait
 *       does NOT float onto side walls.
 *
 * Honesty: this is Firestaff deterministic-runtime evidence, not
 * original-DM1 PC 3.4 pixel parity.  The probe uses real DM1 V1
 * data (PC 3.4 DUNGEON.DAT 33357 bytes + GRAPHICS.DAT with the
 * C026 portrait strip).  No original-vs-Firestaff comparison is
 * claimed.  The panel-on redraw portrait suppression is a
 * Firestaff runtime behavior (BUG-120/121 guard), not an
 * original-DM1 behavior; that suppression is locked here as a
 * Firestaff invariant.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W                = 320,
    FB_H                = 200,
    VIEWPORT_X          = 0,
    VIEWPORT_Y          = 33,
    /* Source-locked D1C portrait destination rectangle (DUNVIEW.C:3913
     * and DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
     * = {96, 127, 35, 63}). */
    D1C_PORTRAIT_X      = VIEWPORT_X + 96,
    D1C_PORTRAIT_Y      = VIEWPORT_Y + 35,
    D1C_PORTRAIT_W      = 32,
    D1C_PORTRAIT_H      = 29,
    /* Source-locked C346 wall-mirror frame (DUNVIEW.C G0205 coordSet 5
     * for the D1C champion-mirror route): viewport origin (80, 29),
     * size 64x43 -- the portrait cutout lives at +16, +6 inside it. */
    D1C_FRAME_X         = VIEWPORT_X + 80,
    D1C_FRAME_Y         = VIEWPORT_Y + 29,
    D1C_FRAME_W         = 64,
    D1C_FRAME_H         = 43,
    /* Side-wall sampling bands (in the same row band as the portrait
     * rect) used to verify the portrait does NOT float onto the
     * ordinary side walls (the BUG-DNY-DM1-2026-06-16 "floating
     * portrait" failure mode). */
    PORTRAIT_BAND_Y0    = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1    = VIEWPORT_Y + 65,
    SIDE_WALL_LEFT_X0   = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_X1   = VIEWPORT_X + 96,
    SIDE_WALL_RIGHT_X0  = VIEWPORT_X + 128,
    SIDE_WALL_RIGHT_X1  = VIEWPORT_X + 144,
    /* Match thresholds: 90% dominance matches the existing ordinal-03
     * after_party_shuffle sibling probe and the south_return probe;
     * <= 20% match while the C040 panel is live matches the BUG-120/
     * 121 panel-guard invariant.  PORTRAIT_WARM_THRESHOLD matches the
     * existing portrait03 / cancel_reopen sibling probes. */
    MATCH_DOMINANCE_PCT     = 90,
    PANEL_GUARD_MATCH_MAX   = 20,
    PORTRAIT_WARM_THRESHOLD = 30,
    /* DM1 V1 direction constants are macros in
     * memory_champion_state_pc34_compat.h (DIR_NORTH=0, DIR_EAST=1,
     * DIR_SOUTH=2, DIR_WEST=3).  Used directly in this probe. */
    TARGET_ORDINAL     = 7,
    /* The shipped DM1 V1 DUNGEON.DAT C127 sensor on the north wall
     * of (2, 18) -- visible from the party at (2, 17) facing SOUTH
     * -- carries sensorData=7 (TIGGY) naturally.  No seeding is
     * required for ordinal 7 at this cell. */
    PARTY_MAP_X        = 2,
    PARTY_MAP_Y        = 17,
    PARTY_MAP_INDEX    = 0
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count "warm" pixels in a framebuffer rectangle.  The C026 portrait
 * sprites use the warm palette set {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}
 * (green / red / orange / peach / yellow / blue) for skin tones,
 * clothing, and backgrounds.  Grey-stone wall texture uses indices
 * 0x01, 0x02, 0x0D.  Counting warm pixels is a coarse but reliable
 * way to distinguish "portrait is here" from "wall only" in the
 * C026 cutout and on the side walls.  Same definition used by the
 * portrait03 / cancel_reopen sibling probes. */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    if (!fb || w <= 0 || h <= 0) return 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++cnt;
                    break;
                default:
                    break;
            }
        }
    }
    return cnt;
}

/* Per-pixel match percent between the C026 atlas cell for `ordinal`
 * (at (ordinal & 7) * 32, (ordinal >> 3) * 29 stride 32x29) and the
 * D1C destination rectangle in the framebuffer.  Atlas pixel index 1
 * is the C01_COLOR_DARK_GRAY transparency mask used by the
 * DUNVIEW.C:3913-3928 blit, so transparent atlas pixels are skipped
 * on both sides.  Returns matched*100/compared. */
static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char src;
            unsigned char dst;
            int sx = srcX + x;
            int sy = srcY + y;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src == 1) continue; /* C01_COLOR_DARK_GRAY transparent */
            dst = M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_Y + y) * FB_W +
                                         (D1C_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Park the party at the (2, 17) D1C front-mirror route facing SOUTH
 * -- the only Hall-of-Champions cell that exposes ordinal 7 in the
 * shipped DM1 V1 PC 3.4 DUNGEON.DAT (C127 sensor on the north wall
 * of (2, 18) with sensorData=7, reached by the party at (2, 17)
 * facing SOUTH through the front-wall filter per DUNGEON.C:2573
 * M011_CELL(sensor) - partyDirection + 3 + wall-only filter).
 * Resets all candidate-panel state so the probe starts from a
 * clean slate. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = PARTY_MAP_INDEX;
    state->world.party.mapX = PARTY_MAP_X;
    state->world.party.mapY = PARTY_MAP_Y;
    state->world.party.direction = DIR_SOUTH;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

/* Drive two consecutive F0284 party-direction rotations in place via
 * M11_GameView_HandleInput(TURN_RIGHT).  Returns 1 on success, 0 if
 * either turn did not return M11_GAME_INPUT_REDRAW.  Each TURN_RIGHT
 * rotates the party by +90 degrees (CHAMPION.C F0284:93-130) and
 * triggers the F0296_CHAMPION_DrawChangedObjectIcons redraw. */
static int rotate_party_twice_right(M11_GameViewState* state) {
    M11_GameInputResult r1, r2;
    r1 = M11_GameView_HandleInput(state, M12_MENU_INPUT_TURN_RIGHT);
    if (r1 != M11_GAME_INPUT_REDRAW) return 0;
    r2 = M11_GameView_HandleInput(state, M12_MENU_INPUT_TURN_RIGHT);
    if (r2 != M11_GAME_INPUT_REDRAW) return 0;
    return 1;
}

static void dump_first_catalog(M11_GameViewState* game, int maxOrdinal) {
    int ord;
    printf("[catalog] DM1 V1 Hall mirror ordinal -> name/title (first %d)\n",
           maxOrdinal);
    for (ord = 0; ord < maxOrdinal; ++ord) {
        char name[64];
        char title[64];
        int rc;
        name[0] = '\0';
        title[0] = '\0';
        rc = M11_GameView_GetMirrorNameByOrdinal(game, ord, name,
                                                 (int)sizeof(name));
        if (rc > 0) {
            (void)M11_GameView_GetMirrorTitleByOrdinal(game, ord, title,
                                                       (int)sizeof(title));
        }
        printf("[catalog]   ordinal=%d name=\"%s\" title=\"%s\"\n",
               ord, name, title);
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    int catalogCount;
    char ordinalName[64];
    char ordinalTitle[64];
    int selectRc, cancelRc, reopenRc;
    int turnRc;
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int frontOrdinal;
    int matchBaseline, matchAfterCancel, matchAfterNorth, matchAfterSouthBack;
    int matchAfterReopen, matchAfterWest;
    int leftSideBaseline, rightSideBaseline;
    int leftSideAfterCancel, rightSideAfterCancel;
    int leftSideAfterNorth, rightSideAfterNorth;
    int leftSideAfterSouthBack, rightSideAfterSouthBack;
    int leftSideAfterWest, rightSideAfterWest;
    int countAfterSelect, countAfterCancel, countAfterReopen;
    int countBaseline;
    unsigned char fbBaseline[FB_W * FB_H];
    unsigned char fbAfterSelect[FB_W * FB_H];
    unsigned char fbAfterCancel[FB_W * FB_H];
    unsigned char fbAfterNorth[FB_W * FB_H];
    unsigned char fbAfterSouthBack[FB_W * FB_H];
    unsigned char fbAfterReopen[FB_W * FB_H];
    unsigned char fbAfterWest[FB_W * FB_H];

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    printf("=== DM1 V1 HoC champion portrait 07 after_party_shuffle "
           "portrait_rect_position probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&state.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    CHECK(portraits && portraits->loaded && portraits->pixels &&
          portraits->width >= 256 && portraits->height >= 87,
          "C026 champion portrait strip is loaded");

    catalogCount = M11_GameView_GetMirrorCatalogCount(&state);
    CHECK(catalogCount >= TARGET_ORDINAL + 1,
          "Hall mirror catalog has at least 8 entries (ordinal 7 must exist)");
    printf("[catalog] ordinal count=%d\n", catalogCount);
    dump_first_catalog(&state, catalogCount < 8 ? catalogCount : 8);

    ordinalName[0] = '\0';
    ordinalTitle[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(&state, TARGET_ORDINAL,
                                              ordinalName,
                                              (int)sizeof(ordinalName));
    (void)M11_GameView_GetMirrorTitleByOrdinal(&state, TARGET_ORDINAL,
                                               ordinalTitle,
                                               (int)sizeof(ordinalTitle));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror ordinal %d catalog name+title resolved "
                 "(name=\"%s\" title=\"%s\")",
                 TARGET_ORDINAL, ordinalName, ordinalTitle);
        CHECK(ordinalName[0] != '\0' && ordinalTitle[0] != '\0', msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror ordinal %d catalog name is \"TIGGY\" (got \"%s\")",
                 TARGET_ORDINAL, ordinalName);
        CHECK(strcmp(ordinalName, "TIGGY") == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror ordinal %d catalog title is \"TAMAL\" (got \"%s\")",
                 TARGET_ORDINAL, ordinalTitle);
        CHECK(strcmp(ordinalTitle, "TAMAL") == 0, msg);
    }
    printf("[catalog] ordinal %d -> name=\"%s\" title=\"%s\"\n",
           TARGET_ORDINAL, ordinalName, ordinalTitle);

    /* ----------------------------------------------------------------
     * Group A - baseline portrait_rect_position at (2, 17) SOUTH
     * ---------------------------------------------------------------- */
    printf("\n[Group A] baseline portrait_rect_position at (2, 17, SOUTH) "
           "(shipped C127 sensor on (2, 18) carries sensorData=7)\n");

    park_d1c_front_route(&state);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "shipped (2, 17, SOUTH) route reports ordinal %d (got %d)",
                 TARGET_ORDINAL, frontOrdinal);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }

    CHECK(M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY,
                                              &ornW, &ornH) == 1,
          "D1C wall-mirror frame zone helper succeeds");
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "D1C wall-mirror frame at viewport (80, 29, 64, 43) "
                 "(got (%d, %d, %d, %d))",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
    }
    CHECK(96 == ornX + 16,
          "D1C portrait rect viewport x is frame x + 16 (96 == 80 + 16)");
    CHECK(35 == ornY + 6,
          "D1C portrait rect viewport y is frame y + 6 (35 == 29 + 6)");
    CHECK(96 >= ornX &&
          35 >= ornY &&
          96 + D1C_PORTRAIT_W <= ornX + ornW &&
          35 + D1C_PORTRAIT_H <= ornY + ornH,
          "D1C portrait rect viewport (96, 35, 32, 29) is fully "
          "contained by C346 wall-mirror frame");

    memset(fbBaseline, 0, sizeof(fbBaseline));
    M11_GameView_Draw(&state, fbBaseline, FB_W, FB_H);
    matchBaseline = match_portrait_at_rect(portraits, fbBaseline,
                                            TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline D1C rect carries ordinal %d pixels at >= %d%% "
                 "match (got %d%%)",
                 TARGET_ORDINAL, MATCH_DOMINANCE_PCT, matchBaseline);
        CHECK(matchBaseline >= MATCH_DOMINANCE_PCT, msg);
    }

    /* No-floating invariant at (2, 17) SOUTH: side walls in the same
     * row band must NOT carry the warm-color palette. */
    leftSideBaseline = rect_warm_count(fbBaseline,
                                       SIDE_WALL_LEFT_X0, PORTRAIT_BAND_Y0,
                                       SIDE_WALL_LEFT_X1 - SIDE_WALL_LEFT_X0,
                                       PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideBaseline = rect_warm_count(fbBaseline,
                                        SIDE_WALL_RIGHT_X0, PORTRAIT_BAND_Y0,
                                        SIDE_WALL_RIGHT_X1 - SIDE_WALL_RIGHT_X0,
                                        PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline left side wall stays below warm threshold %d "
                 "(got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideBaseline);
        CHECK(leftSideBaseline < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "baseline right side wall stays below warm threshold %d "
                 "(got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideBaseline);
        CHECK(rightSideBaseline < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - select (C040 panel live) and cancel (F0282 C162)
     * ----------------------------------------------------------------
     * Source-locked contract: M11_GameView_SelectFrontMirrorCandidate
     * (F0280) sets candidateMirrorPanelActive=1 and recruits the
     * candidate champion.  M11_GameView_CancelMirrorCandidate
     * (F0282 C162 branch) clears the candidate slot, sets
     * candidateMirrorPanelActive=0, and does NOT call
     * m11_disable_front_mirror_route so the C127 sensor stays
     * active and the portrait rect reappears on the next redraw. */
    printf("\n[Group B] select -> cancel (C040 panel lives then dies)\n");

    countBaseline = state.world.party.championCount;
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterSelect = state.world.party.championCount;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (2, 17, SOUTH) returns 1 "
                 "(got %d), championCount=%d (was %d)",
                 selectRc, countAfterSelect, countBaseline);
        CHECK(selectRc == 1 &&
              countAfterSelect == countBaseline + 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after select: panel live (active=%d) ordinal=%d",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL, msg);
    }

    /* Panel-on redraw: portrait rect suppressed by BUG-120/121 panel
     * guard.  Match should drop to <= PANEL_GUARD_MATCH_MAX. */
    memset(fbAfterSelect, 0, sizeof(fbAfterSelect));
    M11_GameView_Draw(&state, fbAfterSelect, FB_W, FB_H);
    {
        int matchSelect = match_portrait_at_rect(portraits, fbAfterSelect,
                                                 TARGET_ORDINAL);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw does not leave ordinal %d as a stale "
                 "D1C sprite (<= %d%% match, got %d%%)",
                 TARGET_ORDINAL, PANEL_GUARD_MATCH_MAX, matchSelect);
        CHECK(matchSelect <= PANEL_GUARD_MATCH_MAX, msg);
    }

    cancelRc = M11_GameView_CancelMirrorCandidate(&state);
    countAfterCancel = state.world.party.championCount;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "CancelMirrorCandidate returns 1 (got %d), "
                 "championCount=%d (was %d before select)",
                 cancelRc, countAfterCancel, countBaseline);
        CHECK(cancelRc == 1 &&
              countAfterCancel == countBaseline, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: panel closed (active=%d) ordinal=%d",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1, msg);
    }

    /* After cancel the panel is closed but the C127 sensor is still
     * at sensorData=7 on (2, 18), so the D1C rect must paint
     * ordinal-7 pixels again (the central no-floating check). */
    memset(fbAfterCancel, 0, sizeof(fbAfterCancel));
    M11_GameView_Draw(&state, fbAfterCancel, FB_W, FB_H);
    matchAfterCancel = match_portrait_at_rect(portraits, fbAfterCancel,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C rect still carries ordinal %d "
                 "pixels at >= %d%% match (got %d%%)",
                 TARGET_ORDINAL, MATCH_DOMINANCE_PCT, matchAfterCancel);
        CHECK(matchAfterCancel >= MATCH_DOMINANCE_PCT, msg);
    }
    leftSideAfterCancel = rect_warm_count(fbAfterCancel,
                                          SIDE_WALL_LEFT_X0, PORTRAIT_BAND_Y0,
                                          SIDE_WALL_LEFT_X1 - SIDE_WALL_LEFT_X0,
                                          PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterCancel = rect_warm_count(fbAfterCancel,
                                           SIDE_WALL_RIGHT_X0, PORTRAIT_BAND_Y0,
                                           SIDE_WALL_RIGHT_X1 - SIDE_WALL_RIGHT_X0,
                                           PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: left side wall stays below warm threshold %d "
                 "(got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterCancel);
        CHECK(leftSideAfterCancel < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: right side wall stays below warm threshold %d "
                 "(got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterCancel);
        CHECK(rightSideAfterCancel < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - F0284 first half: SOUTH -> WEST -> NORTH at (2, 17)
     * ----------------------------------------------------------------
     * Two TURN_RIGHT inputs through M11_GameView_HandleInput drive
     * CHAMPION.C F0284:93-130 SetPartyDirection twice, calling
     * F0296_CHAMPION_DrawChangedObjectIcons after each.  The front
     * cell changes from (2, 18) to (2, 16); the C127 sensor on
     * (2, 18) is no longer in view, so the D1C rect must NOT paint
     * ordinal-7 pixels and the side walls must stay below the warm
     * threshold. */
    printf("\n[Group C] F0284 x2: SOUTH -> WEST -> NORTH at (2, 17)\n");

    turnRc = rotate_party_twice_right(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "two TURN_RIGHT inputs return M11_GAME_INPUT_REDRAW "
                 "(got first-last=%d)",
                 (int)turnRc);
        CHECK(turnRc == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "after two TURN_RIGHT inputs: party direction = NORTH "
                 "(got %d, want %d)",
                 state.world.party.direction, DIR_NORTH);
        CHECK(state.world.party.direction == DIR_NORTH, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2: party still at map (2, 17) (got "
                 "(%d, %d))",
                 state.world.party.mapX, state.world.party.mapY);
        CHECK(state.world.party.mapX == PARTY_MAP_X &&
              state.world.party.mapY == PARTY_MAP_Y, msg);
    }
    {
        int frontNorth = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to NORTH: front mirror ordinal is -1 "
                 "(front cell moved away from natural sensor on (2, 18), "
                 "got %d)",
                 frontNorth);
        CHECK(frontNorth == -1, msg);
    }

    memset(fbAfterNorth, 0, sizeof(fbAfterNorth));
    M11_GameView_Draw(&state, fbAfterNorth, FB_W, FB_H);
    matchAfterNorth = match_portrait_at_rect(portraits, fbAfterNorth,
                                             TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to NORTH: D1C rect does NOT carry "
                 "ordinal %d pixels (got %d%%, must be < %d%%)",
                 TARGET_ORDINAL, matchAfterNorth, MATCH_DOMINANCE_PCT);
        CHECK(matchAfterNorth < MATCH_DOMINANCE_PCT, msg);
    }
    leftSideAfterNorth = rect_warm_count(fbAfterNorth,
                                         SIDE_WALL_LEFT_X0, PORTRAIT_BAND_Y0,
                                         SIDE_WALL_LEFT_X1 - SIDE_WALL_LEFT_X0,
                                         PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterNorth = rect_warm_count(fbAfterNorth,
                                          SIDE_WALL_RIGHT_X0, PORTRAIT_BAND_Y0,
                                          SIDE_WALL_RIGHT_X1 - SIDE_WALL_RIGHT_X0,
                                          PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to NORTH: left side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterNorth);
        CHECK(leftSideAfterNorth < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to NORTH: right side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterNorth);
        CHECK(rightSideAfterNorth < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - F0284 second half: NORTH -> EAST -> SOUTH at (2, 17)
     * ----------------------------------------------------------------
     * Two more TURN_RIGHT inputs rotate the party back to SOUTH at
     * the same cell.  The front cell is now (2, 18) again, with the
     * natural sensorData=7, so the F0128 redraw must paint
     * ordinal-7 pixels at the source-locked (96, 35) destination
     * rectangle.  This is the central "after_party_shuffle"
     * invariant: the portrait_rect_position survives a full
     * 360-degree spin via four F0284 rotations and re-resolves
     * correctly when the front route is back in view. */
    printf("\n[Group D] F0284 x2 again: NORTH -> EAST -> SOUTH at (2, 17) "
           "(portrait_rect_position must re-paint ordinal 7)\n");

    turnRc = rotate_party_twice_right(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "two more TURN_RIGHT inputs return M11_GAME_INPUT_REDRAW "
                 "(got %d)",
                 (int)turnRc);
        CHECK(turnRc == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "after four TURN_RIGHT inputs total: party direction = "
                 "SOUTH (got %d, want %d)",
                 state.world.party.direction, DIR_SOUTH);
        CHECK(state.world.party.direction == DIR_SOUTH, msg);
    }
    {
        int frontSouthBack = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to SOUTH: front mirror ordinal is %d "
                 "(got %d)",
                 TARGET_ORDINAL, frontSouthBack);
        CHECK(frontSouthBack == TARGET_ORDINAL, msg);
    }

    memset(fbAfterSouthBack, 0, sizeof(fbAfterSouthBack));
    M11_GameView_Draw(&state, fbAfterSouthBack, FB_W, FB_H);
    matchAfterSouthBack = match_portrait_at_rect(portraits, fbAfterSouthBack,
                                                 TARGET_ORDINAL);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to SOUTH: D1C portrait_rect_position "
                 "at (96, 35) still carries ordinal %d pixels at >= %d%% "
                 "match (got %d%%) -- central after_party_shuffle invariant",
                 TARGET_ORDINAL, MATCH_DOMINANCE_PCT, matchAfterSouthBack);
        CHECK(matchAfterSouthBack >= MATCH_DOMINANCE_PCT, msg);
    }
    leftSideAfterSouthBack = rect_warm_count(fbAfterSouthBack,
                                             SIDE_WALL_LEFT_X0, PORTRAIT_BAND_Y0,
                                             SIDE_WALL_LEFT_X1 - SIDE_WALL_LEFT_X0,
                                             PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterSouthBack = rect_warm_count(fbAfterSouthBack,
                                              SIDE_WALL_RIGHT_X0, PORTRAIT_BAND_Y0,
                                              SIDE_WALL_RIGHT_X1 - SIDE_WALL_RIGHT_X0,
                                              PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to SOUTH: left side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterSouthBack);
        CHECK(leftSideAfterSouthBack < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to SOUTH: right side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterSouthBack);
        CHECK(rightSideAfterSouthBack < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - reopen-after-shuffle: select (C040 panel live again)
     * ----------------------------------------------------------------
     * A fresh SelectFrontMirrorCandidate at (2, 17) SOUTH opens the
     * C040 panel with ordinal 7 again; the panel-on redraw must
     * suppress the portrait rect (BUG-120/121 panel guard). */
    printf("\n[Group E] reopen after F0284 x4: select again, panel live\n");

    reopenRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterReopen = state.world.party.championCount;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate reopen after shuffle returns 1 "
                 "(got %d), championCount=%d (was %d before select)",
                 reopenRc, countAfterReopen, countBaseline);
        CHECK(reopenRc == 1 &&
              countAfterReopen == countBaseline + 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: panel live (active=%d) ordinal=%d",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL, msg);
    }

    memset(fbAfterReopen, 0, sizeof(fbAfterReopen));
    M11_GameView_Draw(&state, fbAfterReopen, FB_W, FB_H);
    matchAfterReopen = match_portrait_at_rect(portraits, fbAfterReopen,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw after reopen does not leave ordinal %d "
                 "as a stale D1C sprite (<= %d%% match, got %d%%)",
                 TARGET_ORDINAL, PANEL_GUARD_MATCH_MAX, matchAfterReopen);
        CHECK(matchAfterReopen <= PANEL_GUARD_MATCH_MAX, msg);
    }

    /* ----------------------------------------------------------------
     * Group F - cancel + single F0284 to WEST: portrait_rect_position
     * must NOT show ordinal 7 on the side walls (no-floating
     * invariant for the after_party_shuffle route).
     * ---------------------------------------------------------------- */
    printf("\n[Group F] cancel + single F0284 to WEST: portrait must not "
           "float onto side walls\n");

    cancelRc = M11_GameView_CancelMirrorCandidate(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "CancelMirrorCandidate after reopen returns 1 (got %d)",
                 cancelRc);
        CHECK(cancelRc == 1, msg);
    }
    {
        M11_GameInputResult r1west;
        r1west = M11_GameView_HandleInput(&state,
                                           M12_MENU_INPUT_TURN_RIGHT);
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "single TURN_RIGHT to WEST returns "
                     "M11_GAME_INPUT_REDRAW (got %d)",
                     (int)r1west);
            CHECK(r1west == M11_GAME_INPUT_REDRAW, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "after single TURN_RIGHT: party direction = WEST "
                     "(got %d, want %d)",
                     state.world.party.direction, DIR_WEST);
            CHECK(state.world.party.direction == DIR_WEST, msg);
        }
    }

    memset(fbAfterWest, 0, sizeof(fbAfterWest));
    M11_GameView_Draw(&state, fbAfterWest, FB_W, FB_H);
    matchAfterWest = match_portrait_at_rect(portraits, fbAfterWest,
                                            TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x1 to WEST: D1C rect does NOT carry ordinal "
                 "%d pixels (got %d%%, must be < %d%%)",
                 TARGET_ORDINAL, matchAfterWest, MATCH_DOMINANCE_PCT);
        CHECK(matchAfterWest < MATCH_DOMINANCE_PCT, msg);
    }
    leftSideAfterWest = rect_warm_count(fbAfterWest,
                                        SIDE_WALL_LEFT_X0, PORTRAIT_BAND_Y0,
                                        SIDE_WALL_LEFT_X1 - SIDE_WALL_LEFT_X0,
                                        PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterWest = rect_warm_count(fbAfterWest,
                                         SIDE_WALL_RIGHT_X0, PORTRAIT_BAND_Y0,
                                         SIDE_WALL_RIGHT_X1 - SIDE_WALL_RIGHT_X0,
                                         PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x1 to WEST: left side wall stays below warm "
                 "threshold %d (got %d) -- portrait must not float",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterWest);
        CHECK(leftSideAfterWest < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x1 to WEST: right side wall stays below warm "
                 "threshold %d (got %d) -- portrait must not float",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterWest);
        CHECK(rightSideAfterWest < PORTRAIT_WARM_THRESHOLD, msg);
    }

    M11_GameView_Shutdown(&state);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
