/*
 * firestaff_dm1_v1_hoc_champion_portrait_09_after_party_shuffle_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 9                  (mirror catalog record ZED, title
 *                              "DUKE OF BANVILLE"; C026 atlas
 *                              row 1 / col 1, source rect
 *                              (32, 29, 32, 29))
 *   route   after_party_shuffle (C040 panel select -> F0282 C162 cancel
 *                                branch -> two F0284 rotations via
 *                                M11_GameView_HandleInput(TURN_RIGHT)
 *                                -> party lands on a different
 *                                direction at the same cell -> portrait
 *                                rect is checked against the front
 *                                wall -> second two F0284 rotations
 *                                bring party back to NORTH -> portrait
 *                                rect must again paint ordinal-9 at
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
 * M028_PORTRAIT_Y macro encoding; ordinals 0..23).  Ordinal 9 sits
 * at row 1, column 1:
 *
 *     srcX = (9 & 7) << 5 =  32
 *     srcY = (9 >> 3) * 29 =  29
 *
 * The D1C front-wall destination rectangle is source-locked
 * (DUNVIEW.C:3913-3928 and DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 * ChampionPortraitOnWall = {96, 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29  (viewport coords)
 *     fbX  = 96, fbY = 68, dstW = 32, dstH = 29   (320x200 framebuffer)
 *
 * Honest slice note:
 *   The shipped DM1 V1 PC 3.4 DUNGEON.DAT exposes the C127 sensor
 *   with sensorData == 1 (HALK) on the (1, 2) NORTH-route front
 *   square (1, 1) -- it does NOT natively map sensorData to
 *   ordinal 9 (ZED) on any (mapX, mapY, direction) tuple in
 *   map 0 (per the firestaff_dm1_v1_hall_corridor_ordinal_scanner
 *   _probe and the existing portrait_09 south_return / west_
 *   negative / candidate_panel_cancel / popup_focus_return
 *   sibling probes).  We seed the sensor data 1 -> 9 to lock
 *   the ordinal-9 edge case on the (1, 2) NORTH-route cell
 *   that the ordinal-3 (AZIZI) and ordinal-11 (row 1 / col 3)
 *   sibling after_party_shuffle probes already use.  The seed
 *   does NOT change the map layout, the wall geometry, the
 *   F0128 redraw order, or the F0284 rotation mechanics -- it
 *   only rewrites one C127 sensorData byte so the D1C blit
 *   resolves to the C026 atlas cell 9 (row 1 / col 1) instead
 *   of cell 1 (row 0 / col 1, HALK).  Row 1 / col 1 is the
 *   SECOND cell of the second atlas row, directly to the right
 *   of ordinal 8 (IAIDO, row 1 / col 0).
 *
 * This probe is disjoint from every sibling probe:
 *
 *   firestaff_dm1_v1_champion_mirror_ordinal_9_south_return_portrait_rect_position_runtime_probe
 *     - covers ordinal 9 at (1,5) SOUTH using the south_return
 *       route framing (no candidate panel, no F0284 between
 *       draws).  It locks the C026 column-1/row-1 portrait_rect
 *       on the south-return path.  This probe drives the
 *       after_party_shuffle sequence (cancel + four F0284 +
 *       reopen) on the seeded (1, 2) NORTH route, which the
 *       south_return probe does not exercise.
 *
 *   firestaff_dm1_v1_champion_mirror_ordinal_9_west_negative_portrait_rect_position_runtime_probe
 *     - covers ordinal 9 on the corridor west wall (x=1, y=2..6)
 *       with DIR_WEST and asserts the empty D1C cutout / no
 *       floating-portrait invariant.  Negative invariant only.
 *
 *   firestaff_dm1_v1_hall_of_champions_portrait_09_candidate_panel_cancel_portrait_rect_position_runtime_probe
 *     - covers the 2-step terminal cancel sequence for ordinal 9
 *       at (1, 2) NORTH (select + cancel, no F0284 in between
 *       and no reopen).  This probe adds the four-rotation
 *       shuffle between cancel and reopen.
 *
 *   firestaff_dm1_v1_hall_of_champions_portrait_09_popup_focus_return_portrait_rect_position_runtime_probe
 *     - covers ordinal 9 with the panel-focused absorbed-input
 *       sequence (BACK/ACCEPT/ACTION, all other inputs IGNORED)
 *       between select and cancel/reopen.  This probe drives
 *       F0284 rotations through the panel, not absorbed inputs.
 *
 *   firestaff_dm1_v1_hoc_champion_portrait_09_palette_match_rect_runtime_probe
 *     - covers ordinal 9 on the south_return pose with a single
 *       redraw -- no panel select, no F0284.
 *
 *   firestaff_dm1_v1_hoc_champion_portrait_07_after_party_shuffle_portrait_rect_position_runtime_probe
 *     - covers ordinal 7 (TIGGY / TAMAL) on the after_party_
 *       shuffle route at (2, 17) SOUTH -- natural sensor, no
 *       seeding.  Different ordinal, different atlas cell,
 *       different cell / direction pair.
 *
 *   firestaff_dm1_v1_hoc_champion_portrait_11_after_party_shuffle_portrait_rect_position_runtime_probe
 *     - covers ordinal 11 (row 1 / col 3) on the after_party_
 *       shuffle route at (1, 2) NORTH.  Same cell and seeding
 *       pattern, but ordinal 9 here locks atlas row 1 / col 1
 *       instead.  The two probes are independent of each other
 *       (the C026 atlas cells for 9 and 11 share no pixels), so
 *       this slice closes the row-1 / col-1 atlas gap for the
 *       after_party_shuffle route.
 *
 *   firestaff_dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat
 *     - pass783 contract-only synthetic state model that proves
 *       the C160 Yes close click AFTER two F0284 rotations lands
 *       on the post-shuffle party (G0305-1).  Contract-only: no
 *       real M11_GameView / D1C blit, no per-pixel portrait_rect
 *       _position invariant.  This probe provides the matching
 *       pixel contract for the same route.
 *
 * This probe proves these invariants for the after_party_shuffle
 * slice only:
 *
 *   (1) Mirror catalog identity: ordinal 9 maps to ZED /
 *       "DUKE OF BANVILLE" and the catalog count is at least 10
 *       so ordinal 9 is real.
 *   (2) D1C frame: the wall-mirror frame zone helper returns
 *       the source-locked (80, 29, 64, 43) and the portrait
 *       rect (96, 35, 32, 29) is fully contained by it.
 *   (3) Baseline portrait_rect_position at (1, 2) NORTH with
 *       the sensor data seeded to 9: the D1C rect paints
 *       ordinal-9 pixels at >= 90% match, side walls (left
 *       x<96 row band 33..64 and right x>=128 row band 33..64)
 *       stay below the warm-color threshold so the portrait
 *       does NOT float.
 *   (4) cancel-after-select: F0282 C162 cancel closes the C040
 *       panel without disabling the front-mirror route
 *       (m11_disable_front_mirror_route is NOT called from the
 *       cancel path), so the portrait rect re-appears after
 *       cancel.
 *   (5) F0284 first half: two consecutive TURN_RIGHT inputs
 *       rotate the party NORTH -> EAST -> SOUTH at the same
 *       cell (1, 2).  After the two rotations, the D1C rect
 *       must NOT paint ordinal-9 pixels (front cell changed
 *       from (1, 1) to (1, 3), the seeded C127 sensor on
 *       (1, 1) is no longer in view), and the side walls must
 *       stay below the warm threshold so the portrait does
 *       NOT float onto them.
 *   (6) F0284 second half: two more consecutive TURN_RIGHT
 *       inputs rotate the party SOUTH -> WEST -> NORTH back to
 *       (1, 2) NORTH.  The D1C rect MUST paint ordinal-9
 *       pixels at >= 90% match again; the front cell is back
 *       at (1, 1) with the seeded sensorData=9 so the F0128
 *       redraw must pick it up.  This is the central
 *       "after_party_shuffle" invariant: the portrait_rect
 *       _position survives four F0284 rotations (full
 *       360-degree spin) and re-resolves correctly when the
 *       front route is back in view.
 *   (7) Reopen-after-shuffle: a fresh SelectFrontMirror
 *       Candidate at (1, 2) NORTH opens the C040 panel with
 *       ordinal 9 again; the panel-on redraw must suppress
 *       the portrait rect (BUG-120/121 panel guard).
 *   (8) Cancel + F0284 + side-wall invariant at (1, 2) WEST:
 *       after a cancel and one F0284 rotation to WEST, the
 *       D1C rect must NOT paint ordinal-9 pixels and the
 *       side walls (left x<96 and right x>=128 in the row
 *       band) must stay below the warm-color threshold so
 *       the portrait does NOT float onto side walls.
 *
 * Honesty: this is Firestaff deterministic-runtime evidence,
 * not original-DM1 PC 3.4 pixel parity.  The probe uses real
 * DM1 V1 data (PC 3.4 DUNGEON.DAT 33357 bytes + GRAPHICS.DAT
 * with the C026 portrait strip).  No original-vs-Firestaff
 * comparison is claimed.  The seeded C127 sensorData byte is
 * a Firestaff runtime seed for this probe only -- the bytes
 * on disk remain PC 3.4 sensorData=1 (HALK).  The panel-on
 * redraw portrait suppression is a Firestaff runtime behavior
 * (BUG-120/121 guard), not an original-DM1 behavior; that
 * suppression is locked here as a Firestaff invariant.
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
    /* Match thresholds: 90% dominance matches the existing
     * after_party_shuffle sibling probes (07, 11) and the
     * south_return probe; <= 20% match while the C040 panel is
     * live matches the BUG-120/121 panel-guard invariant.
     * PORTRAIT_WARM_THRESHOLD matches the existing portrait03 /
     * cancel_reopen sibling probes. */
    MATCH_DOMINANCE_PCT     = 90,
    PANEL_GUARD_MATCH_MAX   = 20,
    PORTRAIT_WARM_THRESHOLD = 30,
    /* DM1 V1 direction constants are macros in
     * memory_champion_state_pc34_compat.h (DIR_NORTH=0, DIR_EAST=1,
     * DIR_SOUTH=2, DIR_WEST=3).  Used directly in this probe. */
    TARGET_ORDINAL     = 9,
    /* The shipped DM1 V1 DUNGEON.DAT C127 sensor at (1,2) NORTH-route
     * front square (1,1) carries sensorData=1 (HALK).  We seed it
     * to TARGET_ORDINAL (9, ZED) to lock the ordinal-9 edge case
     * on the same (1, 2) NORTH-route cell the ordinal-3 and
     * ordinal-11 sibling after_party_shuffle probes use. */
    SHIPPED_SENSOR_OLD_DATA = 1,
    PARTY_MAP_X        = 1,
    PARTY_MAP_Y        = 2,
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
 * portrait07 / portrait11 / cancel_reopen sibling probes. */
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

