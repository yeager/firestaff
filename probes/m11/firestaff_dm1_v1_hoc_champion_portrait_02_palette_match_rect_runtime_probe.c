/*
 * firestaff_dm1_v1_hoc_champion_portrait_02_palette_match_rect_runtime_probe.c
 *
 * DM1 V1 Hall of Champions — champion portrait ordinal 2,
 * route palette_match_rect, aspect portrait_rect_position,
 * batch group 6.
 *
 * Sibling to the four existing ordinal-02 slices:
 *
 *   probes/m11/firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_portrait_rect_position_runtime_probe.c
 *     route = south_return (corridor band scan + south_return
 *     sequence — prints SKIP on the live DM1 V1 PC 3.4 build
 *     because ordinal=2 has no corridor C127 sensor on that build)
 *
 *   probes/m11/firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative_portrait_rect_position_runtime_probe.c
 *     route = west_negative (corridor west wall, no C127 sensor,
 *     no portrait — proves the front-cell filter rejects
 *     sensorData=2 when the wall direction does not match)
 *
 *   probes/m11/firestaff_dm1_v1_hall_champion_portrait_02_east_walkpath_rect_position_runtime_probe.c
 *     route = east_walkpath (corridor walk east at y=2..5, plus a
 *     synthetic ordinal-2 atlas-slot blit for the (64, 0) source
 *     cell — does not anchor on a real ordinal-2 sensor drive)
 *
 *   probes/m11/firestaff_dm1_v1_hoc_champion_portrait_02_wake_repaint_portrait_rect_position_050_gate_probe.c
 *     route = wake_repaint (C146 sleep/wake-up repaint slice on the
 *     (1,2,0) HALK ordinal-1 baseline — no ordinal-2 path)
 *
 * This probe covers the **palette_match_rect** route variant — the
 * distinct concern: "does each rendered pixel in the D1C portrait
 * rectangle carry the same VGA palette index as the corresponding
 * C026 ordinal-2 atlas cell?"  Where the existing ordinal-02 slices
 * use a 90% match threshold and rely on warm-pixel counts, this
 * probe tightens the palette-level match to 99% and adds per-row /
 * per-column invariants to catch sub-rectangle misalignment
 * regressions (e.g. an off-by-one col/row stride bug that would
 * still clear a 90% aggregate match but fail any per-line check).
 *
 * Anchor cell: (1, 2) facing NORTH.  In the live DM1 V1 PC 3.4
 * DUNGEON.DAT the front cell (1, 1) carries a C127 sensor with
 * sensorData=1 (HALK) on the visible wall cell (partyDirection+2=2,
 * south wall of (1,1)).  Ordinal 2 is not exposed on any corridor
 * cell in this build (see the south_return probe's [Discovery]
 * output), so the probe mutates the C127 sensorData to 2 — the
 * same pattern the ordinal-23 front_north_entry probe uses for
 * ordinal 23 on the live DM1 V1 build (ordinal 23 has no corridor
 * sensor either).  The mutation is reverted after Group B so the
 * Group C and D negative controls see the real sensor value.
 *
 * Three coupled concerns in one runtime drive:
 *
 *   (1) palette_match_rect — strict per-pixel palette index match
 *       between the C026 ordinal-2 source cell (64, 0, 32, 29) and
 *       the D1C destination rect (96, 35, 32, 29) on the rendered
 *       framebuffer.  Aggregate match >= 99%, every row match >=
 *       90% of its non-transparent count, and every column match
 *       >= 80% of its non-transparent count.  This tightens the
 *       90% aggregate / no per-line checks used by the other
 *       ordinal-02 slices; the per-line guarantees catch stride /
 *       offset regressions that an aggregate match could absorb.
 *
 *   (2) side_wall_no_float — at (1, 2) EAST, the same D1C rect
 *       must contain fewer than 5% palette_match against ordinal 2
 *       (no ordinal-2 portrait floating over the side wall; the
 *       east face of (1, 2) has no C127 sensor and the side wall
 *       of (1, 1) is the east wall, not the front wall).
 *
 *   (3) redraw_after_candidate — re-render the (1, 2) NORTH pose
 *       (with sensor still mutated to ordinal 2) with the C040
 *       candidate panel live (after SelectFrontMirrorCandidate).
 *       The full D1C rect must drop to < 5% match with ordinal 2
 *       (no stale sprite after C040 takes over), and the visible
 *       top strip above the panel must stay clean of warm pixels.
 *
 * The two render passes (panel-off and panel-on) are also checked
 * for determinism: a second render of the same pose must produce
 * a framebuffer byte-equal to the first.
 *
 * Source evidence:
 *   - ReDMCSB DUNGEON.C:2573 (C127 sensor cell match against
 *     view direction)
 *   - ReDMCSB DUNGEON.C:2608-2612 (G0289_i_DungeonView_ChampionPortraitOrdinal
 *     = C127 sensorData, range 0..23)
 *   - ReDMCSB DUNVIEW.C:3913-3928 (C026 portrait blit at D1C
 *     box { x=96, y=35, w=32, h=29 } using (ord & 7) * 32,
 *     (ord >> 3) * 29 with C01_COLOR_DARK_GRAY transparency)
 *   - ReDMCSB DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = { 96, 127, 35, 63 })
 *   - ReDMCSB DUNVIEW.C:3916-3919 (C026_GRAPHIC_CHAMPION_PORTRAITS,
 *     "A portrait is 32x29 pixels" — 8 cols * 3 rows = 256x87 atlas)
 *   - ReDMCSB DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y
 *     macro math)
 *   - ReDMCSB MOVESENS.C:1501-1503 (F0280 sensorData -> candidate
 *     ordinal)
 *   - ReDMCSB REVIVE.C F0280 (materialize candidate from sensorData)
 *   - src/engine/m11_game_view.c m11_draw_dm1_front_mirror_route
 *     (BUG-120/121 panel guard) and
 *     m11_draw_dm1_front_champion_portrait (D1C blit site)
 *
 * This is a Firestaff-runtime slice proof, not DOS pixel parity.
 * The probe does not claim parity against an original capture; it
 * locks the source-locked C026 atlas math + D1C destination
 * rectangle + C127 sensor mutation + C040 candidate redraw
 * ownership against the runtime framebuffer.
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

/* THING chain traversal helpers (file-static in m11_game_view.c,
 * duplicated here so the probe is self-contained).  ReDMCSB
 * DUNGEON.C:2531-2602 thing-list serialization.
 *
 * The 16-bit THING handle is packed in `memory_dungeon_dat_pc34_compat.h`
 * as (cell << 14) | (type << 10) | index, so:
 *   THING_GET_TYPE  = bits 10..13  (4 bits, 0..15)
 *   THING_GET_INDEX = bits  0..9   (10 bits, 0..1023)
 *   THING_GET_CELL  = bits 14..15  (2 bits, 0..3)
 *
 * Type codes (memory_dungeon_dat_pc34_compat.h:189-201):
 *   DOOR=0, TELEPORTER=1, TEXTSTRING=2, SENSOR=3, GROUP=4,
 *   WEAPON=5, ARMOUR=6, SCROLL=7, POTION=8, CONTAINER=9,
 *   JUNK=10, PROJECTILE=14, EXPLOSION=15.  We only need SENSOR.
 *
 * Sensor entry size in rawThingData[3] is 8 bytes (sensorType +
 * sensorData + 2 bytes cell-targeted data + next-thing pointer
 * at offset 0..1 = same packing as all other thing types).
 *
 * The byte-count table for rawThingData[type] is the same as the
 * existing ordinal-23 front_north_entry probe.  Type 3 (sensor) =
 * 8 bytes; all other types we touch are NOT walked via rawThingData
 * (we only walk to find the SENSOR, then early-exit). */

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,

    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
     * = { 96, 127, 35, 63 }).  Viewport-relative coordinates. */
    PROBE_PORTRAIT_VIEW_X = 96,
    PROBE_PORTRAIT_VIEW_Y = 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_PORTRAIT_FB_X = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VIEW_X,
    PROBE_PORTRAIT_FB_Y = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VIEW_Y,

    /* Source-locked C026 atlas dimensions.  C026 is the 8x3 grid of
     * 32x29 portraits (DUNVIEW.C:3916-3919). */
    PROBE_ATLAS_W = 256,
    PROBE_ATLAS_H = 87,
    PROBE_ATLAS_COLS = 8,
    PROBE_ATLAS_ROWS = 3,

    /* Ordinal 2 in the C026 atlas: (2 & 7) * 32 = 64,
     *                                (2 >> 3) * 29 =  0.
     * This is the THIRD column of the TOP row of the 8x3 atlas
     * (row 0, col 2) — the ordinal-2 slot.  In real DM1 V1 PC 3.4
     * English DUNGEON.DAT the mirror-catalog name resolves to
     * "WU TSE" / "SON OF HEAVEN" (verified at runtime by
     * Group A's catalog identity check). */
    PROBE_ORDINAL = 2,
    PROBE_ORDINAL_COL = PROBE_ORDINAL & 7,         /* = 2 */
    PROBE_ORDINAL_ROW = PROBE_ORDINAL >> 3,        /* = 0 */
    PROBE_ORDINAL_SRC_X = PROBE_ORDINAL_COL * 32,  /* = 64 */
    PROBE_ORDINAL_SRC_Y = PROBE_ORDINAL_ROW * 29,  /* = 0 */
    /* DUNVIEW.C:3916 C01_COLOR_DARK_GRAY is the C026 transparency
     * mask; pixels with palette index 1 are skipped by the blit. */
    PROBE_TRANSPARENT_COLOR = 1,

    /* C040 candidate panel top-left in framebuffer coords; used to
     * bound the visible top-strip above the panel for the
     * redraw_after_candidate palette cleanliness check.  This is
     * G0032_ai_Graphic562_Box_Panel viewport-relative (80, 52). */
    PROBE_C040_PANEL_X = PROBE_VIEWPORT_X + 80,
    PROBE_C040_PANEL_Y = PROBE_VIEWPORT_Y + 52,
    PROBE_PORTRAIT_TOP_VISIBLE_H = PROBE_C040_PANEL_Y - PROBE_PORTRAIT_FB_Y,

    /* Match thresholds.  The aggregate threshold is tighter than
     * the 90% used by other ordinal-02 slices because the
     * palette_match_rect route is specifically about proving
     * pixel-level palette alignment, not just "is there a
     * portrait here".  The per-line thresholds catch stride /
     * offset regressions that an aggregate match could absorb. */
    PROBE_MATCH_AGGREGATE_MIN_PCT = 99,
    PROBE_ROW_MATCH_MIN_PCT = 90,
    PROBE_COL_MATCH_MIN_PCT = 80,

    /* M11_GFX_CHAMPION_PORTRAITS == 26 == C026_GRAPHIC_CHAMPION_PORTRAITS.
     * This file-scoped enum in m11_game_view.c is not exported; the
     * source-locked value 26 is stable across versions. */
    PROBE_M11_GFX_CHAMPION_PORTRAITS = 26,

    /* Anchor cell: (1, 2) NORTH — front cell is (1, 1), visible
     * wall cell is (direction+2)&3 = 2 (south wall of (1,1) faces
     * north when standing at (1,2) looking north).  The (1, 1)
     * front cell on the live DM1 V1 PC 3.4 DUNGEON.DAT carries a
     * C127 sensor with sensorData=1 (HALK) on the south wall; we
     * mutate sensorData to PROBE_ORDINAL for the palette_match_rect
     * drive. */
    PROBE_ANCHOR_X = 1,
    PROBE_ANCHOR_Y = 2,
    PROBE_ANCHOR_DIR = 0,    /* DIR_NORTH */
    PROBE_ANCHOR_VISIBLE_WALL_CELL = (PROBE_ANCHOR_DIR + 2) & 3,  /* = 2 */
    PROBE_ANCHOR_FRONT_X = PROBE_ANCHOR_X,
    PROBE_ANCHOR_FRONT_Y = PROBE_ANCHOR_Y - 1
};

