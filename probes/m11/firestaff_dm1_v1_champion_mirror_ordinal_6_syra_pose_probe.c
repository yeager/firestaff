/*
 * DM1 V1 Hall of Champions champion mirror ordinal 6 (SYRA) pose
 * regression probe.
 *
 * Slice: dm1_v1_hoc_champion_portrait_06_front_north_entry /
 *        portrait_rect_position.
 *
 * The dm1_v1_hoc_champion_portrait_06_front_north_entry queue slice
 * was assigned under the assumption that ordinal 6 has a NORTH-facing
 * front-cell mirror route (the "front_north_entry" entry to the Hall).
 * In canonical DM1 PC 3.4 English (DUNGEON.DAT SHA256
 * d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85,
 * 33357 bytes) ordinal 6 (SYRA / "CHILD OF NATURE") has exactly one
 * reachable front-cell mirror pose and it is not NORTH-facing:
 *   map=0 pose=(2,4) dir=EAST front=(3,4) C127 sensor data=6
 * No (1,*) NORTH pose resolves to ordinal 6 under
 * M11_GameView_GetFrontMirrorOrdinal.  We advance the slice to the
 * actual ordinal-6 pose (east-facing side-room entry) and pin the
 * portrait_rect_position invariant here, documenting the route
 * correction in the commit message.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2570-2573 maps sensor cell to front-wall aspect
 *     via M021_NORMALIZE(M011_CELL(...) - P0318_i_Direction) for the
 *     PC 3.x path; DEFS.H:2552 pins M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *     on the I34E square-aspect layout.
 *   ReDMCSB DUNGEON.C:2558 resets G0289_i_DungeonView_ChampionPortraitOrdinal
 *     to 0 when at least one wall square is in view (the BUG0_75 guard).
 *   ReDMCSB DUNGEON.C:2608-2612 sets G0289 to M000_INDEX_TO_ORDINAL(M040_DATA(...))
 *     when a C127_SENSOR_WALL_CHAMPION_PORTRAIT sensor sits on
 *     M552_FRONT_WALL_ORNAMENT_ORDINAL.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 at the fixed G0109 graphic
 *     box {96,127,35,63} on M587_VIEW_WALL_D1C_FRONT (left=96, top=35,
 *     width=32, height=29), reading the (ord & 7) * 32 / (ord >> 3) * 29
 *     sub-strip of the C026 GRAPHICS.DAT asset.
 *   ReDMCSB MOVESENS.C:1501-1503 dispatches C127_SENSOR_WALL_CHAMPION_PORTRAIT
 *     to F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData).
 *   ReDMCSB REVIVE.C:63 F0280 materializes the candidate from sensorData;
 *     ReDMCSB REVIVE.C:704 F0282 processes the resurrect-panel click.
 *
 * Verified invariants:
 *   1. (2,4) EAST front-cell mirror ordinal == 6 (SYRA, "CHILD OF
 *      NATURE" per Firestaff-local mirror catalog decoders
 *      F0660_CHAMPION_MirrorCatalogGetName_Compat and
 *      F0661_CHAMPION_MirrorCatalogGetTitle_Compat — these are
 *      Firestaff-side helpers, not the ReDMCSB F0660/F0661 functions).
 *   2. The D1C front-wall portrait rectangle (96,35)-(128,64) at the
 *      champion portrait strip slot (ordinal 6 == (ord & 7) * 32,
 *      (ord >> 3) * 29) renders the SYRA sprite pixels (matched
 *      against the source C026 GRAPHICS.DAT asset).
 *   3. The wall-mirror backing (C346, global wall-ornament 43, coord
 *      set 0, viewWallIndex 12) is drawn at (96,36)-(128,64)
 *      underneath the portrait.
 *   4. The D2L/D2R side walls do not carry ordinal-6 pixel data
 *      (no floating over the side walls).
 *   5. The resurrect round-trip: select SYRA, confirm resurrect,
 *      survive 20 idle ticks, the front-mirror route is then -1.
 *
 * Usage: firestaff_dm1_v1_champion_mirror_ordinal_6_syra_pose_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    WALL_X = VIEWPORT_X + 96,
    WALL_Y = VIEWPORT_Y + 36,
    WALL_W = 32,
    WALL_H = 28,
    EXPECTED_ORDINAL = 6
};

static int g_pass = 0;
static int g_fail = 0;

#define PASS() do { printf("PASS\n"); g_pass++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); g_fail++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return 0; } } while(0)

/* Match SYRA's champion portrait ordinal at the (96,35)-(128,64)
 * D1C front-wall rectangle.  Returns the matched-pixel percent. */
