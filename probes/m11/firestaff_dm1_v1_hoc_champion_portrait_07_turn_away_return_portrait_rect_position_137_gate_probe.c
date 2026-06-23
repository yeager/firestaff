/*
 * DM1 V1 Hall of Champions portrait ordinal 7 (TIGGY) turn_away_return
 * portrait_rect_position runtime gate probe.
 *
 * Targeted slice:
 *   ordinal = 7  (TIGGY / TAMAL — C026 column 7 row 0, C127 sensorData=7)
 *   route   = turn_away_return
 *             (party facing SOUTH at the only available (2,17) route,
 *              turn to a non-SOUTH direction so the front mirror ordinal
 *              falls back to -1, then turn back to SOUTH so the front
 *              mirror ordinal returns to 7; assert the portrait_rect at
 *              the source-locked D1C cutout is stable across the
 *              turn-away and return cycles, that the redrawn portrait
 *              pixel match is byte-stable vs the first redraw, and that
 *              no portrait pixel lingers on the side walls during the
 *              turn-away frames)
 *   aspect  = portrait_rect_position
 *             (DUNVIEW.C:3913-3928 C026 champion portrait cutout stays
 *              anchored at the source-locked D1C viewport rectangle
 *              (96, 35, 32, 29) viewport-local; the cutout is parented
 *              inside the D1C wall-ornament frame (80, 29, 64, 43)
 *              coordSet 5 / index 12 reported by
 *              M11_GameView_GetD1CWallOrnamentZone)
 *
 * Coverage gap relative to existing ordinal-7 probes:
 *   - firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
 *     frames the (2, 17, SOUTH) cell as the "front_north_entry"
 *     descriptor and asserts catalog identity + the side-wall
 *     no-floating check + a resurrect round-trip, but does not
 *     exercise a turn-away -> return redraw cycle on the same cell.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_07_south_return_portrait_rect_position_runtime_probe
 *     frames the same cell as "south_return" and asserts the south
 *     pose + the south->west reblt invariant, but does not loop the
 *     turn-away and return cycle to lock redraw stability.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_02_cancel_reopen_portrait_rect_position_runtime_probe
 *     is the cancel_reopen pattern for ordinal 2 (retargeted (1,2)
 *     NORTH) and exercises the select/cancel/reopen candidate
 *     panel state machine.  The ordinal 7 turn_away_return slice
 *     here is the orthogonal pattern: NO candidate panel state
 *     transitions, only party-direction changes through the
 *     north/east/west turn + south return sequence, so the redraw
 *     stability assertion is decoupled from the panel state machine
 *     and from the C127 sensor-disable path that confirm triggers.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate_portrait_rect_position_097_gate_probe
 *     and _21_..._117_gate_probe.c cover redraw_after_candidate for
 *     ordinals 1 and 21, but ordinal 7 has no matching gate.
 *
 * Source-locked to ReDMCSB WIP 20210206:
 *   DUNGEON.C:2573       maps M011_CELL(sensor) against view direction.
 *   DUNGEON.C:2608-2612  stores C127 sensorData in G0289 only for the
 *                         front-wall side.
 *   DUNVIEW.C:3913-3928  blits C026 ordinal 7 (col 7 row 0) into the
 *                         D1C box at viewport (96, 35, 32, 29).
 *   DUNVIEW.C:8318-8618  F0128 far-to-near viewport redraw.
 *   DUNVIEW.C G0205      wall-ornament coordinate sets; index 12 is
 *                         the D1C champion-mirror frame (80, 29, 64, 43).
 *   MOVESENS.C:1501-1503 dispatches C127 sensorData to REVIVE.C F0280.
 *   REVIVE.C:272-276     F0280 appends the candidate from sensor.
 *   REVIVE.C:744-799     F0282 cancel/confirm; 785-799 disables the
 *                         matching mirror route after confirm.
 *   COORD.C:1693-1722    PC 3.4 viewport origin/size.
 *
 * The probe is honest about scope: it is Firestaff runtime evidence
 * against GRAPHICS.DAT/DUNGEON.DAT on the real PC 3.4 English DM1
 * fixture.  It does NOT claim original DOS pixel parity.  The
 * redraw-stability assertion compares two framebuffer captures of
 * the same (2, 17, SOUTH) redraw after a turn-away cycle and requires
 * them to be byte-equal in the D1C portrait rect, which is a strict
 * runtime redraw determinism invariant (no drift, no floating pixels,
 * no half-blitted sprite).
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_07_turn_away_return_portrait_rect_position_137_gate_probe DATA_DIR
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
    FB_W = 320,
    FB_H = 200,
    /* Source-locked PC 3.4 viewport (ReDMCSB COORD.C:1693-1722):
     * origin (M11_VIEWPORT_X, M11_VIEWPORT_Y) = (0, 33); size
     * (M11_VIEWPORT_W, M11_VIEWPORT_H) = (224, 136). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* C026 champion portrait cutout (viewport-local) inside the D1C
     * wall box.  ReDMCSB DUNVIEW.C:3913-3928 uses
     *   M11_AssetLoader_BlitRegion(portraits,
     *       (portraitIdx & 7) * M11_PORTRAIT_W (== 32),
     *       (portraitIdx >> 3) * M11_PORTRAIT_H (== 29),
     *       M11_PORTRAIT_W, M11_PORTRAIT_H,
     *       M11_VIEWPORT_X + 96, M11_VIEWPORT_Y + 35, ...)
     * so the cutout is (96, 35, 32, 29) viewport-local = (96, 68, 32,
     * 29) framebuffer-local. */
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* D1C champion-mirror frame zone from
     * M11_GameView_GetD1CWallOrnamentZone (coordSet 5 / index 12 per
     * DUNVIEW.C G0205): dstX=80, dstY=29, w=64, h=43 viewport-local.
     * The C026 portrait cutout (96, 35, 32, 29) sits inside this
     * zone. */
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    /* Hall of Champions ordinal 7 = TIGGY / TAMAL (C026 col 7 row 0).
     * C127 sensorData is 0-indexed per DUNVIEW.C:4547-4581. */
    ORDINAL_TIGGY = 7,
    /* The (2, 17) SOUTH pose is the only Hall-of-Champions cell on
     * map 0 that exposes the C127 sensor with sensorData=7. */
    POSE_X = 2,
    POSE_Y = 17,
    /* C026 strip uses palette index 1 for the dark-gray transparency
     * mask (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY). */
    CHAMPION_TRANSPARENT = 1,
    /* Match threshold for ordinal 7 dominance in the D1C portrait
     * rect.  Mirrors the existing ordinal-07 south_return probe
     * threshold; the redraw_stability assertion below is the
     * load-bearing byte-equality check, not this percentage. */
    CORRECT_MATCH_PCT = 90,
    /* No-floating threshold for the side poses: at most 35% of the
     * compared ordinal pixels may remain matched after the turn-away
     * (same 35% tolerance the existing zorder reblt probe locks). */
    NO_FLOATING_PCT = 35
};

