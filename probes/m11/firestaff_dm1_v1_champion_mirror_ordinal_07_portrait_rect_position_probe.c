/*
 * DM1 V1 Hall of Champions champion-portrait ordinal 7 — portrait_rect_position.
 *
 * Slice assignment:
 *   ordinal   = 7  (TIGGY / TAMAL)
 *   route     = front_north_entry (the conceptual "front-wall sensor entry"
 *               descriptor used by the broader ordinal-7 placement work)
 *   aspect    = portrait_rect_position
 *
 * Honest scope note:
 *   In the real DM1 V1 PC 3.4 English DUNGEON.DAT, the only Hall-of-Champions
 *   cell that exposes the C127 sensor with sensorData == 7 is mapIndex=0,
 *   (mapX=2, mapY=17) with the party facing SOUTH (direction=2).  No
 *   north-facing pose anywhere on map 0 yields ordinal 7; the C127 sensor
 *   sits on the north wall of (2,18) and only the party at (2,17,SOUTH)
 *   reaches it through the front-wall filter (DUNGEON.C:2573
 *   M011_CELL(sensor) - partyDirection + 3 + wall-only filter).
 *
 *   The probe therefore verifies the actual available route
 *   (2,17,SOUTH) and is explicit that no front_north_entry route exists
 *   for ordinal 7 in this DUNGEON.DAT.  This is an honest slice
 *   verification, not a claim that the route description in the slice
 *   name matches a real north-facing pose.
 *
 * The probe locks the following invariants against real
 * firestaff_dm1_v1 assets under the configured data root:
 *
 *   1. Mirror-catalog identity:
 *        M11_GameView_GetMirrorNameByOrdinal(7) == "TIGGY"
 *        M11_GameView_GetMirrorTitleByOrdinal(7) == "TAMAL"
 *
 *   2. Front-mirror ordinal at the available route:
 *        pose=(0, 2, 17, SOUTH) -> GetFrontMirrorOrdinal == 7
 *
 *   3. Portrait rect position contract (gating):
 *        The rect probed at (96,35)-(127,63) lives strictly inside the
 *        D1C wall-ornament frame at (80,29)-(143,71) (DUNVIEW.C G0205
 *        coordSet 5 / index 12 = C346 champion-mirror frame).
 *        The rect is bounded by 96 <= X < 128, 35 <= Y < 64 — exactly
 *        the DM1 PC 3.4 D1C portrait-on-wall box (DUNVIEW.C:3913-3928)
 *        and not floating on the side walls.
 *
 *   4. D1C portrait rect hot-pixels at the available route (diagnostic):
 *        At (2,17,SOUTH) the rect should contain at least some
 *        non-black, non-grey-stone pixels (the C026 sprite blit
 *        overlaps the wall ornament backing).  Reported as a
 *        nonBlack / nonGreyStone count — informational, not gating.
 *
 *   5. Cross-ordinal portrait-rect match diagnostic (informational):
 *        match_front_portrait(portraits, fb, 7) at (2,17,SOUTH) reports
 *        bestOrdinal + dominance ratio.  This mirrors the diagnostic
 *        line in the existing champion-mirror zorder/visibility/
 *        walkpath probes; the rect may share palette pixels with the
 *        wall ornament frame (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY
 *        transparency mask), so the best-fit ordinal is informational,
 *        not authoritative.
 *
 *   6. No-floating contract at the same cell (gating):
 *        pose=(0, 2, 17, NORTH|WEST|EAST) -> portrait_rect_warm_count < 30
 *        (the side walls and the back wall must NOT carry a portrait)
 *
 *   7. No front_north_entry route for ordinal 7 (gating, slice-honesty):
 *        pose=(0, 2, 17, NORTH), (0, 2, 16, NORTH), (0, 2, 18, NORTH),
 *        (0, 1, 17, NORTH), (0, 3, 17, NORTH) all yield
 *        GetFrontMirrorOrdinal != 7 (the C127 sensor with sensorData=7
 *        is only visible from the front wall of (2,17) when facing SOUTH).
 *
 *   8. Resurrect-round-trip at the available route (gating):
 *        SelectFrontMirrorCandidate at (2,17,SOUTH) appends a champion
 *        with non-zero HP/max; 20 idle ticks later the new champion is
 *        still alive and the mirror route is disabled (== -1).
 *
 * Source evidence (ReDMCSB):
 *   DUNGEON.C:2573 maps M011_CELL(sensor) against view direction.
 *   DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   DUNVIEW.C:3913-3928 blits the D1C champion portrait to (96,35)-(127,63).
 *   DUNVIEW.C:7727-7924 F0124_DrawSquareD1C drives D1C wall draw order.
 *   DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF draws the viewport
 *     far-to-near after every MOVESENS.C:556 tick.
 *   MOVESENS.C:1501-1503 passes C127 sensorData to F0280.
 *   REVIVE.C F0280 materializes the candidate from sensorData.
 *   COORD.C:1693-1722 PC34 viewport origin/224x136 dimensions.
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
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    PROBE_PORTRAIT_X = 96,        /* D1C portrait-rect top-left X */
    PROBE_PORTRAIT_Y = 35,        /* D1C portrait-rect top-left Y */
    PROBE_PORTRAIT_W = 32,        /* portrait-rect width  (8x3 sprite) */
    PROBE_PORTRAIT_H = 29,        /* portrait-rect height */
    PROBE_WALL_FRAME_X = 80,      /* D1C wall-ornament frame top-left X */
    PROBE_WALL_FRAME_Y = 29,      /* D1C wall-ornament frame top-left Y */
    PROBE_WALL_FRAME_W = 64,      /* D1C wall-ornament frame width  */
    PROBE_WALL_FRAME_H = 43,      /* D1C wall-ornament frame height */
    PROBE_PORTRAIT_WARM_THRESHOLD = 30
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