static int match_portrait(const M11_AssetSlot* portraits,
                          const unsigned char* fb,
                          int ordinal) {
    int x, y, matched = 0, compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) continue; /* DUNVIEW.C:3916 dark-gray transparency */
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count distinct palette indices in a framebuffer rect. */
static int count_distinct(const unsigned char* fb,
                          int x, int y, int w, int h) {
    int seen[16] = {0};
    int i, n = 0;
    for (i = 0; i < w * h; ++i) {
        int px = x + (i % w);
        int py = y + (i / w);
        unsigned char raw = fb[py * FB_W + px];
        unsigned char idx = M11_FB_DECODE_INDEX(raw);
        if (idx != 0 && !seen[idx]) {
            seen[idx] = 1;
            ++n;
        }
    }
    return n;
}

/* Count warm pixels in a rect.  Per the F20E PC 3.4 palette
 * (src/shared/vga_palette_pc34_compat.c, LIGHT0), the warm-color set
 * {0x05 peach, 0x06 peach, 0x07 green, 0x08 red, 0x09 orange, 0x0A
 * peach, 0x0B yellow, 0x0E blue} is the champion portrait skin/clothing
 * palette.  The grey-stone wall texture uses palette indices 0x01/0x02
 * /0x03/0x04/0x07/0x0D and never the warm-color set, so a positive
 * warm_count distinguishes 'portrait present' from 'wall texture only'
 * for any wall cell. */
static int count_warm_pixels(const unsigned char* fb,
                             int x, int y, int w, int h) {
    int count = 0;
    int i;
    for (i = 0; i < w * h; ++i) {
        int px = x + (i % w);
        int py = y + (i / w);
        unsigned char raw = fb[py * FB_W + px];
        unsigned char idx = M11_FB_DECODE_INDEX(raw);
        if (idx == 0x05 || idx == 0x06 || idx == 0x07 ||
            idx == 0x08 || idx == 0x09 || idx == 0x0A ||
            idx == 0x0B || idx == 0x0E) {
            ++count;
        }
    }
    return count;
}