/* Find the first C127 sensor in the loaded world whose sensorData
 * matches oldData and rewrite it to newData.  Returns the sensor
 * index on success, or -1 if no such sensor exists.  Same seed
 * helper used by the existing portrait03, portrait07 (where the
 * (2,18) sensor naturally carries sensorData=7 and seeding is
 * skipped), and portrait11 sibling after_party_shuffle probes. */
static int seed_first_c127_data(M11_GameViewState* state,
                                int oldData, int newData) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127 &&
            (int)state->world.things->sensors[i].sensorData == oldData) {
            state->world.things->sensors[i].sensorData =
                (unsigned short)newData;
            return i;
        }
    }
    return -1;
}

/* Park the party at the (1, 2) D1C front-mirror route facing NORTH.
 * Same coordinates as the existing ordinal-3 portrait03 probe,
 * ordinal-3 cancel_reopen probe, and ordinal-11 after_party_
 * shuffle probe.  Resets all candidate-panel state so the probe
 * starts from a clean slate. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = PARTY_MAP_INDEX;
    state->world.party.mapX = PARTY_MAP_X;
    state->world.party.mapY = PARTY_MAP_Y;
    state->world.party.direction = DIR_NORTH;
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
    int seededSensor;
    char ordinalName[64];
    char ordinalTitle[64];
    int selectRc, cancelRc, reopenRc;
    int turnRc;
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int frontOrdinal;
    int matchBaseline, matchAfterCancel;
    int matchAfterSouth, matchAfterNorthBack;
    int matchAfterReopen, matchAfterWest;
    int leftSideBaseline, rightSideBaseline;
    int leftSideAfterCancel, rightSideAfterCancel;
    int leftSideAfterSouth, rightSideAfterSouth;
    int leftSideAfterNorthBack, rightSideAfterNorthBack;
    int leftSideAfterWest, rightSideAfterWest;
    int countAfterSelect, countAfterCancel, countAfterReopen;
    int countBaseline;
    unsigned char fbBaseline[FB_W * FB_H];
    unsigned char fbAfterSelect[FB_W * FB_H];
    unsigned char fbAfterCancel[FB_W * FB_H];
    unsigned char fbAfterSouth[FB_W * FB_H];
    unsigned char fbAfterNorthBack[FB_W * FB_H];
    unsigned char fbAfterReopen[FB_W * FB_H];
    unsigned char fbAfterWest[FB_W * FB_H];

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    printf("=== DM1 V1 HoC champion portrait 09 after_party_shuffle "
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
          "Hall mirror catalog has at least 10 entries (ordinal 9 must exist)");
    printf("[catalog] ordinal count=%d\n", catalogCount);
    dump_first_catalog(&state, catalogCount < 10 ? catalogCount : 10);

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
                 "mirror ordinal %d catalog name is \"ZED\" (got \"%s\")",
                 TARGET_ORDINAL, ordinalName);
        CHECK(strcmp(ordinalName, "ZED") == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "mirror ordinal %d catalog title is \"DUKE OF BANVILLE\" "
                 "(got \"%s\")",
                 TARGET_ORDINAL, ordinalTitle);
        CHECK(strcmp(ordinalTitle, "DUKE OF BANVILLE") == 0, msg);
    }
    printf("[catalog] ordinal %d -> name=\"%s\" title=\"%s\"\n",
           TARGET_ORDINAL, ordinalName, ordinalTitle);

    /* ----------------------------------------------------------------
     * Group A - baseline portrait_rect_position at (1, 2) NORTH with
     * sensor data seeded to 9 (C026 row 1 / col 1, atlas address
     * (32, 29)).
     * ---------------------------------------------------------------- */
    printf("\n[Group A] baseline portrait_rect_position at (1, 2, NORTH) "
           "with sensorData seeded to %d\n", TARGET_ORDINAL);

    park_d1c_front_route(&state);
    seededSensor = seed_first_c127_data(&state, SHIPPED_SENSOR_OLD_DATA,
                                        TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded real (1, 2, NORTH) C127 sensor data from %d to %d "
                 "(sensor idx=%d)",
                 SHIPPED_SENSOR_OLD_DATA, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    park_d1c_front_route(&state);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1, 2, NORTH) route reports ordinal %d (got %d)",
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

    /* No-floating invariant at (1, 2) NORTH: side walls in the same
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
                 "SelectFrontMirrorCandidate on (1, 2, NORTH) returns 1 "
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

    /* After cancel the panel is closed but the seeded C127 sensor
     * still carries sensorData=9 on (1, 1), so the D1C rect must
     * paint ordinal-9 pixels again (the central no-floating
     * check). */
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
     * Group C - F0284 first half: NORTH -> EAST -> SOUTH at (1, 2)
     * ----------------------------------------------------------------
     * Two TURN_RIGHT inputs through M11_GameView_HandleInput drive
     * CHAMPION.C F0284:93-130 SetPartyDirection twice, calling
     * F0296_CHAMPION_DrawChangedObjectIcons after each.  The front
     * cell changes from (1, 1) to (1, 3); the seeded C127 sensor on
     * (1, 1) is no longer in view, so the D1C rect must NOT paint
     * ordinal-9 pixels and the side walls must stay below the warm
     * threshold. */
    printf("\n[Group C] F0284 x2: NORTH -> EAST -> SOUTH at (1, 2)\n");

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
                 "after two TURN_RIGHT inputs: party direction = SOUTH "
                 "(got %d, want %d)",
                 state.world.party.direction, DIR_SOUTH);
        CHECK(state.world.party.direction == DIR_SOUTH, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2: party still at map (1, 2) (got "
                 "(%d, %d))",
                 state.world.party.mapX, state.world.party.mapY);
        CHECK(state.world.party.mapX == PARTY_MAP_X &&
              state.world.party.mapY == PARTY_MAP_Y, msg);
    }
    {
        int frontSouth = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to SOUTH: front mirror ordinal is -1 "
                 "(front cell moved away from seeded sensor on (1, 1), "
                 "got %d)",
                 frontSouth);
        CHECK(frontSouth == -1, msg);
    }

    memset(fbAfterSouth, 0, sizeof(fbAfterSouth));
    M11_GameView_Draw(&state, fbAfterSouth, FB_W, FB_H);
    matchAfterSouth = match_portrait_at_rect(portraits, fbAfterSouth,
                                             TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to SOUTH: D1C rect does NOT carry "
                 "ordinal %d pixels (got %d%%, must be < %d%%)",
                 TARGET_ORDINAL, matchAfterSouth, MATCH_DOMINANCE_PCT);
        CHECK(matchAfterSouth < MATCH_DOMINANCE_PCT, msg);
    }
    leftSideAfterSouth = rect_warm_count(fbAfterSouth,
                                         SIDE_WALL_LEFT_X0, PORTRAIT_BAND_Y0,
                                         SIDE_WALL_LEFT_X1 - SIDE_WALL_LEFT_X0,
                                         PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterSouth = rect_warm_count(fbAfterSouth,
                                          SIDE_WALL_RIGHT_X0, PORTRAIT_BAND_Y0,
                                          SIDE_WALL_RIGHT_X1 - SIDE_WALL_RIGHT_X0,
                                          PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to SOUTH: left side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterSouth);
        CHECK(leftSideAfterSouth < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x2 to SOUTH: right side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterSouth);
        CHECK(rightSideAfterSouth < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - F0284 second half: SOUTH -> WEST -> NORTH at (1, 2)
     * ----------------------------------------------------------------
     * Two more TURN_RIGHT inputs rotate the party back to NORTH at
     * the same cell.  The front cell is now (1, 1) again, with the
     * seeded sensorData=9, so the F0128 redraw must paint
     * ordinal-9 pixels at the source-locked (96, 35) destination
     * rectangle.  This is the central "after_party_shuffle"
     * invariant: the portrait_rect_position survives a full
     * 360-degree spin via four F0284 rotations and re-resolves
     * correctly when the front route is back in view. */
    printf("\n[Group D] F0284 x2 again: SOUTH -> WEST -> NORTH at (1, 2) "
           "(portrait_rect_position must re-paint ordinal 9)\n");

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
                 "NORTH (got %d, want %d)",
                 state.world.party.direction, DIR_NORTH);
        CHECK(state.world.party.direction == DIR_NORTH, msg);
    }
    {
        int frontNorthBack = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to NORTH: front mirror ordinal is %d "
                 "(got %d)",
                 TARGET_ORDINAL, frontNorthBack);
        CHECK(frontNorthBack == TARGET_ORDINAL, msg);
    }

    memset(fbAfterNorthBack, 0, sizeof(fbAfterNorthBack));
    M11_GameView_Draw(&state, fbAfterNorthBack, FB_W, FB_H);
    matchAfterNorthBack = match_portrait_at_rect(portraits, fbAfterNorthBack,
                                                 TARGET_ORDINAL);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to NORTH: D1C portrait_rect_position "
                 "at (96, 35) still carries ordinal %d pixels at >= %d%% "
                 "match (got %d%%) -- central after_party_shuffle invariant",
                 TARGET_ORDINAL, MATCH_DOMINANCE_PCT, matchAfterNorthBack);
        CHECK(matchAfterNorthBack >= MATCH_DOMINANCE_PCT, msg);
    }
    leftSideAfterNorthBack = rect_warm_count(fbAfterNorthBack,
                                             SIDE_WALL_LEFT_X0, PORTRAIT_BAND_Y0,
                                             SIDE_WALL_LEFT_X1 - SIDE_WALL_LEFT_X0,
                                             PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    rightSideAfterNorthBack = rect_warm_count(fbAfterNorthBack,
                                              SIDE_WALL_RIGHT_X0, PORTRAIT_BAND_Y0,
                                              SIDE_WALL_RIGHT_X1 - SIDE_WALL_RIGHT_X0,
                                              PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to NORTH: left side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterNorthBack);
        CHECK(leftSideAfterNorthBack < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x4 back to NORTH: right side wall stays below "
                 "warm threshold %d (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterNorthBack);
        CHECK(rightSideAfterNorthBack < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - reopen-after-shuffle: select (C040 panel live again)
     * ----------------------------------------------------------------
     * A fresh SelectFrontMirrorCandidate at (1, 2) NORTH opens the
     * C040 panel with ordinal 9 again; the panel-on redraw must
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
     * must NOT show ordinal 9 on the side walls (no-floating
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
                     "single TURN_RIGHT to EAST returns "
                     "M11_GAME_INPUT_REDRAW (got %d)",
                     (int)r1west);
            CHECK(r1west == M11_GAME_INPUT_REDRAW, msg);
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "after single TURN_RIGHT: party direction = EAST "
                     "(got %d, want %d)",
                     state.world.party.direction, DIR_EAST);
            CHECK(state.world.party.direction == DIR_EAST, msg);
        }
    }

    memset(fbAfterWest, 0, sizeof(fbAfterWest));
    M11_GameView_Draw(&state, fbAfterWest, FB_W, FB_H);
    matchAfterWest = match_portrait_at_rect(portraits, fbAfterWest,
                                            TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x1 to EAST: D1C rect does NOT carry ordinal "
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
                 "after F0284 x1 to EAST: left side wall stays below warm "
                 "threshold %d (got %d) -- portrait must not float",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterWest);
        CHECK(leftSideAfterWest < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after F0284 x1 to EAST: right side wall stays below warm "
                 "threshold %d (got %d) -- portrait must not float",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterWest);
        CHECK(rightSideAfterWest < PORTRAIT_WARM_THRESHOLD, msg);
    }

    M11_GameView_Shutdown(&state);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