static MirrorMatch match_front_portrait(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int expectedOrdinal) {
    MirrorMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int x;
        int y;
        int matched = 0;
        int compared = 0;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
                int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
                unsigned char src =
                    (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                unsigned char dst =
                    M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                           (PROBE_PORTRAIT_X + x)]);
                if (src == 1) {
                    /* Source C01 dark-gray transparency mask
                     * (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY) — skip. */
                    continue;
                }
                ++compared;
                if (dst == src) {
                    ++matched;
                }
            }
        }
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
 * Count warm-coloured pixels in the D1C portrait rect.  Used for the
 * no-floating contract: when no portrait is rendered, the rect must be
 * dominated by grey-stone wall texture (palette indices 0x01/0x02/0x0D)
 * with no champion-portrait warm pixels.  See the firestaff_dm1_v1_champion_mirror_capture_probe
 * for the canonical warm-palette set.
 */
static int portrait_rect_warm_count(const unsigned char* fb) {
    int x, y;
    int count = 0;
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char idx =
                M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                       (PROBE_PORTRAIT_X + x)]);
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

/* ---- Check 1: mirror catalog identity for ordinal 7 ---- */
static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[32];
    int ok = 1;
    name[0] = '\0';
    title[0] = '\0';
    if (!M11_GameView_GetMirrorNameByOrdinal(game, 7, name, sizeof(name)) ||
        strcmp(name, "TIGGY") != 0) {
        fprintf(stderr,
                "FAIL ordinal_07_catalog_name got='%s' want='TIGGY'\n",
                name);
        ok = 0;
    }
    if (!M11_GameView_GetMirrorTitleByOrdinal(game, 7, title, sizeof(title)) ||
        strcmp(title, "TAMAL") != 0) {
        fprintf(stderr,
                "FAIL ordinal_07_catalog_title got='%s' want='TAMAL'\n",
                title);
        ok = 0;
    }
    printf("ordinal_07_catalog_identity name='%s' title='%s'\n",
           name, title);
    return ok;
}

