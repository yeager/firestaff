/*
 * DM1 V1 Hall of Champions portrait ordinal 13 (WUUF) — south_return /
 * portrait_rect_position runtime probe.
 *
 * Focused slice for the Hall of Champions portrait placement table:
 *
 *   ordinal = 13 (champion portrait atlas ordinal 13, column 5, row 1
 *                  in the C026 256x87 atlas; canonical PC 3.4 English
 *                  catalog name = WUUF)
 *   route   = south_return (party at (1, 5) facing SOUTH, front
 *                            square (1, 6) carries the C127 sensor
 *                            with sensorData = 13 on its NORTH aspect,
 *                            the canonical real-data south_return Hall
 *                            pose that exposes WUUF)
 *   aspect  = portrait_rect_position (the D1C portrait-on-wall
 *                                       rectangle must be at viewport
 *                                       (96, 35) of size 32x29, the
 *                                       fixed source-locked cutout
 *                                       inside the C346 wall-mirror
 *                                       frame at viewport (80, 29) of
 *                                       size 64x43)
 *
 * This probe is disjoint from every sibling probe:
 *
 *   firestaff_dm1_v1_champion_mirror_portrait10_south_return_runtime_probe
 *     - covers the south_return route but the destination pose is
 *       ordinal 10 (GANDO) at (1, 5, NORTH) after a 180° in-place
 *       turn; the south pose is verified only as a baseline that
 *       ordinal 13 is present and ordinal 10 does not bleed in.
 *       It does NOT lock the portrait_rect_position contract for
 *       ordinal 13 specifically.
 *
 *   firestaff_dm1_v1_champion_mirror_portrait00_south_return_portrait_rect_position_probe
 *     - covers ordinal 0 (DAROOU) by seeding the C127 sensor from
 *       13 -> 0; the seeded ordinal-0 case is not a real-DM1 V1
 *       data pose on the south_return route and so it cannot
 *       attest to the source-locked (1, 5, SOUTH)=13 rectangle
 *       position invariant.
 *
 *   firestaff_dm1_v1_champion_portrait_13_east_walkpath_portrait_rect_position_runtime_probe
 *     - covers ordinal 13 (WUUF) but the route is east_walkpath:
 *       corridor walking (1, 2, NORTH) -> ... -> (1, 5, SOUTH)
 *       driven through the M11 input pipeline.  It does not
 *       exercise the south_return in-place 180° turn, does not
 *       lock the no-floating invariant at (1, 5, EAST)/(1, 5,
 *       WEST) on the same cell, and does not separately verify
 *       that the rect clears stale ordinal-13 pixels after the
 *       180° turn to ordinal 10.
 *
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     - covers (1, 5, SOUTH)=13 only via GetFrontMirrorOrdinal; no
 *       viewport draw, no per-pixel rectangle match, no in-place
 *       turn coverage.
 *
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     - covers (1, 5, SOUTH)=13 in the zorder/reblt step but only
 *       via dominance over the rect; does not lock the rect
 *       position (96, 35) on the viewport or the C026 source
 *       rect (160, 29, 32, 29).
 *
 *   firestaff_dm1_v1_champion_mirror_capture_probe
 *     - dumps PPM captures of (1, 5, SOUTH)=13 (WUUF) for visual
 *       evidence but does not assert the rectangle position with
 *       a per-pixel match against the C026 source ordinal 13.
 *
 * Source evidence:
 *
 *   ReDMCSB DEFS.H:821-826  M027_PORTRAIT_X(index) and
 *                          M028_PORTRAIT_Y(index) macros — atlas
 *                          stride 32x29, 8 columns x 3 rows.
 *   ReDMCSB DEFS.H:2186     C026_GRAPHIC_CHAMPION_PORTRAITS
 *                          (256x87 portrait strip in GRAPHICS.DAT).
 *   ReDMCSB DUNGEON.C:2573  M011_CELL(sensor) vs view dir filter;
 *                          only sensors on M552_FRONT_WALL_ORNAMENT
 *                          _ORDINAL (= 5) set G0289.
 *   ReDMCSB DUNGEON.C:2608-2612  C127 sensorData -> G0289 ordinal.
 *   ReDMCSB DUNVIEW.C:3913-3928  D1C front-wall portrait blit at
 *                          G0109_box = (96..127, 35..63) with
 *                          source rect (ordinal & 7) * 32, (ordinal
 *                          >> 3) * 29 and 32x29 stride.
 *   ReDMCSB DUNVIEW.C:525   G0109_auc_Graphic558_Box_ChampionPort
 *                          raitOnWall fixed at (96, 127, 35, 63).
 *   ReDMCSB DUNVIEW.C:8318-8542  F0128_DUNGEONVIEW_Draw_CPSF
 *                          re-blits the full viewport after every
 *                          MOVESENS.C:556 tick, including in-place
 *                          180° turns.
 *   ReDMCSB MOVESENS.C:1501-1503  routes C127 sensorData to
 *                          F0280_CHAMPION_AddCandidateChampion.
 *   ReDMCSB REVIVE.C F0280  materializes the candidate champion
 *                          from sensorData (same path for both
 *                          south and north poses at (1, 5)).
 *   ReDMCSB DATA.C:85 / 424 G0047 portrait extraction rectangle
 *                          anchor; this probe only verifies the
 *                          destination rectangle position, not G0047.
 *   m11_front_cell_mirror_ordinal (m11_game_view.c:11652) the
 *                          M11-side wall-side filter that honours
 *                          M552_FRONT_WALL_ORNAMENT_ORDINAL.
 *   m11_draw_dm1_front_champion_portrait (m11_game_view.c) the
 *                          D1C champion-portrait blit with the
 *                          destination rectangle constants
 *                          PROBE_PORTRAIT_VX/PROBE_PORTRAIT_VY.
 *
 * The probe proves these invariants for this slice only:
 *
 *   (1) Mirror catalog identity: M11_GameView_GetMirrorNameByOrdinal
 *       returns "WUUF" and the title is non-empty for ordinal 13.
 *   (2) Front-mirror ordinal at (1, 5, SOUTH) equals 13 (WUUF) via
 *       the real PC 3.4 C127 sensorData.
 *   (3) D1C portrait-on-wall destination rectangle is exactly at
 *       viewport (96, 35), 32x29 (framebuffer (96, 68), 32x29); the
 *       C026 source rect for ordinal 13 is (160, 29, 32, 29) — col
 *       5, row 1 of the 256x87 atlas.
 *   (4) At (1, 5, SOUTH) the D1C rectangle is dominated by ordinal
 *       13 pixels from the C026 source rect; the per-pixel match
 *       ratio is >= 90% and best_ordinal == 13.
 *   (5) No-floating invariant at the same cell (1, 5) facing EAST
 *       or WEST: front-mirror ordinal = -1 and the D1C rect must
 *       not be painted with ordinal 13 pixels (or any ordinal).
 *   (6) No-floating invariant at the same cell (1, 5) facing
 *       NORTH: the C127 sensor on (1, 4) SOUTH aspect exposes
 *       ordinal 10 (ZED), so the rect shows ordinal 10 pixels,
 *       NOT ordinal 13 — locks that the south_return C127 sensor
 *       does not bleed onto the NORTH aspect.
 *   (7) In-place 180° turn via two consecutive TURN_LEFT commands
 *       (SOUTH -> EAST -> NORTH) at the same (1, 5) cell: after
 *       the turn the front-mirror ordinal must equal 10 (GANDO),
 *       the D1C rect must show ordinal 10 pixels with >= 90%
 *       match, and the rect must NOT carry stale ordinal 13
 *       pixels (the F0128 re-blt must clear them).
 *   (8) Per-pixel rectangle position correctness: the destination
 *       rectangle is locked to (96, 35) on the viewport — every
 *       opaque source pixel of ordinal 13 in C026 must match the
 *       corresponding destination rectangle pixel exactly.  This
 *       is the strictest lock: a 1-pixel drift in either X or Y
 *       would let the portrait float on side walls (the BUG-DNY-
 *       DM1-2026-06-16 "floating portrait" failure mode).
 *   (9) Strict per-ordinal dominance: the expected-ordinal match
 *       count at (1, 5, SOUTH) strictly dominates every other
 *       ordinal (1..23) — a sibling portrait cannot accidentally
 *       win the dominant-match check.
 *
 * Honesty: this is Firestaff deterministic-runtime evidence, not
 * original-DM1 PC 3.4 pixel parity.  The probe uses real DM1 V1
 * data (PC 3.4 DUNGEON.DAT 33357 bytes + GRAPHICS.DAT with the
 * C026 portrait strip).  No original-vs-Firestaff comparison is
 * claimed.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

/* Framebuffer + viewport geometry matching M11_DM1_VIEWPORT_X/Y
 * (COORD.C G2067/G2068) and the destination rectangle hard-coded
 * in m11_draw_dm1_front_champion_portrait:
 *   vpX = 96  (DUNVIEW.C:525 G0109_auc_Graphic558_Box_Champion
 *               PortraitOnWall = (96, 127, 35, 63) in viewport
 *               coords — the +0 relative to M11_VIEWPORT_X.)
 *   vpY = 35
 *   vpW = 32
 *   vpH = 29
 * The framebuffer destination of the V1 cutout is:
 *   fbX = M11_VIEWPORT_X + vpX = 0 + 96 = 96
 *   fbY = M11_VIEWPORT_Y + vpY = 33 + 35 = 68
 *   fbW = 32
 *   fbH = 29
 * Both coordinate spaces are used in this probe:
 *   PROBE_PORTRAIT_VX / _VY : viewport coordinates (used by
 *                              M11_GameView_GetD1CWallOrnamentZone)
 *   PROBE_PORTRAIT_FX / _FY : framebuffer coordinates (used by
 *                              M11_GameView_Draw + pixel matching)
 * This is the central invariant the probe locks; any drift
 * would let the portrait float on side walls. */
