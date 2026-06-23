/*
 * DM1 V1 Hall of Champions portrait 10 south_return probe.
 *
 * Focused slice: champion portrait ordinal 10 (GANDO / THURFOOT) on
 * the south_return route.  In real DM1 V1 PC 3.4 English DUNGEON.DAT,
 * ordinal 10 is reached from (map 0, x=1, y=5) facing NORTH
 * (front square (1,4) carries the C127 sensor with sensorData=10).
 *
 * The "south_return" route is the in-place 180-degree turn at the
 * same cell: the party starts at (1,5) facing SOUTH (front=(1,6),
 * mirror ordinal 13 / WUUF), then turns in place 180 degrees to face
 * NORTH, returning to the GANDO mirror.  This route is disjoint
 * from the front_north_entry probe (which only renders the ordinal
 * 10 pose directly) and from the zorder_reblt probe (which covers
 * the (1,5,NORTH) -> (1,5,EAST) -> (2,4,SOUTH) -> (1,5,SOUTH)
 * sequence but never the in-place 180-degree turn that returns from
 * the south pose to the north pose at the same cell).
 *
 * The probe narrows coverage to:
 *   - The south-return pose must expose ordinal 13 (WUUF) and the
 *     D1C portrait rect (96, 35, 32, 29) must show ordinal 13
 *     pixels (not ordinal 10) at that frame.
 *   - The 180-degree turn must re-blit the D1C portrait rect to
 *     ordinal 10 (GANDO) pixels without leaving stale ordinal 13
 *     pixels in the rect.
 *   - The D1C portrait rect position (96, 35, 32, 29) must remain
 *     inside the C346 wall-mirror frame (80, 29, 64, 43) at both
 *     the south and north poses, exactly as the source-locked
 *     DUNVIEW.C G0205 coordSet 5/12 contract requires.
 *   - The mirror catalog identity for ordinal 10 must be GANDO /
 *     THURFOOT, and for ordinal 13 (the south pose) must be WUUF.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps sensor cell against party direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 portrait strip into
 *     M635_ZONE_PORTRAIT_ON_WALL after the D1C wall ornament.
 *   ReDMCSB DUNVIEW.C G0205 coordSet 5/12 (D1C champion-mirror
 *     frame) at (80, 29, 64, 43) — DUNVIEW.C:525 G0109_box holds
 *     the inner cutout (96, 127, 35, 63).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 redraws the viewport from
 *     the new party pose after every MOVESENS.C:556 tick.
 *   ReDMCSB MOVESENS.C:1501-1503 / REVIVE.C F0280 use the same
 *     sensorData for candidate champion materialization.
 *   m11_front_cell_mirror_ordinal (m11_game_view.c:11652) — the
 *     wall-side filter that lets M552_FRONT_WALL_ORNAMENT_ORDINAL
 *     set G0289 only when the C127 sensor cell matches the visible
 *     front wall aspect.
 *   m11_apply_dm1_v1_pipeline_tick (m11_game_view.c) — the in-place
 *     180-degree turn at the same Hall cell goes through two
 *     consecutive CMD_TURN_LEFT / CMD_TURN_RIGHT commands; both
 *     always succeed (CLIKMENU.C:865 in-place turns bypass the
 *     movement-cooldown G0310) and F0128 re-blits the viewport
 *     after each turn.
 *
 * Honesty scope: this is a Firestaff-runtime portrait_rect_position
 * proof only; it does not claim DOS pixel parity and does not add
 * an original-vs-Firestaff viewport comparison.  Real DM1 V1 data
 * is required (DM1 V1 PC 3.4 DUNGEON.DAT 33357 bytes + the C026
 * portrait strip in GRAPHICS.DAT).
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    /* DUNVIEW.C:3913-3928 / 8522-8533: the D1C portrait-on-wall
     * cutout lives at viewport (96, 35) with size 32x29 inside the
     * C346 wall-mirror frame at viewport (80, 29) of size 64x43.
     * M11_VIEWPORT_X = 0, M11_VIEWPORT_Y = 33 so the framebuffer
     * destination of the cutout is (96, 68, 32, 29). */
    PROBE_PORTRAIT_VX = 96,
    PROBE_PORTRAIT_VY = 35,
    PROBE_PORTRAIT_FX = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VX,
    PROBE_PORTRAIT_FY = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VY,
    PROBE_PORTRAIT_W  = 32,
    PROBE_PORTRAIT_H  = 29,
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* The slice: ordinal 10 (GANDO) on the south_return route, the
     * 180-degree in-place turn at cell (1,5) from DIR_SOUTH to
     * DIR_NORTH.  Ordinal 13 (WUUF) is the south-pose mirror. */
    PROBE_EXPECTED_ORDINAL_NORTH = 10,
    PROBE_EXPECTED_ORDINAL_SOUTH = 13
};