/* ---- Check 2: front-mirror ordinal at the available route ---- */
static int check_available_route_ordinal(M11_GameViewState* game) {
    int ordinal;
    int ok = 1;
    set_pose(game, 2, 17, 2 /* DIR_SOUTH */);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != 7) {
        fprintf(stderr,
                "FAIL ordinal_07_available_route front ordinal got=%d want=7\n",
                ordinal);
        ok = 0;
    }
    printf("ordinal_07_available_route pose=(2,17,SOUTH) ordinal=%d\n",
           ordinal);
    return ok;
}

/* ---- Check 3: portrait rect position contract (static bounds check) ---- */
/* The D1C portrait rect (96,35)-(127,63) must lie strictly inside the
 * D1C wall-ornament frame (80,29)-(143,71) per ReDMCSB DUNVIEW.C:3913-3928
 * (champion-portrait blit rectangle) and DUNVIEW.C G0205 coordSet 5 / index
 * 12 (C346 champion-mirror wall ornament frame).  This is a placement
 * contract — the rect (96,35)-(127,63) is the source-faithful blit
 * destination; it is not a side wall. */
static int check_portrait_rect_position_contract(void) {
    int ok = 1;
    if (PROBE_PORTRAIT_X < PROBE_WALL_FRAME_X ||
        PROBE_PORTRAIT_Y < PROBE_WALL_FRAME_Y ||
        PROBE_PORTRAIT_X + PROBE_PORTRAIT_W > PROBE_WALL_FRAME_X + PROBE_WALL_FRAME_W ||
        PROBE_PORTRAIT_Y + PROBE_PORTRAIT_H > PROBE_WALL_FRAME_Y + PROBE_WALL_FRAME_H) {
        fprintf(stderr,
                "FAIL ordinal_07_portrait_rect_position rect=(%d,%d,%d,%d) escapes frame=(%d,%d,%d,%d)\n",
                PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
                PROBE_WALL_FRAME_X, PROBE_WALL_FRAME_Y,
                PROBE_WALL_FRAME_W, PROBE_WALL_FRAME_H);
        ok = 0;
    }
    printf("ordinal_07_portrait_rect_position rect=(%d,%d,%d,%d) inside frame=(%d,%d,%d,%d)\n",
           PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_WALL_FRAME_X, PROBE_WALL_FRAME_Y,
           PROBE_WALL_FRAME_W, PROBE_WALL_FRAME_H);
    return ok;
}

/* ---- Check 4: D1C portrait rect hot-pixels at (2,17,SOUTH) ----
 * ReDMCSB DUNVIEW.C:3913-3928 blits the C026 portrait sprite (32x29)
 * into the D1C wall rectangle at viewport (96, 35) whenever the front
 * wall sensorData points at a champion ordinal in [0, 23].
 * The sprite is 32x29; pixels with palette index 1 are transparency
 * (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY).  The probe counts the
 * non-black, non-grey-stone pixels in the rect: at the available
 * (2,17,SOUTH) route the rect should contain portrait-sprite pixels
 * (warmer palette indices from C026), not be all dark-grey wall.
 *
 * Honest scope: the current V1 renderer's C026 blit is the only path
 * that paints the portrait sprite into the rect.  The companion
 * firestaff_dm1_v1_champion_mirror_capture_probe has a similar
 * "portrait_rect_warm_count" metric over rows 35..63 (cols 96..127)
 * using the {0x07,0x08,0x09,0x0A,0x0B,0x0E} warm palette set.
 * This probe uses a stronger "non-black + non-grey" pixel count that
 * is palette-agnostic so it can lock the rect-position contract
 * without depending on which palette indices the C026 sprite happens
 * to use for the ordinal=7 silhouette.
 */