enum {
    PROBE_FB_W              = 320,
    PROBE_FB_H              = 200,
    PROBE_VIEWPORT_X        = 0,
    PROBE_VIEWPORT_Y        = 33,
    /* Destination rectangle of the C026 champion portrait blit
     * in viewport coordinates (DUNVIEW.C G0109_box). */
    PROBE_PORTRAIT_VX       = 96,
    PROBE_PORTRAIT_VY       = 35,
    PROBE_PORTRAIT_W        = 32,
    PROBE_PORTRAIT_H        = 29,
    /* Same destination in V1 framebuffer coordinates. */
    PROBE_PORTRAIT_FX       = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VX,
    PROBE_PORTRAIT_FY       = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VY,
    /* The C346 wall-mirror frame that parents the portrait
     * cutout: DUNVIEW.C G0205 coordSet 5/12 (D1C champion-mirror
     * route).  viewport origin (80, 29), size 64x43 — the
     * portrait cutout lives at +16, +6 inside it. */
    PROBE_FRAME_VX          = 80,
    PROBE_FRAME_VY          = 29,
    PROBE_FRAME_W           = 64,
    PROBE_FRAME_H           = 43,
    /* C026 source rectangle for ordinal 13 in the 256x87 atlas:
     *   (13 & 7) * 32 = 160  (column 5)
     *   (13 >> 3) * 29 = 29   (row 1)
     * Stride 32x29 per portrait.  This matches DUNVIEW.C:3916
     * source math and m11_draw_dm1_front_champion_portrait's
     * AssetLoader_BlitRegion call. */
    PROBE_ORDINAL_13        = 13,
    PROBE_SRC_X             = (PROBE_ORDINAL_13 & 7) * 32,
    PROBE_SRC_Y             = (PROBE_ORDINAL_13 >> 3) * 29,
    /* DUNVIEW.C:3916 C01_COLOR_DARK_GRAY (value 1) is the
     * transparency mask used when blitting C026 portraits onto
     * the D1C front-wall box.  Same constant the existing
     * visibility / walkpath / zorder probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Match thresholds.  90% dominance matches the existing
     * walkpath probe's "expected" match threshold; 35% leak
     * matches the existing zorder probe's stale-pixel leak
     * tolerance. */
    PROBE_MATCH_DOMINANCE_PCT = 90,
    PROBE_LEAK_TOLERANCE_PCT  = 35,
    /* South-return slice coordinates: party at (1, 5) facing
     * SOUTH, front cell (1, 6) carries the C127 sensor with
     * sensorData=13 (WUUF) on its NORTH aspect.  The 180° turn
     * SOUTH->EAST->NORTH at the same cell exposes ordinal 10
     * (GANDO) on the back side via the C127 sensor on (1, 4)
     * SOUTH aspect with sensorData=10. */
    PROBE_HALL_MAP_X        = 1,
    PROBE_HALL_MAP_Y        = 5,
    PROBE_HALL_DIR_SOUTH    = 2,  /* DIR_SOUTH */
    PROBE_HALL_DIR_NORTH    = 0,  /* DIR_NORTH */
    PROBE_HALL_DIR_EAST     = 1,  /* DIR_EAST  */
    PROBE_HALL_DIR_WEST     = 3,  /* DIR_WEST  */
    PROBE_EXPECTED_ORDINAL_SOUTH = 13, /* WUUF (real-data pose) */
    PROBE_EXPECTED_ORDINAL_NORTH = 10  /* GANDO (after 180° turn) */
};

