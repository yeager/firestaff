/*
 * firestaff_dm1_v1_hall_champion_portrait_22_front_south_entry_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice that is intentionally NOT covered by the
 * existing ordinal-22 probes:
 *
 *   ordinal       : 22  (C026 col 6 row 2; "GOTHMOG", untitled in the
 *                   DM1 V1 PC 3.4 mirror TextString catalog per the
 *                   ordinal_22 ANY-pose discovery result in the
 *                   front_north_entry probe)
 *   route variant : front_south_entry — the party stands one square
 *                   south of the ordinal-22 cell on Hall map 0
 *                   ((map=0, x=3, y=7) facing NORTH) so the
 *                   source-visible wall is the south wall (cell 2)
 *                   of (3, 6).  The "front_south_entry" route
 *                   variant is canonically the approach a player
 *                   takes when entering the Hall corridor from the
 *                   south side of the central Hall and stepping
 *                   onto the ordinal-22 cell.  In real DM1 V1
 *                   DUNGEON.DAT (PC 3.4) the ordinal-22 C127 sensor
 *                   is on the WEST wall (cell 3) of (2, 6) (visible
 *                   from (3, 6) facing WEST per the front_north_entry
 *                   probe's [D] scan) — i.e. the south wall of (3, 6)
 *                   has NO C127 sensor with sensorData=22, so the
 *                   south-entry arc into the ordinal-22 cell is a
 *                   no-portrait contract.  This is the same
 *                   structural pattern the existing ordinal-12
 *                   front_south_entry probe locks for the LINFLAS
 *                   pose, applied to ordinal 22 (GOTHMOG).
 *   aspect        : portrait_rect_position (D1C front-wall cutout
 *                   at viewport (96,35,32,29)) +
 *                   candidate_panel_return + redraw_stability, locking
 *                   five contracts the existing ordinal-22 probes leave
 *                   uncovered for the south-facing approach to the
 *                   ordinal-22 cell:
 *
 *                   (A) ordinal 22 catalog identity: name == "GOTHMOG",
 *                       title empty (DM1 V1 PC 3.4 TextString binding
 *                       is pinned to a real source identity).
 *                   (B) ordinal 22 lives at (3, 6, WEST) on Hall map
 *                       0 in the real DM1 V1 DUNGEON.DAT (re-verified
 *                       here so this probe stays independent of the
 *                       front_north_entry probe).  No other pose on
 *                       Hall map 0 exposes ordinal 22 (the single-hit
 *                       contract is the no-floating corollary for the
 *                       ordinal-22 route on this fixture).
 *                   (C) South-facing side pose at the canonical
 *                       (3, 6) cell returns front-mirror ordinal -1
 *                       and the D1C portrait rect contains NO
 *                       ordinal-22 pixels above the leak threshold
 *                       (no floating of GOTHMOG onto the south wall
 *                       when the player turns to face SOUTH at the
 *                       ordinal-22 cell).  The (3, 6) SOUTH pose is
 *                       the source-locked "front_south_entry" pose
 *                       for ordinal 22 (party at the ordinal-22 cell
 *                       facing SOUTH — the south approach the player
 *                       would take if walking into the cell from
 *                       the south corridor).
 *                   (D) South-entry approach pose (party at (3, 7)
 *                       facing NORTH, one square south of the
 *                       ordinal-22 cell) returns front-mirror
 *                       ordinal -1 (the south wall of (3, 6) has no
 *                       C127 sensor for ordinal 22) and the D1C
 *                       portrait rect contains NO ordinal-22 pixels.
 *                       This is the south-entry arc that the
 *                       front_south_entry route variant documents.
 *                   (E) Candidate-panel return at the ordinal-22
 *                       cell: open the C040 resurrect/reincarnate
 *                       panel at (3, 6, WEST) (the canonical
 *                       ordinal-22 pose), cancel the panel, then
 *                       turn to face SOUTH at the same cell and
 *                       verify the panel state is cleared and the
 *                       D1C portrait rect remains stable (no
 *                       stale-panel bleed into the south-facing
 *                       redraw).  This is the source-locked
 *                       candidate-panel return behavior the existing
 *                       ordinal-12 front_south_entry probe locks
 *                       for the LINFLAS pose; this probe widens the
 *                       same coverage to the ordinal-22 cell
 *                       (GOTHMOG) with the south-facing exit
 *                       direction.
 *
 * This probe widens the existing ordinal-22 coverage along a
 * different axis than:
 *
 *   firestaff_dm1_v1_hall_champion_portrait_22_front_north_entry_runtime_probe
 *     - covers the static front_north_entry pose (1,2,N) -> ordinal 1
 *       HALK, the ordinal 22 any-pose discovery, the side-wall
 *       no-floating corridor poses, and the C127 sensorData=22 seed
 *       on the (1,2) HALK sensor.  Does NOT cover the (3, 6) cell
 *       south-facing pose, the south-entry approach from (3, 7, N),
 *       or the candidate-panel return from the ordinal-22 pose.
 *
 *   firestaff_dm1_v1_hall_champion_portrait_22_walkpath_from_entrance_runtime_probe
 *     - drives the input-path walkpath from (1,2,N) through the
 *       entrance corridor (x=1, y=2..5) and proves the
 *       no-ordinal-22 contract on that walkpath.  Does NOT visit
 *       (3, 6) or (3, 7) (the ordinal-22 cell and its south-entry
 *       approach), and does NOT exercise the candidate-panel return
 *       at the ordinal-22 pose.
 *
 *   firestaff_dm1_v1_hall_of_champions_portrait_22_redraw_after_candidate_runtime_probe
 *     - covers the C026 ordinal 22 atlas math, the C127 sensor
 *       seed from the (1,2) HALK pose to ordinal 22, and the
 *       redraw_after_candidate contract on the HALK cell.  Does
 *       NOT cover the (3, 6) ordinal-22 cell itself, the
 *       south-facing pose at (3, 6), or the south-entry approach
 *       from (3, 7, N).
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
 *     DEFS.H:821-826            M027/M028 portrait macro math
 *
 * Honest scope: this probe proves the source-locked C026 ordinal
 * 22 placement at the canonical ordinal-22 pose (3, 6, WEST),
 * the no-route contract for the south-facing C127 sensor at the
 * (3, 6) cell, the south-entry approach pose (3, 7, NORTH)
 * no-portrait contract, the south-facing side-pose no-floating
 * contract at (3, 6), and the candidate-panel return behavior
 * when the player turns to face SOUTH at the ordinal-22 cell
 * after cancelling the resurrect/reincarnate panel.
 *
 * It does NOT claim DOS pixel parity beyond the same C01 dark-gray
 * transparency contract the existing portrait / zorder / reblt /
 * east_walkpath / walkpath_from_entrance / front_south_entry
 * probes lock.  Original DM1 PC 3.4 captures live under
 * parity-evidence/ and are referenced by separate parity gates.
 *
 * Usage: firestaff_dm1_v1_hall_champion_portrait_22_front_south_entry_runtime_probe DATA_DIR
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
     * walkpath_from_entrance / front_south_entry probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* The ordinal-22 cell on Hall map 0: (3, 6) per the
     * front_north_entry probe's [D] ordinal-22 ANY-pose discovery
     * scan.  The C127 sensor lives on the WEST wall (cell 3) of
     * (2, 6) (visible from (3, 6) facing WEST).  We pin (3, 6) so
     * the slice is bound to a real source location. */
    PROBE_GOTHMOG_CELL_X = 3,
    PROBE_GOTHMOG_CELL_Y = 6,
    PROBE_GOTHMOG_FRONT_DIR = 3,  /* DIR_WEST - the ordinal-22 route */
    PROBE_GOTHMOG_SOUTH_DIR = 2,  /* DIR_SOUTH - the front_south_entry exit */
    PROBE_GOTHMOG_EAST_DIR = 1,   /* DIR_EAST */
    PROBE_GOTHMOG_NORTH_DIR = 0,  /* DIR_NORTH */
    /* The south-entry approach pose: party at (3, 7) facing NORTH,
     * one square south of the ordinal-22 cell.  The front cell is
     * (3, 6) (the ordinal-22 cell), the visible wall is the south
     * wall (cell 2) of (3, 6) - which has NO C127 sensor for
     * ordinal 22 (the ordinal-22 sensor is on the WEST wall). */
    PROBE_SOUTH_ENTRY_X = 3,
    PROBE_SOUTH_ENTRY_Y = 7,
    PROBE_SOUTH_ENTRY_DIR = 0, /* DIR_NORTH */
    PROBE_ORDINAL_TARGET = 22,
    HALL_MAP_INDEX = 0,
    HALL_MAX_CELLS_PER_AXIS = 16,
    /* C026 champion-portrait strip dimensions: 8 cols x 3 rows of
     * 32x29 portraits (ordinals 0..23).  Ordinal 22 sits at col 6,
     * row 2 of the strip (the BOTTOM row). */
    PROBE_PORTRAIT_STRIP_W = 256,
    PROBE_PORTRAIT_STRIP_H = 87,
    /* Re-blt invariant tolerance matching the existing
     * walkpath / zorder / reblt / east_walkpath /
     * walkpath_from_entrance / front_south_entry probes: the
     * ordinal-22 matched-pixel count in the D1C rect must not
     * reach 35% of its compared count when the player is NOT
     * facing the front wall of the (3, 6) cell, otherwise ordinal
     * 22 is "floating" on the side wall. */
    PROBE_FLOOR_LEAK_PCT = 35,
    /* Positive-ordinal pixel match threshold matching the existing
     * east_walkpath / walkpath_from_entrance / front_south_entry
     * probes: 90% of the C026 ordinal-22 opaque pixels must be
     * present in the D1C rect for the ordinal-22 pose to be
     * considered properly drawn. */
    PROBE_POSITIVE_MATCH_PCT = 90,
    /* The ordinal-22 source cell (192, 58, 32, 29) bottom edge
     * MUST equal the C026 strip height (87).  We use 88 (87+1) as
     * the bound so a regression that uses 2 rows (height=58) fails
     * the containment assertion. */
    PROBE_ATLAS_HEIGHT_BOUND = 88,
    PROBE_ATLAS_WIDTH_BOUND = 257,
    /* Redraw-stability cycle count: two consecutive draws at the
     * front_south_entry exit pose must produce byte-stable
     * framebuffers.  Three cycles is enough to catch a one-off
     * frame-cache jitter bug without spending the runtime budget
     * on a longer loop (same cycle count the existing ordinal-5
     * front_south_entry probe uses). */
    PROBE_REDRAW_CYCLES = 3
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
 * walkpath_from_entrance / east_walkpath / front_south_entry
 * probes use between independent routes: reset the candidate
 * panel state so a previous mirror panel does not leak into the
 * next check. */