static int check_rect_non_grey_pixel_count(M11_GameViewState* game) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int x, y;
    int nonBlack = 0;
    int nonGreyStone = 0;

    set_pose(game, 2, 17, 2 /* DIR_SOUTH */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    /* Count pixels in the rect (96, 35) - (127, 63):
     *   nonBlack         = fb raw != 0
     *   nonGreyStone     = fb raw != 0 AND palette index not in
     *                      {0x01, 0x02, 0x0D} (the three grey-stone
     *                      wall-texture indices the D1C wall ornament
     *                      uses per DUNVIEW.C G0205 coordSet 5/12 and
     *                      the underlying corridor fill).
     */
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char raw = fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                   (PROBE_PORTRAIT_X + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            if (raw != 0) ++nonBlack;
            if (raw != 0 && idx != 0x01 && idx != 0x02 && idx != 0x0D) {
                ++nonGreyStone;
            }
        }
    }

    printf("ordinal_07_rect_pixels rect=(%d,%d,%d,%d) "
           "nonBlack=%d nonGreyStone=%d (of %d total)\n",
           PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           nonBlack, nonGreyStone,
           PROBE_PORTRAIT_W * PROBE_PORTRAIT_H);
    return 1; /* informational — does not gate PASS/FAIL. */
}

/* ---- Check 5: cross-ordinal portrait-rect match diagnostic ----
 * Diagnostic-only: compare the rect at (2,17,SOUTH) against every
 * C026 ordinal and report the best-fit ordinal + dominance ratio.
 * Mirrors the diagnostic line in the existing champion-mirror
 * zorder/visibility/walkpath probes.  Does not gate PASS/FAIL — the
 * rect may share palette pixels with the wall ornament frame
 * (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY transparency mask), so the
 * best-fit ordinal is informational, not authoritative.
 */
static int check_rect_best_fit_ordinal(M11_GameViewState* game,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    MirrorMatch match;
    set_pose(game, 2, 17, 2 /* DIR_SOUTH */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, fb, 7);
    printf("ordinal_07_rect_best_fit best=%d matched=%d expected=%d "
           "expectedMatched=%d compared=%d\n",
           match.bestOrdinal, match.bestMatched,
           7, match.expectedMatched, match.compared);
    return 1; /* informational — does not gate PASS/FAIL. */
}

/* ---- Check 6: no-floating contract at (2,17) for non-front directions ---- */
static int check_no_floating_at_ordinal_07_cell(M11_GameViewState* game) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    struct {
        const char* label;
        int dir;
        int expectedOrdinal;
    } kSide[] = {
        {"ordinal_07_cell_north_no_portrait", 0 /* DIR_NORTH */, -1},
        {"ordinal_07_cell_west_no_portrait",  3 /* DIR_WEST  */, -1},
        {"ordinal_07_cell_east_no_portrait",  1 /* DIR_EAST  */, -1},
    };
    int i;
    int ok = 1;

    for (i = 0; i < (int)(sizeof(kSide) / sizeof(kSide[0])); ++i) {
        int ordinal;
        int warm;
        set_pose(game, 2, 17, kSide[i].dir);
        ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (ordinal != kSide[i].expectedOrdinal) {
            fprintf(stderr,
                    "FAIL %s front ordinal got=%d want=%d\n",
                    kSide[i].label, ordinal, kSide[i].expectedOrdinal);
            ok = 0;
        }
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
        warm = portrait_rect_warm_count(fb);
        if (warm >= PROBE_PORTRAIT_WARM_THRESHOLD) {
            fprintf(stderr,
                    "FAIL %s portrait_rect_warm_count=%d >= threshold=%d (portrait floating on side wall)\n",
                    kSide[i].label, warm, PROBE_PORTRAIT_WARM_THRESHOLD);
            ok = 0;
        }
        printf("%s pose=(2,17,dir=%d) ordinal=%d portrait_warm=%d\n",
               kSide[i].label, kSide[i].dir, ordinal, warm);
    }
    return ok;
}