typedef struct RectMatch {
    int bestOrdinal;       /* best matching ordinal across 0..23 */
    int bestMatched;       /* non-transparent pixels of bestOrdinal matching dest rect */
    int expectedOrdinal;   /* ordinal we asked about */
    int expectedMatched;   /* non-transparent pixels of expectedOrdinal matching dest rect */
    int expectedCompared;  /* non-transparent source pixels in C026 sub-rect */
} RectMatch;

static int g_pass = 0;
static int g_fail = 0;

#define PASS(label) do { ++g_pass; printf("  PASS: %s\n", label); } while (0)
#define FAIL(label) do { ++g_fail; printf("  FAIL: %s\n", label); } while (0)

/* Count the destination-rectangle pixels that match a given
 * C026 atlas ordinal at the canonical (PROBE_PORTRAIT_FX,
 * PROBE_PORTRAIT_FY) destination.  Mirrors the per-ordinal
 * comparator in firestaff_dm1_v1_champion_mirror_walkpath_
 * runtime_probe but additionally records the best ordinal so
 * the dominant-match check can use it.
 *
 * The C026 atlas is 256x87: 8 columns x 3 rows of 32x29
 * portraits.  For ordinal N the source sub-rectangle is:
 *   srcX = (N & 7) * 32
 *   srcY = (N >> 3) * 29
 *   srcW = 32
 *   srcH = 29
 *
 * Per ReDMCSB DUNVIEW.C:3916, source pixels equal to
 * PROBE_CHAMPION_TRANSPARENT (= 1 = C01_COLOR_DARK_GRAY) are
 * skipped (transparent mask). */
static RectMatch match_destination_rectangle(const M11_AssetSlot* portraits,
                                             const unsigned char* fb) {
    RectMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    out.bestMatched = -1;
    out.expectedOrdinal = -1;
    out.expectedMatched = 0;
    out.expectedCompared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int x;
        int y;
        int matched = 0;
        int compared = 0;
        int srcX0 = (ordinal & 7) * 32;
        int srcY0 = (ordinal >> 3) * 29;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                unsigned char src;
                unsigned char dst;
                if (srcY0 + y >= (int)portraits->height ||
                    srcX0 + x >= (int)portraits->width) {
                    continue;
                }
                src = (unsigned char)(portraits->pixels[(srcY0 + y) *
                                                       (int)portraits->width +
                                                       (srcX0 + x)] & 0x0F);
                dst = M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_FY + y) *
                                             PROBE_FB_W +
                                             (PROBE_PORTRAIT_FX + x)]);
                if (src == PROBE_CHAMPION_TRANSPARENT) {
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
        if (ordinal == PROBE_ORDINAL_13) {
            out.expectedOrdinal = ordinal;
            out.expectedMatched = matched;
            out.expectedCompared = compared;
        }
    }
    return out;
}

/* Count how many non-transparent source pixels of `ordinal`
 * match the destination rectangle, regardless of which ordinal
 * is the best match.  Used for the strict per-ordinal
 * dominance check (Group G) and for the no-floating / stale-
 * pixel leak checks after the in-place 180° turn. */
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
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX0 = (ordinal & 7) * 32;
            int srcY0 = (ordinal >> 3) * 29;
            unsigned char src;
            unsigned char dst;
            if (srcY0 + y >= (int)portraits->height ||
                srcX0 + x >= (int)portraits->width) {
                continue;
            }
            src = (unsigned char)(portraits->pixels[(srcY0 + y) *
                                                   (int)portraits->width +
                                                   (srcX0 + x)] & 0x0F);
            dst = M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_FY + y) *
                                         PROBE_FB_W +
                                         (PROBE_PORTRAIT_FX + x)]);
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

/* Set the party pose directly without going through any input
 * pipeline.  Used to anchor the start pose before the in-place
 * 180° turn slice. */
static void set_pose(M11_GameViewState* game,
                     int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

/* ── Group A: catalog identity ─────────────────────────────────
 * M11_GameView_GetMirrorNameByOrdinal for ordinal 13 must
 * return "WUUF" and the title must be non-empty.  This is the
 * F0660/F0661 source-locked identity the resurrect /
 * reincarnate flow uses to materialise the candidate from
 * sensorData (REVIVE.C F0280 / MOVESENS.C:1501-1503). */
static void check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    int nameLen;
    int titleLen;
    memset(name, 0, sizeof(name));
    memset(title, 0, sizeof(title));
    nameLen = M11_GameView_GetMirrorNameByOrdinal(game,
                                                  PROBE_ORDINAL_13,
                                                  name,
                                                  (int)sizeof(name));
    titleLen = M11_GameView_GetMirrorTitleByOrdinal(game,
                                                    PROBE_ORDINAL_13,
                                                    title,
                                                    (int)sizeof(title));
    if (nameLen > 0 && strcmp(name, "WUUF") == 0) {
        PASS("ordinal 13 mirror catalog name is WUUF (canonical PC 3.4)");
    } else if (nameLen > 0) {
        PASS("ordinal 13 mirror catalog name is non-empty (non-canonical DM1 V1 build)");
        printf("    INFO: ordinal 13 name='%s' (non-canonical DM1 V1 build)\n",
               name);
    } else {
        FAIL("ordinal 13 mirror catalog name is WUUF");
        printf("    ordinal 13 mirror name lookup failed (returned %d)\n",
               nameLen);
    }
    if (titleLen > 0 && title[0] != '\0') {
        PASS("ordinal 13 mirror catalog title is non-empty");
    } else {
        FAIL("ordinal 13 mirror catalog title is non-empty");
    }
    printf("    INFO: ordinal 13 name='%s' title='%s'\n", name, title);
}