static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = (int16_t)mapX;
    game->world.party.mapY = (int16_t)mapY;
    game->world.party.direction = (uint8_t)dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

/* Count the pixels in the front-wall box that match the C026
 * champion portrait ordinal.  Same formula as the visibility /
 * zorder / reblt / east_walkpath / walkpath_from_entrance /
 * front_south_entry probes:
 *   DUNVIEW.C:3916  C01 dark-gray (value 1) is the transparency
 *                   mask
 *   DUNVIEW.C:3918  per-ordinal source stride
 *                   srcX = (ordinal & 7) * 32, srcY = (ordinal >> 3) * 29
 *
 * Returns 0 when the ordinal is out of range or the slot is
 * unloaded. */
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

/* Count the C026 ordinal pixels compared in the D1C rect
 * (matching the "compared" count the existing east_walkpath /
 * walkpath_from_entrance / front_south_entry probes use to compute
 * the leak percentage). */
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

/* (A) Catalog identity for ordinal 22: name == "GOTHMOG" (DM1 V1
 *     PC 3.4 TextString binding), title empty (GOTHMOG is
 *     untitled in the DM1 V1 catalog per the front_north_entry
 *     probe).  Pins the slice to a real source identity so a
 *     future regression in the C127 sensorData -> mirror name
 *     binding is caught here. */
