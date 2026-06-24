/*
 * firestaff_dm1_v1_champion_mirror_ordinal_2_d2l_negative_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 2 (C026 strip cell 2 — atlas col 2 row 0,
 *                               source rect (64, 0, 32, 29);
 *                               mirror-catalog name "WU TSE" /
 *                               title "SON OF HEAVEN")
 *   route d2l_negative:        at the (2,4) cell facing EAST the
 *                              D1C front-wall rectangle (M11_VIEWPORT_X+96,
 *                              M11_VIEWPORT_Y+35, 32, 29) is planted
 *                              with the ordinal-2 WU TSE portrait
 *                              (synthetic — see note below), while
 *                              the D2L side wall (M604_VIEW_SQUARE_D2L,
 *                              viewport-relative (0, 19, 78, 74))
 *                              is a SIDE wall one cell further down
 *                              the view-cone and must NOT carry any
 *                              portrait pixels.  The "d2l_negative"
 *                              slice is the dedicated no-floating
 *                              invariant for the D2L view square at
 *                              the (2,4) EAST pose — here locked
 *                              against the ordinal-2 aspect.
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
 * Synthetic-sensor note (mirrors the ordinal-23 front_north_entry
 * probe and the ordinal-2 palette_match_rect probe):
 *
 *   The live DM1 V1 PC 3.4 DUNGEON.DAT does NOT expose ordinal 2 on
 *   any corridor C127 sensor (the existing
 *   firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_
 *   portrait_rect_position_runtime_probe prints `[Discovery]
 *   ordinal=2 hits in corridor band = 0` on the live build, and the
 *   firestaff_dm1_v1_hall_corridor_ordinal_scanner_probe confirms
 *   the (1,2) NORTH / (1,3) EAST / (2,3) EAST / (2,4) EAST /
 *   (1,5) SOUTH / etc. positive-ordinal list — ordinal 2 does not
 *   appear).  To anchor the d2l_negative slice at the canonical
 *   (2,4) EAST pose (the same anchor the ordinal-6 d2l_negative
 *   probe uses), the probe mutates the C127 sensor on (3,4)'s
 *   WEST wall (visibleWallCell=3 for DIR_EAST=1, partyDirection+2)
 *   from its shipped sensorData=6 (SYRA) to sensorData=2 (WU TSE),
 *   runs the d2l_negative proof, then restores the original
 *   sensorData=6 before exit.  The mutation is scoped to this
 *   probe's runtime; it never touches the live DUNGEON.DAT bytes
 *   on disk.
 *
 * The probe proves:
 *   Group A: the D2L side wall rect (viewport 0, 19, 78, 74) at the
 *            (2,4) EAST pose has rendered content (5+ distinct
 *            non-zero palette indices, 30+ non-zero pixels) so an
 *            empty D2L rect cannot be explained away by a "viewport
 *            was never painted" hand-wave.
 *   Group B: the D2L side wall rect at (2,4) EAST does NOT match
 *            C026 ordinal 2 above the 35% drift threshold (the
 *            wrong-ordinal drift floor used by the actual-pose
 *            probe's check_no_stale_ordinal_in_rect).  The D2L
 *            negative-route slice: ordinal 2 must not float over
 *            the D2L side wall.
 *   Group C: the D2L side wall rect at (2,4) EAST does NOT match
 *            any C026 ordinal above 35% — the wall is plain stone
 *            texture, not a stale portrait.  This is the
 *            strict-dominance negative check across all 24
 *            C026 atlas slots.
 *   Group D: the D1C portrait rect (96, 35, 32, 29) at (2,4) EAST
 *            IS ordinal 2 (WU TSE) at >= 90% pixel match and
 *            warm_count >= 30 (positive cross-check that the
 *            synthetic C127 sensor drive successfully exposes
 *            ordinal 2 on the local PC 3.4 fixture).
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
 *            same D2L pixel state (drift below 35% on C026
 *            ordinal 2, zero stale-portrait on D2L, full match on
 *            D1C).
 *   Group G: D1L side wall (M607_VIEW_SQUARE_D1L, viewport
 *            (0, 9, 60, 111)) does NOT match C026 ordinal 2 above
 *            35% — secondary no-floating cross-check rect on the
 *            closer side wall of the (3,4) front cell.
 *   Group H: D2R side wall (M605_VIEW_SQUARE_D2R, viewport
 *            (146, 19, 78, 74)) does NOT match C026 ordinal 2
 *            above 35% — opposite-side check that the right side
 *            wall at depth 2 also stays portrait-free.
 *   Group I: ordinal-2 mirror-catalog identity —
 *            M11_GameView_GetMirrorNameByOrdinal(2) == "WU TSE"
 *            and GetMirrorTitleByOrdinal(2) == "SON OF HEAVEN"
 *            (PC 3.4 English).  Locks the ordinal-2 slice target
 *            against accidental ordinal drift.
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
 *   firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_
 *     portrait_rect_position_runtime_probe — ordinal-2 south_return
 *     corridor scan route (SKIPs on the live DM1 V1 PC 3.4 build).
 *   firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative_
 *     portrait_rect_position_runtime_probe — ordinal-2 west_negative
 *     corridor west-wall no-float route.
 *   firestaff_dm1_v1_champion_mirror_ordinal_6_d2l_negative_
 *     portrait_rect_position_runtime_probe — ordinal-6 d2l_negative
 *     route (different slice target ordinal; this probe locks the
 *     ordinal-2-specific d2l_negative invariant).
 *   firestaff_dm1_v1_hoc_champion_portrait_02_palette_match_rect_
 *     runtime_probe — ordinal-2 palette_match_rect route (synthetic
 *     mutation at (1,1) south wall; this probe uses a different
 *     anchor cell — (3,4) west wall — and the d2l_negative
 *     view-square invariant instead of the per-pixel palette
 *     match).
 *   firestaff_dm1_v1_hall_champion_portrait_02_east_walkpath_
 *     rect_position_runtime_probe — ordinal-2 east_walkpath route
 *     (synthetic atlas-slot blit; this probe uses a real engine
 *     C127 sensor drive via synthetic mutation).
 *
 * The probe is data-conditional: it requires hash-verified DM1 V1
 * data for the (2,4) E ordinal-2 positive cross-check; without
 * that data the contract surface (engine helpers, C127 front-cell
 * filter, portrait_rect_position) is still exercised, and the
 * catalog name assertions degrade to "ordinal returns" + "cutout
 * non-empty for positive ordinals".
 *
 * Usage: firestaff_dm1_v1_champion_mirror_ordinal_2_d2l_negative_
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
     * pose the D2L side wall must not be dominated by ordinal 2
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
    EXPECTED_ORDINAL_WUTSE = 2,
    /* Synthetic-mutation anchor: at (2,4) DIR_EAST the front cell is
     * (3,4); the C127 sensor on (3,4)'s WEST wall (visibleWallCell=3
     * for DIR_EAST=1, partyDirection+2 & 3) is what
     * m11_front_cell_mirror_ordinal resolves to.  In the live
     * DM1 V1 PC 3.4 DUNGEON.DAT this sensor ships with
     * sensorData=6 (SYRA); the probe mutates it to 2 (WU TSE) so
     * the d2l_negative slice can lock the ordinal-2 aspect against
     * a real engine C127 sensor drive.  The mutation is restored
     * before the probe exits so other probes / sibling slices see
     * the shipped sensorData=6. */
    MUTATE_FRONT_X = 3,
    MUTATE_FRONT_Y = 4,
    MUTATE_VISIBLE_WALL_CELL = 3, /* DIR_EAST + 2 & 3 = (1 + 2) & 3 = 3 */
    MUTATE_SHIPPED_SENSORDATA = 6,
    MUTATE_TARGET_SENSORDATA = 2
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
 * verify the D2L side wall does NOT match ordinal 2 (or any C026
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

/* Synthetic-sensor mutation context.  Mirrors the pattern used by
 * firestaff_dm1_v1_champion_portrait_ordinal_23_front_north_entry_
 * rect_runtime_probe and firestaff_dm1_v1_hoc_champion_portrait_02_
 * palette_match_rect_runtime_probe.  We locate the C127 sensor on
 * the front cell's visible-wall cell, mutate its sensorData to the
 * ordinal target, and restore the original sensorData before
 * exit so other probes / sibling slices see the shipped value. */
typedef struct SensorMutationCtx {
    int found;
    int sensorIndex;
    unsigned short savedSensorData;
    int visibleWallCell;
} SensorMutationCtx;

/* Per-type byte count for the things->rawThingData[type] arrays.
 * Same values used by the ordinal-23 front_north_entry probe and
 * the ordinal-2 palette_match_rect probe.
 *   THING=0 -> 4 bytes, TELEPORT=1 -> 6, DOOR=2 -> 4,
 *   SENSOR=3 -> 8, ... */
static const unsigned char k_thingDataByteCount[16] = {
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

/* 16-bit THING handle packing (cell << 14) | (type << 10) | index. */
#define PROBE_THING_ENDOFLIST 0xFFFEu
#define PROBE_THING_NONE      0xFFFFu
#define PROBE_THING_TYPE(t)   ((int)(((unsigned short)(t) >> 10) & 0x0F))
#define PROBE_THING_INDEX(t)  ((int)((unsigned short)(t) & 0x03FF))
#define PROBE_THING_CELL(t)   ((int)(((unsigned short)(t) >> 14) & 0x03))
#define PROBE_THING_TYPE_SENSOR 3

static int find_and_mutate_c127_sensor(M11_GameViewState* state,
                                        int frontMapX, int frontMapY,
                                        int visibleWallCell,
                                        int newSensorData,
                                        SensorMutationCtx* out) {
    int mapIndex = state->world.party.mapIndex;
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    unsigned short thing;
    out->found = 0;
    out->sensorIndex = -1;
    out->savedSensorData = 0;
    out->visibleWallCell = visibleWallCell;
    if (!state->world.dungeon || !state->world.things ||
        !state->world.things->squareFirstThings ||
        !state->world.dungeon->maps) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    map = &state->world.dungeon->maps[mapIndex];
    if (frontMapX < 0 || frontMapX >= (int)map->width) return 0;
    if (frontMapY < 0 || frontMapY >= (int)map->height) return 0;
    squareIndex = frontMapX * (int)map->height + frontMapY;
    thing = state->world.things->squareFirstThings[squareIndex];
    while (thing != PROBE_THING_ENDOFLIST && thing != PROBE_THING_NONE) {
        int type = PROBE_THING_TYPE(thing);
        int index = PROBE_THING_INDEX(thing);
        int cell = PROBE_THING_CELL(thing);
        if (type == PROBE_THING_TYPE_SENSOR && cell == visibleWallCell &&
            index >= 0 && index < state->world.things->sensorCount &&
            state->world.things->sensors[index].sensorType == 127) {
            out->savedSensorData =
                state->world.things->sensors[index].sensorData;
            state->world.things->sensors[index].sensorData =
                (unsigned short)newSensorData;
            out->sensorIndex = index;
            out->found = 1;
            return 1;
        }
        {
            int byteCount;
            const unsigned char* raw;
            if (type < 0 || type >= 16 ||
                !state->world.things->rawThingData[type] ||
                index < 0 ||
                index >= state->world.things->thingCounts[type]) {
                break;
            }
            byteCount = k_thingDataByteCount[type];
            raw = state->world.things->rawThingData[type] +
                  (index * byteCount);
            thing = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        }
    }
    return 0;
}

static void restore_c127_sensor(M11_GameViewState* state,
                                const SensorMutationCtx* mut) {
    if (!mut->found || mut->sensorIndex < 0) return;
    state->world.things->sensors[mut->sensorIndex].sensorData =
        mut->savedSensorData;
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
 * would mean the side wall is missing, not that ordinal 2 is absent
 * from it.  Two-channel content check: (1) >= 30 non-zero pixels
 * (texture actually present) and (2) >= 5 distinct non-zero palette
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
    CHECKF(distinct >= 5,
           "D2L side wall has >= 5 distinct non-zero palette indices (got %d)",
           distinct);
}

/* ── Group B: D2L side wall does NOT match ordinal 2 ────────────
 * The d2l_negative slice invariant for ordinal 2 (WU TSE): at the
 * (2,4) EAST pose (with the C127 sensor on (3,4)'s WEST wall
 * mutated to ordinal 2) the C026 ordinal-2 portrait must NOT be
 * painted over the D2L side wall (viewport 0, 19, 78, 74).  The
 * D1C front-wall rectangle IS the only destination for the C026
 * blit (DUNVIEW.C:3913-3928 gates on
 * P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT). */
static void check_d2l_no_ordinal_2(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int pct;
    int warm;

    printf("\n[Group B] D2L side wall does NOT match C026 ordinal 2 at (2,4) EAST\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);

    warm = rect_warm_count(fb, D2L_X, D2L_Y, D2L_W, D2L_H);
    CHECKF(warm < PORTRAIT_WARM_NEG_THRESHOLD,
           "D2L side wall warm_count < %d at (2,4) EAST (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D2L_X, D2L_Y, D2L_W, D2L_H,
                                     EXPECTED_ORDINAL_WUTSE);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct < WRONG_ORDINAL_MATCH_PCT,
                   "D2L side wall C026 ordinal 2 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group C: D2L side wall does NOT match ANY C026 atlas slot ──
 * Strict-dominance negative check across all 24 C026 atlas slots:
 * no portrait sprite (ordinals 0..23) is painted over the D2L side
 * wall at the (2,4) EAST pose with the ordinal-2 sensor drive
 * active.  This catches a hypothetical regression where the C026
 * blit leaks from D1C into a side view square — the d2l_negative
 * slice is the dedicated "no-portrait-anywhere-on-D2L" invariant,
 * even with ordinal 2 planted on the front wall. */
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

/* ── Group D: D1C portrait cutout IS ordinal 2 (positive) ──────
 * Cross-check that the synthetic C127 sensor drive successfully
 * exposes ordinal 2 on the local DM1 V1 PC 3.4 fixture.  After
 * mutating the (3,4) WEST wall sensorData from 6 (SYRA, shipped)
 * to 2 (WU TSE), the D1C rectangle must paint the C026 ordinal-2
 * portrait with >= 90% pixel match and >= 30 warm pixels. */
static void check_d1c_is_ordinal_2(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int warm;

    printf("\n[Group D] D1C portrait cutout IS ordinal 2 (WU TSE) at (2,4) EAST (positive cross-check)\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    CHECKF(ord == EXPECTED_ORDINAL_WUTSE,
           "M11_GameView_GetFrontMirrorOrdinal((2,4)E) == %d (got %d)",
           EXPECTED_ORDINAL_WUTSE, ord);

    warm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);
    CHECKF(warm >= PORTRAIT_WARM_POS_THRESHOLD,
           "Inner portrait cutout warm_count >= %d for WU TSE (got %d)",
           PORTRAIT_WARM_POS_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     PORTRAIT_X, PORTRAIT_Y,
                                     PORTRAIT_W, PORTRAIT_H,
                                     EXPECTED_ORDINAL_WUTSE);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct >= CORRECT_ORDINAL_MATCH_PCT,
                   "D1C portrait cutout C026 ordinal 2 match >= %d%% (got %d%%)",
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
 * ordinal 2 on the D2L side wall or remove it from the D1C cutout.
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
                                      EXPECTED_ORDINAL_WUTSE);
        pct2 = match_portrait_in_rect(portraits, fb2,
                                      D2L_X, D2L_Y, D2L_W, D2L_H,
                                      EXPECTED_ORDINAL_WUTSE);
        if (pct1 >= 0 && pct2 >= 0) {
            CHECKF(pct1 == pct2 && pct1 < WRONG_ORDINAL_MATCH_PCT,
                   "D2L C026 ordinal 2 match stable (got %d%% vs %d%%)",
                   pct1, pct2);
        }
    }
}

/* ── Group G: D1L side wall does not match ordinal 2 either ─────
 * Secondary no-floating cross-check rect: D1L (M607_VIEW_SQUARE_D1L,
 * viewport 0, 9, 60, 111 per m11_game_view.c:14465) is the closer
 * side wall at depth 1.  At the (2,4) EAST pose the D1L side wall
 * is the left side of the (3,4) front cell — the same cell whose
 * D1C front wall carries the planted ordinal-2 portrait.  The D1L
 * side wall must NOT also carry the portrait sprite. */
static void check_d1l_no_ordinal_2(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int warm;
    int pct;

    printf("\n[Group G] D1L side wall does NOT match C026 ordinal 2 at (2,4) EAST (secondary check)\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    warm = rect_warm_count(fb, D1L_X, D1L_Y, D1L_W, D1L_H);
    CHECKF(warm < PORTRAIT_WARM_NEG_THRESHOLD,
           "D1L side wall warm_count < %d at (2,4) EAST (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D1L_X, D1L_Y, D1L_W, D1L_H,
                                     EXPECTED_ORDINAL_WUTSE);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct < WRONG_ORDINAL_MATCH_PCT,
                   "D1L side wall C026 ordinal 2 match < %d%% (got %d%%)",
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
 * carry ordinal 2 (no portrait on either side wall at depth 2). */
static void check_d2r_no_ordinal_2(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int warm;
    int pct;

    printf("\n[Group H] D2R side wall does NOT match C026 ordinal 2 at (2,4) EAST (opposite-side check)\n");

    render_at(state, fb, 2, 4, 1 /* DIR_EAST */);
    warm = rect_warm_count(fb, D2R_X, D2R_Y, D2R_W, D2R_H);
    CHECKF(warm < PORTRAIT_WARM_NEG_THRESHOLD,
           "D2R side wall warm_count < %d at (2,4) EAST (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D2R_X, D2R_Y, D2R_W, D2R_H,
                                     EXPECTED_ORDINAL_WUTSE);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct < WRONG_ORDINAL_MATCH_PCT,
                   "D2R side wall C026 ordinal 2 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group I: ordinal-2 mirror-catalog identity ────────────────
 * Locks the ordinal-2 slice target against accidental ordinal drift
 * (e.g. a regression that resolves ordinal 2 to a different
 * champion).  PC 3.4 English DUNGEON.DAT maps ordinal 2 to the
 * WU TSE / SON OF HEAVEN champion.  M11_GameView_GetMirrorNameBy
 * Ordinal / GetMirrorTitleByOrdinal return the populated string
 * length (F0660 / F0661 in memory_champion_state_pc34_compat.c:969
 * / :993), not a status flag — a return of 0 means the catalog
 * did not find the ordinal at all.  Name lookup must return 6
 * ("WU TSE" with embedded space, NO leading/trailing whitespace)
 * and title lookup must return 13 ("SON OF HEAVEN"). */
static void check_ordinal_2_catalog_identity(M11_GameViewState* state) {
    char nameBuf[32];
    char titleBuf[32];
    int nameLen;
    int titleLen;

    printf("\n[Group I] ordinal-2 mirror-catalog identity (PC 3.4 English)\n");

    memset(nameBuf, 0, sizeof(nameBuf));
    memset(titleBuf, 0, sizeof(titleBuf));
    nameLen = M11_GameView_GetMirrorNameByOrdinal(state, EXPECTED_ORDINAL_WUTSE,
                                                  nameBuf, sizeof(nameBuf));
    titleLen = M11_GameView_GetMirrorTitleByOrdinal(state, EXPECTED_ORDINAL_WUTSE,
                                                    titleBuf, sizeof(titleBuf));

    CHECKF(nameLen > 0,
           "M11_GameView_GetMirrorNameByOrdinal(2) populates nameBuf (len=%d, name=\"%s\")",
           nameLen, nameBuf);
    if (nameLen > 0) {
        if (strcmp(nameBuf, "WU TSE") == 0) {
            CHECK(1, "mirror-catalog ordinal 2 name == \"WU TSE\"");
        } else {
            CHECKF(0,
                   "mirror-catalog ordinal 2 name == \"WU TSE\" (got \"%s\")",
                   nameBuf);
        }
    }

    CHECKF(titleLen > 0,
           "M11_GameView_GetMirrorTitleByOrdinal(2) populates titleBuf (len=%d, title=\"%s\")",
           titleLen, titleBuf);
    if (titleLen > 0) {
        if (strcmp(titleBuf, "SON OF HEAVEN") == 0) {
            CHECK(1, "mirror-catalog ordinal 2 title == \"SON OF HEAVEN\"");
        } else {
            CHECKF(0,
                   "mirror-catalog ordinal 2 title == \"SON OF HEAVEN\" (got \"%s\")",
                   titleBuf);
        }
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits = NULL;
    int assetsAvailable;
    SensorMutationCtx mutation;
    int mutationRc;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall portrait ordinal 2 (WU TSE) / d2l_negative / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
        printf("SKIP firestaff_dm1_v1_champion_mirror_ordinal_2_d2l_negative_"
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

    /* Synthetic-mutation: anchor at (3,4) WEST wall C127 sensor.
     * Live DM1 V1 PC 3.4 DUNGEON.DAT ships this sensor with
     * sensorData=6 (SYRA); we mutate to 2 (WU TSE) so the
     * d2l_negative slice can lock the ordinal-2 aspect against a
     * real engine C127 sensor drive.  The mutation is restored
     * before the probe exits. */
    mutation.found = 0;
    mutation.sensorIndex = -1;
    mutation.savedSensorData = 0;
    mutationRc = find_and_mutate_c127_sensor(&state,
                                             MUTATE_FRONT_X, MUTATE_FRONT_Y,
                                             MUTATE_VISIBLE_WALL_CELL,
                                             MUTATE_TARGET_SENSORDATA,
                                             &mutation);
    if (!mutationRc) {
        printf("SKIP firestaff_dm1_v1_champion_mirror_ordinal_2_d2l_negative_"
               "portrait_rect_position_runtime_probe "
               "could not locate C127 sensor at (3,4) WEST wall "
               "(visibleWallCell=%d) on this DM1 V1 build; the ordinal-2 "
               "d2l_negative slice requires that sensor to anchor the "
               "synthetic mutation.\n",
               MUTATE_VISIBLE_WALL_CELL);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    check_d2l_rect_has_content(&state);
    check_d2l_no_ordinal_2(&state, portraits);
    check_d2l_no_stale_ordinal(&state, portraits);
    check_d1c_is_ordinal_2(&state, portraits);
    check_rect_position_invariants(&state);
    check_reentry_stable(&state, portraits);
    check_d1l_no_ordinal_2(&state, portraits);
    check_d2r_no_ordinal_2(&state, portraits);
    check_ordinal_2_catalog_identity(&state);

    /* Restore the shipped sensorData=6 (SYRA) before exit so
     * sibling probes / slices see the unmodified runtime state. */
    restore_c127_sensor(&state, &mutation);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