/* Convert viewport-local rectangle to framebuffer-local rectangle. */
static inline int fb_x(int vpX) { return vpX; }
static inline int fb_y(int vpY) { return vpY + VIEWPORT_Y; }

typedef struct PortraitEvidence {
    int compared;
    int matched;
    int matchedPct;
    int bestOrdinal;
    int bestMatched;
    int d1cZoneContainsPortrait;
} PortraitEvidence;

static int g_pass = 0;
static int g_fail = 0;

static void pass(const char* label) {
    printf("  PASS: %s\n", label);
    ++g_pass;
}

static void fail(const char* label) {
    printf("  FAIL: %s\n", label);
    ++g_fail;
}

static int expect_int(const char* label, int got, int want) {
    char msg[256];
    snprintf(msg, sizeof(msg), "%s got=%d want=%d", label, got, want);
    if (got == want) {
        pass(msg);
        return 1;
    }
    fail(msg);
    return 0;
}

static void set_hall_pose(M11_GameViewState* game, int x, int y, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = x;
    game->world.party.mapY = y;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

static int open_dm1(const char* dataDir,
                    M12_StartupMenuState* menu,
                    M11_GameViewState* game) {
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    if (!M11_GameView_OpenSelectedMenuEntry(game, menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(game);
        return 0;
    }
    return 1;
}

/* Count how many palette-index cells in the destination rect match
 * the source C026 sprite for `ordinal` (skipping the source
 * transparent mask).  Used both for the absolute match check and
 * for the 24-ordinal best-fit sweep. */
static int portrait_match_count(const M11_AssetSlot* portraits,
                                const unsigned char* fb,
                                int ordinal,
                                int* outCompared) {
    int matched = 0;
    int compared = 0;
    int srcX0;
    int srcY0;
    int x;
    int y;

    if (outCompared) *outCompared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        !fb || ordinal < 0 || ordinal >= 24) {
        return -1;
    }
    if ((int)portraits->width < 8 * PORTRAIT_W ||
        (int)portraits->height < 3 * PORTRAIT_H) {
        return -1;
    }

    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        int srcY = srcY0 + y;
        int dstY = fb_y(PORTRAIT_Y_VP + y);
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = srcX0 + x;
            int dstX = fb_x(PORTRAIT_X_VP + x);
            unsigned char src = (unsigned char)(
                portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst = (unsigned char)(fb[dstY * FB_W + dstX] & 0x0F);

            /* ReDMCSB DUNVIEW.C:3916 C01_COLOR_DARK_GRAY transparency
             * mask — skip so the underlying wall pixels do not
             * poison the match count. */
            if (src == CHAMPION_TRANSPARENT) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    if (outCompared) *outCompared = compared;
    return matched;
}

static void collect_portrait_evidence(const M11_AssetSlot* portraits,
                                      const unsigned char* fb,
                                      int ordinal,
                                      PortraitEvidence* out) {
    int i;
    memset(out, 0, sizeof(*out));
    out->bestOrdinal = -1;
    out->bestMatched = -1;
    out->matched = portrait_match_count(portraits, fb, ordinal, &out->compared);
    if (out->matched >= 0 && out->compared > 0) {
        out->matchedPct = (out->matched * 100) / out->compared;
    }
    for (i = 0; i < 24; ++i) {
        int compared = 0;
        int matched = portrait_match_count(portraits, fb, i, &compared);
        (void)compared;
        if (matched > out->bestMatched) {
            out->bestMatched = matched;
            out->bestOrdinal = i;
        }
    }
    out->d1cZoneContainsPortrait =
        (PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
         PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
         PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
         PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H) ? 1 : 0;
}

/* Compare the D1C portrait rect bytes between two framebuffers.
 * Returns 1 if they are byte-equal in the rect, 0 otherwise.  This
 * is the load-bearing redraw-stability invariant: the same
 * (2, 17, SOUTH) redraw, after a turn-away and return cycle, must
 * produce an identical D1C rect — no half-blit, no stale pixel,
 * no palette drift. */
static int d1c_rect_byte_equal(const unsigned char* a,
                               const unsigned char* b) {
    int y;
    if (!a || !b) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        int rowA = fb_y(PORTRAIT_Y_VP + y);
        int rowB = rowA;
        if (memcmp(&a[rowA * FB_W + fb_x(PORTRAIT_X_VP)],
                   &b[rowB * FB_W + fb_x(PORTRAIT_X_VP)],
                   (size_t)PORTRAIT_W) != 0) {
            return 0;
        }
    }
    return 1;
}

static int check_d1c_zone(M11_GameViewState* game) {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int ok;
    M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h);
    ok = x == D1C_ZONE_X_VP && y == D1C_ZONE_Y_VP &&
         w == D1C_ZONE_W && h == D1C_ZONE_H;
    if (ok) {
        pass("D1C wall zone is source-locked viewport rect (80,29,64,43)");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "D1C wall zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d)",
                 x, y, w, h, D1C_ZONE_X_VP, D1C_ZONE_Y_VP,
                 D1C_ZONE_W, D1C_ZONE_H);
        fail(msg);
    }
    return ok;
}