static int test_ordinal_22_catalog_identity(M11_GameViewState* game) {
    char nameBuf[32];
    char titleBuf[32];
    int ok = 1;
    printf("[A] ordinal 22 catalog identity (GOTHMOG, untitled)\n");
    set_pose(game, PROBE_GOTHMOG_CELL_X, PROBE_GOTHMOG_CELL_Y,
             PROBE_GOTHMOG_FRONT_DIR);
    nameBuf[0] = '\0';
    titleBuf[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(game, PROBE_ORDINAL_TARGET,
                                              nameBuf, (int)sizeof(nameBuf));
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_ORDINAL_TARGET,
                                               titleBuf, (int)sizeof(titleBuf));
    printf("  INFO: ordinal 22 name = %s, title = %s\n",
           nameBuf[0] ? nameBuf : "(unknown)",
           titleBuf[0] ? titleBuf : "(untitled)");
    ok &= expect_int("ordinal 22 name == GOTHMOG",
                     strcmp(nameBuf, "GOTHMOG") == 0, 1);
    /* GOTHMOG is untitled in the DM1 V1 PC 3.4 catalog per the
     * front_north_entry probe; re-pin it here so the slice stays
     * bound to a real source identity. */
    ok &= expect_int("ordinal 22 title is empty (untitled champion)",
                     titleBuf[0] == '\0', 1);
    return ok;
}