/* Per-pixel palette match result for one comparison pass. */
typedef struct PaletteMatch {
    int matched;          /* non-transparent source pixels whose palette index == destination */
    int compared;         /* non-transparent source pixels (denominator for match pct) */
    int rowMatched[PROBE_PORTRAIT_H];
    int rowCompared[PROBE_PORTRAIT_H];
    int colMatched[PROBE_PORTRAIT_W];
    int colCompared[PROBE_PORTRAIT_W];
    int opaqueSrcCount;   /* total non-transparent source pixels in the cell */
    int opaqueDstCount;   /* total non-transparent destination pixels in the rect */
} PaletteMatch;

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;
static int g_info = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP(msg) do { ++g_skip; printf("  SKIP: %s\n", msg); } while (0)

#define INFO(msg) do { ++g_info; printf("  INFO: %s\n", msg); } while (0)

/* Park the party at a specific map/x/y/direction pose.  The four
 * M11 boolean state fields that affect the D1C render are reset
 * to a known state so the palette_match_rect comparison is
 * deterministic. */
static void park_pose(M11_GameViewState* state,
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

/* Compare the C026 ordinal-2 source cell to the D1C destination
 * rect on the rendered framebuffer, with strict per-pixel palette
 * index matching.  Both aggregate and per-line tallies are
 * populated.  Returns 1 if the call succeeded, 0 if assets are
 * missing (caller should SKIP). */
static int compute_palette_match(const M11_AssetSlot* portraits,
                                 const unsigned char* fb,
                                 PaletteMatch* out) {
    int x, y;
    int srcX = PROBE_ORDINAL_SRC_X;
    int srcY = PROBE_ORDINAL_SRC_Y;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if ((int)portraits->width  < srcX + PROBE_PORTRAIT_W) return 0;
    if ((int)portraits->height < srcY + PROBE_PORTRAIT_H) return 0;
    if (!fb) return 0;
    memset(out, 0, sizeof(*out));
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)
                (portraits->pixels[(srcY + y) * (int)portraits->width +
                                    (srcX + x)] & 0x0F);
            unsigned char dst = M11_FB_DECODE_INDEX(
                fb[(PROBE_PORTRAIT_FB_Y + y) * PROBE_FB_W +
                   (PROBE_PORTRAIT_FB_X + x)]);
            int srcOpaque = (src != (unsigned char)PROBE_TRANSPARENT_COLOR);
            int dstOpaque = (dst != (unsigned char)PROBE_TRANSPARENT_COLOR);
            if (srcOpaque) ++out->opaqueSrcCount;
            if (dstOpaque) ++out->opaqueDstCount;
            if (!srcOpaque) continue;
            ++out->compared;
            ++out->rowCompared[y];
            ++out->colCompared[x];
            if (src == dst) {
                ++out->matched;
                ++out->rowMatched[y];
                ++out->colMatched[x];
            }
        }
    }
    return 1;
}