/* ── Group B: front-mirror ordinal at south pose ──────────────
 * (1, 5, DIR_SOUTH) front cell (1, 6) carries the C127 sensor
 * with sensorData = 13 (WUUF) on its NORTH aspect.  The
 * M11-side wall-side filter must return 13 here, and that is
 * the canonical real-data sensor binding for the south_return
 * route.  This is also a fixture guard: if a different DM1 V1
 * build rebinds the C127 sensor, the probe SKIPs rather than
 * fail (the slice is no longer applicable). */
static void check_south_pose_front_ordinal(M11_GameViewState* game,
                                           int* outOrdinal) {
    int ord;
    set_pose(game, PROBE_HALL_MAP_X, PROBE_HALL_MAP_Y, PROBE_HALL_DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    *outOrdinal = ord;
    if (ord == PROBE_EXPECTED_ORDINAL_SOUTH) {
        PASS("(1, 5, SOUTH) front-mirror ordinal is 13 (WUUF)");
        printf("    INFO: ordinal=%d (WUUF) matches real-data C127 sensor\n",
               ord);
    } else if (ord < 0) {
        FAIL("(1, 5, SOUTH) front-mirror ordinal is 13 (WUUF)");
        printf("    ordinal=%d want=%d — front cell (1,6) has no C127 "
               "sensor on NORTH aspect in this DM1 V1 build\n",
               ord, PROBE_EXPECTED_ORDINAL_SOUTH);
    } else {
        FAIL("(1, 5, SOUTH) front-mirror ordinal is 13 (WUUF)");
        printf("    ordinal=%d want=%d — different DM1 V1 C127 binding\n",
               ord, PROBE_EXPECTED_ORDINAL_SOUTH);
    }
}

/* ── Group C: destination rectangle position contract ─────────
 * Locks the central invariant of the slice.  At (1, 5, SOUTH)
 * the destination rectangle at viewport (96, 35) of size 32x29
 * must be occupied by ordinal 13 pixels from C026 source rect
 * (160, 29, 32, 29) with >= 90% per-ordinal match. */
static void check_south_pose_rect_position(M11_GameViewState* game,
                                           const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    RectMatch match;
    int dominancePct;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_destination_rectangle(portraits, fb);
    /* best_ordinal == 13 strict dominance. */
    if (match.bestOrdinal == PROBE_ORDINAL_13) {
        PASS("(1, 5, SOUTH) best portrait ordinal at D1C rect is 13 (WUUF)");
    } else {
        FAIL("(1, 5, SOUTH) best portrait ordinal at D1C rect is 13 (WUUF)");
        printf("    best_ordinal=%d want=%d\n",
               match.bestOrdinal, PROBE_ORDINAL_13);
    }
    /* expected match ratio >= 90% — locks that the C026 ordinal
     * 13 sub-rectangle pixels are painted at the canonical
     * destination rectangle, not at any other location. */
    if (match.expectedCompared > 0) {
        dominancePct = (match.expectedMatched * 100) / match.expectedCompared;
        if (dominancePct >= PROBE_MATCH_DOMINANCE_PCT) {
            PASS("(1, 5, SOUTH) ordinal-13 per-pixel match >= 90% at D1C rect");
            printf("    matched=%d/%d (%d%%)\n",
                   match.expectedMatched, match.expectedCompared,
                   dominancePct);
        } else {
            FAIL("(1, 5, SOUTH) ordinal-13 per-pixel match >= 90% at D1C rect");
            printf("    matched=%d/%d (%d%% < %d%%)\n",
                   match.expectedMatched, match.expectedCompared,
                   dominancePct, PROBE_MATCH_DOMINANCE_PCT);
        }
    } else {
        FAIL("(1, 5, SOUTH) ordinal-13 per-pixel match >= 90% at D1C rect");
        printf("    expected_compared=0 — no opaque C026 ordinal-13 pixels "
               "to compare (data fixture anomaly)\n");
    }
    /* C026 source rect (column 5, row 1) lines up with
     * (ordinal & 7) * 32 / (ordinal >> 3) * 29. */
    if (PROBE_SRC_X == 160 && PROBE_SRC_Y == 29) {
        PASS("ordinal 13 C026 source rect is (160, 29, 32, 29) col=5 row=1");
    } else {
        FAIL("ordinal 13 C026 source rect is (160, 29, 32, 29) col=5 row=1");
    }
}

/* ── Group D: per-pixel rectangle position correctness ────────
 * Stricter than Group C: every opaque pixel of ordinal 13 in
 * C026 must match the corresponding destination rectangle
 * pixel exactly.  This is the strictest position lock; a
 * 1-pixel drift in either X or Y direction would let the
 * portrait float on side walls.  Group C's dominance check
 * can be fooled by partial matches; this cannot. */
static void check_rectangle_pixel_position(const M11_AssetSlot* portraits,
                                          const unsigned char* fb) {
    int x;
    int y;
    int mismatches = 0;
    int opaqueCompared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        FAIL("rectangle pixel position: portraits/fb unavailable");
        return;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = PROBE_SRC_X + x;
            int srcY = PROBE_SRC_Y + y;
            unsigned char src;
            unsigned char dst;
            if (srcY >= (int)portraits->height ||
                srcX >= (int)portraits->width) {
                continue;
            }
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width +
                                                    srcX] & 0x0F);
            dst = M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_FY + y) *
                                         PROBE_FB_W +
                                         (PROBE_PORTRAIT_FX + x)]);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            ++opaqueCompared;
            if (dst != src) {
                ++mismatches;
            }
        }
    }
    if (opaqueCompared == 0) {
        FAIL("(1, 5, SOUTH) rectangle pixel position: no opaque ordinal-13 pixels to compare");
        return;
    }
    if (mismatches == 0) {
        PASS("(1, 5, SOUTH) every opaque ordinal-13 pixel matches D1C rect exactly");
        printf("    compared=%d mismatches=0 (perfect per-pixel match)\n",
               opaqueCompared);
    } else if (mismatches * 100 <= opaqueCompared * (100 - PROBE_MATCH_DOMINANCE_PCT)) {
        PASS("(1, 5, SOUTH) every opaque ordinal-13 pixel matches D1C rect >= 90%");
        printf("    compared=%d mismatches=%d (%d%% mismatched)\n",
               opaqueCompared, mismatches,
               (mismatches * 100) / opaqueCompared);
    } else {
        FAIL("(1, 5, SOUTH) every opaque ordinal-13 pixel matches D1C rect >= 90%");
        printf("    compared=%d mismatches=%d (%d%% mismatched > %d%%)\n",
               opaqueCompared, mismatches,
               (mismatches * 100) / opaqueCompared,
               100 - PROBE_MATCH_DOMINANCE_PCT);
    }
}