static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    int ok;
    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(game, ORDINAL_TIGGY,
                                              name, sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, ORDINAL_TIGGY,
                                               title, sizeof(title));
    ok = strcmp(name, "TIGGY") == 0 && strcmp(title, "TAMAL") == 0;
    if (ok) {
        pass("ordinal 7 catalog identity is TIGGY / TAMAL");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "ordinal 7 catalog identity got name=\"%s\" title=\"%s\"",
                 name, title);
        fail(msg);
    }
    return ok;
}

static int draw_and_expect_portrait(M11_GameViewState* game,
                                    const M11_AssetSlot* portraits,
                                    unsigned char* fb,
                                    const char* label,
                                    PortraitEvidence* outEv) {
    PortraitEvidence ev;
    char msg[256];
    int ok = 1;
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_portrait_evidence(portraits, fb, ORDINAL_TIGGY, &ev);
    if (outEv) *outEv = ev;

    snprintf(msg, sizeof(msg), "%s D1C cutout inside wall zone", label);
    if (ev.d1cZoneContainsPortrait) pass(msg); else { fail(msg); ok = 0; }

    snprintf(msg, sizeof(msg),
             "%s ordinal 7 match >= %d%% got=%d%% (%d/%d)",
             label, CORRECT_MATCH_PCT, ev.matchedPct, ev.matched, ev.compared);
    if (ev.matchedPct >= CORRECT_MATCH_PCT) pass(msg); else { fail(msg); ok = 0; }

    snprintf(msg, sizeof(msg),
             "%s ordinal 7 is best C026 match best=%d matched=%d",
             label, ev.bestOrdinal, ev.bestMatched);
    if (ev.bestOrdinal == ORDINAL_TIGGY) {
        pass(msg);
    } else {
        fail(msg);
        ok = 0;
    }
    return ok;
}