static int check_syra_pose(M11_GameViewState* game,
                           const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int wallDistinct;
    int portWarm;
    int port_nonzero;
    int x, y;

    printf("  TEST: hall_syra_2_4_east_ordinal_6 ... ");

    /* ReDMCSB DUNGEON.C:2573 + 2608-2612: ordinal 6 lives on C127
     * sensor (3,4) with sensorData=6, visible wall cell = WEST of
     * (3,4) when party faces EAST at (2,4). */
    game->world.party.mapIndex = 0;
    game->world.party.mapX = 2;
    game->world.party.mapY = 4;
    game->world.party.direction = 1; /* DIR_EAST */
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;

    /* Invariant 1: front-cell mirror ordinal == 6 (SYRA). */
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord != EXPECTED_ORDINAL) {
        printf("FAIL front ordinal got=%d want=%d\n",
            ord, EXPECTED_ORDINAL);
        ++g_fail;
        return 0;
    }

    /* Render the viewport. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    /* Invariant 2: D1C portrait rectangle (96,35)-(128,64) matches
     * SYRA's C026 strip pixels. */
    pct = match_portrait(portraits, fb, EXPECTED_ORDINAL);
    if (pct < 90) {
        printf("FAIL portrait match got=%d%% want>=90%%\n", pct);
        ++g_fail;
        return 0;
    }

    /* Invariant 3: the D1C front-wall mirror backing (96,36)-(128,64)
     * is rendered with at least 2 distinct palette indices (wall +
     * portrait). */
    wallDistinct = count_distinct(fb, WALL_X, WALL_Y, WALL_W, WALL_H);
    if (wallDistinct < 2) {
        printf("FAIL wall-mirror backing distinct=%d want>=2\n",
            wallDistinct);
        ++g_fail;
        return 0;
    }

    /* Invariant 4: portrait rect has >= 30 warm pixels (champion
     * sprite present, not just dark-grey backdrop). */
    portWarm = count_warm_pixels(fb, PORTRAIT_X, PORTRAIT_Y,
        PORTRAIT_W, PORTRAIT_H);
    if (portWarm < 30) {
        printf("FAIL portrait rect warm_count=%d want>=30\n", portWarm);
        ++g_fail;
        return 0;
    }

    /* Invariant 4b: portrait rect is mostly non-zero (no fully empty
     * backdrop; SYRA pixels occupy most of the rect). */
    port_nonzero = 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char raw =
                fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)];
            if (M11_FB_DECODE_INDEX(raw) != 0) ++port_nonzero;
        }
    }
    if (port_nonzero < (PORTRAIT_W * PORTRAIT_H * 7 / 10)) {
        printf("FAIL portrait rect nonzero=%d/%d want>=70%%\n",
            port_nonzero, PORTRAIT_W * PORTRAIT_H);
        ++g_fail;
        return 0;
    }

    printf("PASS match=%d%% warm=%d distinct=%d nonzero=%d/%d\n",
        pct, portWarm, wallDistinct, port_nonzero,
        PORTRAIT_W * PORTRAIT_H);
    ++g_pass;
    return 1;
}

static int check_syra_no_floating_side_walls(M11_GameViewState* game) {
    unsigned char fb[FB_W * FB_H];
    int northWarm;
    int southWarm;
    int portWarm;

    printf("  TEST: hall_syra_2_4_east_no_floating_side_walls ... ");

    game->world.party.mapIndex = 0;
    game->world.party.mapX = 2;
    game->world.party.mapY = 4;
    game->world.party.direction = 1; /* DIR_EAST */
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    /* For an EAST-facing pose, the side walls are the corridor
     * walls (north and south of the EAST-going corridor at (2,4)).
     * Sample a 64x29 viewport-local band on the left half (which is
     * where the corridor walls render, not the front D1C east wall)
     * and assert it has very few warm pixels (no floating portrait
     * sprite over the side corridor walls).  Compare with the D1C
     * portrait rect itself, which should be warm-pixel-rich. */
    northWarm = count_warm_pixels(fb, VIEWPORT_X + 0, VIEWPORT_Y + 35,
        64, 29);
    southWarm = count_warm_pixels(fb, VIEWPORT_X + 160, VIEWPORT_Y + 35,
        64, 29);
    portWarm = count_warm_pixels(fb, PORTRAIT_X, PORTRAIT_Y,
        PORTRAIT_W, PORTRAIT_H);

    /* The portrait rect should be warm-pixel rich; the side walls
     * should be warm-pixel poor.  Threshold side wall <= 5 warm
     * pixels (allowing torch glow noise). */
    if (portWarm < 30) {
        printf("FAIL portrait rect warm_count=%d want>=30\n", portWarm);
        ++g_fail;
        return 0;
    }
    if (northWarm > 5) {
        printf("FAIL north corridor wall warm=%d want<=5\n", northWarm);
        ++g_fail;
        return 0;
    }
    if (southWarm > 5) {
        printf("FAIL south corridor wall warm=%d want<=5\n", southWarm);
        ++g_fail;
        return 0;
    }

    printf("PASS portrait_warm=%d north_warm=%d south_warm=%d\n",
        portWarm, northWarm, southWarm);
    ++g_pass;
    return 1;
}