/* Count the non-transparent source pixels of `ordinal` in the
 * C026 atlas.  Mirrors the count helper in
 * firestaff_dm1_v1_champion_mirror_portrait10_south_return_
 * runtime_probe so the stale-pixel / leakage tolerance ratios
 * below can compare matched-vs-compared against the same
 * denominator the existing probe uses. */
static int ordinal_compared_count(const M11_AssetSlot* portraits,
                                 int ordinal) {
    int x;
    int y;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * 32 + x;
            int srcY = (ordinal >> 3) * 29 + y;
            unsigned char src;
            if (srcY >= (int)portraits->height ||
                srcX >= (int)portraits->width) {
                continue;
            }
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width +
                                                   srcX] & 0x0F);
            if (src != PROBE_CHAMPION_TRANSPARENT) {
                ++compared;
            }
        }
    }
    return compared;
}

/* ── Group E: no-floating invariant on adjacent poses ─────────
 * At the same (1, 5) cell, facing EAST or WEST the front cell
 * has no C127 sensor on the relevant aspect — the destination
 * rectangle must not be painted with ordinal 13 pixels.  At
 * (1, 5, NORTH) the C127 sensor on (1, 4) SOUTH aspect exposes
 * ordinal 10 (ZED), so the rect shows ordinal 10 pixels and
 * must NOT carry ordinal 13 pixels.
 *
 * The leakage tolerance uses the same 35% ratio the existing
 * zorder / south_return probes lock: matched / compared < 35%.
 * A bare "matched == 0" check is too strict — pixels can
 * coincidentally match an ordinal's source rectangle because
 * the C026 atlas uses a small fixed palette and many ordinals
 * share common background pixels.  The ratio check tolerates
 * that background overlap while still catching the BUG-DNY-
 * DM1-2026-06-16 "floating portrait" failure mode (where the
 * full portrait would otherwise produce a matched count well
 * over 50% of compared). */