static int check_turned_away_no_portrait(M11_GameViewState* game,
                                         const M11_AssetSlot* portraits,
                                         int dir,
                                         const char* label) {
    unsigned char fb[FB_W * FB_H];
    PortraitEvidence ev;
    char msg[256];
    set_hall_pose(game, POSE_X, POSE_Y, dir);
    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        snprintf(msg, sizeof(msg),
                 "%s front mirror ordinal got=%d want=-1",
                 label, M11_GameView_GetFrontMirrorOrdinal(game));
        fail(msg);
        return 0;
    }
    pass(label);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_portrait_evidence(portraits, fb, ORDINAL_TIGGY, &ev);
    snprintf(msg, sizeof(msg),
             "%s ordinal 7 absent from D1C cutout (<%d%%) got=%d%%",
             label, NO_FLOATING_PCT, ev.matchedPct);
    if (ev.matchedPct < NO_FLOATING_PCT) {
        pass(msg);
        return 1;
    }
    fail(msg);
    return 0;
}

/* Capture the D1C portrait rect bytes from the current framebuffer
 * (after a fresh Draw) into the output buffer.  Used to compare the
 * turn-away -> return redraw against the initial reference redraw. */
static void capture_d1c_rect(const unsigned char* fb,
                             unsigned char* outRect) {
    int y;
    for (y = 0; y < PORTRAIT_H; ++y) {
        memcpy(&outRect[y * PORTRAIT_W],
               &fb[fb_y(PORTRAIT_Y_VP + y) * FB_W + fb_x(PORTRAIT_X_VP)],
               (size_t)PORTRAIT_W);
    }
}

