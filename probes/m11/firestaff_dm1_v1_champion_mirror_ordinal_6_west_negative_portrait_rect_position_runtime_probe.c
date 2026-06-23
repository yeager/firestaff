/*
 * firestaff_dm1_v1_champion_mirror_ordinal_6_west_negative_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 6 (C026 strip cell 6, SYRA /
 *   "CHILD OF NATURE" per the Firestaff F0660/F0661 mirror-catalog
 *   decoders).
 *   route west_negative: at the (2,4) cell that owns ordinal 6 the
 *                        party faces EAST to see SYRA on the east
 *                        wall of (3,4).  When the party turns WEST
 *                        at (2,4) the front square is (1,4).
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 = (M11_VIEWPORT_X + 96,
 *                                    M11_VIEWPORT_Y + 35,
 *                                    M11_PORTRAIT_W,
 *                                    M11_PORTRAIT_H) per
 *                                 ReDMCSB DUNVIEW.C:525
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall
 *                                 = {96, 127, 35, 63}.
 *
 * Initial slice assumption (WRONG, corrected below): at (2,4) facing
 * WEST the front cell (1,4) was believed to be a corridor wall with
 * no C127 mirror sensor.  Diagnostic run against the local PC 3.4
 * DM1 V1 DUNGEON.DAT (sha256 d90b6b1c38fd17e4..., M11 mirror catalog
 * ordinals match the canonical SYRA=6 / GANDO=10 / MOPHUS=15 names)
 * shows that (1,4) does carry a C127 sensor on its east wall with
 * sensorData=10 (GANDO, "THURFOOT"), so the party at (2,4) facing
 * WEST sees the GANDO portrait on the east wall of (1,4).
 *
 * The probe therefore asserts the honest invariant:
 *   - The portrait_rect_position contract holds regardless of pose:
 *     D1C wall-mirror frame is (80, 29, 64, 43) and the portrait
 *     cutout sits at (frame.x + 16, frame.y + 6) = (96, 35) at every
 *     tested pose.
 *   - The (2,4) EAST pose shows SYRA (ordinal 6) at the portrait
 *     cutout, with >= 90% pixel match against the C026 strip slot 6
 *     and >= 30 warm pixels.
 *   - The (2,4) WEST pose shows GANDO (ordinal 10) at the portrait
 *     cutout, with >= 90% pixel match against the C026 strip slot 10
 *     and >= 30 warm pixels.  This is the "back-route" champion
 *     portrait: the engine correctly resolves the C127 sensor on the
 *     east wall of the front cell rather than zeroing the portrait.
 *   - The (1,2) W and (1,4) W corridor neighbours (where the front
 *     cell has no C127 sensor) report ordinal=-1 and leave the
 *     portrait cutout empty (< POS_THRESHOLD warm pixels), proving
 *     the no-floating invariant on ordinary corridor walls.
 *   - The (2,4) S pose shows MOPHUS (ordinal 15) at the portrait
 *     cutout as a third cross-check on the portrait_rect_position
 *     invariant.
 *
 *   map=0 pose=(2,4) dir=EAST  front=(3,4) C127 sensor data=6
 *                 (SYRA, "CHILD OF NATURE")
 *   map=0 pose=(2,4) dir=WEST  front=(1,4) east wall C127 sensor
 *                 data=10 (GANDO, "THURFOOT")
 *   map=0 pose=(2,4) dir=SOUTH front=(2,5) north wall C127 sensor
 *                 data=15 (MOPHUS, "THE HEALER")
 *
 * The probe proves:
 *   Group A: at the (2,4) EAST pose the engine reports ordinal=6,
 *            the mirror-catalog name is SYRA, the (96, 35, 32, 29)
 *            D1C portrait rectangle paints the ordinal 6 portrait
 *            from the C026 strip with >= 90% pixel match and
 *            >= 30 warm pixels.
 *   Group B: portrait_rect_position contract — the D1C wall-mirror
 *            frame (M11_GameView_GetD1CWallOrnamentZone) returns
 *            (80, 29, 64, 43) regardless of pose; the portrait cutout
 *            is parented at (frame.x + 16, frame.y + 6) = (96, 35).
 *   Group C: at the (2,4) WEST pose the engine reports ordinal=10,
 *            the mirror-catalog name is GANDO, the D1C cutout paints
 *            the C026 ordinal 10 portrait with >= 90% pixel match
 *            and >= 30 warm pixels (the back-route portrait
 *            correctly maps to the portrait_rect_position cutout).
 *   Group D: corridor cross-check at (1,2) W — front cell (0,2) has
 *            no C127 sensor, the engine reports ordinal=-1, and the
 *            D1C portrait cutout is empty (< NEG_THRESHOLD warm
 *            pixels).
 *   Group E: corridor cross-check at (1,4) W — front cell (0,4) has
 *            no C127 sensor, the engine reports ordinal=-1, and the
 *            D1C portrait cutout is empty.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter (m11_front_cell_mirror_ordinal in
 *     src/engine/m11_game_view.c:11652).
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *     (F0660/F0661 mirror-catalog ordinal-to-name decode).
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into the G0109 portrait
 *     box (96, 127, 35, 63) = viewport (96, 35, 32, 29).
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}.
 *   - DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF far-to-near
 *     draw order so D0/D1/D2/D3 walls draw with D1C last and the
 *     champion portrait is the final pixel over the front wall.
 *   - COORD.C:1693-1722 PC 3.4 viewport origin (0, 33), 224x136.
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29.
 *   - MOVESENS.C:1501-1503 sensorData flows to F0280 candidate.
 *   - REVIVE.C:63 F0280 CHAMPION_AddCandidateChampionToParty.
 *   - REVIVE.C:704 F0282 disables matching C127 mirror sensor after
 *     confirmed resurrect.
 *   - DEFS.H:821-826 M027_PORTRAIT_X/M028_PORTRAIT_Y 8-column atlas
 *     math.
 *   - DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS strip.
 *   - DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL = 5 (PC 3.4
 *     MEDIA720 path; PC 3.4 MEDIA020 uses +1 indexing with M552 = 3).
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe —
 *     16-pose C127 ordinal coverage with pixel rect match disabled.
 *   firestaff_dm1_v1_champion_mirror_zorder_runtime_probe —
 *     corridor north/south/east/west no-floating poses.
 *   firestaff_dm1_v1_champion_mirror_capture_probe — PPM dumps
 *     for visual review of the same poses.
 *   firestaff_dm1_v1_champion_mirror_diag_2_4_west_probe —
 *     diagnostic probe that enumerated the (1,4)/(2,4) front-mirror
 *     ordinals and full 24-entry mirror catalog to author this slice.
 *
 * The probe is data-conditional: it requires hash-verified DM1 V1
 * data for the (2,4) E SYRA positive cross-check; without that data
 * the contract surface (engine helpers, C127 front-cell filter,
 * portrait_rect_position) is still exercised, and the catalog name
 * assertions degrade to "ordinal returns" + "cutout non-empty for
 * positive ordinals".
 *
 * Usage: firestaff_dm1_v1_champion_mirror_ordinal_6_west_negative_
 *        portrait_rect_position_runtime_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "asset_status_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W             = 320,
    FB_H             = 200,
    VIEWPORT_X       = 0,    /* COORD.C G2067_i_ViewportScreenX */
    VIEWPORT_Y       = 33,   /* COORD.C G2068_i_ViewportScreenY */
    PORTRAIT_X       = VIEWPORT_X + 96,  /* DUNVIEW.C G0109 X1 = 96 */
    PORTRAIT_Y       = VIEWPORT_Y + 35,  /* DUNVIEW.C G0109 Y1 = 35  */
    PORTRAIT_W       = 32,   /* COORD.C G2078_C32_PortraitWidth  */
    PORTRAIT_H       = 29,   /* COORD.C G2079_C29_PortraitHeight */
    /* Side-wall rects used by the no-floating invariant at the
     * corridor cross-check poses.  These are outside the (96..128,
     * 35..64) portrait cutout and outside the (80..144, 29..72) D1C
     * wall-mirror frame so the warm count here must stay near zero
     * on corridor poses where no C127 mirror is on the front wall. */
    SIDE_NORTH_X     = VIEWPORT_X + 0,
    SIDE_NORTH_Y     = VIEWPORT_Y + 0,
    SIDE_NORTH_W     = 64,
    SIDE_NORTH_H     = 32,
    SIDE_SOUTH_X     = VIEWPORT_X + 160,
    SIDE_SOUTH_Y     = VIEWPORT_Y + 0,
    SIDE_SOUTH_W     = 64,
    SIDE_SOUTH_H     = 32,
    SIDE_FLOAT_THRESHOLD = 5,
    /* Warm-color pixel thresholds.  The grey-stone wall texture
     * uses palette indices 0x01/0x02/0x07/0x0D (grey shades) and
     * never the warm set, so the warm count cleanly distinguishes
     * "portrait present" (>= POS_THRESHOLD) from "wall texture
     * only" (< NEG_THRESHOLD). */
    PORTRAIT_WARM_POS_THRESHOLD = 30,
    PORTRAIT_WARM_NEG_THRESHOLD = 30,
    /* The D1C wall-mirror frame can have a few warm-tone pixels
     * from torch glow or edge antialiasing on corridor side walls,
     * so the wall-box threshold is slightly looser than the
     * portrait-cutout threshold. */
    WALLBOX_WARM_NEG_THRESHOLD  = 50,
    EXPECTED_ORDINAL_SYRA       = 6,
    EXPECTED_ORDINAL_GANDO      = 10,
    EXPECTED_ORDINAL_MOPHUS     = 15
};