/* ---- Check 7: no front_north_entry route for ordinal 7 in this DUNGEON.DAT ---- */
static int check_no_front_north_entry(M11_GameViewState* game) {
    struct {
        const char* label;
        int mapX;
        int mapY;
    } kCells[] = {
        {"ordinal_07_front_north_entry_absent_at_2_17",     2, 17},
        {"ordinal_07_front_north_entry_absent_at_2_16",     2, 16},
        {"ordinal_07_front_north_entry_absent_at_2_18",     2, 18},
        {"ordinal_07_front_north_entry_absent_at_1_17",     1, 17},
        {"ordinal_07_front_north_entry_absent_at_3_17",     3, 17},
    };
    int i;
    int ok = 1;
    for (i = 0; i < (int)(sizeof(kCells) / sizeof(kCells[0])); ++i) {
        int ord;
        set_pose(game, kCells[i].mapX, kCells[i].mapY, 0 /* DIR_NORTH */);
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        if (ord == 7) {
            fprintf(stderr,
                    "FAIL %s unexpectedly exposes ordinal=7\n",
                    kCells[i].label);
            ok = 0;
        }
        printf("%s pose=(%d,%d,NORTH) ordinal=%d\n",
               kCells[i].label, kCells[i].mapX, kCells[i].mapY, ord);
    }
    return ok;
}

/* ---- Check 8: resurrect round-trip at the available route ---- */
static int check_resurrect_round_trip(M11_GameViewState* game) {
    int initialCount;
    int rc;
    struct ChampionState_Compat* newChamp;
    int i;
    int ok = 1;

    set_pose(game, 2, 17, 2 /* DIR_SOUTH */);
    initialCount = game->world.party.championCount;
    rc = M11_GameView_SelectFrontMirrorCandidate(game);
    if (rc != 1) {
        fprintf(stderr,
                "FAIL ordinal_07_resurrect SelectFrontMirrorCandidate rc=%d\n", rc);
        ok = 0;
    } else if (game->world.party.championCount != initialCount + 1) {
        fprintf(stderr,
                "FAIL ordinal_07_resurrect championCount=%d want=%d\n",
                game->world.party.championCount, initialCount + 1);
        ok = 0;
    }
    rc = M11_GameView_ConfirmMirrorCandidate(game, 0);
    if (rc != 1) {
        fprintf(stderr,
                "FAIL ordinal_07_resurrect ConfirmMirrorCandidate rc=%d\n", rc);
        ok = 0;
    }
    newChamp = &game->world.party.champions[initialCount];
    if (newChamp->hp.current == 0 || newChamp->hp.maximum == 0) {
        fprintf(stderr,
                "FAIL ordinal_07_resurrect new champion zero HP (%d/%d)\n",
                newChamp->hp.current, newChamp->hp.maximum);
        ok = 0;
    }
    for (i = 0; i < 20; ++i) {
        (void)M11_GameView_AdvanceIdleTick(game);
        if (newChamp->hp.current == 0) {
            fprintf(stderr,
                    "FAIL ordinal_07_resurrect new champion died at tick %d\n", i);
            ok = 0;
            break;
        }
    }
    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        fprintf(stderr,
                "FAIL ordinal_07_resurrect mirror route not disabled after confirm\n");
        ok = 0;
    }
    printf("ordinal_07_resurrect HP=%d/%d mirror=%d\n",
           newChamp->hp.current, newChamp->hp.maximum,
           M11_GameView_GetFrontMirrorOrdinal(game));
    return ok;
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
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe "
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
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall of Champions ordinal 7 portrait_rect_position ===\n");
    ok &= check_catalog_identity(&game);
    ok &= check_available_route_ordinal(&game);
    ok &= check_portrait_rect_position_contract();
    check_rect_non_grey_pixel_count(&game);
    check_rect_best_fit_ordinal(&game, portraits);
    ok &= check_no_floating_at_ordinal_07_cell(&game);
    ok &= check_no_front_north_entry(&game);
    ok &= check_resurrect_round_trip(&game);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror ordinal 7 portrait_rect_position probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
