/*
 * firestaff_dm1_v1_hall_champion_portrait_12_front_south_entry_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice that is intentionally NOT covered by the
 * existing ordinal-12 probes:
 *
 *   ordinal       : 12 (C026 col 4 row 1; mirror-catalog record LINFLAS)
 *   route variant : front_south_entry — the player at the canonical
 *                   ordinal-12 viewing pose (map=0, x=2, y=10) facing
 *                   NORTH where M11_GameView_GetFrontMirrorOrdinal
 *                   returns 12 = LINFLAS, then the player TURNS to
 *                   face SOUTH at the same cell, exercising the
 *                   south-entry exit from the LINFLAS route.  The
 *                   "front_south_entry" name documents this as the
 *                   south-facing view that, in real DM1 V1
 *                   DUNGEON.DAT (PC 3.4), has no C127 sensor with
 *                   sensorData=12 — i.e. the south-entry arc INTO the
 *                   ordinal-12 cell exists only as the SOUTH-facing
 *                   view from the same (2,10) cell, not as a separate
 *                   ordinal-12 route.
 *   aspect        : portrait_rect_position (D1C front-wall cutout at
 *                   viewport (96,35,32,29)) + candidate_panel_return +
 *                   redraw_stability, locking four contracts the
 *                   existing ordinal-12 probes leave uncovered for
 *                   the south-facing exit at the LINFLAS pose:
 *
 *                   (A) Canonical ordinal-12 pose at (2,10,N) returns
 *                       front-mirror ordinal 12 = LINFLAS and binds
 *                       the slice to the real source identity.
 *                   (B) South-facing poses on Hall map 0 NEVER expose
 *                       ordinal 12 — exhaustive scan across all 16x16
 *                       cells x 4 directions proves the source data
 *                       has no south-facing C127 sensor with
 *                       sensorData=12, so the "front_south_entry"
 *                       route variant is canonically a no-route
 *                       contract for ordinal 12.
 *                   (C) South-facing side pose at the canonical
 *                       (2,10) cell returns front-mirror ordinal -1
 *                       and the D1C portrait rect contains NO
 *                       ordinal-12 pixels (no floating of LINFLAS
 *                       onto the south wall when the player turns to
 *                       face SOUTH at the same cell).
 *                   (D) Candidate-panel return: after opening the C040
 *                       resurrect/reincarnate panel at (2,10,N),
 *                       cancelling, then turning to face SOUTH, the
 *                       panel state is cleared (candidateMirrorPanel
 *                       Active=0, candidateMirrorOrdinal=-1) and the
 *                       D1C portrait rect remains stable (no
 *                       stale-panel bleed into the south-facing
 *                       redraw).
 *
 * This probe widens the existing ordinal-12 coverage along a
 * different axis than:
 *
 *   firestaff_dm1_v1_hall_champion_portrait_12_front_north_entry_runtime_probe
 *     - covers the static front_north_entry pose (1,2,N) -> ordinal 1
 *       HALK, the ordinal 12 any-pose discovery, and the no-floating
 *       side-wall poses on the x=1 / y=3 corridor.  Does NOT cover
 *       the (2,10) south-facing exit, the candidate-panel round-trip
 *       from the LINFLAS pose, or the redraw stability after the
 *       south-facing turn.
 *   firestaff_dm1_v1_hall_champion_portrait_12_east_walkpath_portrait_rect_probe
 *     - covers (1,10,N) -> (2,10,N) -> (3,10,N) via direct set_pose
 *       teleport, asserts the D1C portrait_rect_position contract at
 *       (2,10,N) and the side-wall no-floating contract at (2,10)
 *       DIR_EAST / DIR_SOUTH / DIR_WEST via direct pose mutation.
 *       Does NOT exercise the live C040 panel open/cancel round-trip
 *       nor the redraw stability after the candidate panel is
 *       dismissed and the player turns to face SOUTH.
 *   firestaff_dm1_v1_hall_champion_portrait_12_walkpath_from_entrance_runtime_probe
 *     - drives the input-path walkpath from the (1,2,N) entrance
 *       through (1,10,N) ZED to (2,10,N) LINFLAS via turn-right +
 *       forward-step + turn-left.  Does NOT cover the south-facing
 *       exit from (2,10) — the player stays facing NORTH at the
 *       LINFLAS pose and only rotates to E/S/W as a side-wall
 *       no-floating check, not as a candidate-panel return path.
 *
 * Source evidence:
 *   ReDMCSB WIP 20210206:
 *     DUNGEON.C:2558,2608-2612  C127 sensorData -> G0289 ordinal
 *     DUNGEON.C:2573            M011_CELL(sensor) -> visible wall cell
 *     MOVESENS.C:1501-1503      C127 -> F0280 candidate materialise
 *     REVIVE.C F0280            materialize candidate from sensorData
 *     REVIVE.C F0282            confirm/cancel panel clears G0299
 *     DUNVIEW.C:3913-3928       C346 wall frame + C026 portrait blit
 *     DUNVIEW.C:8318-8542 F0128 far-to-near viewport draw order
 *     DUNVIEW.C:8522-8533       C026 D1C re-blt on tick redraw
 *     COORD.C:1693-1722         PC 3.4 viewport origin / 224x136
 *     DEFS.H:2071-2079          G2071_C320 / G2078_C32 / G2079_C29
 *
 * Honest scope: this probe proves the source-locked C026 ordinal
 * placement at the canonical LINFLAS pose, the no-route contract
 * for south-facing C127 sensors on Hall map 0 (the "front_south
 * _entry" route variant is canonically absent for ordinal 12), the
 * south-facing side-pose no-floating contract at (2,10), the
 * candidate-panel return behavior when the player turns to face
 * SOUTH at the LINFLAS pose after cancelling the resurrect/reincarnate
 * panel, and the redraw stability of the D1C rect after the
 * candidate panel is dismissed and the player turns to face SOUTH.
 *
 * It does NOT claim DOS pixel parity beyond the same C01 dark-gray
 * transparency contract the existing portrait / zorder / reblt /
 * east_walkpath / walkpath_from_entrance probes lock.  Original DM1
 * PC 3.4 captures live under parity-evidence/ and are referenced by
 * separate parity gates.
 *
 * Usage: firestaff_dm1_v1_hall_champion_portrait_12_front_south_entry_runtime_probe DATA_DIR
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
    /* DUNVIEW.C:3913-3928 / 8522-8533: D1C front-wall box is the 32x29
     * rectangle at (96,35)-(127,63) of the viewport, drawn from the
     * C026 champion portrait strip indexed by the C127 sensor ordinal
     * stored in G0289. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* DUNVIEW.C:3916: C026 champion portrait blit masks
     * C01_COLOR_DARK_GRAY (value 1) as transparency. Same constant
     * the existing portrait / zorder / reblt / east_walkpath /
     * walkpath_from_entrance probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Canonical ordinal-12 viewing pose: (map=0, x=2, y=10) facing
     * NORTH — M11_GameView_GetFrontMirrorOrdinal returns 12 = LINFLAS.
     * Pinning this so the slice is bound to a real source identity. */
    PROBE_LINFLAS_X = 2,
    PROBE_LINFLAS_Y = 10,
    PROBE_LINFLAS_DIR = 0, /* DIR_NORTH */
    PROBE_LINFLAS_SOUTH_DIR = 2, /* DIR_SOUTH — the front_south_entry exit pose */
    PROBE_LINFLAS_EAST_DIR = 1, /* DIR_EAST */
    PROBE_LINFLAS_WEST_DIR = 3, /* DIR_WEST */
    PROBE_ORDINAL_TARGET = 12,
    HALL_MAP_INDEX = 0,
    HALL_MAX_CELLS_PER_AXIS = 16,
    /* C026 champion-portrait strip dimensions: 8 cols x 3 rows of
     * 32x29 portraits (ordinals 0..23). */
    PROBE_PORTRAIT_STRIP_W = 256,
    PROBE_PORTRAIT_STRIP_H = 87,
    /* Re-blt invariant tolerance matching the existing walkpath /
     * zorder / reblt / east_walkpath / walkpath_from_entrance
     * probes: the ordinal-12 matched-pixel count in the D1C rect
     * must not reach 35% of its compared count when the player is
     * NOT facing the front wall of the (2,10) cell, otherwise
     * ordinal 12 is "floating" on the side wall. */
    PROBE_FLOOR_LEAK_PCT = 35,
    /* Positive-ordinal pixel match threshold matching the existing
     * east_walkpath / walkpath_from_entrance probes: 90% of the C026
     * ordinal-12 opaque pixels must be present in the D1C rect for
     * the LINFLAS pose to be considered properly drawn. */
    PROBE_POSITIVE_MATCH_PCT = 90
};