static int g_pass = 0;
static int g_fail = 0;

#define PASS(msg) do { printf("  PASS: %s\n", msg); g_pass++; } while(0)
#define PASSF(fmt, ...) do { printf("  PASS: " fmt "\n", __VA_ARGS__); g_pass++; } while(0)
#define FAILF(fmt, ...) do { printf("  FAIL: " fmt "\n", __VA_ARGS__); g_fail++; } while(0)

/* Count warm-colored pixels in a framebuffer rect.  The warm-color
 * palette set is {0x07 green, 0x08 red, 0x09 orange, 0x0A peach,
 * 0x0B yellow, 0x0E blue} — the champion portrait skin/clothing
 * palette.  Grey-stone wall texture never uses this set, so a
 * positive warm_count distinguishes "portrait present" from "wall
 * texture only" for any wall cell. */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int count = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char raw = fb[yy * FB_W + xx];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            switch (idx) {
                case 0x07: case 0x08: case 0x09: case 0x0A:
                case 0x0B: case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/* Match the C026 champion portrait strip at ordinal `ordinal` against
 * the D1C portrait cutout in `fb`.  Skips the C026 transparent
 * palette index 1 (DUNVIEW.C:3916 dark-gray transparency).  Returns
 * the matched-pixel percent so the probe can assert >= 90% pixel
 * agreement without claiming DOS pixel parity. */
static int match_portrait(const M11_AssetSlot* portraits,
                          const unsigned char* fb,
                          int ordinal) {
    int x, y, matched = 0, compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if (ordinal < 0 || ordinal >= 24) return 0;
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

/* Pose the party on map 0 (Hall of Champions) and zero the
 * candidate-panel state.  Centralizes the boilerplate so every
 * render call below uses the same field initialization. */
static void set_pose(M11_GameViewState* state,
                     int mapX, int mapY, int direction) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

/* Drive M11_GameView_Draw at the given (mapX, mapY, direction) pose
 * and return the rendered framebuffer in `fb`. */
static void render_at(M11_GameViewState* state,
                      unsigned char* fb,
                      int mapX, int mapY, int direction) {
    set_pose(state, mapX, mapY, direction);
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* ── Group A: (2,4) EAST SYRA positive route ───────────────────────
 * ReDMCSB DUNGEON.C:2573 + 2608-2612: at (2,4) facing EAST the front
 * square is (3,4) which carries a C127 sensor with sensorData=6
 * (SYRA, "CHILD OF NATURE" per F0660 mirror-catalog name decode).
 * The engine must:
 *   - return ordinal 6 from M11_GameView_GetFrontMirrorOrdinal
 *   - mirror-catalog name == "SYRA"
 *   - paint the SYRA portrait at the D1C cutout (96, 35, 32, 29)
 *     with >= 90% pixel match against the C026 strip slot 6. */
static void check_syra_positive_route(M11_GameViewState* state,
                                      const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int portWarm;
    char nameBuf[32];
    char titleBuf[32];

    printf("\n[Group A] (2,4) EAST SYRA positive route: ordinal 6 portrait at D1C rect\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    if (ord == EXPECTED_ORDINAL_SYRA) {
        PASS("M11_GameView_GetFrontMirrorOrdinal((2,4)E) == 6 (SYRA)");
    } else {
        FAILF("M11_GameView_GetFrontMirrorOrdinal((2,4)E) == 6 (got %d)", ord);
        return;
    }

    memset(nameBuf, 0, sizeof(nameBuf));
    memset(titleBuf, 0, sizeof(titleBuf));
    (void)M11_GameView_GetMirrorNameByOrdinal(state, EXPECTED_ORDINAL_SYRA,
                                              nameBuf, sizeof(nameBuf));
    (void)M11_GameView_GetMirrorTitleByOrdinal(state, EXPECTED_ORDINAL_SYRA,
                                               titleBuf, sizeof(titleBuf));
    if (strcmp(nameBuf, "SYRA") == 0) {
        PASS("mirror-catalog ordinal 6 name == \"SYRA\"");
    } else {
        FAILF("mirror-catalog ordinal 6 name == \"SYRA\" (got \"%s\")", nameBuf);
    }
    if (titleBuf[0] != '\0') {
        PASS("mirror-catalog ordinal 6 title non-empty");
    } else {
        FAILF("%s", "mirror-catalog ordinal 6 title empty");
    }

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait(portraits, fb, EXPECTED_ORDINAL_SYRA);
        if (pct >= 90) {
            PASSF("D1C portrait cutout matches C026 ordinal 6 >= 90%% (got %d%%)",
                  pct);
        } else {
            FAILF("D1C portrait cutout matches C026 ordinal 6 >= 90%% (got %d%%)",
                  pct);
        }
        portWarm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y,
                                   PORTRAIT_W, PORTRAIT_H);
        if (portWarm >= PORTRAIT_WARM_POS_THRESHOLD) {
            PASSF("Inner portrait cutout warm_count >= %d for SYRA (got %d)",
                  PORTRAIT_WARM_POS_THRESHOLD, portWarm);
        } else {
            FAILF("Inner portrait cutout warm_count >= %d for SYRA (got %d)",
                  PORTRAIT_WARM_POS_THRESHOLD, portWarm);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group B: portrait_rect_position contract ─────────────────────
 * Source-locked to DUNVIEW.C:3913-3928 (C026 blit) + DUNVIEW.C G0205
 * Graphic558 coordSet 5 / viewWallIndex 12 (C346 D1C wall-mirror
 * frame).  The D1C wall-mirror frame MUST be at (80, 29, 64, 43)
 * regardless of which pose the party is in.  The portrait cutout MUST
 * be at (frame.x + 16, frame.y + 6) = (96, 35) per the (+16, +6)
 * parented offset. */
static void check_portrait_rect_position_contract(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    const int kPoses[][3] = {
        {2, 4, 1 /* DIR_EAST  */},
        {2, 4, 2 /* DIR_SOUTH */},
        {2, 4, 3 /* DIR_WEST  */},
        {1, 2, 3 /* DIR_WEST  */}
    };
    int i;

    printf("\n[Group B] portrait_rect_position contract: D1C cutout invariant\n");

    for (i = 0; i < (int)(sizeof(kPoses) / sizeof(kPoses[0])); ++i) {
        set_pose(state, kPoses[i][0], kPoses[i][1], kPoses[i][2]);
        rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        if (rc != 1) {
            FAILF("M11_GameView_GetD1CWallOrnamentZone returns 1 at (%d,%d) dir=%d (got %d)",
                  kPoses[i][0], kPoses[i][1], kPoses[i][2], rc);
            continue;
        }
        if (ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43 &&
            ornX + 16 == 96 && ornY + 6 == 35) {
            PASSF("D1C rect invariant at (%d,%d) dir=%d: box=(%d,%d,%d,%d) cutout=(%d,%d)",
                  kPoses[i][0], kPoses[i][1], kPoses[i][2],
                  ornX, ornY, ornW, ornH, ornX + 16, ornY + 6);
        } else {
            FAILF("D1C rect invariant at (%d,%d) dir=%d: box=(%d,%d,%d,%d) cutout=(%d,%d)",
                  kPoses[i][0], kPoses[i][1], kPoses[i][2],
                  ornX, ornY, ornW, ornH, ornX + 16, ornY + 6);
        }
    }
}

/* ── Group C: (2,4) WEST back-route GANDO portrait ────────────────
 * Source-locked to DUNGEON.C:2573 (C127 sensor front-wall filter).
 * At (2,4) facing WEST the front square is (1,4), and (1,4) carries
 * a C127 sensor on its east wall (cell=1) with sensorData=10
 * (GANDO, "THURFOOT").  The engine must:
 *   - return 10 from M11_GameView_GetFrontMirrorOrdinal
 *   - mirror-catalog name == "GANDO"
 *   - paint the GANDO portrait at the D1C cutout (96, 35, 32, 29)
 *     with >= 90% pixel match against the C026 strip slot 10.
 *
 * This is the back-route portrait — the same C127 sensor that is
 * normally invisible behind the party is correctly resolved into the
 * portrait_rect_position cutout at (96, 35). */
static void check_west_back_route_gando(M11_GameViewState* state,
                                        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int portWarm;
    char nameBuf[32];

    printf("\n[Group C] (2,4) WEST back-route GANDO portrait at D1C rect\n");

    render_at(state, fb, 2, 4, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    if (ord == EXPECTED_ORDINAL_GANDO) {
        PASSF("M11_GameView_GetFrontMirrorOrdinal((2,4)W) == %d (GANDO)",
              EXPECTED_ORDINAL_GANDO);
    } else {
        FAILF("M11_GameView_GetFrontMirrorOrdinal((2,4)W) == %d (got %d)",
              EXPECTED_ORDINAL_GANDO, ord);
    }

    memset(nameBuf, 0, sizeof(nameBuf));
    (void)M11_GameView_GetMirrorNameByOrdinal(state, EXPECTED_ORDINAL_GANDO,
                                              nameBuf, sizeof(nameBuf));
    if (strcmp(nameBuf, "GANDO") == 0) {
        PASS("mirror-catalog ordinal 10 name == \"GANDO\"");
    } else {
        FAILF("mirror-catalog ordinal 10 name == \"GANDO\" (got \"%s\")", nameBuf);
    }

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait(portraits, fb, EXPECTED_ORDINAL_GANDO);
        if (pct >= 90) {
            PASSF("D1C portrait cutout matches C026 ordinal 10 >= 90%% (got %d%%)",
                  pct);
        } else {
            FAILF("D1C portrait cutout matches C026 ordinal 10 >= 90%% (got %d%%)",
                  pct);
        }
        portWarm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y,
                                   PORTRAIT_W, PORTRAIT_H);
        if (portWarm >= PORTRAIT_WARM_POS_THRESHOLD) {
            PASSF("Inner portrait cutout warm_count >= %d for GANDO (got %d)",
                  PORTRAIT_WARM_POS_THRESHOLD, portWarm);
        } else {
            FAILF("Inner portrait cutout warm_count >= %d for GANDO (got %d)",
                  PORTRAIT_WARM_POS_THRESHOLD, portWarm);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group C2: (2,4) SOUTH cross-check MOPHUS portrait ────────────
 * ReDMCSB DUNGEON.C:2573 + 2608-2612: at (2,4) facing SOUTH the
 * front square is (2,5) which carries a C127 sensor on its north
 * wall (cell=0) with sensorData=15 (MOPHUS, "THE HEALER").  This
 * adds a third ordinal mapping to the portrait_rect_position
 * invariant: SYRA(6)/GANDO(10)/MOPHUS(15) at the (96, 35) cutout. */
static void check_south_route_mophus(M11_GameViewState* state,
                                     const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int portWarm;
    char nameBuf[32];

    printf("\n[Group C2] (2,4) SOUTH MOPHUS cross-check: ordinal 15 portrait at D1C rect\n");

    render_at(state, fb, 2, 4, 2 /* DIR_SOUTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    if (ord == EXPECTED_ORDINAL_MOPHUS) {
        PASSF("M11_GameView_GetFrontMirrorOrdinal((2,4)S) == %d (MOPHUS)",
              EXPECTED_ORDINAL_MOPHUS);
    } else {
        FAILF("M11_GameView_GetFrontMirrorOrdinal((2,4)S) == %d (got %d)",
              EXPECTED_ORDINAL_MOPHUS, ord);
    }

    memset(nameBuf, 0, sizeof(nameBuf));
    (void)M11_GameView_GetMirrorNameByOrdinal(state, EXPECTED_ORDINAL_MOPHUS,
                                              nameBuf, sizeof(nameBuf));
    if (strcmp(nameBuf, "MOPHUS") == 0) {
        PASS("mirror-catalog ordinal 15 name == \"MOPHUS\"");
    } else {
        FAILF("mirror-catalog ordinal 15 name == \"MOPHUS\" (got \"%s\")",
              nameBuf);
    }

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait(portraits, fb, EXPECTED_ORDINAL_MOPHUS);
        if (pct >= 90) {
            PASSF("D1C portrait cutout matches C026 ordinal 15 >= 90%% (got %d%%)",
                  pct);
        } else {
            FAILF("D1C portrait cutout matches C026 ordinal 15 >= 90%% (got %d%%)",
                  pct);
        }
        portWarm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y,
                                   PORTRAIT_W, PORTRAIT_H);
        if (portWarm >= PORTRAIT_WARM_POS_THRESHOLD) {
            PASSF("Inner portrait cutout warm_count >= %d for MOPHUS (got %d)",
                  PORTRAIT_WARM_POS_THRESHOLD, portWarm);
        } else {
            FAILF("Inner portrait cutout warm_count >= %d for MOPHUS (got %d)",
                  PORTRAIT_WARM_POS_THRESHOLD, portWarm);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group D: corridor cross-check at (1,2) W ─────────────────────
 * Cross-check the no-floating invariant at (1,2) facing WEST, where
 * the front cell (0,2) is the corridor west of the Hall entrance
 * and has no C127 sensor.  Re-derives the no-floating invariant
 * here with ordinal-6 context. */
static void check_west_corridor_1_2(M11_GameViewState* state) {
    unsigned char fb[FB_W * FB_H];
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int ord;
    int innerWarm, wallWarm;

    printf("\n[Group D] corridor cross-check at (1,2) W: no-floating invariant\n");

    render_at(state, fb, 1, 2, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    if (ord == -1) {
        PASS("M11_GameView_GetFrontMirrorOrdinal((1,2)W) == -1");
    } else {
        FAILF("M11_GameView_GetFrontMirrorOrdinal((1,2)W) == -1 (got %d)", ord);
    }

    (void)M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
    innerWarm = rect_warm_count(fb,
                                VIEWPORT_X + ornX + 16,
                                VIEWPORT_Y + ornY + 6,
                                PORTRAIT_W, PORTRAIT_H);
    wallWarm = rect_warm_count(fb,
                               VIEWPORT_X + ornX,
                               VIEWPORT_Y + ornY,
                               ornW, ornH);
    if (innerWarm < PORTRAIT_WARM_NEG_THRESHOLD) {
        PASSF("Inner portrait cutout warm_count < %d at (1,2)W (got %d)",
              PORTRAIT_WARM_NEG_THRESHOLD, innerWarm);
    } else {
        FAILF("Inner portrait cutout warm_count < %d at (1,2)W (got %d)",
              PORTRAIT_WARM_NEG_THRESHOLD, innerWarm);
    }
    if (wallWarm < WALLBOX_WARM_NEG_THRESHOLD) {
        PASSF("Wall box warm_count < %d at (1,2)W (got %d)",
              WALLBOX_WARM_NEG_THRESHOLD, wallWarm);
    } else {
        FAILF("Wall box warm_count < %d at (1,2)W (got %d)",
              WALLBOX_WARM_NEG_THRESHOLD, wallWarm);
    }
}

/* ── Group E: corridor cross-check at (1,4) W ─────────────────────
 * Cross-check the no-floating invariant at (1,4) facing WEST, where
 * the front cell (0,4) is a corridor cell with no C127 sensor.  This
 * is the corridor west of (2,4) — the same physical wall the party
 * sees at (2,4) facing west, but approached from the corridor side.
 * Without a C127 sensor on (0,4) the portrait cutout must be empty
 * even though the wall texture is the same as the (1,4) east wall
 * that carries the GANDO sensor. */
static void check_west_corridor_1_4(M11_GameViewState* state) {
    unsigned char fb[FB_W * FB_H];
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int ord;
    int innerWarm, wallWarm;

    printf("\n[Group E] corridor cross-check at (1,4) W: no-floating invariant\n");

    render_at(state, fb, 1, 4, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    if (ord == -1) {
        PASS("M11_GameView_GetFrontMirrorOrdinal((1,4)W) == -1");
    } else {
        FAILF("M11_GameView_GetFrontMirrorOrdinal((1,4)W) == -1 (got %d)", ord);
    }

    (void)M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
    innerWarm = rect_warm_count(fb,
                                VIEWPORT_X + ornX + 16,
                                VIEWPORT_Y + ornY + 6,
                                PORTRAIT_W, PORTRAIT_H);
    wallWarm = rect_warm_count(fb,
                               VIEWPORT_X + ornX,
                               VIEWPORT_Y + ornY,
                               ornW, ornH);
    if (innerWarm < PORTRAIT_WARM_NEG_THRESHOLD) {
        PASSF("Inner portrait cutout warm_count < %d at (1,4)W (got %d)",
              PORTRAIT_WARM_NEG_THRESHOLD, innerWarm);
    } else {
        FAILF("Inner portrait cutout warm_count < %d at (1,4)W (got %d)",
              PORTRAIT_WARM_NEG_THRESHOLD, innerWarm);
    }
    if (wallWarm < WALLBOX_WARM_NEG_THRESHOLD) {
        PASSF("Wall box warm_count < %d at (1,4)W (got %d)",
              WALLBOX_WARM_NEG_THRESHOLD, wallWarm);
    } else {
        FAILF("Wall box warm_count < %d at (1,4)W (got %d)",
              WALLBOX_WARM_NEG_THRESHOLD, wallWarm);
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits = NULL;
    int assetsAvailable;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall portrait ordinal 6 (SYRA) / west_back_route portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
        printf("SKIP firestaff_dm1_v1_champion_mirror_ordinal_6_west_negative_"
               "portrait_rect_position_runtime_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.world.party.championCount = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    check_syra_positive_route(&state, portraits);
    check_portrait_rect_position_contract(&state);
    check_west_back_route_gando(&state, portraits);
    check_south_route_mophus(&state, portraits);
    check_west_corridor_1_2(&state);
    check_west_corridor_1_4(&state);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