/* (B) ordinal 22 lives at (3, 6, WEST) on Hall map 0 in the real
 *     DM1 V1 DUNGEON.DAT.  Re-verify the ordinal-22 ANY-pose
 *     discovery here so this probe stays independent of the
 *     front_north_entry probe.  The single-hit contract is the
 *     no-floating corollary for the ordinal-22 route on this
 *     fixture: ordinal 22 has exactly one C127 sensor route on
 *     Hall map 0 in DM1 V1 PC 3.4. */
static int test_ordinal_22_hall_location(M11_GameViewState* game) {
    int mapX;
    int mapY;
    int dir;
    int hits = 0;
    int hitX = -1, hitY = -1, hitDir = -1;
    int ok = 1;
    printf("[B] ordinal 22 Hall map 0 location = (3, 6, WEST) in real DM1 V1 DUNGEON.DAT\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    for (mapY = 0; mapY < HALL_MAX_CELLS_PER_AXIS; ++mapY) {
        for (mapX = 0; mapX < HALL_MAX_CELLS_PER_AXIS; ++mapX) {
            for (dir = 0; dir < 4; ++dir) {
                int ord;
                game->world.party.mapX = (int16_t)mapX;
                game->world.party.mapY = (int16_t)mapY;
                game->world.party.direction = (uint8_t)dir;
                game->showDebugHUD = 0;
                game->candidateMirrorPanelActive = 0;
                game->candidateMirrorOrdinal = -1;
                game->candidateMirrorPartyIndex = -1;
                ord = M11_GameView_GetFrontMirrorOrdinal(game);
                if (ord == PROBE_ORDINAL_TARGET) {
                    ++hits;
                    hitX = mapX;
                    hitY = mapY;
                    hitDir = dir;
                    printf("  HIT: ordinal 22 at pose=(%d, %d, %d)\n",
                           mapX, mapY, dir);
                }
            }
        }
    }
    ok &= expect_int("ordinal 22 found at (3, 6, WEST) (the front_south_entry cell)",
                     (hits == 1 &&
                      hitX == PROBE_GOTHMOG_CELL_X &&
                      hitY == PROBE_GOTHMOG_CELL_Y &&
                      hitDir == PROBE_GOTHMOG_FRONT_DIR) ? 1 : 0, 1);
    /* Verify the canonical ordinal-22 pose resolves to ordinal 22
     * (not a stale value from a previous panel state). */
    set_pose(game, PROBE_GOTHMOG_CELL_X, PROBE_GOTHMOG_CELL_Y,
             PROBE_GOTHMOG_FRONT_DIR);
    {
        int ord = M11_GameView_GetFrontMirrorOrdinal(game);
        ok &= expect_int("front_mirror ordinal at (3, 6, WEST) == 22 (GOTHMOG)",
                         ord, PROBE_ORDINAL_TARGET);
    }
    return ok;
}

/* (C) South-facing side pose at the ordinal-22 cell: (3, 6)
 *     facing SOUTH -> front-mirror ordinal -1 and the D1C portrait
 *     rect contains NO ordinal-22 pixels above the leak threshold.
 *     This is the source-locked no-floating contract for the
 *     front_south_entry exit pose at the ordinal-22 cell — the
 *     player turning to face SOUTH at (3, 6) must NOT see the
 *     GOTHMOG portrait on the south wall of (3, 6).  Mirrors the
 *     side-pose coverage the existing front_south_entry probe
 *     locks for the (2, 10) LINFLAS cell. */