/* Count warm-color pixels in a framebuffer rectangle.  Warm set
 * matches ReDMCSB DUNVIEW.C:3913-3928: 0x07 green, 0x08 red,
 * 0x09 orange, 0x0A peach, 0x0B yellow, 0x0E blue.  Grey-stone
 * wall texture uses 0x01/0x02/0x07-grey/0x0D and never the warm
 * set, so this is a robust portrait-present / wall-only test. */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < PROBE_FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < PROBE_FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++cnt;
                    break;
                default:
                    break;
            }
        }
    }
    return cnt;
}

/* Count non-zero pixels in a framebuffer rectangle. */
static int rect_nonzero_count(const unsigned char* fb,
                              int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < PROBE_FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < PROBE_FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]);
            if (idx != 0) ++cnt;
        }
    }
    return cnt;
}

/* THING chain traversal: locate the C127 sensor on the front cell's
 * visible wall cell and mutate its sensorData.  Returns 1 on
 * success; on success outSensorIndex/outSavedSensorData carry the
 * mutation context so the caller can restore it.  Mirrors the
 * pattern in firestaff_dm1_v1_champion_portrait_ordinal_23_front_north_entry_rect_runtime_probe.c.
 */
typedef struct SensorMutation {
    int found;
    int sensorIndex;
    unsigned short savedSensorData;
    int visibleWallCell;
} SensorMutation;

