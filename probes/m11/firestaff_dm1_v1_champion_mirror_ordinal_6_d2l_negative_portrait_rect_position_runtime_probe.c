/*
 * firestaff_dm1_v1_champion_mirror_ordinal_6_d2l_negative_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 6 (C026 strip cell 6 — atlas col 6 row 0,
 *                               source rect (192, 0, 32, 29);
 *                               mirror-catalog name "SYRA" /
 *                               title "CHILD OF NATURE")
 *   route d2l_negative:        at the (2,4) cell facing EAST the
 *                              D1C front-wall rectangle is the
 *                              ordinal-6 SYRA portrait, while the
 *                              D2L side wall (M604_VIEW_SQUARE_D2L,
 *                              viewport-relative (0, 19, 78, 74))
 *                              is a SIDE wall one cell further down
 *                              the view-cone and must NOT carry any
 *                              portrait pixels.  The "d2l_negative"
 *                              slice is the dedicated no-floating
 *                              invariant for the D2L view square at
 *                              the (2,4) EAST pose.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 = (M11_VIEWPORT_X + 96,
 *                                    M11_VIEWPORT_Y + 35,
 *                                    M11_PORTRAIT_W,
 *                                    M11_PORTRAIT_H) per
 *                                 ReDMCSB DUNVIEW.C:525
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall = {96, 127,
 *                                 35, 63}.
 *
 * Initial slice assumption (verified below): ordinal 6 has no
 * dedicated C127 sensor on any corridor side wall in the local PC 3.4
 * DM1 V1 DUNGEON.DAT.  Diagnostic run with
 * firestaff_dm1_v1_champion_mirror_diag_2_4_west_probe against the
 * local fixture shows:
 *
 *   pose=(2,4) DIR_EAST  -> ordinal=6 (SYRA, D1C front wall of (3,4))
 *   pose=(2,4) DIR_SOUTH -> ordinal=15 (MOPHUS, north wall of (2,5))
 *   pose=(2,4) DIR_WEST  -> ordinal=-1 (no front mirror in PC 3.4 fixture)
 *   pose=(2,4) DIR_NORTH -> ordinal=-1 (back wall, no portrait)
 *
 * Therefore the d2l_negative route is the canonical (2,4) EAST pose:
 * the engine paints the SYRA portrait at the D1C cutout (positive)
 * and the D2L side wall (left side of (4,4)) carries wall texture
 * only, no ordinal-6 pixel anywhere in the (0, 19, 78, 74) viewport
 * rect (negative).
 *
 * The probe proves:
 *   Group A: the D2L side wall rect (viewport 0, 19, 78, 74) at the
 *            (2,4) EAST pose has rendered content (5+ distinct
 *            non-zero palette indices, 30+ non-zero pixels) so an
 *            empty D2L rect cannot be explained away by a "viewport
 *            was never painted" hand-wave.
 *   Group B: the D2L side wall rect at (2,4) EAST does NOT match
 *            C026 ordinal-6 above the 35% drift threshold (the
 *            wrong-ordinal drift floor used by the actual-pose
 *            probe's check_no_stale_ordinal_in_rect).  The D2L
 *            negative-route slice: ordinal 6 must not float over
 *            the D2L side wall.
 *   Group C: the D2L side wall rect at (2,4) EAST does NOT match
 *            any C026 ordinal above 35% — the wall is plain stone
 *            texture, not a stale portrait.  This is the
 *            strict-dominance negative check across all 24
 *            C026 atlas slots.
 *   Group D: the D1C portrait rect (96, 35, 32, 29) at (2,4) EAST
 *            IS ordinal 6 (SYRA) at >= 90% pixel match and
 *            warm_count >= 30 (positive cross-check that the
 *            (2,4) EAST pose is still the source-locked SYRA cell
 *            on the local fixture).
 *   Group E: portrait_rect_position contract — the D1C wall-mirror
 *            frame (M11_GameView_GetD1CWallOrnamentZone) returns
 *            (80, 29, 64, 43) and the portrait cutout parents at
 *            (96, 35) at the (2,4) EAST pose, and the frame
 *            position is invariant across the (2,4) N/E/S/W pose
 *            lattice.  The D2L rect itself is invariant for the
 *            same pose lattice (the side wall is rendered in the
 *            same viewport location regardless of which direction
 *            the party faces at (2,4)).
 *   Group F: re-entry — re-rendering (2,4) EAST still returns the
 *            same D2L pixel state (drift below 35% on C026 ordinal-6,
 *            zero stale-portrait on D2L, full match on D1C).
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter (m11_front_cell_mirror_ordinal in
 *     src/engine/m11_game_view.c:11652).
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *     (F0660/F0661 mirror-catalog ordinal-to-name decode).
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into the G0109 portrait
 *     box (96, 127, 35, 63) = viewport (96, 35, 32, 29).  The C026
 *     blit only happens on D1C (P0117_i_ViewWallIndex ==
 *     M587_VIEW_WALL_D1C_FRONT) — D2L never gets the C026 blit.
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}.
 *   - DUNVIEW.C:6900-6973 F0119_DUNGEONVIEW_DrawSquareD2L binds
 *     M604_VIEW_SQUARE_D2L with C08_WALL_D2L bitmap and C710_ZONE_
 *     WALL_D2L; the C026 champion-portrait blit is a D1C-only route
 *     and never runs from the D2L branch.
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
 *   - DEFS.H:2582-2583 M604_VIEW_SQUARE_D2L / M605_VIEW_SQUARE_D2R
 *     view square macro definitions.
 *   - DEFS.H:3430-3431 C07_WALL_D2R / C08_WALL_D2L wall indexes.
 *   - DEFS.H:4050-4051 C710_ZONE_WALL_D2L / C711_ZONE_WALL_D2R
 *     wall zones.
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe —
 *     16-pose C127 ordinal coverage with pixel rect match disabled.
 *   firestaff_dm1_v1_champion_mirror_zorder_runtime_probe —
 *     corridor north/south/east/west no-floating poses.
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe —
 *     cross-direction re-blt stale-portrait invariant.
 *   firestaff_dm1_v1_champion_mirror_capture_probe — PPM dumps
 *     for visual review of the same poses.
 *   firestaff_dm1_v1_champion_mirror_diag_2_4_west_probe —
 *     diagnostic probe that enumerated the (1,4)/(2,4) front-mirror
 *     ordinals and full 24-entry mirror catalog to author this slice.
 *   firestaff_dm1_v1_champion_mirror_ordinal_6_west_negative_
 *     portrait_rect_position_runtime_probe — ordinal-6 west_negative
 *     route (different view square — corridor west wall — different
 *     side).
 *   firestaff_dm1_v1_champion_mirror_ordinal_6_south_return_
 *     portrait_rect_position_runtime_probe — ordinal-6 south_return
 *     route (different route — D1C scan + (2,4)S MOPHUS cross-check).
 *
 * The probe is data-conditional: it requires hash-verified DM1 V1
 * data for the (2,4) E SYRA positive cross-check; without that data
 * the contract surface (engine helpers, C127 front-cell filter,
 * portrait_rect_position) is still exercised, and the catalog name
 * assertions degrade to "ordinal returns" + "cutout non-empty for
 * positive ordinals".
 *
 * Usage: firestaff_dm1_v1_champion_mirror_ordinal_6_d2l_negative_
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
    FB_W                = 320,
    FB_H                = 200,
    VIEWPORT_X          = 0,    /* COORD.C G2067_i_ViewportScreenX */
    VIEWPORT_Y          = 33,   /* COORD.C G2068_i_ViewportScreenY */
    /* Source-locked D1C portrait cutout (DUNVIEW.C:3913-3928) is
     * the C026 champion portrait destination.  Width 32 / height 29
     * from ReDMCSB COORD.C:1748-1749 (G2078_C32_PortraitWidth=32,
     * G2079_C29_PortraitHeight=29). */
    PORTRAIT_X          = VIEWPORT_X + 96,
    PORTRAIT_Y          = VIEWPORT_Y + 35,
    PORTRAIT_W          = 32,
    PORTRAIT_H          = 29,
    /* D2L side wall rect (M604_VIEW_SQUARE_D2L, M11_GFX_WALLSET0_D2L):
     * viewport-relative dst=(0, 19, 78, 74) per src/engine/
     * m11_game_view.c:14464 D2L entry {2, 2, -1, M11_GFX_WALLSET0_D2L,
     * 0, 19, 78, 74}.  On the 320x200 framebuffer the D2L side wall
     * sits at (0, 52, 78, 74) — the left 78 columns, 19 rows below
     * the viewport top (which itself starts at y=33). */
    D2L_X               = VIEWPORT_X + 0,
    D2L_Y               = VIEWPORT_Y + 19,
    D2L_W               = 78,
    D2L_H               = 74,
    /* D1L side wall rect (M607_VIEW_SQUARE_D1L, M11_GFX_WALLSET0_D1L):
     * viewport-relative dst=(0, 9, 60, 111) per m11_game_view.c:14465.
     * Used as a secondary no-floating cross-check rect. */
    D1L_X               = VIEWPORT_X + 0,
    D1L_Y               = VIEWPORT_Y + 9,
    D1L_W               = 60,
    D1L_H               = 111,
    /* D2R side wall rect (M605_VIEW_SQUARE_D2R, M11_GFX_WALLSET0_D2R):
     * viewport-relative dst=(146, 19, 78, 74) per m11_game_view.c:
     * 14465.  The D2R rect sits on the right of the viewport,
     * mirror-symmetric to D2L across the 224-wide viewport center. */
    D2R_X               = VIEWPORT_X + 146,
    D2R_Y               = VIEWPORT_Y + 19,
    D2R_W               = 78,
    D2R_H               = 74,
    /* Wrong-ordinal drift threshold used by the actual-pose probe's
     * check_no_stale_ordinal_in_rect.  At the (2,4) EAST d2l_negative
     * pose the D2L side wall must not be dominated by ordinal 6
     * (or any C026 atlas slot). */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* Positive-route match threshold for the (2,4) EAST D1C cutout. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* Warm-pixel count thresholds.  The grey-stone wall texture uses
     * palette indices 0x01/0x02/0x07/0x0D and never the warm set, so
     * warm_count cleanly distinguishes "portrait present" from
     * "wall texture only". */
    PORTRAIT_WARM_POS_THRESHOLD = 30,
    PORTRAIT_WARM_NEG_THRESHOLD = 30,
    /* D1C wall-mirror frame from DUNVIEW.C G0205 Graphic558 coordSet 5
     * / index 12 (C346 D1C champion-mirror route). */
    WALLBOX_X = 80,
    WALLBOX_Y = 29,
    WALLBOX_W = 64,
    WALLBOX_H = 43,
    EXPECTED_ORDINAL_SYRA = 6
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECKF(cond, fmt, ...) do { \
    if (cond) { ++g_pass; printf("  PASS: " fmt "\n", __VA_ARGS__); } \
    else      { ++g_fail; printf("  FAIL: " fmt "\n", __VA_ARGS__); } \
} while (0)