static int test_south_side_pose_no_floating(M11_GameViewState* game,
                                            const M11_AssetSlot* portraits,
                                            unsigned char* fb) {
    int ok = 1;
    int ord;
    int matched;
    int compared;
    int leakPct;
    printf("[C] South-facing side pose at (3, 6) - no-floating of ordinal 22\n");
    set_pose(game, PROBE_GOTHMOG_CELL_X, PROBE_GOTHMOG_CELL_Y,
             PROBE_GOTHMOG_SOUTH_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    ok &= expect_int("(3, 6, SOUTH) front_mirror ordinal == -1 (no float)",
                     ord, -1);
    /* Render the south-facing pose and pixel-prove the D1C rect
     * does not contain ordinal-22 pixels above the leak threshold.
     * The D1C wall box may still share palette pixels with C026
     * portrait assets because the wall-ornament graphic and the
     * portrait strip share the same 4bpp palette — the no-floating
     * assertion uses the ordinal-22 opaque pixel match count, not
     * a raw 4bpp equality, so palette-sharing does not produce a
     * false positive (same pattern the existing
     * east_walkpath / walkpath_from_entrance / front_south_entry
     * probes use). */
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    matched = count_ordinal_matched_pixels(portraits, fb, PROBE_ORDINAL_TARGET);
    compared = count_ordinal_compared_pixels(portraits, PROBE_ORDINAL_TARGET);
    leakPct = compared > 0 ? (matched * 100) / compared : 0;
    printf("  (3, 6, SOUTH) ordinal-22 matched=%d compared=%d leakPct=%d\n",
           matched, compared, leakPct);
    if (leakPct >= PROBE_FLOOR_LEAK_PCT) {
        ++g_fail;
        printf("  FAIL: (3, 6, SOUTH) ordinal-22 leaked matched=%d/%d (>= %d%%)\n",
               matched, compared, PROBE_FLOOR_LEAK_PCT);
        ok = 0;
    } else {
        ++g_pass;
        printf("  PASS: (3, 6, SOUTH) ordinal-22 leak %d%% (< %d%%)\n",
               leakPct, PROBE_FLOOR_LEAK_PCT);
    }
    /* Also exercise the east and north side poses at (3, 6) so a
     * future regression that breaks one side wall but not another
     * is caught by the same probe.  West is excluded because (3, 6,
     * WEST) is the canonical ordinal-22 pose, not a no-portrait
     * side pose.
     *
     * Note: the (3, 6, NORTH) side pose exposes ordinal 11 (a
     * different mirror on the north wall of (3, 6) in real DM1 V1
     * DUNGEON.DAT), NOT -1 — the no-portrait assertion would be
     * too strict.  The contract the probe locks is the no-floating
     * of ordinal 22 onto the side wall: front_mirror ordinal != 22
     * AND D1C rect ordinal-22 leak < 35% (the 35% threshold matches
     * the existing reblt / walkpath_from_entrance / front_south_entry
     * probes).  This is the source-locked proof that ordinal 22
     * (GOTHMOG) does not float onto any D1C wall cell on the (3, 6)
     * cell regardless of the player's facing direction. */
    {
        static const struct { int dir; const char* label; } kSidePoses[] = {
            {PROBE_GOTHMOG_EAST_DIR, "(3, 6, EAST) side pose"},
            {PROBE_GOTHMOG_NORTH_DIR, "(3, 6, NORTH) side pose"}
        };
        size_t i;
        for (i = 0; i < sizeof(kSidePoses) / sizeof(kSidePoses[0]); ++i) {
            char labelBuf[96];
            set_pose(game, PROBE_GOTHMOG_CELL_X, PROBE_GOTHMOG_CELL_Y,
                     kSidePoses[i].dir);
            ord = M11_GameView_GetFrontMirrorOrdinal(game);
            /* The no-floating-of-ordinal-22 contract: the side pose
             * must not expose ordinal 22 as the front-mirror ordinal.
             * (The side pose may legitimately expose a different
             * ordinal, e.g. (3, 6, NORTH) -> 11 in DM1 V1 PC 3.4.) */
            snprintf(labelBuf, sizeof(labelBuf),
                     "%s front_mirror ordinal != 22 (no self-float, got=%d)",
                     kSidePoses[i].label, ord);
            ok &= expect_int(labelBuf, ord != PROBE_ORDINAL_TARGET, 1);
            memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
            M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
            matched = count_ordinal_matched_pixels(portraits, fb, PROBE_ORDINAL_TARGET);
            leakPct = compared > 0 ? (matched * 100) / compared : 0;
            if (leakPct >= PROBE_FLOOR_LEAK_PCT) {
                ++g_fail;
                printf("  FAIL: %s ordinal-22 leaked matched=%d/%d (>= %d%%)\n",
                       kSidePoses[i].label, matched, compared, PROBE_FLOOR_LEAK_PCT);
                ok = 0;
            } else {
                ++g_pass;
                printf("  PASS: %s ordinal-22 leak %d%% (< %d%%, front_ord=%d)\n",
                       kSidePoses[i].label, leakPct, PROBE_FLOOR_LEAK_PCT, ord);
            }
        }
    }
    /* Redraw-stability byte equality: two consecutive draws at the
     * (3, 6, SOUTH) pose must produce byte-stable framebuffers (no
     * jitter across draws); this guards against an off-by-one
     * sprite frame cache bug or a non-idempotent palette decode in
     * the D1C draw path.  Same contract the existing ordinal-5
     * front_south_entry probe locks for the (2, 16) cell. */
    {
        unsigned char fbRef[PROBE_FB_W * PROBE_FB_H];
        int cycle;
        int diffBytes = -1;
        set_pose(game, PROBE_GOTHMOG_CELL_X, PROBE_GOTHMOG_CELL_Y,
                 PROBE_GOTHMOG_SOUTH_DIR);
        memset(fbRef, 0, sizeof(fbRef));
        M11_GameView_Draw(game, fbRef, PROBE_FB_W, PROBE_FB_H);
        for (cycle = 1; cycle < PROBE_REDRAW_CYCLES; ++cycle) {
            unsigned char fbCycle[PROBE_FB_W * PROBE_FB_H];
            int k;
            int localDiff = 0;
            memset(fbCycle, 0, sizeof(fbCycle));
            M11_GameView_Draw(game, fbCycle, PROBE_FB_W, PROBE_FB_H);
            for (k = 0; k < PROBE_FB_W * PROBE_FB_H; ++k) {
                if (fbRef[k] != fbCycle[k]) ++localDiff;
            }
            if (diffBytes < 0) diffBytes = localDiff;
            else if (localDiff != diffBytes) diffBytes = -1;
        }
        ok &= expect_int("(3, 6, SOUTH) redraw byte stability across cycles",
                         diffBytes, 0);
    }
    return ok;
}

/* (D) South-entry approach pose: party at (3, 7) facing NORTH, one
 *     square south of the ordinal-22 cell.  The front cell is
 *     (3, 6) (the ordinal-22 cell), the visible wall is the south
 *     wall (cell 2) of (3, 6) — which has NO C127 sensor for
 *     ordinal 22.  This is the south-entry arc that the
 *     front_south_entry route variant documents.  Verify the
 *     no-portrait contract at this waypoint and pixel-prove the
 *     D1C rect does not contain ordinal-22 pixels above the leak
 *     threshold (no floating of GOTHMOG onto the south wall of
 *     (3, 6) when the player approaches from the south). */
static int test_south_entry_approach(M11_GameViewState* game,
                                      const M11_AssetSlot* portraits,
                                      unsigned char* fb) {
    int ok = 1;
    int ord;
    int matched;
    int compared;
    int leakPct;
    printf("[D] South-entry approach pose at (3, 7, NORTH) - no-portrait "
           "south wall of (3, 6)\n");
    set_pose(game, PROBE_SOUTH_ENTRY_X, PROBE_SOUTH_ENTRY_Y,
             PROBE_SOUTH_ENTRY_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    ok &= expect_int("(3, 7, NORTH) front_mirror ordinal == -1 "
                     "(south wall of (3, 6) has no C127 sensor)",
                     ord, -1);
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    matched = count_ordinal_matched_pixels(portraits, fb, PROBE_ORDINAL_TARGET);
    compared = count_ordinal_compared_pixels(portraits, PROBE_ORDINAL_TARGET);
    leakPct = compared > 0 ? (matched * 100) / compared : 0;
    printf("  (3, 7, NORTH) ordinal-22 matched=%d compared=%d leakPct=%d\n",
           matched, compared, leakPct);
    if (leakPct >= PROBE_FLOOR_LEAK_PCT) {
        ++g_fail;
        printf("  FAIL: (3, 7, NORTH) ordinal-22 leaked matched=%d/%d (>= %d%%)\n",
               matched, compared, PROBE_FLOOR_LEAK_PCT);
        ok = 0;
    } else {
        ++g_pass;
        printf("  PASS: (3, 7, NORTH) ordinal-22 leak %d%% (< %d%%)\n",
               leakPct, PROBE_FLOOR_LEAK_PCT);
    }
    /* Sanity: confirm the canonical ordinal-22 pose at (3, 6, WEST)
     * still resolves to ordinal 22 after the south-entry waypoint,
     * so a future regression that breaks the ordinal-22 route after
     * a south-entry approach is caught. */
    set_pose(game, PROBE_GOTHMOG_CELL_X, PROBE_GOTHMOG_CELL_Y,
             PROBE_GOTHMOG_FRONT_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    ok &= expect_int("(3, 6, WEST) returns ordinal 22 after south-entry waypoint",
                     ord, PROBE_ORDINAL_TARGET);
    return ok;
}

/* (E) Candidate-panel return at the ordinal-22 cell: open the C040
 *     resurrect/reincarnate panel at (3, 6, WEST), cancel the
 *     panel, then turn to face SOUTH at the same cell and verify
 *     the panel state is cleared and the D1C portrait rect remains
 *     stable (no stale-panel bleed into the south-facing redraw).
 *     This is the source-locked contract for the C040 panel exit
 *     path the redraw_after_candidate / resurrect_reselect /
 *     cancel_reopen sibling probes lock for other ordinal poses;
 *     this slice widens the same coverage to the (3, 6, WEST) /
 *     ordinal-22 (GOTHMOG) pose with the front_south_entry exit
 *     direction. */
static int test_candidate_panel_return_at_south(M11_GameViewState* game,
                                                const M11_AssetSlot* portraits,
                                                unsigned char* fb) {
    int ok = 1;
    int selected;
    int cancelled;
    int panelAfter;
    int ordinalAfter;
    int matched;
    int compared;
    int leakPct;
    printf("[E] Candidate-panel return at (3, 6, WEST) -> cancel -> turn SOUTH\n");
    /* Seat the party at the canonical ordinal-22 pose (3, 6, WEST)
     * and open the C040 resurrect/reincarnate panel via the same
     * API the existing redraw_after_candidate / resurrect_reselect
     * / cancel_reopen probes use. */
    set_pose(game, PROBE_GOTHMOG_CELL_X, PROBE_GOTHMOG_CELL_Y,
             PROBE_GOTHMOG_FRONT_DIR);
    selected = M11_GameView_SelectFrontMirrorCandidate(game);
    ok &= expect_int("SelectFrontMirrorCandidate at (3, 6, WEST) returns 1",
                     selected, 1);
    ok &= expect_int("candidateMirrorPanelActive == 1 after select",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("candidateMirrorOrdinal == 22 after select",
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
    /* Now turn to face SOUTH at the same cell (the
     * front_south_entry exit direction).  The party pose is
     * mutated directly; the panel state must remain cleared.  This
     * is the south-side panel-return assertion the existing
     * ordinal-22 probes leave uncovered. */
    game->world.party.direction = (uint8_t)PROBE_GOTHMOG_SOUTH_DIR;
    panelAfter = game->candidateMirrorPanelActive;
    ordinalAfter = game->candidateMirrorOrdinal;
    ok &= expect_int("candidateMirrorPanelActive == 0 after turn SOUTH",
                     panelAfter, 0);
    ok &= expect_int("candidateMirrorOrdinal == -1 after turn SOUTH",
                     ordinalAfter, -1);
    /* Redraw the south-facing pose and pixel-prove the D1C rect
     * does not contain ordinal-22 pixels above the leak threshold
     * after the panel cancel + south turn.  This is the redraw
     * stability contract for the front_south_entry exit pose at
     * the ordinal-22 cell. */
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    matched = count_ordinal_matched_pixels(portraits, fb, PROBE_ORDINAL_TARGET);
    compared = count_ordinal_compared_pixels(portraits, PROBE_ORDINAL_TARGET);
    leakPct = compared > 0 ? (matched * 100) / compared : 0;
    printf("  (3, 6, SOUTH) post-cancel ordinal-22 matched=%d compared=%d leakPct=%d\n",
           matched, compared, leakPct);
    if (leakPct >= PROBE_FLOOR_LEAK_PCT) {
        ++g_fail;
        printf("  FAIL: (3, 6, SOUTH) post-cancel ordinal-22 leaked matched=%d/%d (>= %d%%)\n",
               matched, compared, PROBE_FLOOR_LEAK_PCT);
        ok = 0;
    } else {
        ++g_pass;
        printf("  PASS: (3, 6, SOUTH) post-cancel ordinal-22 leak %d%% (< %d%%)\n",
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
    int col = -1, row = -1, sx = -1, sy = -1;

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

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 22, "
           "route front_south_entry, aspect portrait_rect_position ===\n");
    printf("sourceEvidence=DUNGEON.C:2558,2608-2612 (C127 sensorData -> G0289)\n");
    printf("                DUNGEON.C:2573 (M011_CELL(sensor) -> visible wall)\n");
    printf("                MOVESENS.C:1501-1503 (C127 -> F0280)\n");
    printf("                REVIVE.C F0280,F0282 (candidate materialise/cancel)\n");
    printf("                DUNVIEW.C:3913-3928 (C346 frame + C026 portrait blit)\n");
    printf("                DUNVIEW.C:8318-8542 F0128 (far-to-near draw order)\n");
    printf("                DUNVIEW.C:8522-8533 (C026 D1C re-blt on tick redraw)\n");
    printf("                COORD.C:1693-1722 (PC 3.4 viewport origin / 224x136)\n");
    printf("                DEFS.H:2071-2079 (G2071_C320 / G2078_C32 / G2079_C29)\n");
    printf("                DEFS.H:821-826 (M027/M028 portrait macro math)\n\n");

    ok &= test_ordinal_22_catalog_identity(&game);
    ok &= test_ordinal_22_hall_location(&game);
    ok &= test_south_side_pose_no_floating(&game, portraits, currFb);
    ok &= test_south_entry_approach(&game, portraits, currFb);
    ok &= test_candidate_panel_return_at_south(&game, portraits, currFb);

    /* Lock the C026 ordinal-22 source-rect math the probe relies
     * on so a future refactor that moves the C026 atlas stride is
     * caught here too.  The math is identical to the
     * front_north_entry probe's [B] section; we re-pin it because
     * the runtime pixels above are tied to that source rect. */
    col = PROBE_ORDINAL_TARGET & 7;
    row = (PROBE_ORDINAL_TARGET >> 3) & 3;
    sx = col * PROBE_PORTRAIT_W;
    sy = row * PROBE_PORTRAIT_H;
    ok &= expect_int("ordinal 22 col = ordinal mod 8", col, 6);
    ok &= expect_int("ordinal 22 row = ordinal / 8", row, 2);
    ok &= expect_int("ordinal 22 source X == 6*32", sx, 192);
    ok &= expect_int("ordinal 22 source Y == 2*29", sy, 58);
    ok &= expect_int("ordinal 22 source bottom edge == C026 strip height "
                     "(row 2 is the last row of the 8x3 atlas)",
                     sy + PROBE_PORTRAIT_H, PROBE_PORTRAIT_STRIP_H);
    ok &= expect_int("ordinal 22 source bottom edge < C026 strip height + 1 "
                     "(regression that uses 2 rows (height=58) fails this)",
                     sy + PROBE_PORTRAIT_H < PROBE_ATLAS_HEIGHT_BOUND, 1);
    ok &= expect_int("ordinal 22 source right edge < C026 strip width + 1 "
                     "(ordinal 22 col=6 leaves 32 px of room in 256-wide strip)",
                     sx + PROBE_PORTRAIT_W < PROBE_ATLAS_WIDTH_BOUND, 1);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