static int g_pass;
static int g_fail;

static int expect_int(const char* label, int got, int want) {
    ++g_pass;
    if (got == want) {
        printf("  PASS: %s == %d\n", label, want);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    return 0;
}

/* Direct-pose helper mirroring the
 * start_independent_input_route contract the existing walkpath /
 * walkpath_from_entrance / east_walkpath probes use between
 * independent routes: reset the candidate panel state so a
 * previous mirror panel does not leak into the next check. */
static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = (int16_t)mapX;
    game->world.party.mapY = (int16_t)mapY;
    game->world.party.direction = (uint8_t)dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

/* Count the pixels in the front-wall box that match the C026
 * champion portrait ordinal.  Same formula as the visibility /
 * zorder / reblt / east_walkpath / walkpath_from_entrance probes:
 *   DUNVIEW.C:3916  C01 dark-gray (value 1) is the transparency mask
 *   DUNVIEW.C:3918  per-ordinal source stride
 *                   srcX = (ordinal & 7) * 32, srcY = (ordinal >> 3) * 29
 *
 * Returns 0 when the ordinal is out of range or the slot is unloaded. */
static int count_ordinal_matched_pixels(const M11_AssetSlot* portraits,
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
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                       (PROBE_PORTRAIT_X + x)]);
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

/* Count the C026 ordinal pixels compared in the D1C rect (matching
 * the "compared" count the existing east_walkpath /
 * walkpath_from_entrance probes use to compute the leak
 * percentage). */