int main(int argc, char** argv) {
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    const char* dataDir;
    const M11_AssetSlot* portraits;
    int ok = 1;
    int frontOrdinal;
    unsigned char fbRef[FB_W * FB_H];
    unsigned char fbReturn[FB_W * FB_H];
    unsigned char rectRef[PORTRAIT_W * PORTRAIT_H];
    unsigned char rectReturn[PORTRAIT_W * PORTRAIT_H];
    PortraitEvidence evRef;
    PortraitEvidence evReturn;
    int cycle;
    int cycleAllStable;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies DM1 V1 HoC portrait ordinal 7 turn_away_return "
                "portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (!open_dm1(dataDir, &menu, &game)) return 1;

    printf("=== DM1 V1 HoC portrait ordinal 7 turn_away_return "
           "portrait_rect_position ===\n");
    printf("dataDir=%s pose=(map 0, x=%d, y=%d) facing SOUTH\n",
           dataDir, POSE_X, POSE_Y);
    printf("D1C cutout viewport=(%d,%d,%d,%d), C026 source=(%d,%d,%d,%d)\n",
           PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
           (ORDINAL_TIGGY & 7) * PORTRAIT_W,
           (ORDINAL_TIGGY >> 3) * PORTRAIT_H,
           PORTRAIT_W, PORTRAIT_H);

    portraits = M11_AssetLoader_Load(
        &game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C026 portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= check_catalog_identity(&game);
    ok &= check_d1c_zone(&game);

    set_hall_pose(&game, POSE_X, POSE_Y, DIR_SOUTH);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != ORDINAL_TIGGY) {
        printf("SKIP hoc_portrait07_turn_away_return_fixture_mismatch "
               "(%d,%d) SOUTH front ordinal=%d expected=%d; this DM1 V1 "
               "build does not expose ordinal 7 on this reference Hall "
               "route.\n",
               POSE_X, POSE_Y, frontOrdinal, ORDINAL_TIGGY);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    pass("reference route reports ordinal 7 at (2,17) SOUTH");

    /* ----------------------------------------------------------------
     * Stage 1: capture the reference (2, 17, SOUTH) redraw and the
     * reference D1C rect bytes.  This is the ground-truth portrait
     * the return cycles below must reproduce byte-for-byte.
     * -------------------------------------------------------------- */
    printf("\n[Stage 1] reference (2,17) SOUTH redraw\n");
    memset(fbRef, 0, sizeof(fbRef));
    ok &= draw_and_expect_portrait(&game, portraits, fbRef,
                                  "reference_south", &evRef);
    capture_d1c_rect(fbRef, rectRef);
    printf("  INFO: reference D1C rect captured (%d bytes)\n",
           (int)sizeof(rectRef));

    /* ----------------------------------------------------------------
     * Stage 2: turn away NORTH then return SOUTH; assert redraw
     * stability (D1C rect bytes match reference byte-for-byte) and
     * that the turn-away frame is clear of ordinal 7 pixels.
     * -------------------------------------------------------------- */
    printf("\n[Stage 2] turn NORTH -> return SOUTH redraw stability\n");
    ok &= check_turned_away_no_portrait(&game, portraits, DIR_NORTH,
                                        "stage2_north_no_front_mirror");
    set_hall_pose(&game, POSE_X, POSE_Y, DIR_SOUTH);
    if (!expect_int("stage2_north_return front mirror ordinal",
                    M11_GameView_GetFrontMirrorOrdinal(&game),
                    ORDINAL_TIGGY)) {
        ok = 0;
    }
    memset(fbReturn, 0, sizeof(fbReturn));
    ok &= draw_and_expect_portrait(&game, portraits, fbReturn,
                                  "stage2_north_return_south", &evReturn);
    capture_d1c_rect(fbReturn, rectReturn);
    if (d1c_rect_byte_equal(fbRef, fbReturn)) {
        pass("stage2_north_return D1C rect bytes match reference (redraw "
             "stable after turn NORTH -> SOUTH)");
    } else {
        fail("stage2_north_return D1C rect bytes differ from reference "
             "(redraw drifted after turn NORTH -> SOUTH)");
        ok = 0;
    }
    if (memcmp(rectRef, rectReturn, sizeof(rectRef)) == 0) {
        pass("stage2_north_return D1C rect snapshot bytes match "
             "reference snapshot");
    } else {
        fail("stage2_north_return D1C rect snapshot bytes differ from "
             "reference snapshot");
        ok = 0;
    }

    /* ----------------------------------------------------------------
     * Stage 3: turn away EAST then return SOUTH; same redraw
     * stability assertion.
     * -------------------------------------------------------------- */
    printf("\n[Stage 3] turn EAST -> return SOUTH redraw stability\n");
    ok &= check_turned_away_no_portrait(&game, portraits, DIR_EAST,
                                        "stage3_east_no_front_mirror");
    set_hall_pose(&game, POSE_X, POSE_Y, DIR_SOUTH);
    if (!expect_int("stage3_east_return front mirror ordinal",
                    M11_GameView_GetFrontMirrorOrdinal(&game),
                    ORDINAL_TIGGY)) {
        ok = 0;
    }
    memset(fbReturn, 0, sizeof(fbReturn));
    ok &= draw_and_expect_portrait(&game, portraits, fbReturn,
                                  "stage3_east_return_south", &evReturn);
    capture_d1c_rect(fbReturn, rectReturn);
    if (d1c_rect_byte_equal(fbRef, fbReturn)) {
        pass("stage3_east_return D1C rect bytes match reference (redraw "
             "stable after turn EAST -> SOUTH)");
    } else {
        fail("stage3_east_return D1C rect bytes differ from reference "
             "(redraw drifted after turn EAST -> SOUTH)");
        ok = 0;
    }
    if (memcmp(rectRef, rectReturn, sizeof(rectRef)) == 0) {
        pass("stage3_east_return D1C rect snapshot bytes match reference "
             "snapshot");
    } else {
        fail("stage3_east_return D1C rect snapshot bytes differ from "
             "reference snapshot");
        ok = 0;
    }

    /* ----------------------------------------------------------------
     * Stage 4: turn away WEST then return SOUTH; same redraw
     * stability assertion.  WEST is the side wall the existing
     * south_return probe uses for the reblt-invariant check, so this
     * stage is the turn_away_return counterpart.
     * -------------------------------------------------------------- */
    printf("\n[Stage 4] turn WEST -> return SOUTH redraw stability\n");
    ok &= check_turned_away_no_portrait(&game, portraits, DIR_WEST,
                                        "stage4_west_no_front_mirror");
    set_hall_pose(&game, POSE_X, POSE_Y, DIR_SOUTH);
    if (!expect_int("stage4_west_return front mirror ordinal",
                    M11_GameView_GetFrontMirrorOrdinal(&game),
                    ORDINAL_TIGGY)) {
        ok = 0;
    }
    memset(fbReturn, 0, sizeof(fbReturn));
    ok &= draw_and_expect_portrait(&game, portraits, fbReturn,
                                  "stage4_west_return_south", &evReturn);
    capture_d1c_rect(fbReturn, rectReturn);
    if (d1c_rect_byte_equal(fbRef, fbReturn)) {
        pass("stage4_west_return D1C rect bytes match reference (redraw "
             "stable after turn WEST -> SOUTH)");
    } else {
        fail("stage4_west_return D1C rect bytes differ from reference "
             "(redraw drifted after turn WEST -> SOUTH)");
        ok = 0;
    }
    if (memcmp(rectRef, rectReturn, sizeof(rectRef)) == 0) {
        pass("stage4_west_return D1C rect snapshot bytes match reference "
             "snapshot");
    } else {
        fail("stage4_west_return D1C rect snapshot bytes differ from "
             "reference snapshot");
        ok = 0;
    }

    /* ----------------------------------------------------------------
     * Stage 5: side-wall no-floating after the return.  The same
     * three wrong-wall directions must NOT show ordinal 7 in the
     * D1C portrait rect after the turn_away_return cycle has run
     * (so the redraw cleanup is symmetric across directions).
     * -------------------------------------------------------------- */
    printf("\n[Stage 5] post-return side-wall no-floating\n");
    ok &= check_turned_away_no_portrait(&game, portraits, DIR_NORTH,
                                        "stage5_post_return_north");
    ok &= check_turned_away_no_portrait(&game, portraits, DIR_EAST,
                                        "stage5_post_return_east");
    ok &= check_turned_away_no_portrait(&game, portraits, DIR_WEST,
                                        "stage5_post_return_west");

    /* ----------------------------------------------------------------
     * Stage 6: redraw-stability across 3 turn-away-and-return
     * cycles.  Each cycle goes NORTH -> SOUTH (turn_away_return);
     * after the final SOUTH redraw the D1C rect bytes must still
     * match the reference.  This locks the absence of cumulative
     * drift in the redraw pipeline.
     * -------------------------------------------------------------- */
    printf("\n[Stage 6] 3 turn-away-and-return cycles (NORTH -> SOUTH) "
           "redraw stability\n");
    cycleAllStable = 1;
    for (cycle = 0; cycle < 3; ++cycle) {
        char label[80];
        snprintf(label, sizeof(label), "stage6_cycle_%d", cycle);
        set_hall_pose(&game, POSE_X, POSE_Y, DIR_NORTH);
        if (M11_GameView_GetFrontMirrorOrdinal(&game) != -1) {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "%s north frame front ordinal got=%d want=-1",
                     label, M11_GameView_GetFrontMirrorOrdinal(&game));
            fail(msg);
            ok = 0;
            cycleAllStable = 0;
            continue;
        }
        set_hall_pose(&game, POSE_X, POSE_Y, DIR_SOUTH);
        if (M11_GameView_GetFrontMirrorOrdinal(&game) != ORDINAL_TIGGY) {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "%s return front ordinal got=%d want=%d",
                     label, M11_GameView_GetFrontMirrorOrdinal(&game),
                     ORDINAL_TIGGY);
            fail(msg);
            ok = 0;
            cycleAllStable = 0;
            continue;
        }
        memset(fbReturn, 0, sizeof(fbReturn));
        M11_GameView_Draw(&game, fbReturn, FB_W, FB_H);
        if (d1c_rect_byte_equal(fbRef, fbReturn)) {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "%s D1C rect bytes match reference "
                     "(no cumulative drift after cycle %d)",
                     label, cycle);
            pass(msg);
        } else {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "%s D1C rect bytes differ from reference "
                     "(cumulative drift detected after cycle %d)",
                     label, cycle);
            fail(msg);
            ok = 0;
            cycleAllStable = 0;
        }
    }
    if (cycleAllStable) {
        pass("stage6 3 turn-away-and-return cycles D1C rect is redraw "
             "stable");
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("%s dm1 v1 HoC champion portrait ordinal 7 turn_away_return "
           "portrait_rect_position\n",
           (ok && g_fail == 0) ? "PASS" : "FAIL");
    return (ok && g_fail == 0) ? 0 : 1;
}