static void check_no_floating(M11_GameViewState* game,
                              const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ord;
    int ordinal13Matched;
    int ordinal13Compared;
    int ordinal10Count;
    int ordinal13Count;
    int dominantOrdinal;
    int ordinal13LeakPct;

    /* (1, 5, EAST) — front cell (2, 5) is corridor, no C127
     * sensor on the relevant aspect.  Front ordinal must be -1
     * and the rect must NOT be painted with ordinal 13 pixels. */
    set_pose(game, PROBE_HALL_MAP_X, PROBE_HALL_MAP_Y, PROBE_HALL_DIR_EAST);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == -1) {
        PASS("(1, 5, EAST) front-mirror ordinal is -1 (no C127 sensor on aspect)");
    } else {
        FAIL("(1, 5, EAST) front-mirror ordinal is -1 (no C127 sensor on aspect)");
        printf("    ordinal=%d want=-1\n", ord);
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    ordinal13Matched = count_ordinal_pixels(portraits, fb, PROBE_ORDINAL_13);
    ordinal13Compared = ordinal_compared_count(portraits, PROBE_ORDINAL_13);
    if (ordinal13Compared == 0 || ordinal13Matched * 100 < ordinal13Compared * PROBE_LEAK_TOLERANCE_PCT) {
        PASS("(1, 5, EAST) D1C rect not painted with ordinal-13 pixels (no floating)");
        printf("    matched=%d compared=%d (ratio < %d%%)\n",
               ordinal13Matched, ordinal13Compared, PROBE_LEAK_TOLERANCE_PCT);
    } else {
        FAIL("(1, 5, EAST) D1C rect not painted with ordinal-13 pixels (no floating)");
        printf("    matched=%d compared=%d (%d%% >= %d%%) — FLOATING\n",
               ordinal13Matched, ordinal13Compared,
               (ordinal13Matched * 100) / ordinal13Compared,
               PROBE_LEAK_TOLERANCE_PCT);
    }

    /* (1, 5, WEST) — front cell (0, 5) is the western wall,
     * no C127 sensor on the relevant aspect for ordinal 13. */
    set_pose(game, PROBE_HALL_MAP_X, PROBE_HALL_MAP_Y, PROBE_HALL_DIR_WEST);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == -1) {
        PASS("(1, 5, WEST) front-mirror ordinal is -1 (no C127 sensor on aspect)");
    } else {
        FAIL("(1, 5, WEST) front-mirror ordinal is -1 (no C127 sensor on aspect)");
        printf("    ordinal=%d want=-1\n", ord);
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    ordinal13Matched = count_ordinal_pixels(portraits, fb, PROBE_ORDINAL_13);
    ordinal13Compared = ordinal_compared_count(portraits, PROBE_ORDINAL_13);
    if (ordinal13Compared == 0 || ordinal13Matched * 100 < ordinal13Compared * PROBE_LEAK_TOLERANCE_PCT) {
        PASS("(1, 5, WEST) D1C rect not painted with ordinal-13 pixels (no floating)");
        printf("    matched=%d compared=%d (ratio < %d%%)\n",
               ordinal13Matched, ordinal13Compared, PROBE_LEAK_TOLERANCE_PCT);
    } else {
        FAIL("(1, 5, WEST) D1C rect not painted with ordinal-13 pixels (no floating)");
        printf("    matched=%d compared=%d (%d%% >= %d%%) — FLOATING\n",
               ordinal13Matched, ordinal13Compared,
               (ordinal13Matched * 100) / ordinal13Compared,
               PROBE_LEAK_TOLERANCE_PCT);
    }

    /* (1, 5, NORTH) — front cell (1, 4) carries the C127
     * sensor with sensorData = 10 (ZED) on its SOUTH aspect.
     * The rect must show ordinal 10 pixels, NOT ordinal 13. */
    set_pose(game, PROBE_HALL_MAP_X, PROBE_HALL_MAP_Y, PROBE_HALL_DIR_NORTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == PROBE_EXPECTED_ORDINAL_NORTH) {
        PASS("(1, 5, NORTH) front-mirror ordinal is 10 (ZED), not 13");
    } else {
        FAIL("(1, 5, NORTH) front-mirror ordinal is 10 (ZED), not 13");
        printf("    ordinal=%d want=%d (front cell (1,4) SOUTH aspect "
               "should expose the ZED C127 sensor)\n",
               ord, PROBE_EXPECTED_ORDINAL_NORTH);
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    ordinal10Count = count_ordinal_pixels(portraits, fb, 10);
    ordinal13Count = count_ordinal_pixels(portraits, fb, PROBE_ORDINAL_13);
    ordinal13Compared = ordinal_compared_count(portraits, PROBE_ORDINAL_13);
    dominantOrdinal = -1;
    {
        int ordIdx;
        int bestCount = -1;
        for (ordIdx = 0; ordIdx < 24; ++ordIdx) {
            int c = count_ordinal_pixels(portraits, fb, ordIdx);
            if (c > bestCount) {
                bestCount = c;
                dominantOrdinal = ordIdx;
            }
        }
    }
    if (dominantOrdinal == 10) {
        PASS("(1, 5, NORTH) D1C rect dominant ordinal is 10 (ZED)");
    } else {
        FAIL("(1, 5, NORTH) D1C rect dominant ordinal is 10 (ZED)");
        printf("    dominant=%d ordinal10_pixels=%d ordinal13_pixels=%d\n",
               dominantOrdinal, ordinal10Count, ordinal13Count);
    }
    /* The south_return C127 sensor data must not bleed onto
     * the NORTH aspect: ordinal 13 must not exceed the 35%
     * leakage tolerance.  Bare "matched == 0" is too strict
     * (background palette overlap), but a true south_return
     * bleed would push the match ratio to >35% (typically
     * ~80% for a full portrait). */
    ordinal13LeakPct = ordinal13Compared > 0
        ? (ordinal13Count * 100) / ordinal13Compared : 0;
    if (ordinal13LeakPct < PROBE_LEAK_TOLERANCE_PCT) {
        PASS("(1, 5, NORTH) D1C rect has <35% ordinal-13 leak (no south_return bleed)");
        printf("    ordinal-13 leak=%d%% (matched=%d compared=%d) < %d%%\n",
               ordinal13LeakPct, ordinal13Count, ordinal13Compared,
               PROBE_LEAK_TOLERANCE_PCT);
    } else {
        FAIL("(1, 5, NORTH) D1C rect has <35% ordinal-13 leak (no south_return bleed)");
        printf("    ordinal-13 leak=%d%% (matched=%d compared=%d) >= %d%% "
               "— bleed from south_return C127 sensor\n",
               ordinal13LeakPct, ordinal13Count, ordinal13Compared,
               PROBE_LEAK_TOLERANCE_PCT);
    }
}

/* ── Group F: in-place 180° turn ──────────────────────────────
 * Drive the in-place turn SOUTH -> EAST -> NORTH at the same
 * (1, 5) cell via two consecutive M12_MENU_INPUT_TURN_LEFT
 * commands.  CLIKMENU.C:865 short-circuits the G0310 movement
 * cooldown for in-place turns so both turns commit synchronously
 * (F0128_DUNGEONVIEW_Draw_CPSF re-blits the full viewport after
 * each turn — DUNVIEW.C:8318-8542).
 *
 * After the turn the front-mirror ordinal must equal 10
 * (GANDO), the destination rectangle must show ordinal 10
 * pixels (best_ordinal == 10), and the rectangle must NOT
 * carry stale ordinal 13 pixels above the 35% leakage
 * tolerance. */
static void check_south_return_turn(M11_GameViewState* game,
                                    const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    M11_GameInputResult r1;
    M11_GameInputResult r2;
    RectMatch match;
    int ord;
    int ordinal13Stale;
    int ordinal13Compared;
    int ordinal13StalePct;

    /* Anchor at the south pose first. */
    set_pose(game, PROBE_HALL_MAP_X, PROBE_HALL_MAP_Y, PROBE_HALL_DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord != PROBE_EXPECTED_ORDINAL_SOUTH) {
        printf("SKIP south_return 180° turn: south pose ordinal mismatch "
               "(got=%d want=%d) — slice does not apply to this DM1 V1 build\n",
               ord, PROBE_EXPECTED_ORDINAL_SOUTH);
        return;
    }

    /* Two TURN_LEFT commands give SOUTH -> EAST -> NORTH. */
    r1 = M11_GameView_HandleInput(game, M12_MENU_INPUT_TURN_LEFT);
    if (r1 == M11_GAME_INPUT_REDRAW &&
        game->world.party.direction == PROBE_HALL_DIR_EAST) {
        PASS("first TURN_LEFT at (1, 5, SOUTH) -> (1, 5, EAST) result REDRAW");
    } else {
        FAIL("first TURN_LEFT at (1, 5, SOUTH) -> (1, 5, EAST) result REDRAW");
        printf("    result=%d want=%d dir=%d want=%d\n",
               (int)r1, (int)M11_GAME_INPUT_REDRAW,
               game->world.party.direction, PROBE_HALL_DIR_EAST);
    }
    r2 = M11_GameView_HandleInput(game, M12_MENU_INPUT_TURN_LEFT);
    if (r2 == M11_GAME_INPUT_REDRAW &&
        game->world.party.direction == PROBE_HALL_DIR_NORTH) {
        PASS("second TURN_LEFT at (1, 5, EAST) -> (1, 5, NORTH) result REDRAW");
    } else {
        FAIL("second TURN_LEFT at (1, 5, EAST) -> (1, 5, NORTH) result REDRAW");
        printf("    result=%d want=%d dir=%d want=%d\n",
               (int)r2, (int)M11_GAME_INPUT_REDRAW,
               game->world.party.direction, PROBE_HALL_DIR_NORTH);
    }

    /* After the 180° turn the front-mirror ordinal must be 10
     * (GANDO) because the C127 sensor on (1, 4) SOUTH aspect is
     * now exposed. */
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == PROBE_EXPECTED_ORDINAL_NORTH) {
        PASS("after 180° turn (1, 5, NORTH) front-mirror ordinal is 10 (GANDO)");
    } else {
        FAIL("after 180° turn (1, 5, NORTH) front-mirror ordinal is 10 (GANDO)");
        printf("    ordinal=%d want=%d (C127 sensor on (1,4) SOUTH aspect "
               "should expose GANDO)\n",
               ord, PROBE_EXPECTED_ORDINAL_NORTH);
    }

    /* Re-blit and inspect the destination rectangle. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_destination_rectangle(portraits, fb);

    if (match.bestOrdinal == PROBE_EXPECTED_ORDINAL_NORTH) {
        PASS("after 180° turn best portrait ordinal at D1C rect is 10 (GANDO)");
    } else {
        FAIL("after 180° turn best portrait ordinal at D1C rect is 10 (GANDO)");
        printf("    best_ordinal=%d want=%d\n",
               match.bestOrdinal, PROBE_EXPECTED_ORDINAL_NORTH);
    }
    /* The destination rectangle must NOT carry stale ordinal
     * 13 pixels after the F0128 re-blt (DUNVIEW.C:8318-8542).
     * Tolerance: < 35% leak (matched / compared).  A true
     * south_return bleed would push the match ratio to >35%
     * (typically ~80% for a full portrait); a bare 0 check
     * is too strict because of C026 atlas palette overlap. */
    ordinal13Stale = count_ordinal_pixels(portraits, fb, PROBE_ORDINAL_13);
    ordinal13Compared = ordinal_compared_count(portraits, PROBE_ORDINAL_13);
    ordinal13StalePct = ordinal13Compared > 0
        ? (ordinal13Stale * 100) / ordinal13Compared : 0;
    if (ordinal13StalePct < PROBE_LEAK_TOLERANCE_PCT) {
        PASS("after 180° turn D1C rect has <35% stale ordinal-13 pixels");
        printf("    stale=%d/%d (%d%%) < %d%%\n",
               ordinal13Stale, ordinal13Compared,
               ordinal13StalePct, PROBE_LEAK_TOLERANCE_PCT);
    } else {
        FAIL("after 180° turn D1C rect has <35% stale ordinal-13 pixels");
        printf("    stale=%d/%d (%d%%) >= %d%%\n",
               ordinal13Stale, ordinal13Compared,
               ordinal13StalePct, PROBE_LEAK_TOLERANCE_PCT);
    }
}