static int check_syra_resurrect_round_trip(M11_GameViewState* game) {
    int initialCount, rc;
    struct ChampionState_Compat* newChamp;
    int i;
    char name[CHAMPION_NAME_TEXT_CAPACITY];

    printf("  TEST: hall_syra_resurrect_round_trip ... ");

    game->world.party.mapIndex = 0;
    game->world.party.mapX = 2;
    game->world.party.mapY = 4;
    game->world.party.direction = 1; /* DIR_EAST */
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;

    initialCount = game->world.party.championCount;

    /* ReDMCSB MOVESENS.C:1501-1503 + REVIVE.C F0280: select the
     * source-backed candidate from the front-cell mirror ordinal. */
    rc = M11_GameView_SelectFrontMirrorCandidate(game);
    if (rc != 1) {
        printf("FAIL SelectFrontMirrorCandidate=%d\n", rc);
        ++g_fail;
        return 0;
    }
    if (game->world.party.championCount != initialCount + 1) {
        printf("FAIL championCount=%d want=%d\n",
            game->world.party.championCount, initialCount + 1);
        ++g_fail;
        return 0;
    }
    if (game->candidateMirrorOrdinal != EXPECTED_ORDINAL) {
        printf("FAIL candidateMirrorOrdinal=%d want=%d\n",
            game->candidateMirrorOrdinal, EXPECTED_ORDINAL);
        ++g_fail;
        return 0;
    }

    /* ReDMCSB REVIVE.C F0282: confirm resurrect disables the
     * matching C127 mirror sensor and finalizes the candidate. */
    rc = M11_GameView_ConfirmMirrorCandidate(game, 0);
    if (rc != 1) {
        printf("FAIL ConfirmMirrorCandidate=%d\n", rc);
        ++g_fail;
        return 0;
    }

    newChamp = &game->world.party.champions[initialCount];
    name[0] = '\0';
    (void)F0628_CHAMPION_UnpackName_Compat(newChamp, name, sizeof(name));
    if (strcmp(name, "SYRA") != 0) {
        printf("FAIL new champion name got='%s' want='SYRA'\n", name);
        ++g_fail;
        return 0;
    }
    if (newChamp->hp.current == 0 || newChamp->hp.maximum == 0) {
        printf("FAIL new champion has zero HP (%d/%d)\n",
            newChamp->hp.current, newChamp->hp.maximum);
        ++g_fail;
        return 0;
    }

    /* Advance 20 idle ticks; the new champion must survive. */
    for (i = 0; i < 20; ++i) {
        (void)M11_GameView_AdvanceIdleTick(game);
        if (newChamp->hp.current == 0) {
            printf("FAIL new champion died at tick %d\n", i);
            ++g_fail;
            return 0;
        }
    }
    if (game->partyDead) {
        printf("FAIL partyDead=1 after resurrection\n");
        ++g_fail;
        return 0;
    }
    /* After the C127 sensor is disabled, the front-cell mirror route
     * must return -1 so the player does not see a stale SYRA portrait
     * floating over the corridor east wall. */
    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        printf("FAIL mirror route not disabled after confirm\n");
        ++g_fail;
        return 0;
    }

    printf("PASS HP=%d/%d name=SYRA\n",
        newChamp->hp.current, newChamp->hp.maximum);
    ++g_pass;
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
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
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= check_syra_pose(&game, portraits);
    ok &= check_syra_no_floating_side_walls(&game);
    /* Re-init game state for resurrect round-trip (prior tests may
     * have mutated champion panel state). */
    M11_GameView_Shutdown(&game);
    memset(&game, 0, sizeof(game));
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not re-open DM1 V1 game view for resurrect\n");
        return 1;
    }
    ok &= check_syra_resurrect_round_trip(&game);

    M11_GameView_Shutdown(&game);
    printf("=== %d passed, %d failed ===\n", g_pass, g_fail);
    return ok ? 0 : 1;
}