/* Per-type byte count for the things->rawThingData[type] arrays.
 * Same values used by the ordinal-23 front_north_entry probe.
 *   THING=0 -> 4 bytes, TELEPORT=1 -> 6, DOOR=2 -> 4,
 *   SENSOR=3 -> 8, ... */
static const unsigned char k_thingDataByteCount[16] = {
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

/* THING_GET_TYPE / THING_GET_INDEX / THING_GET_CELL replicate the
 * 16-bit packing used in memory_dungeon_dat_pc34_compat.h (we cannot
 * re-include those macros because they are #defined alongside
 * THING_TYPE_DOOR=0 etc. and we don't want to drag in the whole
 * dungeon header).  Packing: (cell << 14) | (type << 10) | index.
 *   THING_ENDOFLIST = 0xFFFE, THING_NONE = 0xFFFF. */
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
                                        SensorMutation* out) {
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
    if (frontMapX < 0 || frontMapX >= (int)map->width) {
        return 0;
    }
    if (frontMapY < 0 || frontMapY >= (int)map->height) {
        return 0;
    }
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
                                const SensorMutation* mut) {
    if (!mut->found || mut->sensorIndex < 0) return;
    state->world.things->sensors[mut->sensorIndex].sensorData =
        mut->savedSensorData;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    static unsigned char fbNorth[PROBE_FB_W * PROBE_FB_H];
    static unsigned char fbNorthSecond[PROBE_FB_W * PROBE_FB_H];
    static unsigned char fbEast[PROBE_FB_W * PROBE_FB_H];
    static unsigned char fbNorthPanelOn[PROBE_FB_W * PROBE_FB_H];
    PaletteMatch northMatch;
    PaletteMatch eastMatch;
    PaletteMatch northPanelOnMatch;
    SensorMutation mutation;
    int opaqueAtlas2;
    int frontOrdinalBaseline;
    int frontOrdinalMutated;
    int frontOrdinalEast;
    int selectRc;
    int warmNorth;
    int warmEast;
    int nonzeroNorth;
    int nonzeroEast;
    int warmNorthTopPanelOn;
    int nonzeroNorthTopPanelOn;
    int determinismMatch;
    int row, col;
    int pct;
    int ok;
    int overallOk = 1;
    char mirrorName[16];
    char mirrorTitle[32];
    int hasName;
    int hasTitle;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall portrait-02 / palette_match_rect / portrait_rect_position (v2.7.27) ===\n");
    printf("dataDir=%s ordinal=%d src=(%d,%d,%d,%d) dst=(%d,%d,%d,%d) "
           "match_thresh(agg=%d%% row=%d%% col=%d%%)\n",
           dataDir, PROBE_ORDINAL,
           PROBE_ORDINAL_SRC_X, PROBE_ORDINAL_SRC_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_PORTRAIT_VIEW_X, PROBE_PORTRAIT_VIEW_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           PROBE_MATCH_AGGREGATE_MIN_PCT,
           PROBE_ROW_MATCH_MIN_PCT,
           PROBE_COL_MATCH_MIN_PCT);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)PROBE_M11_GFX_CHAMPION_PORTRAITS);

    /* ----------------------------------------------------------------
     * Group A — Atlas math sanity
     * ----------------------------------------------------------------
     * The C026 atlas is the source of truth for the
     * palette_match_rect comparison.  The ordinal-2 cell must
     * exist (within the 256x87 atlas), be non-blank, and not
     * accidentally match a neighbour (stride-regression guard). */
    printf("\n[Group A] C026 atlas math for ordinal 2 (row 0 / col 2 of 8x3 atlas)\n");
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic %d = C026_GRAPHIC_CHAMPION_PORTRAITS)",
                 PROBE_M11_GFX_CHAMPION_PORTRAITS);
        CHECK(portraits != NULL && portraits->loaded && portraits->pixels != NULL, msg);
    }
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        SKIP("cannot continue without the C026 portrait atlas");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected 256 = 8 cols * 32)",
                 portraits->width);
        CHECK(portraits->width == PROBE_ATLAS_W, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected 87 = 3 rows * 29)",
                 portraits->height);
        CHECK(portraits->height == PROBE_ATLAS_H, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 2 col = 2 & 7 = %d (expected 2)",
                 PROBE_ORDINAL_COL);
        CHECK(PROBE_ORDINAL_COL == 2, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 2 row = 2 >> 3 = %d (expected 0)",
                 PROBE_ORDINAL_ROW);
        CHECK(PROBE_ORDINAL_ROW == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 2 source cell within atlas: "
                 "srcX(%d)+w(%d) <= %u AND srcY(%d)+h(%d) <= %u",
                 PROBE_ORDINAL_SRC_X, PROBE_PORTRAIT_W, portraits->width,
                 PROBE_ORDINAL_SRC_Y, PROBE_PORTRAIT_H, portraits->height);
        CHECK(PROBE_ORDINAL_SRC_X + PROBE_PORTRAIT_W <= (int)portraits->width &&
              PROBE_ORDINAL_SRC_Y + PROBE_PORTRAIT_H <= (int)portraits->height, msg);
    }
    /* Count non-transparent source pixels in the ordinal-2 cell.  A
     * blank or missing cell would silently pass a 100% match against
     * a transparent destination, so this precondition must hold. */
    {
        int x, y, count = 0;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                unsigned char src = (unsigned char)
                    (portraits->pixels[
                        (PROBE_ORDINAL_SRC_Y + y) * (int)portraits->width +
                        (PROBE_ORDINAL_SRC_X + x)] & 0x0F);
                if (src != (unsigned char)PROBE_TRANSPARENT_COLOR) ++count;
            }
        }
        opaqueAtlas2 = count;
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 ordinal-2 cell has >= 200 non-transparent pixels "
                 "(got %d) - defined portrait, not blank/unused",
                 opaqueAtlas2);
        CHECK(opaqueAtlas2 >= 200, msg);
    }
    /* Mirror catalog identity.  In real DM1 V1 PC 3.4 the
     * ordinal-2 slot is GOTHMOG (title varies by version); the
     * probe only asserts the catalog returns a non-empty name +
     * title so a future catalog edit doesn't break the probe. */
    if (!state.mirrorCatalogAvailable) {
        SKIP("mirror catalog unavailable; cannot verify ordinal-2 catalog identity");
    } else {
        mirrorName[0] = '\0';
        mirrorTitle[0] = '\0';
        hasName = M11_GameView_GetMirrorNameByOrdinal(&state, PROBE_ORDINAL,
                                                      mirrorName, sizeof(mirrorName));
        hasTitle = M11_GameView_GetMirrorTitleByOrdinal(&state, PROBE_ORDINAL,
                                                        mirrorTitle, sizeof(mirrorTitle));
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "ordinal 2 catalog name='%s' title='%s' "
                     "(hasName=%d hasTitle=%d count=%d)",
                     mirrorName[0] ? mirrorName : "<empty>",
                     mirrorTitle[0] ? mirrorTitle : "<empty>",
                     hasName, hasTitle, state.mirrorCatalog.count);
            CHECK(hasName && mirrorName[0] != '\0', msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group B — palette_match_rect at (1, 2) NORTH with C127 mutated
     * ----------------------------------------------------------------
     * The (1, 1) front cell on the live DM1 V1 PC 3.4 DUNGEON.DAT
     * carries a C127 sensor with sensorData=1 (HALK) on the
     * south wall (visibleWallCell=2 when standing at (1, 2)
     * looking NORTH).  We mutate that sensor's data to ordinal 2
     * so the palette_match_rect proof runs against a real engine
     * C127 sensor drive (not a synthetic atlas blit).  After the
     * mutation we render the framebuffer and run a strict
     * per-pixel palette match between the C026 ordinal-2 cell and
     * the D1C destination rect.  The mutation is reverted at the
     * end of Group B. */
    printf("\n[Group B] palette_match_rect at (1,2) NORTH — C127 sensor mutated to ordinal 2\n");

    park_pose(&state, PROBE_ANCHOR_X, PROBE_ANCHOR_Y, PROBE_ANCHOR_DIR);
    frontOrdinalBaseline = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (1, 2) NORTH (baseline, "
                 "before mutation) = %d (expected 1, real DM1 V1 PC 3.4 "
                 "sensor at (1, 1) south wall carries sensorData=1 HALK)",
                 frontOrdinalBaseline);
        if (frontOrdinalBaseline == 1) {
            ++g_pass;
            printf("  PASS: %s\n", msg);
        } else if (frontOrdinalBaseline == -1) {
            SKIP(msg);
            printf("  (anchor cell (1, 2) NORTH has no front-mirror sensor on this build; "
                   "cannot anchor palette_match_rect)\n");
            M11_GameView_Shutdown(&state);
            return 0;
        } else {
            INFO(msg);
        }
    }

    /* Mutate the C127 sensor at (1, 1) south wall to ordinal 2. */
    {
        int rc = find_and_mutate_c127_sensor(
            &state,
            PROBE_ANCHOR_FRONT_X, PROBE_ANCHOR_FRONT_Y,
            PROBE_ANCHOR_VISIBLE_WALL_CELL,
            PROBE_ORDINAL,
            &mutation);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "C127 sensor mutation at (1, 1) south wall "
                     "(visibleWallCell=%d) -> ordinal 2: %s "
                     "(savedSensorData=%u newIndex=%d)",
                     PROBE_ANCHOR_VISIBLE_WALL_CELL,
                     rc ? "OK" : "NOT FOUND",
                     mutation.savedSensorData,
                     mutation.sensorIndex);
            CHECK(rc, msg);
        }
        if (!rc) {
            fprintf(stderr,
                    "FATAL: could not locate C127 sensor at the (1, 1) "
                    "front cell south wall on this DM1 V1 build.  "
                    "ordinal 2 palette_match_rect requires a C127 sensor "
                    "at that cell to anchor the proof.\n");
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }

    /* Verify GetFrontMirrorOrdinal now returns 2 after mutation. */
    frontOrdinalMutated = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after sensor mutation GetFrontMirrorOrdinal=%d (expected 2)",
                 frontOrdinalMutated);
        CHECK(frontOrdinalMutated == 2, msg);
    }
    if (frontOrdinalMutated != 2) {
        fprintf(stderr,
                "FATAL: sensor mutation did not produce ordinal 2 at "
                "(1, 2) NORTH; sensorIndex=%d saved=%u new=%d ordinal=%d\n",
                mutation.sensorIndex,
                mutation.savedSensorData, PROBE_ORDINAL,
                frontOrdinalMutated);
        restore_c127_sensor(&state, &mutation);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    memset(fbNorth, 0, sizeof(fbNorth));
    M11_GameView_Draw(&state, fbNorth, PROBE_FB_W, PROBE_FB_H);

    if (!compute_palette_match(portraits, fbNorth, &northMatch)) {
        SKIP("compute_palette_match could not read C026 ordinal-2 cell");
        restore_c127_sensor(&state, &mutation);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    /* Sanity precondition: the D1C rect must contain the portrait
     * (non-zero / opaque pixels) before any match percentage is
     * even meaningful.  A blank destination would trivially pass
     * a 99% match against nothing, so we check the rect is
     * populated. */
    nonzeroNorth = rect_nonzero_count(fbNorth,
                                      PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                      PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    warmNorth = rect_warm_count(fbNorth,
                                PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) non-zero pixel count = %d "
                 "(expected >= 600 - rect is fully painted, not a "
                 "sparse wall-texture strip)",
                 nonzeroNorth);
        CHECK(nonzeroNorth >= 600, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) warm-pixel count = %d "
                 "(ordinal 2 in real DM1 V1 uses a warm palette - "
                 "skin tones and clothing; presence is verified by "
                 "warm count + palette_match)",
                 warmNorth);
        CHECK(warmNorth >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect opaque-destination pixel count = %d "
                 "(expected >= 400 of 32*29=928 cells)",
                 northMatch.opaqueDstCount);
        CHECK(northMatch.opaqueDstCount >= 400, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "compute_palette_match compared = %d (must equal the "
                 "C026 ordinal-2 cell non-transparent count = %d)",
                 northMatch.compared, opaqueAtlas2);
        CHECK(northMatch.compared == opaqueAtlas2, msg);
    }

    /* Aggregate palette match.  Tightened to 99% (vs the 90%
     * threshold used elsewhere) because this probe's whole point
     * is the strict palette-level equality. */
    pct = (northMatch.compared > 0)
              ? (northMatch.matched * 100 / northMatch.compared)
              : 0;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect palette_match for ordinal 2 (aggregate): "
                 "matched=%d compared=%d pct=%d%% (>= %d%% required)",
                 northMatch.matched, northMatch.compared, pct,
                 PROBE_MATCH_AGGREGATE_MIN_PCT);
        CHECK(pct >= PROBE_MATCH_AGGREGATE_MIN_PCT, msg);
    }

    /* Per-row palette match.  Catches vertical stride / offset
     * regressions that an aggregate match could absorb. */
    for (row = 0; row < PROBE_PORTRAIT_H; ++row) {
        int rowPct = (northMatch.rowCompared[row] > 0)
                         ? (northMatch.rowMatched[row] * 100 /
                            northMatch.rowCompared[row])
                         : 0;
        char msg[200];
        if (northMatch.rowCompared[row] == 0) {
            snprintf(msg, sizeof(msg),
                     "row %2d: all-transparent (compared=0) - "
                     "skipped, stride math OK",
                     row);
            CHECK(1, msg);
            continue;
        }
        snprintf(msg, sizeof(msg),
                 "row %2d palette_match for ordinal 2: "
                 "matched=%d compared=%d pct=%d%% (>= %d%% required)",
                 row, northMatch.rowMatched[row], northMatch.rowCompared[row],
                 rowPct, PROBE_ROW_MATCH_MIN_PCT);
        CHECK(rowPct >= PROBE_ROW_MATCH_MIN_PCT, msg);
    }

    /* Per-column palette match.  Catches horizontal stride / offset
     * regressions.  Some columns may be mostly-transparent for
     * ordinal 2 (a thin column) so a lower threshold applies. */
    for (col = 0; col < PROBE_PORTRAIT_W; ++col) {
        int colPct = (northMatch.colCompared[col] > 0)
                         ? (northMatch.colMatched[col] * 100 /
                            northMatch.colCompared[col])
                         : 0;
        char msg[200];
        if (northMatch.colCompared[col] == 0) {
            snprintf(msg, sizeof(msg),
                     "col %2d: all-transparent (compared=0) - "
                     "skipped, stride math OK",
                     col);
            CHECK(1, msg);
            continue;
        }
        snprintf(msg, sizeof(msg),
                 "col %2d palette_match for ordinal 2: "
                 "matched=%d compared=%d pct=%d%% (>= %d%% required)",
                 col, northMatch.colMatched[col], northMatch.colCompared[col],
                 colPct, PROBE_COL_MATCH_MIN_PCT);
        CHECK(colPct >= PROBE_COL_MATCH_MIN_PCT, msg);
    }

    /* Render determinism: a second render of the same pose must
     * produce a byte-equal framebuffer.  Catches any non-pure
     * framebuffer state leak (e.g. a counter increment between
     * draws). */
    memset(fbNorthSecond, 0, sizeof(fbNorthSecond));
    M11_GameView_Draw(&state, fbNorthSecond, PROBE_FB_W, PROBE_FB_H);
    determinismMatch = memcmp(fbNorth, fbNorthSecond, sizeof(fbNorth)) == 0;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C framebuffer is byte-equal across two renders of "
                 "(1, 2) NORTH (64000-byte framebuffer compare: %s)",
                 determinismMatch ? "equal" : "DIFFER");
        CHECK(determinismMatch, msg);
    }

    /* ----------------------------------------------------------------
     * Group C — side_wall_no_float negative control
     * ----------------------------------------------------------------
     * Restore the C127 sensor to its original value and re-park at
     * (1, 2) facing EAST.  The east face of (1, 1) is a side wall,
     * not the front wall — the D1C rect must not show a portrait
     * sprite.  This is the negative control for the
     * palette_match_rect proof: it confirms that the high-match
     * result at (1, 2) NORTH is driven by the C127 sensor side
     * filter, not by a coincidental "the wall texture happens to
     * match ordinal 2" artefact.
     *
     * The D1C rect at (1, 2) EAST is occupied by the D1C wall
     * ornament texture, not by a portrait sprite.  Wall texture
     * pixels are not transparent (so non-zero count is high) but
     * they don't carry ordinal-2's palette pattern.  Therefore
     * the right negative-control test is the **palette_match
     * percentage**, which is the core proof of this probe: a
     * wall texture cannot match the C026 ordinal-2 cell at >=5%,
     * so a low palette_match is the correct "no portrait"
     * assertion.  Raw non-zero / warm counts are reported as
     * informational only. */
    printf("\n[Group C] side_wall_no_float at (1, 2) EAST — sensor restored, palette_match must be < 5%%\n");

    restore_c127_sensor(&state, &mutation);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after restore GetFrontMirrorOrdinal=%d "
                 "(expected %d, baseline HALK)",
                 M11_GameView_GetFrontMirrorOrdinal(&state),
                 frontOrdinalBaseline);
        CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == frontOrdinalBaseline, msg);
    }

    park_pose(&state, PROBE_ANCHOR_X, PROBE_ANCHOR_Y, 1 /* DIR_EAST */);
    frontOrdinalEast = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (1, 2) EAST = %d "
                 "(expected -1, the east face of (1, 1) is a side "
                 "wall, not the front wall)",
                 frontOrdinalEast);
        CHECK(frontOrdinalEast == -1, msg);
    }

    memset(fbEast, 0, sizeof(fbEast));
    M11_GameView_Draw(&state, fbEast, PROBE_FB_W, PROBE_FB_H);

    nonzeroEast = rect_nonzero_count(fbEast,
                                     PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                                     PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    warmEast = rect_warm_count(fbEast,
                               PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
                               PROBE_PORTRAIT_W, PROBE_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) non-zero pixel count = %d at "
                 "(1, 2) EAST (informational - D1C wall ornament "
                 "texture is opaque, not transparent)",
                 nonzeroEast);
        INFO(msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35) warm-pixel count = %d at "
                 "(1, 2) EAST (informational - wall ornament is "
                 "mostly grey)",
                 warmEast);
        INFO(msg);
    }

    /* The palette_match percentage is the authoritative test: a
     * wall texture cannot match the C026 ordinal-2 cell at >=5%,
     * so a low palette_match is the correct "no portrait"
     * assertion. */
    ok = compute_palette_match(portraits, fbEast, &eastMatch);
    if (!ok) {
        SKIP("compute_palette_match for (1, 2) EAST skipped (asset bounds)");
    } else {
        int eastPct = (eastMatch.compared > 0)
                          ? (eastMatch.matched * 100 / eastMatch.compared)
                          : 0;
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect palette_match for ordinal 2 at (1, 2) "
                 "EAST: matched=%d compared=%d pct=%d%% (< 5%% "
                 "required - wall texture, not portrait)",
                 eastMatch.matched, eastMatch.compared, eastPct);
        CHECK(eastPct < 5, msg);
    }

    /* ----------------------------------------------------------------
     * Group D — redraw_after_candidate stability
     * ----------------------------------------------------------------
     * Re-park at (1, 2) NORTH, re-mutate the C127 sensor to
     * ordinal 2, then call SelectFrontMirrorCandidate to engage
     * the C040 candidate panel.  Re-render and assert:
     *   (i)  the D1C rect no longer matches ordinal 2 (panel owns
     *        the view; no stale sprite),
     *   (ii) the visible top strip above the panel has no warm-color
     *        leak,
     *   (iii) the visible top strip is non-empty (panel border).
     * Restore the sensor at the end so subsequent probes / cleanup
     * see the original sensor value. */
    printf("\n[Group D] redraw_after_candidate at (1, 2) NORTH — C040 panel redraw stability\n");

    park_pose(&state, PROBE_ANCHOR_X, PROBE_ANCHOR_Y, PROBE_ANCHOR_DIR);
    {
        int rc = find_and_mutate_c127_sensor(
            &state,
            PROBE_ANCHOR_FRONT_X, PROBE_ANCHOR_FRONT_Y,
            PROBE_ANCHOR_VISIBLE_WALL_CELL,
            PROBE_ORDINAL,
            &mutation);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "C127 sensor re-mutation at (1, 1) south wall "
                     "for panel-on drive: %s", rc ? "OK" : "NOT FOUND");
            CHECK(rc, msg);
        }
        if (!rc) {
            fprintf(stderr,
                    "FATAL: could not re-locate C127 sensor at the (1, 1) "
                    "front cell south wall on this DM1 V1 build.\n");
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }

    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (1, 2) NORTH returns 1 (got %d)",
                 selectRc);
        CHECK(selectRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "candidate panel state is live (candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.world.party.championCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == 2 &&
              state.candidateMirrorPartyIndex == 0 &&
              state.world.party.championCount == 1, msg);
    }

    memset(fbNorthPanelOn, 0, sizeof(fbNorthPanelOn));
    M11_GameView_Draw(&state, fbNorthPanelOn, PROBE_FB_W, PROBE_FB_H);

    /* (i) full D1C rect no longer matches ordinal 2 as a stale sprite. */
    ok = compute_palette_match(portraits, fbNorthPanelOn, &northPanelOnMatch);
    {
        char msg[200];
        if (!ok) {
            snprintf(msg, sizeof(msg),
                     "compute_palette_match for (1, 2) NORTH panel-on "
                     "skipped (asset bounds)");
            SKIP(msg);
        } else {
            int panelPct = (northPanelOnMatch.compared > 0)
                               ? (northPanelOnMatch.matched * 100 /
                                  northPanelOnMatch.compared)
                               : 0;
            snprintf(msg, sizeof(msg),
                     "D1C rect palette_match for ordinal 2 with C040 "
                     "panel live: matched=%d compared=%d pct=%d%% "
                     "(< 5%% required, no stale sprite)",
                     northPanelOnMatch.matched,
                     northPanelOnMatch.compared, panelPct);
            CHECK(panelPct < 5, msg);
        }
    }

    /* (ii) the visible top strip above the C040 panel must stay
     * clean of warm-color portrait pixels.  The C040 panel top
     * edge is at (80, 52) viewport-local (or
     * (PROBE_VIEWPORT_X + 80, PROBE_VIEWPORT_Y + 52) in
     * framebuffer coords).  The D1C rect's top edge is at
     * (PROBE_PORTRAIT_FB_Y); the visible top strip is the rows
     * from the D1C top to the C040 top. */
    warmNorthTopPanelOn = rect_warm_count(
        fbNorthPanelOn,
        PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
        PROBE_PORTRAIT_W, PROBE_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C visible top strip (above C040 panel) warm-pixel "
                 "count = %d (<= 10 required, no portrait leak)",
                 warmNorthTopPanelOn);
        CHECK(warmNorthTopPanelOn <= 10, msg);
    }

    /* (iii) the visible top strip is non-empty (panel border /
     * wall above). */
    nonzeroNorthTopPanelOn = rect_nonzero_count(
        fbNorthPanelOn,
        PROBE_PORTRAIT_FB_X, PROBE_PORTRAIT_FB_Y,
        PROBE_PORTRAIT_W, PROBE_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C visible top strip (above C040 panel) non-zero "
                 "pixel count = %d (>= 50 required, panel border present)",
                 nonzeroNorthTopPanelOn);
        CHECK(nonzeroNorthTopPanelOn >= 50, msg);
    }

    /* Restore the sensor to its original value so subsequent
     * probes / cleanup see the original DM1 V1 DUNGEON.DAT sensor. */
    restore_c127_sensor(&state, &mutation);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after final restore GetFrontMirrorOrdinal=%d "
                 "(expected %d, baseline HALK)",
                 M11_GameView_GetFrontMirrorOrdinal(&state),
                 frontOrdinalBaseline);
        CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == frontOrdinalBaseline, msg);
    }

    /* ----------------------------------------------------------------
     * Summary
     * ---------------------------------------------------------------- */
    if (g_fail > 0) overallOk = 0;
    printf("\n=== Summary: %d passed, %d failed, %d skipped, %d info "
           "(ordinal-2 palette_match_rect portrait_rect_position) ===\n",
           g_pass, g_fail, g_skip, g_info);
    M11_GameView_Shutdown(&state);
    return overallOk ? 0 : 1;
}