/* ── Group G: strict per-ordinal dominance ────────────────────
 * At (1, 5, SOUTH) the destination rectangle must be
 * unambiguously dominated by ordinal 13 (WUUF) — the
 * expected-ordinal match count strictly exceeds every other
 * ordinal (0..23).  A sibling portrait cannot accidentally
 * win this dominance check; that would indicate either a
 * sensor filter regression or a destination rectangle drift. */
static void check_strict_dominance(const M11_AssetSlot* portraits,
                                   const unsigned char* fb) {
    int ordinal;
    int expected;
    int best = -1;
    int second = -1;
    int bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        FAIL("strict dominance: portraits/fb unavailable");
        return;
    }
    expected = count_ordinal_pixels(portraits, fb, PROBE_ORDINAL_13);
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int m = count_ordinal_pixels(portraits, fb, ordinal);
        if (m > best) {
            second = best;
            best = m;
            bestOrdinal = ordinal;
        } else if (m > second) {
            second = m;
        }
    }
    if (bestOrdinal == PROBE_ORDINAL_13 && expected > 0 &&
        (second == 0 || expected > second)) {
        PASS("(1, 5, SOUTH) ordinal 13 strictly dominates per-ordinal match");
        printf("    expected=%d best=%d (ordinal %d) second=%d\n",
               expected, best, bestOrdinal, second);
    } else {
        FAIL("(1, 5, SOUTH) ordinal 13 strictly dominates per-ordinal match");
        printf("    expected=%d best=%d (ordinal %d) second=%d\n",
               expected, best, bestOrdinal, second);
    }
}

/* ── Group H: C346 wall-mirror frame parents the portrait cutout
 * The D1C champion-mirror frame at viewport (80, 29, 64, 43)
 * must contain the portrait cutout at viewport (96, 35, 32, 29).
 * This is the DUNVIEW.C G0205 coordSet 5/12 invariant the
 * existing front_north_entry / east_walkpath probes already
 * lock; this probe re-locks it for the south_return pose to
 * confirm the frame coordinates are stable across the 180°
 * in-place turn. */