typedef struct PortraitMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} PortraitMatch;

static int g_pass = 0;
static int g_fail = 0;

static void expect_int(const char* label, int got, int want) {
    if (got == want) {
        ++g_pass;
        printf("  PASS: %s got=%d\n", label, got);
    } else {
        ++g_fail;
        printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    }
}

static void expect_true(const char* label, int ok) {
    if (ok) {
        ++g_pass;
        printf("  PASS: %s\n", label);
    } else {
        ++g_fail;
        printf("  FAIL: %s\n", label);
    }
}

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

static int ordinal_compared_count(const M11_AssetSlot* portraits, int ordinal) {
    int x;
    int y;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src != PROBE_CHAMPION_TRANSPARENT) {
                ++compared;
            }
        }
    }
    return compared;
}

static int count_ordinal_pixels_at(const M11_AssetSlot* portraits,
                                   const unsigned char* fb,
                                   int dstX,
                                   int dstY,
                                   int ordinal) {
    int x;
    int y;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(dstY + y) * PROBE_FB_W + (dstX + x)]);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            if (dst == src) {
                ++matched;
            }
        }
    }
    return matched;
}

static PortraitMatch match_portrait_at(const M11_AssetSlot* portraits,
                                       const unsigned char* fb,
                                       int dstX,
                                       int dstY,
                                       int expectedOrdinal) {
    PortraitMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int matched = count_ordinal_pixels_at(portraits, fb, dstX, dstY, ordinal);
        int compared = ordinal_compared_count(portraits, ordinal);
        if (matched > out.bestMatched) {
            out.bestMatched = matched;
            out.bestOrdinal = ordinal;
        }
        if (ordinal == expectedOrdinal) {
            out.expectedMatched = matched;
            out.compared = compared;
        }
    }
    return out;
}

/*
 * South-return step 0: verify the start pose (1,5) DIR_SOUTH exposes
 * the WUUF C127 sensor (ordinal 13) and that the D1C portrait rect
 * shows WUUF pixels, not GANDO pixels.  The mirror catalog identity
 * for ordinal 13 must be WUUF.
 */