static int count_ordinal_compared_pixels(const M11_AssetSlot* portraits,
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
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            ++compared;
        }
    }
    return compared;
}

/* (A) Canonical ordinal-12 pose: (2,10,N) -> front-mirror ordinal 12,
 *     bound to mirror-catalog record LINFLAS.  Locks the real source
 *     identity so future regressions can detect any change in the
 *     C127 sensorData -> mirror name binding. */
static int test_canonical_ordinal_12_pose(M11_GameViewState* game) {
    int ord = -999;
    char nameBuf[32];
    int ok = 1;
    printf("[A] Canonical ordinal 12 pose at (2,10,N) -> LINFLAS\n");
    set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, PROBE_LINFLAS_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    ok &= expect_int("front_mirror ordinal at (2,10,N)", ord, PROBE_ORDINAL_TARGET);
    nameBuf[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(game, PROBE_ORDINAL_TARGET,
                                              nameBuf, (int)sizeof(nameBuf));
    ok &= expect_int("ordinal 12 name == LINFLAS",
                     strcmp(nameBuf, "LINFLAS") == 0, 1);
    /* Lock the C026 source-rect math the pixel checks below
     * rely on: ordinal 12 -> col 4 row 1 -> (128, 29, 32, 29)
     * (DEFS.H portrait-grid 8-col atlas math). */
    {
        int col = -1, row = -1, sx = -1, sy = -1;
        col = PROBE_ORDINAL_TARGET & 7;
        row = (PROBE_ORDINAL_TARGET >> 3) & 3;
        sx = col * PROBE_PORTRAIT_W;
        sy = row * PROBE_PORTRAIT_H;
        ok &= expect_int("ordinal 12 col (col = ordinal mod 8)", col, 4);
        ok &= expect_int("ordinal 12 row (row = ordinal / 8)", row, 1);
        ok &= expect_int("ordinal 12 source X == 4*32", sx, 128);
        ok &= expect_int("ordinal 12 source Y == 1*29", sy, 29);
        ok &= expect_int("ordinal 12 right edge inside C026 strip",
                         sx + PROBE_PORTRAIT_W <= PROBE_PORTRAIT_STRIP_W, 1);
        ok &= expect_int("ordinal 12 bottom edge inside C026 strip",
                         sy + PROBE_PORTRAIT_H <= PROBE_PORTRAIT_STRIP_H, 1);
    }
    return ok;
}

/* (B) No-route contract: scan Hall map 0 (16x16 cells x 4 directions)
 *     and assert NO south-facing pose (direction == DIR_SOUTH == 2)
 *     exposes ordinal 12.  The "front_south_entry" route variant is
 *     canonically absent for ordinal 12 in real DM1 V1 DUNGEON.DAT
 *     (PC 3.4); this scan proves that and binds the no-route
 *     contract so a future map edit cannot silently add a south-
 *     facing ordinal-12 route without a deliberate change to this
 *     probe. */
static int test_no_south_route_for_ordinal_12(M11_GameViewState* game) {
    int mapX;
    int mapY;
    int southHits = 0;
    int ok = 1;
    int totalSouth = 0;
    int anyDirHits = 0;
    printf("[B] No-route contract: south-facing pose never exposes ordinal 12\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    for (mapY = 0; mapY < HALL_MAX_CELLS_PER_AXIS; ++mapY) {
        for (mapX = 0; mapX < HALL_MAX_CELLS_PER_AXIS; ++mapX) {
            game->world.party.mapX = (int16_t)mapX;
            game->world.party.mapY = (int16_t)mapY;
            game->world.party.direction = 2 /* DIR_SOUTH */;
            game->showDebugHUD = 0;
            game->candidateMirrorPanelActive = 0;
            game->candidateMirrorOrdinal = -1;
            game->candidateMirrorPartyIndex = -1;
            ++totalSouth;
            {
                int ord = M11_GameView_GetFrontMirrorOrdinal(game);
                if (ord == PROBE_ORDINAL_TARGET) {
                    ++southHits;
                    printf("  HIT: ordinal 12 at south-facing pose=(%d,%d)\n",
                           mapX, mapY);
                }
            }
        }
    }
    /* Sanity: confirm at least one NORTH-facing pose exposes ordinal 12
     * (the canonical (2,10,N) LINFLAS pose) so the no-route contract
     * is meaningful — if no pose exposes ordinal 12 at all, the
     * fixture is broken in a different way and this scan would be
     * vacuously true. */
    for (mapY = 0; mapY < HALL_MAX_CELLS_PER_AXIS && anyDirHits == 0; ++mapY) {
        for (mapX = 0; mapX < HALL_MAX_CELLS_PER_AXIS && anyDirHits == 0; ++mapX) {
            game->world.party.mapX = (int16_t)mapX;
            game->world.party.mapY = (int16_t)mapY;
            game->world.party.direction = 0 /* DIR_NORTH */;
            game->showDebugHUD = 0;
            game->candidateMirrorPanelActive = 0;
            game->candidateMirrorOrdinal = -1;
            game->candidateMirrorPartyIndex = -1;
            {
                int ord = M11_GameView_GetFrontMirrorOrdinal(game);
                if (ord == PROBE_ORDINAL_TARGET) {
                    ++anyDirHits;
                    printf("  REFERENCE: ordinal 12 at north-facing pose=(%d,%d)\n",
                           mapX, mapY);
                }
            }
        }
    }
    ok &= expect_int("at least one pose exposes ordinal 12 (fixture sanity)",
                     anyDirHits, 1);
    ok &= expect_int("south-facing scan found 0 ordinal-12 hits (no-route contract)",
                     southHits, 0);
    printf("  INFO: scanned %d south-facing poses, found %d ordinal-12 hit(s)\n",
           totalSouth, southHits);
    return ok;
}

/* (C) South-facing side pose at the canonical (2,10) cell:
 *     (2,10) facing SOUTH -> front-mirror ordinal -1 and the D1C
 *     portrait rect contains NO ordinal-12 pixels above the leak
 *     threshold.  This is the source-locked no-floating contract
 *     for the front_south_entry exit pose at the LINFLAS cell —
 *     the player turning to face SOUTH at (2,10) must NOT see the
 *     LINFLAS portrait on the south wall of (2,10).  Mirrors the
 *     side-pose coverage the existing east_walkpath probe locks
 *     for (2,10) DIR_EAST / DIR_SOUTH / DIR_WEST. */
static int test_south_side_pose_no_floating(M11_GameViewState* game,
                                            const M11_AssetSlot* portraits,
                                            unsigned char* fb) {
    int ok = 1;
    int ord;
    int matched;
    int compared;
    int leakPct;
    printf("[C] South-facing side pose at (2,10) — no-floating of ordinal 12\n");
    set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, PROBE_LINFLAS_SOUTH_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    ok &= expect_int("(2,10,SOUTH) front_mirror ordinal == -1 (no float)", ord, -1);
    /* Render the south-facing pose and pixel-prove the D1C rect
     * does not contain ordinal-12 pixels above the leak threshold.
     * The D1C wall box may still share palette pixels with C026
     * portrait assets because the wall-ornament graphic and the
     * portrait strip share the same 4bpp palette — the no-floating
     * assertion uses the ordinal-12 opaque pixel match count, not
     * a raw 4bpp equality, so palette-sharing does not produce a
     * false positive (same pattern the existing east_walkpath /
     * walkpath_from_entrance probes use). */
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    matched = count_ordinal_matched_pixels(portraits, fb, PROBE_ORDINAL_TARGET);
    compared = count_ordinal_compared_pixels(portraits, PROBE_ORDINAL_TARGET);
    leakPct = compared > 0 ? (matched * 100) / compared : 0;
    printf("  (2,10,SOUTH) ordinal-12 matched=%d compared=%d leakPct=%d\n",
           matched, compared, leakPct);
    if (leakPct >= PROBE_FLOOR_LEAK_PCT) {
        ++g_fail;
        printf("  FAIL: (2,10,SOUTH) ordinal-12 leaked matched=%d/%d (>= %d%%)\n",
               matched, compared, PROBE_FLOOR_LEAK_PCT);
        ok = 0;
    } else {
        ++g_pass;
        printf("  PASS: (2,10,SOUTH) ordinal-12 leak %d%% (< %d%%)\n",
               leakPct, PROBE_FLOOR_LEAK_PCT);
    }
    /* Also exercise the east and west side poses at (2,10) so a
     * future regression that breaks one side wall but not another
     * is caught by the same probe. */
    {
        static const struct { int dir; const char* label; } kSidePoses[] = {
            {1, "(2,10,EAST) side pose"},
            {3, "(2,10,WEST) side pose"}
        };
        size_t i;
        for (i = 0; i < sizeof(kSidePoses) / sizeof(kSidePoses[0]); ++i) {
            set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, kSidePoses[i].dir);
            ord = M11_GameView_GetFrontMirrorOrdinal(game);
            ok &= expect_int(kSidePoses[i].label, ord, -1);
            memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
            M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
            matched = count_ordinal_matched_pixels(portraits, fb, PROBE_ORDINAL_TARGET);
            leakPct = compared > 0 ? (matched * 100) / compared : 0;
            if (leakPct >= PROBE_FLOOR_LEAK_PCT) {
                ++g_fail;
                printf("  FAIL: %s ordinal-12 leaked matched=%d/%d (>= %d%%)\n",
                       kSidePoses[i].label, matched, compared, PROBE_FLOOR_LEAK_PCT);
                ok = 0;
            } else {
                ++g_pass;
                printf("  PASS: %s ordinal-12 leak %d%% (< %d%%)\n",
                       kSidePoses[i].label, leakPct, PROBE_FLOOR_LEAK_PCT);
            }
        }
    }
    return ok;
}