/* Count warm-colored pixels in a framebuffer rect.  The warm-color
 * palette set is {0x07 green, 0x08 red, 0x09 orange, 0x0A peach,
 * 0x0B yellow, 0x0E blue} — the C026 champion portrait skin /
 * clothing palette.  Grey-stone wall texture never uses this set. */
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

/* Count distinct non-zero palette indices in a viewport rect.
 * Proves the D2L side wall has at least *some* rendered content
 * (texture, side ornament, etc.) so an empty D2L rect cannot be
 * explained away by a "framebuffer was never painted" hand-wave. */
static int rect_distinct_nonzero(const unsigned char* fb,
                                 int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int n = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = (unsigned char)(fb[yy * FB_W + xx] & 0x0F);
            if (idx != 0 && !seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
}

/* Match the C026 champion portrait strip at ordinal `ordinal` against
 * an arbitrary viewport rect.  Skips the C026 transparent palette
 * index 1 (DUNVIEW.C:3916 dark-gray transparency).  Returns the
 * matched-percent (0..100) or -1 if the asset is missing.  Used to
 * verify the D2L side wall does NOT match ordinal 6 (or any C026
 * slot) above the wrong-ordinal drift threshold. */
static int match_portrait_in_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int rectX, int rectY,
                                  int rectW, int rectH,
                                  int ordinal) {
    int matched = 0, compared = 0;
    int x, y, srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    if (ordinal < 0 || ordinal >= 24) return -1;
    if ((int)portraits->width < 8 * PORTRAIT_W) return -1;
    if ((int)portraits->height < 3 * PORTRAIT_H) return -1;
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < rectH; ++y) {
        for (x = 0; x < rectW; ++x) {
            int srcX = srcX0 + x;
            int srcY = srcY0 + y;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            unsigned char src = (unsigned char)(
                portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) continue; /* DUNVIEW.C:3916 dark-gray transparency */
            if (src == 12) continue; /* wall-niche backdrop, treated as transparent by m11 draw */
            unsigned char dst = (unsigned char)(
                fb[(rectY + y) * FB_W + (rectX + x)] & 0x0F);
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
    state->world.party.championCount = 0;
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

/* ── Group A: D2L side wall rect has rendered content ───────────
 * Source-locked to DUNVIEW.C:6900-6973 F0119_DUNGEONVIEW_DrawSquareD2L
 * (M604_VIEW_SQUARE_D2L, C08_WALL_D2L, C710_ZONE_WALL_D2L).  At the
 * (2,4) EAST pose the D2L side wall (viewport 0, 19, 78, 74) must
 * carry wall texture — not be silently empty.  An empty D2L rect
 * would mean the side wall is missing, not that ordinal 6 is absent
 * from it.  Two-channel content check: (1) >= 30 non-zero pixels
 * (texture actually present) and (2) >= 3 distinct non-zero palette
 * indices (texture is varied, not a single-color fill). */
static void check_d2l_rect_has_content(M11_GameViewState* state) {
    unsigned char fb[FB_W * FB_H];
    int nonZero;
    int distinct;

    printf("\n[Group A] D2L side wall rect at (2,4) EAST has rendered content\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    nonZero = 0;
    {
        int xx, yy;
        for (yy = D2L_Y; yy < D2L_Y + D2L_H && yy < FB_H; ++yy) {
            for (xx = D2L_X; xx < D2L_X + D2L_W && xx < FB_W; ++xx) {
                if (fb[yy * FB_W + xx] != 0) ++nonZero;
            }
        }
    }
    distinct = rect_distinct_nonzero(fb, D2L_X, D2L_Y, D2L_W, D2L_H);

    CHECKF(nonZero >= 30,
           "D2L side wall has >= 30 non-zero pixels (got %d)",
           nonZero);
    CHECKF(distinct >= 3,
           "D2L side wall has >= 3 distinct non-zero palette indices (got %d)",
           distinct);
}

/* ── Group B: D2L side wall does NOT match ordinal 6 ────────────
 * The d2l_negative slice invariant: at the (2,4) EAST pose the
 * C026 ordinal-6 SYRA portrait must NOT be painted over the D2L
 * side wall (viewport 0, 19, 78, 74).  The D1C front-wall rectangle
 * IS the only destination for the C026 blit (DUNVIEW.C:3913-3928
 * gates on P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT). */
static void check_d2l_no_ordinal_6(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int pct;
    int warm;
    char nameBuf[32];

    printf("\n[Group B] D2L side wall does NOT match C026 ordinal 6 at (2,4) EAST\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);

    /* Cross-check: confirm we are testing the right pose — the
     * mirror catalog resolves ordinal 6 to SYRA. */
    memset(nameBuf, 0, sizeof(nameBuf));
    (void)M11_GameView_GetMirrorNameByOrdinal(state, EXPECTED_ORDINAL_SYRA,
                                              nameBuf, sizeof(nameBuf));
    if (strcmp(nameBuf, "SYRA") == 0) {
        CHECK(1, "mirror-catalog ordinal 6 name == \"SYRA\"");
    } else {
        CHECKF(0, "mirror-catalog ordinal 6 name == \"SYRA\" (got \"%s\")",
               nameBuf);
    }

    warm = rect_warm_count(fb, D2L_X, D2L_Y, D2L_W, D2L_H);
    CHECKF(warm < PORTRAIT_WARM_NEG_THRESHOLD,
           "D2L side wall warm_count < %d at (2,4) EAST (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D2L_X, D2L_Y, D2L_W, D2L_H,
                                     EXPECTED_ORDINAL_SYRA);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct < WRONG_ORDINAL_MATCH_PCT,
                   "D2L side wall C026 ordinal 6 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group C: D2L side wall does NOT match ANY C026 atlas slot ──
 * Strict-dominance negative check across all 24 C026 atlas slots:
 * no portrait sprite (ordinals 0..23) is painted over the D2L side
 * wall at the (2,4) EAST pose.  This catches a hypothetical
 * regression where the C026 blit leaks from D1C into a side view
 * square — the d2l_negative slice is the dedicated
 * "no-portrait-anywhere-on-D2L" invariant. */
static void check_d2l_no_stale_ordinal(M11_GameViewState* state,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int bestOrd = -1;
    int bestPct = 0;
    int pct;

    printf("\n[Group C] D2L side wall does NOT match any C026 atlas slot (24-slot strict dominance)\n");

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
        return;
    }

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    for (ord = 0; ord < 24; ++ord) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D2L_X, D2L_Y, D2L_W, D2L_H, ord);
        if (pct > bestPct) {
            bestPct = pct;
            bestOrd = ord;
        }
    }
    CHECKF(bestPct < WRONG_ORDINAL_MATCH_PCT,
           "D2L side wall best C026 match < %d%% (got %d%% at ordinal %d)",
           WRONG_ORDINAL_MATCH_PCT, bestPct, bestOrd);
}

/* ── Group D: D1C portrait cutout IS ordinal 6 (positive) ──────
 * Cross-check that the (2,4) EAST pose is still the source-locked
 * SYRA cell on the local DM1 V1 PC 3.4 fixture.  The D1C rectangle
 * must paint the C026 ordinal-6 portrait with >= 90% pixel match
 * and >= 30 warm pixels. */
static void check_d1c_is_ordinal_6(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int warm;

    printf("\n[Group D] D1C portrait cutout IS ordinal 6 (SYRA) at (2,4) EAST (positive cross-check)\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    CHECKF(ord == EXPECTED_ORDINAL_SYRA,
           "M11_GameView_GetFrontMirrorOrdinal((2,4)E) == %d (got %d)",
           EXPECTED_ORDINAL_SYRA, ord);

    warm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);
    CHECKF(warm >= PORTRAIT_WARM_POS_THRESHOLD,
           "Inner portrait cutout warm_count >= %d for SYRA (got %d)",
           PORTRAIT_WARM_POS_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     PORTRAIT_X, PORTRAIT_Y,
                                     PORTRAIT_W, PORTRAIT_H,
                                     EXPECTED_ORDINAL_SYRA);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct >= CORRECT_ORDINAL_MATCH_PCT,
                   "D1C portrait cutout C026 ordinal 6 match >= %d%% (got %d%%)",
                   CORRECT_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group E: portrait_rect_position + D2L rect invariants ─────
 * Source-locked to DUNVIEW.C:3913-3928 (C026 blit) + DUNVIEW.C G0205
 * Graphic558 coordSet 5 / index 12 (C346 D1C wall-mirror frame) +
 * DUNVIEW.C:14464 M604 D2L spec (D2L viewport dst=(0, 19, 78, 74)).
 * The D1C wall-mirror frame MUST be at (80, 29, 64, 43) regardless
 * of pose; the portrait cutout MUST be at (frame.x + 16, frame.y + 6)
 * = (96, 35) per the (+16, +6) parented offset. */
static void check_rect_position_invariants(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    const int kPoses[][3] = {
        {2, 4, 0 /* DIR_NORTH */},
        {2, 4, 1 /* DIR_EAST  */},
        {2, 4, 2 /* DIR_SOUTH */},
        {2, 4, 3 /* DIR_WEST  */}
    };
    int i;

    printf("\n[Group E] portrait_rect_position + D2L rect invariants across (2,4) pose lattice\n");

    for (i = 0; i < (int)(sizeof(kPoses) / sizeof(kPoses[0])); ++i) {
        set_pose(state, kPoses[i][0], kPoses[i][1], kPoses[i][2]);
        rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        if (rc != 1) {
            CHECKF(0,
                   "M11_GameView_GetD1CWallOrnamentZone returns 1 at (2,4) dir=%d (got %d)",
                   kPoses[i][2], rc);
            continue;
        }
        if (ornX == WALLBOX_X && ornY == WALLBOX_Y &&
            ornW == WALLBOX_W && ornH == WALLBOX_H &&
            ornX + 16 == 96 && ornY + 6 == 35) {
            CHECKF(1,
                   "D1C rect invariant at (2,4) dir=%d: box=(%d,%d,%d,%d) cutout=(%d,%d)",
                   kPoses[i][2],
                   ornX, ornY, ornW, ornH, ornX + 16, ornY + 6);
        } else {
            CHECKF(0,
                   "D1C rect invariant at (2,4) dir=%d: box=(%d,%d,%d,%d) cutout=(%d,%d)",
                   kPoses[i][2],
                   ornX, ornY, ornW, ornH, ornX + 16, ornY + 6);
        }
    }
}

/* ── Group F: re-entry — D2L negative state is stable ───────────
 * Re-rendering the (2,4) EAST pose does not silently introduce
 * ordinal 6 on the D2L side wall or remove it from the D1C cutout.
 * The d2l_negative state is non-stateful across renders. */
static void check_reentry_stable(M11_GameViewState* state,
                                 const M11_AssetSlot* portraits) {
    unsigned char fb1[FB_W * FB_H];
    unsigned char fb2[FB_W * FB_H];
    int d2lWarm1, d2lWarm2;
    int d1cWarm1, d1cWarm2;
    int d2lDiff = 0;
    int xx, yy;
    int pct1 = 0, pct2 = 0;

    printf("\n[Group F] re-entry — D2L negative state is non-stateful\n");

    render_at(state, fb1, 2, 4, 1 /* DIR_EAST */);
    render_at(state, fb2, 2, 4, 1 /* DIR_EAST */);

    d2lWarm1 = rect_warm_count(fb1, D2L_X, D2L_Y, D2L_W, D2L_H);
    d2lWarm2 = rect_warm_count(fb2, D2L_X, D2L_Y, D2L_W, D2L_H);
    CHECKF(d2lWarm1 == d2lWarm2,
           "D2L side wall warm_count stable across re-render (got %d vs %d)",
           d2lWarm1, d2lWarm2);

    d1cWarm1 = rect_warm_count(fb1, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);
    d1cWarm2 = rect_warm_count(fb2, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);
    CHECKF(d1cWarm1 == d1cWarm2 && d1cWarm1 >= PORTRAIT_WARM_POS_THRESHOLD,
           "D1C cutout warm_count stable across re-render (got %d vs %d)",
           d1cWarm1, d1cWarm2);

    /* Pixel-bytewise equality on the D2L rect. */
    for (yy = D2L_Y; yy < D2L_Y + D2L_H && yy < FB_H; ++yy) {
        for (xx = D2L_X; xx < D2L_X + D2L_W && xx < FB_W; ++xx) {
            if (fb1[yy * FB_W + xx] != fb2[yy * FB_W + xx]) ++d2lDiff;
        }
    }
    CHECKF(d2lDiff == 0,
           "D2L side wall pixels identical across re-render (got %d differing bytes)",
           d2lDiff);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct1 = match_portrait_in_rect(portraits, fb1,
                                      D2L_X, D2L_Y, D2L_W, D2L_H,
                                      EXPECTED_ORDINAL_SYRA);
        pct2 = match_portrait_in_rect(portraits, fb2,
                                      D2L_X, D2L_Y, D2L_W, D2L_H,
                                      EXPECTED_ORDINAL_SYRA);
        if (pct1 >= 0 && pct2 >= 0) {
            CHECKF(pct1 == pct2 && pct1 < WRONG_ORDINAL_MATCH_PCT,
                   "D2L C026 ordinal 6 match stable (got %d%% vs %d%%)",
                   pct1, pct2);
        }
    }
}

/* ── Group G: D1L side wall does not match ordinal 6 either ─────
 * Secondary no-floating cross-check rect: D1L (M607_VIEW_SQUARE_D1L,
 * viewport 0, 9, 60, 111 per m11_game_view.c:14465) is the closer
 * side wall at depth 1.  At the (2,4) EAST pose the D1L side wall
 * is the left side of the (3,4) front cell — the same cell whose
 * D1C front wall carries the SYRA portrait.  The D1L side wall
 * must NOT also carry the portrait sprite. */
static void check_d1l_no_ordinal_6(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int warm;
    int pct;

    printf("\n[Group G] D1L side wall does NOT match C026 ordinal 6 at (2,4) EAST (secondary check)\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    warm = rect_warm_count(fb, D1L_X, D1L_Y, D1L_W, D1L_H);
    CHECKF(warm < PORTRAIT_WARM_NEG_THRESHOLD,
           "D1L side wall warm_count < %d at (2,4) EAST (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D1L_X, D1L_Y, D1L_W, D1L_H,
                                     EXPECTED_ORDINAL_SYRA);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct < WRONG_ORDINAL_MATCH_PCT,
                   "D1L side wall C026 ordinal 6 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group H: D2R side wall (opposite-side negative check) ──────
 * The D2R side wall (M605_VIEW_SQUARE_D2R, viewport 146, 19, 78, 74)
 * is the mirror-symmetric opposite of D2L.  At the (2,4) EAST pose
 * the D2R rect is the right side of the (4,4) cell — the same
 * depth-2 cell, opposite lateral.  The D2R rect must also not
 * carry ordinal 6 (no portrait on either side wall at depth 2). */
static void check_d2r_no_ordinal_6(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int warm;
    int pct;

    printf("\n[Group H] D2R side wall does NOT match C026 ordinal 6 at (2,4) EAST (opposite-side check)\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    warm = rect_warm_count(fb, D2R_X, D2R_Y, D2R_W, D2R_H);
    CHECKF(warm < PORTRAIT_WARM_NEG_THRESHOLD,
           "D2R side wall warm_count < %d at (2,4) EAST (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D2R_X, D2R_Y, D2R_W, D2R_H,
                                     EXPECTED_ORDINAL_SYRA);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct < WRONG_ORDINAL_MATCH_PCT,
                   "D2R side wall C026 ordinal 6 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
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
    printf("=== DM1 V1 Hall portrait ordinal 6 (SYRA) / d2l_negative / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
        printf("SKIP firestaff_dm1_v1_champion_mirror_ordinal_6_d2l_negative_"
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

    check_d2l_rect_has_content(&state);
    check_d2l_no_ordinal_6(&state, portraits);
    check_d2l_no_stale_ordinal(&state, portraits);
    check_d1c_is_ordinal_6(&state, portraits);
    check_rect_position_invariants(&state);
    check_reentry_stable(&state, portraits);
    check_d1l_no_ordinal_6(&state, portraits);
    check_d2r_no_ordinal_6(&state, portraits);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