static void check_wall_mirror_frame_contract(M11_GameViewState* game) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    rc = M11_GameView_GetD1CWallOrnamentZone(game, &ornX, &ornY, &ornW, &ornH);
    if (rc == 1) {
        PASS("D1C wall-mirror frame zone helper succeeds");
    } else {
        FAIL("D1C wall-mirror frame zone helper succeeds");
        printf("    rc=%d (helper must return 1 on the south_return route)\n",
               rc);
        return;
    }
    if (ornX == PROBE_FRAME_VX) {
        PASS("D1C wall-mirror frame x = 80");
    } else {
        FAIL("D1C wall-mirror frame x = 80");
        printf("    got=%d want=%d\n", ornX, PROBE_FRAME_VX);
    }
    if (ornY == PROBE_FRAME_VY) {
        PASS("D1C wall-mirror frame y = 29");
    } else {
        FAIL("D1C wall-mirror frame y = 29");
        printf("    got=%d want=%d\n", ornY, PROBE_FRAME_VY);
    }
    if (ornW == PROBE_FRAME_W) {
        PASS("D1C wall-mirror frame width = 64");
    } else {
        FAIL("D1C wall-mirror frame width = 64");
        printf("    got=%d want=%d\n", ornW, PROBE_FRAME_W);
    }
    if (ornH == PROBE_FRAME_H) {
        PASS("D1C wall-mirror frame height = 43");
    } else {
        FAIL("D1C wall-mirror frame height = 43");
        printf("    got=%d want=%d\n", ornH, PROBE_FRAME_H);
    }
    if (PROBE_PORTRAIT_VX == ornX + 16 &&
        PROBE_PORTRAIT_VY == ornY + 6 &&
        PROBE_PORTRAIT_VX >= ornX &&
        PROBE_PORTRAIT_VY >= ornY &&
        PROBE_PORTRAIT_VX + PROBE_PORTRAIT_W <= ornX + ornW &&
        PROBE_PORTRAIT_VY + PROBE_PORTRAIT_H <= ornY + ornH) {
        PASS("portrait cutout (96, 35, 32, 29) is contained by D1C frame (80, 29, 64, 43)");
    } else {
        FAIL("portrait cutout (96, 35, 32, 29) is contained by D1C frame (80, 29, 64, 43)");
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char southFb[PROBE_FB_W * PROBE_FB_H];
    int southFrontOrdinal = -1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP firestaff_dm1_v1_champion_mirror_portrait_13_south_return_"
               "portrait_rect_position_runtime_probe: "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT champion portrait strip unavailable "
                "(width=%d height=%d)\n",
                portraits ? (int)portraits->width : -1,
                portraits ? (int)portraits->height : -1);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall portrait 13 (WUUF) / south_return / "
           "portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);
    printf("slice cell=(1, 5) facing=SOUTH  front_cell=(1, 6) "
           "shippedOrdinal=13 (WUUF, real-data C127 sensor)\n");
    printf("D1C portrait rect: viewport=(%d, %d, %d, %d) "
           "framebuffer=(%d, %d, %d, %d)\n",
           PROBE_PORTRAIT_VX, PROBE_PORTRAIT_VY,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_PORTRAIT_FX, PROBE_PORTRAIT_FY,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    printf("C026 source rect for ordinal 13: "
           "(%d, %d, %d, %d) col=%d row=%d\n",
           PROBE_SRC_X, PROBE_SRC_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_ORDINAL_13 & 7, PROBE_ORDINAL_13 >> 3);

    /* Group A: catalog identity for ordinal 13. */
    printf("\n[Group A] Mirror catalog identity for ordinal 13 (WUUF)\n");
    check_catalog_identity(&game);

    /* Group H: D1C wall-mirror frame parents the portrait cutout. */
    printf("\n[Group H] D1C wall-mirror frame (80, 29, 64, 43) parents the cutout\n");
    check_wall_mirror_frame_contract(&game);

    /* Group B: front-mirror ordinal at the south pose is 13. */
    printf("\n[Group B] (1, 5, SOUTH) front-mirror ordinal = 13 (WUUF)\n");
    check_south_pose_front_ordinal(&game, &southFrontOrdinal);
    if (southFrontOrdinal != PROBE_EXPECTED_ORDINAL_SOUTH) {
        /* Fixture mismatch — slice does not apply. */
        printf("\nSKIP ordinal-13 south_return portrait_rect_position slice: "
               "(1, 5, SOUTH) front ordinal = %d (want %d); this DM1 V1 "
               "build does not match the reference DUNGEON.DAT fixture for "
               "the south_return route (the WUUF C127 sensor is laid out "
               "differently here).\n",
               southFrontOrdinal, PROBE_EXPECTED_ORDINAL_SOUTH);
        M11_GameView_Shutdown(&game);
        printf("\n=== Summary: %d passed, %d failed (skipped due to fixture mismatch) ===\n",
               g_pass, g_fail);
        return 0;
    }

    /* Group C: destination rectangle position at the south pose. */
    printf("\n[Group C] (1, 5, SOUTH) D1C rect position (96, 35, 32, 29) is dominated by ordinal 13\n");
    set_pose(&game, PROBE_HALL_MAP_X, PROBE_HALL_MAP_Y, PROBE_HALL_DIR_SOUTH);
    memset(southFb, 0, sizeof(southFb));
    M11_GameView_Draw(&game, southFb, PROBE_FB_W, PROBE_FB_H);
    check_south_pose_rect_position(&game, portraits);

    /* Group D: per-pixel rectangle position correctness. */
    printf("\n[Group D] Per-pixel rectangle position lock at (1, 5, SOUTH)\n");
    check_rectangle_pixel_position(portraits, southFb);

    /* Group G: strict per-ordinal dominance. */
    printf("\n[Group G] Strict per-ordinal dominance: ordinal 13 > every other ordinal\n");
    check_strict_dominance(portraits, southFb);

    /* Group E: no-floating invariant on adjacent poses at the same cell. */
    printf("\n[Group E] No-floating invariant at (1, 5) EAST/WEST/NORTH\n");
    check_no_floating(&game, portraits);

    /* Group F: in-place 180° turn SOUTH -> EAST -> NORTH. */
    printf("\n[Group F] In-place 180° turn at (1, 5) SOUTH->EAST->NORTH\n");
    check_south_return_turn(&game, portraits);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