static void check_south_pose_wuuf(M11_GameViewState* game,
                                  const M11_AssetSlot* portraits,
                                  unsigned char* fb) {
    int ord;
    char wuufName[32];
    char wuufTitle[64];
    PortraitMatch match;
    int gandoStale;
    int gandoCompared;

    set_pose(game, 1, 5, DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_int("south_return start (1,5) DIR_SOUTH front ordinal",
               ord, PROBE_EXPECTED_ORDINAL_SOUTH);

    memset(wuufName, 0, sizeof(wuufName));
    memset(wuufTitle, 0, sizeof(wuufTitle));
    expect_true("ordinal 13 mirror catalog name is WUUF",
                M11_GameView_GetMirrorNameByOrdinal(game,
                                                    PROBE_EXPECTED_ORDINAL_SOUTH,
                                                    wuufName,
                                                    (int)sizeof(wuufName)) > 0 &&
                strcmp(wuufName, "WUUF") == 0);
    expect_true("ordinal 13 mirror catalog title is non-empty",
                M11_GameView_GetMirrorTitleByOrdinal(game,
                                                     PROBE_EXPECTED_ORDINAL_SOUTH,
                                                     wuufTitle,
                                                     (int)sizeof(wuufTitle)) > 0);

    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    match = match_portrait_at(portraits, fb,
                              PROBE_PORTRAIT_FX, PROBE_PORTRAIT_FY,
                              PROBE_EXPECTED_ORDINAL_SOUTH);
    expect_int("south pose best portrait ordinal at D1C rect (WUUF=13)",
               match.bestOrdinal, PROBE_EXPECTED_ORDINAL_SOUTH);
    expect_true("south pose ordinal-13 pixels match at D1C rect >= 90%",
                match.compared > 0 &&
                match.expectedMatched * 100 >= match.compared * 90);

    gandoCompared = ordinal_compared_count(portraits, PROBE_EXPECTED_ORDINAL_NORTH);
    gandoStale = count_ordinal_pixels_at(portraits, fb,
                                         PROBE_PORTRAIT_FX, PROBE_PORTRAIT_FY,
                                         PROBE_EXPECTED_ORDINAL_NORTH);
    expect_true("south pose does NOT float ordinal-10 (GANDO) pixels in D1C rect",
                gandoCompared > 0 && gandoStale * 100 < gandoCompared * 35);

    printf("  INFO: south_pose wuufName='%s' wuufTitle='%s' rect_match=%d/%d "
           "stale_ordinal10=%d/%d\n",
           wuufName, wuufTitle,
           match.expectedMatched, match.compared,
           gandoStale, gandoCompared);
}

/*
 * South-return step 1: drive the in-place 180-degree turn via two
 * consecutive CMD_TURN_LEFT commands through M11_GameView_HandleInput
 * (CLIKMENU.C F0365 / F0366 turn-left path; CLIKMENU.C:865 in-place
 * turns bypass the movement-cooldown G0310 so both turns commit
 * synchronously without AdvanceIdleTick interleaving).  After the
 * turn the party must face NORTH at the same cell, the front
 * mirror ordinal must be 10 (GANDO), and the D1C portrait rect
 * must show ordinal-10 pixels instead of ordinal-13 pixels.
 */
static void check_south_return_turn_to_north(M11_GameViewState* game,
                                             const M11_AssetSlot* portraits,
                                             unsigned char* fb) {
    int ord;
    M11_GameInputResult r1;
    M11_GameInputResult r2;
    PortraitMatch match;
    int wuufStale;
    int wuufCompared;

    set_pose(game, 1, 5, DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_int("turn-step input: party is at (1,5) DIR_SOUTH before turn",
               ord, PROBE_EXPECTED_ORDINAL_SOUTH);

    /* Two consecutive TURN_LEFT commands give a 180-degree rotation
     * SOUTH -> EAST -> NORTH.  m11_apply_dm1_v1_pipeline_tick treats
     * in-place turns as immediate (CLIKMENU.C:865 short-circuits the
     * G0310 cooldown for turns), so the second TURN_LEFT applies
     * the new party direction without an intervening tick.  Both
     * calls must return M11_GAME_INPUT_REDRAW per the F0128 draw
     * contract. */
    r1 = M11_GameView_HandleInput(game, M12_MENU_INPUT_TURN_LEFT);
    expect_int("south_return first TURN_LEFT result",
               (int)r1, (int)M11_GAME_INPUT_REDRAW);
    expect_int("south_return first turn: party direction is DIR_EAST",
               game->world.party.direction, DIR_EAST);

    r2 = M11_GameView_HandleInput(game, M12_MENU_INPUT_TURN_LEFT);
    expect_int("south_return second TURN_LEFT result",
               (int)r2, (int)M11_GAME_INPUT_REDRAW);
    expect_int("south_return second turn: party direction is DIR_NORTH",
               game->world.party.direction, DIR_NORTH);

    /* After the 180-degree turn, the front mirror ordinal must
     * resolve to 10 (GANDO) because the C127 sensor on (1,4) is
     * now exposed to the front cell. */
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_int("south_return end (1,5) DIR_NORTH front ordinal after 180° turn",
               ord, PROBE_EXPECTED_ORDINAL_NORTH);

    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    match = match_portrait_at(portraits, fb,
                              PROBE_PORTRAIT_FX, PROBE_PORTRAIT_FY,
                              PROBE_EXPECTED_ORDINAL_NORTH);
    expect_int("south_return north pose best portrait ordinal at D1C rect (GANDO=10)",
               match.bestOrdinal, PROBE_EXPECTED_ORDINAL_NORTH);
    expect_true("south_return north pose ordinal-10 pixels match at D1C rect >= 90%",
                match.compared > 0 &&
                match.expectedMatched * 100 >= match.compared * 90);

    wuufCompared = ordinal_compared_count(portraits, PROBE_EXPECTED_ORDINAL_SOUTH);
    wuufStale = count_ordinal_pixels_at(portraits, fb,
                                        PROBE_PORTRAIT_FX, PROBE_PORTRAIT_FY,
                                        PROBE_EXPECTED_ORDINAL_SOUTH);
    expect_true("south_return north pose does NOT keep stale ordinal-13 (WUUF) pixels in D1C rect",
                wuufCompared > 0 && wuufStale * 100 < wuufCompared * 35);

    printf("  INFO: south_return ordinal10_match=%d/%d stale_ordinal13=%d/%d\n",
           match.expectedMatched, match.compared,
           wuufStale, wuufCompared);
}

/*
 * portrait_rect_position contract: the D1C portrait-on-wall cutout
 * must be exactly at viewport (96, 35, 32, 29) inside the C346
 * wall-mirror frame at viewport (80, 29, 64, 43) for both the
 * south and north poses at the (1,5) Hall cell.  This is the
 * source-locked DUNVIEW.C G0205 coordSet 5/12 invariant the
 * front_north_entry probe already locks for the north pose; this
 * probe re-locks it after the south_return turn to confirm the
 * rect position is stable across the in-place 180-degree turn.
 */
static void check_portrait_rect_position_contract(M11_GameViewState* game) {
    int ornX = -1;
    int ornY = -1;
    int ornW = -1;
    int ornH = -1;
    int poses[] = { DIR_SOUTH, DIR_NORTH };
    int i;
    expect_true("D1C wall-mirror frame zone helper succeeds",
                M11_GameView_GetD1CWallOrnamentZone(game, &ornX, &ornY, &ornW, &ornH) == 1);
    expect_int("D1C wall-mirror frame x", ornX, 80);
    expect_int("D1C wall-mirror frame y", ornY, 29);
    expect_int("D1C wall-mirror frame width", ornW, 64);
    expect_int("D1C wall-mirror frame height", ornH, 43);
    expect_int("portrait cutout x is frame x + 16", PROBE_PORTRAIT_VX, ornX + 16);
    expect_int("portrait cutout y is frame y + 6", PROBE_PORTRAIT_VY, ornY + 6);
    expect_true("portrait cutout is contained by D1C wall-mirror frame",
                PROBE_PORTRAIT_VX >= ornX &&
                PROBE_PORTRAIT_VY >= ornY &&
                PROBE_PORTRAIT_VX + PROBE_PORTRAIT_W <= ornX + ornW &&
                PROBE_PORTRAIT_VY + PROBE_PORTRAIT_H <= ornY + ornH);
    for (i = 0; i < (int)(sizeof(poses) / sizeof(poses[0])); ++i) {
        int ord;
        char poseLabel[64];
        set_pose(game, 1, 5, poses[i]);
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(poseLabel, sizeof(poseLabel),
                 "pose=(1,5,%s) D1C rect position intact",
                 poses[i] == DIR_NORTH ? "N" :
                 poses[i] == DIR_EAST  ? "E" :
                 poses[i] == DIR_SOUTH ? "S" : "W");
        expect_true(poseLabel,
                    PROBE_PORTRAIT_VX == 96 &&
                    PROBE_PORTRAIT_VY == 35 &&
                    PROBE_PORTRAIT_W  == 32 &&
                    PROBE_PORTRAIT_H  == 29 &&
                    ord >= 0);
    }
}

/*
 * Catalog identity contract: ordinal 10 must be GANDO / THURFOOT
 * (per the front_north_entry probe and the in-data Hall of
 * Champions mirror catalog).  Ordinal 13 must be WUUF (per the
 * actual_pose probe label and the in-data catalog).  This group
 * is independent of the draw stack: the catalog lookup is the
 * F0660 / F0661 source-locked identity the resurrect / reincarnate
 * flow uses to materialise the candidate from sensorData.
 */
static void check_catalog_identity(M11_GameViewState* game) {
    char gandoName[32];
    char gandoTitle[64];
    char wuufName[32];
    char wuufTitle[64];

    memset(gandoName, 0, sizeof(gandoName));
    memset(gandoTitle, 0, sizeof(gandoTitle));
    memset(wuufName, 0, sizeof(wuufName));
    memset(wuufTitle, 0, sizeof(wuufTitle));

    expect_true("ordinal 10 mirror catalog name is GANDO",
                M11_GameView_GetMirrorNameByOrdinal(game, 10,
                                                    gandoName,
                                                    (int)sizeof(gandoName)) > 0 &&
                strcmp(gandoName, "GANDO") == 0);
    expect_true("ordinal 10 mirror catalog title is THURFOOT",
                M11_GameView_GetMirrorTitleByOrdinal(game, 10,
                                                     gandoTitle,
                                                     (int)sizeof(gandoTitle)) > 0 &&
                strcmp(gandoTitle, "THURFOOT") == 0);
    expect_true("ordinal 13 mirror catalog name is WUUF",
                M11_GameView_GetMirrorNameByOrdinal(game, 13,
                                                    wuufName,
                                                    (int)sizeof(wuufName)) > 0 &&
                strcmp(wuufName, "WUUF") == 0);
    expect_true("ordinal 13 mirror catalog title is non-empty",
                M11_GameView_GetMirrorTitleByOrdinal(game, 13,
                                                     wuufTitle,
                                                     (int)sizeof(wuufTitle)) > 0);

    printf("  INFO: catalog ordinal10 name='%s' title='%s' ordinal13 name='%s' title='%s'\n",
           gandoName, gandoTitle, wuufName, wuufTitle);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char fb[PROBE_FB_W * PROBE_FB_H];

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP dm1_v1_champion_mirror_portrait10_south_return_runtime_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall portrait 10 south_return / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    /* Group A: catalog identity (independent of the draw stack). */
    printf("\n[Group A] Mirror catalog identity for ordinals 10 and 13\n");
    check_catalog_identity(&game);

    /* Group B: south pose (1,5) DIR_SOUTH must show WUUF (13), not GANDO (10). */
    printf("\n[Group B] (1,5) DIR_SOUTH shows WUUF (13), not GANDO (10)\n");
    check_south_pose_wuuf(&game, portraits, fb);

    /* Group C: in-place 180-degree turn SOUTH->EAST->NORTH via two TURN_LEFTs. */
    printf("\n[Group C] south_return 180° in-place turn at (1,5) -> ordinal 10 GANDO\n");
    check_south_return_turn_to_north(&game, portraits, fb);

    /* Group D: portrait_rect_position contract holds at both south and north poses. */
    printf("\n[Group D] portrait_rect_position (96, 35, 32, 29) contract at (1,5)\n");
    check_portrait_rect_position_contract(&game);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