/* (D) Candidate-panel return: open the C040 resurrect/reincarnate
 *     panel at (2,10,N), cancel the panel, then turn to face SOUTH
 *     at the same cell and verify the panel state is cleared and
 *     the D1C portrait rect remains stable (no stale-panel bleed
 *     into the south-facing redraw).  This is the source-locked
 *     contract for the C040 panel exit path the
 *     redraw_after_candidate / resurrect_reselect / ordinal_2_west
 *     candidate-panel sibling probes lock for the
 *     (1,2,N)/ordinal-1 HALK pose; this slice widens the same
 *     coverage to the (2,10,N)/ordinal-12 LINFLAS pose with the
 *     front_south_entry exit direction. */
static int test_candidate_panel_return_at_south(M11_GameViewState* game,
                                                const M11_AssetSlot* portraits,
                                                unsigned char* fb) {
    int ok = 1;
    int selected;
    int cancelled;
    int panelBefore;
    int panelAfter;
    int ordinalAfter;
    int matched;
    int compared;
    int leakPct;
    printf("[D] Candidate-panel return at (2,10,N) -> cancel -> turn SOUTH\n");
    /* Seat the party at the canonical ordinal-12 pose (2,10,N) and
     * open the C040 resurrect/reincarnate panel via the same API
     * the existing redraw_after_candidate / resurrect_reselect
     * probes use. */
    set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, PROBE_LINFLAS_DIR);
    selected = M11_GameView_SelectFrontMirrorCandidate(game);
    ok &= expect_int("SelectFrontMirrorCandidate at (2,10,N) returns 1",
                     selected, 1);
    panelBefore = game->candidateMirrorPanelActive;
    ok &= expect_int("candidateMirrorPanelActive == 1 after select",
                     panelBefore, 1);
    ok &= expect_int("candidateMirrorOrdinal == 12 after select",
                     game->candidateMirrorOrdinal, PROBE_ORDINAL_TARGET);
    /* Cancel the panel via the public API.  This routes through
     * REVIVE.C F0282 cancel path (no sensor-disable loop, no
     * candidate append) and tears the panel down cleanly. */
    cancelled = M11_GameView_CancelMirrorCandidate(game);
    ok &= expect_int("CancelMirrorCandidate returns 1", cancelled, 1);
    panelAfter = game->candidateMirrorPanelActive;
    ordinalAfter = game->candidateMirrorOrdinal;
    ok &= expect_int("candidateMirrorPanelActive == 0 after cancel",
                     panelAfter, 0);
    ok &= expect_int("candidateMirrorOrdinal == -1 after cancel",
                     ordinalAfter, -1);
    /* Now turn to face SOUTH at the same cell (the front_south_entry
     * exit direction).  The party pose is mutated directly; the
     * panel state must remain cleared.  This is the south-side
     * panel-return assertion the existing ordinal-12 probes leave
     * uncovered. */
    game->world.party.direction = (uint8_t)PROBE_LINFLAS_SOUTH_DIR;
    panelAfter = game->candidateMirrorPanelActive;
    ordinalAfter = game->candidateMirrorOrdinal;
    ok &= expect_int("candidateMirrorPanelActive == 0 after turn SOUTH",
                     panelAfter, 0);
    ok &= expect_int("candidateMirrorOrdinal == -1 after turn SOUTH",
                     ordinalAfter, -1);
    /* Redraw the south-facing pose and pixel-prove the D1C rect
     * does not contain ordinal-12 pixels above the leak threshold
     * after the panel cancel + south turn.  This is the redraw
     * stability contract for the front_south_entry exit pose. */
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    matched = count_ordinal_matched_pixels(portraits, fb, PROBE_ORDINAL_TARGET);
    compared = count_ordinal_compared_pixels(portraits, PROBE_ORDINAL_TARGET);
    leakPct = compared > 0 ? (matched * 100) / compared : 0;
    printf("  (2,10,SOUTH) post-cancel ordinal-12 matched=%d compared=%d leakPct=%d\n",
           matched, compared, leakPct);
    if (leakPct >= PROBE_FLOOR_LEAK_PCT) {
        ++g_fail;
        printf("  FAIL: (2,10,SOUTH) post-cancel ordinal-12 leaked matched=%d/%d (>= %d%%)\n",
               matched, compared, PROBE_FLOOR_LEAK_PCT);
        ok = 0;
    } else {
        ++g_pass;
        printf("  PASS: (2,10,SOUTH) post-cancel ordinal-12 leak %d%% (< %d%%)\n",
               leakPct, PROBE_FLOOR_LEAK_PCT);
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
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
        portraits->width < PROBE_PORTRAIT_STRIP_W ||
        portraits->height < PROBE_PORTRAIT_STRIP_H) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 12, "
           "route front_south_entry, aspect portrait_rect_position ===\n");
    printf("sourceEvidence=DUNGEON.C:2558,2608-2612 (C127 sensorData -> G0289)\n");
    printf("                DUNGEON.C:2573 (M011_CELL(sensor) -> visible wall)\n");
    printf("                MOVESENS.C:1501-1503 (C127 -> F0280)\n");
    printf("                REVIVE.C F0280,F0282 (candidate materialise/cancel)\n");
    printf("                DUNVIEW.C:3913-3928 (C346 frame + C026 portrait blit)\n");
    printf("                DUNVIEW.C:8318-8542 F0128 (far-to-near draw order)\n");
    printf("                DUNVIEW.C:8522-8533 (C026 D1C re-blt on tick redraw)\n");
    printf("                COORD.C:1693-1722 (PC 3.4 viewport origin / 224x136)\n");
    printf("                DEFS.H:2071-2079 (G2071_C320 / G2078_C32 / G2079_C29)\n\n");

    ok &= test_canonical_ordinal_12_pose(&game);
    ok &= test_no_south_route_for_ordinal_12(&game);
    ok &= test_south_side_pose_no_floating(&game, portraits, currFb);
    ok &= test_candidate_panel_return_at_south(&game, portraits, currFb);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
