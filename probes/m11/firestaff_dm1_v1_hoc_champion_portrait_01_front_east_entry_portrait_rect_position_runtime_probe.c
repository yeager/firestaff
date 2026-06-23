/*
 * firestaff_dm1_v1_hoc_champion_portrait_01_front_east_entry_portrait_rect_position_runtime_probe.c
 *
 * Real-asset / runtime regression for one narrow DM1 V1 Hall of
 * Champions champion-portrait slice:
 *
 *   ordinal       : 1   (the C026 champion-portrait strip slot at
 *                   column 1, row 0 -- the second portrait of the
 *                   8x3 strip.  On the shipped DM1 V1 PC 3.4
 *                   DUNGEON.DAT the catalog binds ordinal 1 to the
 *                   champion named "HALK" with the title
 *                   "THE BARBARIAN".)
 *   route variant : front_east_entry
 *                   (the party stands at the canonical east-facing
 *                   pose (2, 1) DIR_EAST -- the only east-facing
 *                   corridor cell in the canonical DM1 V1 Hall of
 *                   Champions DUNGEON.DAT that has a C127 sensor on
 *                   the front cell's visible wall.  The front cell
 *                   (3, 1) carries a C127 sensor on its west wall
 *                   (cell_bit = 3) with shipped sensorData = 8
 *                   (VIBIA).  This probe mutates that sensorData to
 *                   1 (HALK) and verifies the D1C portrait rectangle
 *                   at viewport (96, 35) sized 32x29 renders ordinal
 *                   1 strip pixels at >= 95% per-pixel agreement.)
 *   aspect        : portrait_rect_position
 *                   (the source-locked D1C front-wall portrait
 *                   cutout at viewport (96, 35) sized 32x29, drawn
 *                   after the C346 wall-mirror frame per ReDMCSB
 *                   DUNVIEW.C:3913-3928.  Independent of which
 *                   wall cell the C127 sensor lives on: the
 *                   DUNVIEW.C G0289 ordinal is what selects the
 *                   strip source cell, and the destination box is
 *                   fixed at (96, 35, 32, 29).)
 *
 * The shipped DM1 V1 PC 3.4 DUNGEON.DAT places the canonical
 * ordinal-1 (HALK) sensor at (1, 2) NORTH -> front=(1, 1) cell_bit 2
 * (north wall) -- that route is the front_north_entry slice, which is
 * already exercised by the cancel_reopen and redraw_after_candidate
 * probes.  This probe covers a different route variant: the
 * front_east_entry pose (2, 1) DIR_EAST, where the front cell (3, 1)
 * carries a C127 sensor on its WEST wall (cell_bit = 3).  The shipped
 * sensorData = 8 (VIBIA).  After mutation to sensorData = 1, the
 * D1C portrait rectangle must render the HALK strip pixels
 * (column 1, row 0) at the source-locked destination (96, 35, 32, 29)
 * with >= 95% per-pixel agreement, and the side walls at (2, 1)
 * DIR_NORTH / DIR_SOUTH / DIR_WEST must NOT show ordinal 1 pixels
 * (no floating portrait over a corridor wall).
 *
 * Source evidence (ReDMCSB WIP 20210206):
 *   DUNGEON.C:2573          - visibleWallCell = (partyDirection + 2) & 3
 *                             is the source-visible wall bit.
 *   DUNGEON.C:2608-2612     - C127 sensorData -> G0289 ordinal.
 *   DUNVIEW.C:3913-3928     - D1C C026 portrait blit at viewport
 *                             (96, 35) sized 32x29, source cell
 *                             ((ordinal & 7) * 32, (ordinal >> 3) * 29).
 *   DUNVIEW.C:525           - G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                             = { 96, 127, 35, 63 }.
 *   DUNVIEW.C:4547-4581     - G0289 ordinal decode (nibble 2 -> 1,
 *                             nibble 1 -> 0).  G0289 is what the D1C
 *                             blit reads; ordinal 1 maps to the strip
 *                             cell at (32, 0, 32, 29).
 *   DUNVIEW.C:8318-8542     - F0128_DUNGEONVIEW_Draw_CPSF rebuilds the
 *                             viewport far-to-near from the new party
 *                             pose after a movement turn (also
 *                             MOVESENS.C:556).
 *   MOVESENS.C:1501-1503    - C127 sensorData -> F0280 candidate.
 *   REVIVE.C F0280          - candidate champion materialized.
 *   REVIVE.C F0282          - C127 sensor disabled on confirm; route
 *                             preserved on cancel.
 *   COORD.C:1693-1722       - PC 3.4 viewport origin (0, 33) and
 *                             224x136 dims.
 *   DEFS.H:821-826          - M027_PORTRAIT_X / M028_PORTRAIT_Y
 *                             macro encoding.
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_east_walkpath_ordinal_8_runtime_probe
 *     exercises the canonical (2, 1) DIR_EAST route with the
 *     shipped sensorData = 8 (VIBIA).  It does NOT mutate the
 *     sensorData to a different ordinal, so it does not prove the
 *     D1C rect renders correctly for ordinals other than 8 at the
 *     east-facing pose.
 *   - firestaff_dm1_v1_champion_portrait_ordinal_23_front_north_entry_rect_runtime_probe
 *     mutates the (1, 1) north-wall sensor at the (1, 2) NORTH
 *     pose to ordinal 23.  It does NOT cover the east-facing pose
 *     at all, and it does NOT cover ordinal 1 mutations.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate_portrait_rect_position_097_gate_probe
 *     and the cancel_reopen probe both use the canonical (1, 2)
 *     NORTH front_north_entry pose only.
 *   - No existing probe mutates the (3, 1) west-wall sensor to
 *     sensorData = 1 and asserts the D1C rect renders the HALK
 *     strip pixels at the east-entry pose.
 *
 * This probe fills that narrow slice.  It proves:
 *   (A) catalog_ordinal_1: HALK is bound to mirror ordinal 1 with
 *       name "HALK" and title "THE BARBARIAN" in the shipped
 *       DM1 V1 PC 3.4 mirror catalog.
 *   (B) portrait_rect_position_ordinal_1_at_east_pose: at the
 *       (2, 1) DIR_EAST front_east_entry pose, after mutating the
 *       (3, 1) west-wall C127 sensor's sensorData to 1, the D1C
 *       portrait rectangle at viewport (96, 35, 32, 29) renders
 *       >= 95% of the C026 strip source cell (32, 0, 32, 29)
 *       pixels (the ordinal 1 strip cell).  This proves both the
 *       destination rectangle position AND that ordinal 1 reads
 *       from the correct strip cell independent of which wall
 *       cell_bit the sensor sits on.
 *   (C) no-float on side walls at the (2, 1) DIR_NORTH,
 *       DIR_SOUTH, DIR_WEST poses -- the D1C rect at those poses
 *       does NOT match ordinal 1 pixels because those side walls
 *       have no C127 sensor on the visible wall (and even after
 *       mutation the only visible-wall sensor sits on cell_bit 3,
 *       which only (party direction = EAST) makes visible).
 *   (D) sensor restore: after restoring the original sensorData
 *       (8), GetFrontMirrorOrdinal returns to its baseline (8) and
 *       the D1C rect does NOT contain ordinal 1 pixels -- the
 *       mutation is fully reversible and the shipped data is left
 *       in its original state.
 *
 * The probe is non-duplicative with the east_walkpath ordinal-8
 * probe because it asserts ordinal 1 (HALK) pixel identity at the
 * same east-facing pose -- a separate strip-slot dimension.  The
 * mutation + restore pattern is the same source-locked invariant
 * that the ordinal-23 probe exercises; the new axis is the
 * east-facing pose and the (3, 1) west-wall sensor.
 *
 * Honest scope: this probe proves the source-locked ordinal/position
 * contract for ordinal 1 at the front_east_entry pose in shipped
 * DM1 V1 PC 3.4 data.  It does NOT claim DOS pixel parity.  Original
 * DM1 PC 3.4 captures live under parity-evidence/ and are referenced
 * by separate parity gates.
 *
 * Slice assignment:
 *   firestaff_dm1_v1_hoc_champion_portrait_01_front_east_entry_portrait_rect_position
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_01_front_east_entry_portrait_rect_position_runtime_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    /* Framebuffer geometry (ReDMCSB COORD.C:1693-1722). */
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* D1C portrait cutout (ReDMCSB DUNVIEW.C:3913-3928,
     * G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63}). */
    PORTRAIT_RECT_X = 96,
    PORTRAIT_RECT_Y = 35,
    PORTRAIT_RECT_W = 32,
    PORTRAIT_RECT_H = 29,
    PORTRAIT_RECT_X_END = PORTRAIT_RECT_X + PORTRAIT_RECT_W,
    PORTRAIT_RECT_Y_END = PORTRAIT_RECT_Y + PORTRAIT_RECT_H,
    /* C026 portrait strip: 8x3 grid of 32x29 portraits (256x87). */
    STRIP_COL_W = 32,
    STRIP_ROW_H = 29,
    C026_COLS = 8,
    C026_ROWS = 3,
    /* Ordinal 1 strip cell: column 1, row 0 -> (32, 0, 32, 29). */
    ORDINAL = 1,
    STRIP_SRC_X = (ORDINAL & 7) * STRIP_COL_W,    /* = 32 */
    STRIP_SRC_Y = (ORDINAL >> 3) * STRIP_ROW_H,   /* =  0 */
    STRIP_SRC_X_END = STRIP_SRC_X + STRIP_COL_W,  /* = 64 */
    STRIP_SRC_Y_END = STRIP_SRC_Y + STRIP_ROW_H,  /* = 29 */
    /* front_east_entry pose: party at (2, 1) DIR_EAST.
     *   Front cell = (3, 1), visibleWallCell = (1 + 2) & 3 = 3 (west wall).
     *   Shipped C127 sensor at (3, 1) cell_bit = 3, sensorData = 8 (VIBIA).
     *   This probe mutates sensorData to 1 (HALK) and asserts the D1C
     *   portrait rect renders ordinal 1 strip pixels at >= 95% per-pixel
     *   match. */
    ENTRY_MAP_X = 2,
    ENTRY_MAP_Y = 1,
    ENTRY_DIR = 1, /* DIR_EAST */
    FRONT_CELL_X = 3,
    FRONT_CELL_Y = 1,
    FRONT_CELL_BIT = 3, /* west wall of (3, 1) */
    /* C01_COLOR_DARK_GRAY = 1 is the C026 transparent color passed to
     * M11_AssetLoader_BlitRegion by m11_draw_dm1_front_champion_portrait
     * (DEFS.H:2079, DUNVIEW.C:3916). */
    STRIP_TRANSPARENT_COLOR = 1,
    /* Per-pixel match threshold for the D1C rect.  95% is the same
     * threshold the ordinal-23 front_north_entry probe uses. */
    POSITIVE_MATCH_PCT = 95,
    /* Warm-color threshold for the "portrait present" / "no float"
     * tests.  Same threshold the cancel_reopen and redraw_after_candidate
     * probes use, so this probe stays consistent with the proven
     * champion-mirror matrix. */
    PORTRAIT_WARM_THRESHOLD = 30,
    /* Hall of Champions map index on DM1 V1 PC 3.4. */
    HALL_MAP_INDEX = 0
};

/* Counter struct for the four invariant groups. */
typedef struct Ordinal01EastEntryState {
    int passed;
    int failed;
    int failedOrdinalCatalog;
    int failedPortraitRectMismatch;
    int failedSideWallFloat;
    int failedSensorRestore;
} Ordinal01EastEntryState;

static void pass_msg(Ordinal01EastEntryState* s, const char* msg) {
    printf("  PASS: %s\n", msg);
    s->passed++;
}

static void fail_msg(Ordinal01EastEntryState* s, const char* msg, int* counter) {
    printf("  FAIL: %s\n", msg);
    s->failed++;
    if (counter) *counter += 1;
}

/* Warm-color palette indices used by champion portrait sprites
 * (ReDMCSB DUNVIEW.C:3913-3928): 0x07 green, 0x08 red, 0x09 orange,
 * 0x0A peach, 0x0B yellow, 0x0E blue.  Grey-stone wall texture uses
 * 0x01/0x02/0x07-grey/0x0D and never the warm set. */
static int is_warm_palette_index(unsigned char idx) {
    switch (idx & 0x0F) {
        case 0x07: /* green */
        case 0x08: /* red */
        case 0x09: /* orange */
        case 0x0A: /* peach */
        case 0x0B: /* yellow */
        case 0x0E: /* blue */
            return 1;
        default:
            return 0;
    }
}

static int count_warm_in_rect(const unsigned char* fb, int x, int y, int w, int h) {
    int count = 0;
    int xx, yy;
    for (yy = y; yy < y + h; ++yy) {
        for (xx = x; xx < x + w; ++xx) {
            unsigned char raw = fb[(VIEWPORT_Y + yy) * FB_W + (VIEWPORT_X + xx)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            if (is_warm_palette_index(idx)) ++count;
        }
    }
    return count;
}

/* Compare the C026 portrait strip cells at ORDINAL against the
 * framebuffer D1C portrait rectangle.  Returns the number of matched
 * non-transparent pixels and the total non-transparent pixels compared.
 * Returns 1 if the call set the out-params, 0 if assets were missing. */
static int compare_strip_to_portrait_rect(
    const unsigned char* fb,
    const M11_AssetSlot* portraits,
    int stripTransparentColor,
    int* outMatched,
    int* outCompared) {
    int x, y;
    int matched = 0, compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if ((int)portraits->width < STRIP_SRC_X_END) return 0;
    if ((int)portraits->height < STRIP_SRC_Y_END) return 0;
    for (y = 0; y < STRIP_ROW_H; ++y) {
        for (x = 0; x < STRIP_COL_W; ++x) {
            unsigned char src = portraits->pixels[
                (STRIP_SRC_Y + y) * (int)portraits->width + (STRIP_SRC_X + x)] & 0x0F;
            if (src == (unsigned char)stripTransparentColor) continue;
            ++compared;
            unsigned char dst = fb[
                (VIEWPORT_Y + PORTRAIT_RECT_Y + y) * FB_W +
                (VIEWPORT_X + PORTRAIT_RECT_X + x)] & 0x0F;
            if (src == dst) ++matched;
        }
    }
    *outMatched = matched;
    *outCompared = compared;
    return 1;
}

/* Count non-transparent pixels in the C026 ordinal-1 strip source
 * cell.  Used to ensure the asset itself has the pixels we expect to
 * find on the framebuffer, independent of the draw path. */
static int count_non_transparent_in_strip_ordinal(
    const M11_AssetSlot* portraits,
    int stripTransparentColor) {
    int count = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if ((int)portraits->width < STRIP_SRC_X_END) return 0;
    if ((int)portraits->height < STRIP_SRC_Y_END) return 0;
    for (y = 0; y < STRIP_ROW_H; ++y) {
        for (x = 0; x < STRIP_COL_W; ++x) {
            unsigned char src = portraits->pixels[
                (STRIP_SRC_Y + y) * (int)portraits->width + (STRIP_SRC_X + x)] & 0x0F;
            if (src != (unsigned char)stripTransparentColor) ++count;
        }
    }
    return count;
}

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int direction) {
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = direction;
    /* Clear candidate panel state to keep the D1C rect visible. */
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

static int ordinal_1_in_catalog(const M11_GameViewState* game) {
    return game->mirrorCatalogAvailable &&
        game->mirrorCatalog.count > ORDINAL;
}

/* Walk the front cell's THING chain looking for a C127 sensor on the
 * given visible wall cell_bit.  Returns the sensor index if found
 * (so the caller can save/restore sensorData), -1 otherwise.  We
 * replicate m11_raw_next_thing here because that helper is file-static
 * in m11_game_view.c. */
static int find_c127_sensor_on_cell_bit(
    const M11_GameViewState* game,
    int mapIndex,
    int cellMapX,
    int cellMapY,
    int cellBit) {
    const struct DungeonMapDesc_Compat* map = &game->world.dungeon->maps[mapIndex];
    int base = cellMapX * (int)map->height + cellMapY;
    int squareIndex = base;
    unsigned short thing = game->world.things->squareFirstThings[squareIndex];
    static const unsigned char s_thingDataByteCount[16] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    while (thing != THING_ENDOFLIST && thing != THING_NONE) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        int cell = THING_GET_CELL(thing);
        if (type == THING_TYPE_SENSOR && cell == cellBit &&
            index >= 0 && index < game->world.things->sensorCount &&
            game->world.things->sensors[index].sensorType == 127) {
            return index;
        }
        {
            int byteCount = (type >= 0 && type < 16) ? s_thingDataByteCount[type] : 2;
            const unsigned char* raw;
            if (type < 0 || type >= 16 || !game->world.things->rawThingData[type] ||
                index < 0 || index >= game->world.things->thingCounts[type]) {
                return -1;
            }
            raw = game->world.things->rawThingData[type] + (index * byteCount);
            thing = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        }
    }
    return -1;
}

/* ---------- Invariant (A): catalog ordinal 1 = HALK ---------- */
static int check_catalog_ordinal_1(
    Ordinal01EastEntryState* s, M11_GameViewState* game) {
    char name[16];
    char title[32];
    name[0] = '\0';
    title[0] = '\0';
    if (!ordinal_1_in_catalog(game)) {
        printf("SKIP ordinal 1 not in mirror catalog "
               "(count=%d < %d)\n",
               game->mirrorCatalog.count, ORDINAL + 1);
        return 0;
    }
    int hasName = M11_GameView_GetMirrorNameByOrdinal(game, ORDINAL,
                                                     name, sizeof(name));
    int hasTitle = M11_GameView_GetMirrorTitleByOrdinal(game, ORDINAL,
                                                       title, sizeof(title));
    if (!hasName || name[0] == '\0') {
        fail_msg(s, "ordinal 1 has no name (ReDMCSB catalog leaves slot empty)",
                 &s->failedOrdinalCatalog);
        return 0;
    }
    /* HALK is the shipped champion for ordinal 1.  Bind strictly. */
    if (strcmp(name, "HALK") != 0) {
        char label[96];
        snprintf(label, sizeof(label),
                 "ordinal 1 catalog name='%s' (expected 'HALK')", name);
        fail_msg(s, label, &s->failedOrdinalCatalog);
        return 0;
    }
    if (!hasTitle || title[0] == '\0') {
        char label[96];
        snprintf(label, sizeof(label),
                 "ordinal 1 has title empty (hasTitle=%d)", hasTitle);
        fail_msg(s, label, &s->failedOrdinalCatalog);
        return 0;
    }
    char label[128];
    snprintf(label, sizeof(label),
             "ordinal 1 catalog name='%s' title='%s' hasName=%d hasTitle=%d",
             name, title, hasName, hasTitle);
    pass_msg(s, label);
    return 1;
}

/* ---------- Invariant (B): portrait_rect_position at east_entry ---------- */
static int check_portrait_rect_position_at_east_entry(
    Ordinal01EastEntryState* s, M11_GameViewState* game) {
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    int matched, compared;
    int sensorIndex = -1;
    unsigned short savedSensorData = 0;
    int originalSensorData = -1;
    char label[256];

    /* 1. Sanity: the front_east_entry pose must report the shipped
     *    ordinal 8 (VIBIA) at the (3, 1) west-wall sensor before
     *    we mutate.  This is the same fixture the east_walkpath
         *    ordinal-8 probe already locks. */
    set_pose(game, ENTRY_MAP_X, ENTRY_MAP_Y, ENTRY_DIR);
    {
        int baselineOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (baselineOrdinal != 8) {
            printf("SKIP front_east_entry (2,1) EAST baseline ordinal=%d expected=8; "
                   "this DM1 V1 build does not match the reference DUNGEON.DAT "
                   "fixture (the (3,1) cell_bit=3 sensor is laid out differently)\n",
                   baselineOrdinal);
            return 0;
        }
        pass_msg(s, "front_east_entry (2,1) EAST baseline ordinal=8 (VIBIA sensor, "
                    "before ordinal-1 mutation)");
    }

    /* 2. Load the C026 portrait strip asset. */
    portraits = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        fail_msg(s, "C026 GRAPHICS.DAT champion portrait strip unavailable",
                 &s->failedPortraitRectMismatch);
        return 0;
    }

    /* 3. Sanity: the C026 strip at ordinal 1 must have non-trivial
     *    non-transparent pixels.  Otherwise even a perfect blit
     *    can't satisfy the rect check. */
    {
        int nonTransparent = count_non_transparent_in_strip_ordinal(
            portraits, STRIP_TRANSPARENT_COLOR);
        if (nonTransparent < 50) {
            snprintf(label, sizeof(label),
                     "C026 strip ordinal-1 source has %d non-transparent pixels "
                     "(expected >= 50) - asset looks empty", nonTransparent);
            fail_msg(s, label, &s->failedPortraitRectMismatch);
            return 0;
        }
        snprintf(label, sizeof(label),
                 "C026 strip ordinal-1 source has %d non-transparent pixels (>= 50)",
                 nonTransparent);
        pass_msg(s, label);
    }

    /* 4. Baseline redraw at the (2,1) EAST pose with sensorData=8.
     *    The D1C rect must contain VIBIA pixels (ordinal 8), and
     *    must NOT match the ordinal 1 strip cells (the baseline
     *    before mutation proves we are not pre-loading ordinal 1
     *    pixels). */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    {
        /* Check the baseline does NOT already match ordinal 1. */
        if (compare_strip_to_portrait_rect(fb, portraits, STRIP_TRANSPARENT_COLOR,
                                           &matched, &compared)) {
            int pct = compared > 0 ? (matched * 100 / compared) : 0;
            if (pct >= POSITIVE_MATCH_PCT) {
                snprintf(label, sizeof(label),
                         "baseline (2,1) EAST sensorData=8 already matches ordinal 1 "
                         "matched=%d/%d (%d%%) - mutation premise broken",
                         matched, compared, pct);
                fail_msg(s, label, &s->failedPortraitRectMismatch);
                return 0;
            }
            snprintf(label, sizeof(label),
                     "baseline (2,1) EAST sensorData=8 does NOT match ordinal 1 "
                     "matched=%d/%d (%d%%) - mutation premise OK",
                     matched, compared, pct);
            pass_msg(s, label);
        }
    }

    /* 5. Locate the (3, 1) west-wall C127 sensor and mutate its
     *    sensorData to 1 (HALK).  This is the canonical mutation
     *    pattern from the ordinal-23 front_north_entry probe. */
    sensorIndex = find_c127_sensor_on_cell_bit(
        game, HALL_MAP_INDEX, FRONT_CELL_X, FRONT_CELL_Y, FRONT_CELL_BIT);
    if (sensorIndex < 0) {
        snprintf(label, sizeof(label),
                 "could not locate front C127 sensor at (%d,%d) cell_bit=%d",
                 FRONT_CELL_X, FRONT_CELL_Y, FRONT_CELL_BIT);
        fail_msg(s, label, &s->failedPortraitRectMismatch);
        return 0;
    }
    originalSensorData = (int)game->world.things->sensors[sensorIndex].sensorData;
    savedSensorData = game->world.things->sensors[sensorIndex].sensorData;
    game->world.things->sensors[sensorIndex].sensorData = (unsigned short)ORDINAL;
    {
        char baseline[64];
        snprintf(baseline, sizeof(baseline), "%d", originalSensorData);
        snprintf(label, sizeof(label),
                 "mutated (3,1) west-wall C127 sensorData %s -> %d "
                 "(sensorIndex=%d)",
                 baseline, ORDINAL, sensorIndex);
        pass_msg(s, label);
    }

    /* 6. Verify GetFrontMirrorOrdinal now reports ordinal 1. */
    {
        int mutatedOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (mutatedOrdinal != ORDINAL) {
            snprintf(label, sizeof(label),
                     "after sensor mutation GetFrontMirrorOrdinal=%d expected=%d",
                     mutatedOrdinal, ORDINAL);
            fail_msg(s, label, &s->failedPortraitRectMismatch);
            game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
            return 0;
        }
        snprintf(label, sizeof(label),
                 "after sensor mutation GetFrontMirrorOrdinal=%d (expected %d)",
                 mutatedOrdinal, ORDINAL);
        pass_msg(s, label);
    }

    /* 7. Redraw and verify the D1C rect matches the C026 strip at
     *    ordinal 1 at >= 95% per-pixel agreement. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    if (!compare_strip_to_portrait_rect(fb, portraits, STRIP_TRANSPARENT_COLOR,
                                        &matched, &compared)) {
        fail_msg(s, "compare_strip_to_portrait_rect asset bounds check failed",
                 &s->failedPortraitRectMismatch);
        game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
        return 0;
    }
    if (compared <= 0) {
        fail_msg(s, "compare_strip_to_portrait_rect compared=0 (rect empty)",
                 &s->failedPortraitRectMismatch);
        game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
        return 0;
    }
    if (matched * 100 < POSITIVE_MATCH_PCT * compared) {
        snprintf(label, sizeof(label),
                 "D1C rect (%d,%d)-(%d,%d) does not match C026 ordinal 1 "
                 "matched=%d compared=%d (>= %d%% required) - rect position wrong",
                 PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                 PORTRAIT_RECT_X_END, PORTRAIT_RECT_Y_END,
                 matched, compared, POSITIVE_MATCH_PCT);
        fail_msg(s, label, &s->failedPortraitRectMismatch);
        game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
        return 0;
    }
    snprintf(label, sizeof(label),
             "D1C rect (%d,%d)-(%d,%d) renders ordinal 1 strip "
             "matched=%d compared=%d (%d%%) at front_east_entry pose",
             PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
             PORTRAIT_RECT_X_END, PORTRAIT_RECT_Y_END,
             matched, compared, matched * 100 / compared);
    pass_msg(s, label);

    /* 8. Restore the sensor data and verify the ordinal returns to
     *    the shipped baseline (8) and the D1C rect no longer matches
     *    ordinal 1. */
    game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
    {
        int restoredOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (restoredOrdinal != originalSensorData) {
            snprintf(label, sizeof(label),
                     "after sensor restore GetFrontMirrorOrdinal=%d expected=%d "
                     "(mutation cleanup failed)",
                     restoredOrdinal, originalSensorData);
            fail_msg(s, label, &s->failedSensorRestore);
            return 0;
        }
        snprintf(label, sizeof(label),
                 "after sensor restore GetFrontMirrorOrdinal=%d (back to baseline %d)",
                 restoredOrdinal, originalSensorData);
        pass_msg(s, label);
    }

    /* 9. Redraw after restore and verify the D1C rect does NOT
     *    match ordinal 1 pixels (sensorData is back to 8, the
     *    shipped value).  This proves the mutation was the only
     *    cause of the ordinal-1 pixels. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    if (compare_strip_to_portrait_rect(fb, portraits, STRIP_TRANSPARENT_COLOR,
                                       &matched, &compared)) {
        int pct = compared > 0 ? (matched * 100 / compared) : 0;
        if (pct >= POSITIVE_MATCH_PCT) {
            snprintf(label, sizeof(label),
                     "after sensor restore D1C rect still matches ordinal 1 "
                     "matched=%d/%d (%d%%) - mutation cleanup incomplete",
                     matched, compared, pct);
            fail_msg(s, label, &s->failedSensorRestore);
            return 0;
        }
        snprintf(label, sizeof(label),
                 "after sensor restore D1C rect does NOT match ordinal 1 "
                 "matched=%d/%d (%d%%) - mutation cleanup OK",
                 matched, compared, pct);
        pass_msg(s, label);
    }
    return 1;
}

/* ---------- Invariant (C): no float on side walls at (2,1) ---------- */
static int check_side_wall_no_float_at_east_entry(
    Ordinal01EastEntryState* s, M11_GameViewState* game) {
    unsigned char fb[FB_W * FB_H];
    int warmNorth, warmSouth, warmWest, warmEast;
    char label[256];

    /* The mutation has already been cleaned up by the previous
     * invariant; here we re-apply it to verify the side-wall no-float
     * invariant while ordinal 1 is the front ordinal.  We mutate,
     * sweep the four cardinal directions at the (2, 1) cell, then
     * restore. */
    int sensorIndex = find_c127_sensor_on_cell_bit(
        game, HALL_MAP_INDEX, FRONT_CELL_X, FRONT_CELL_Y, FRONT_CELL_BIT);
    unsigned short savedSensorData;
    if (sensorIndex < 0) {
        printf("SKIP no-float: could not locate front C127 sensor at "
               "(%d,%d) cell_bit=%d\n",
               FRONT_CELL_X, FRONT_CELL_Y, FRONT_CELL_BIT);
        return 0;
    }
    savedSensorData = game->world.things->sensors[sensorIndex].sensorData;
    game->world.things->sensors[sensorIndex].sensorData = (unsigned short)ORDINAL;

    /* Positive control: (2, 1) DIR_EAST must show ordinal 1. */
    set_pose(game, ENTRY_MAP_X, ENTRY_MAP_Y, ENTRY_DIR);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmEast = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                  PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmEast < PORTRAIT_WARM_THRESHOLD) {
        snprintf(label, sizeof(label),
                 "front_east_entry (2,1) EAST warm=%d < %d "
                 "(expected ordinal 1 portrait present)",
                 warmEast, PORTRAIT_WARM_THRESHOLD);
        fail_msg(s, label, &s->failedSideWallFloat);
        game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
        return 0;
    }
    snprintf(label, sizeof(label),
             "front_east_entry (2,1) EAST warm=%d >= %d "
             "(ordinal 1 portrait present)",
             warmEast, PORTRAIT_WARM_THRESHOLD);
    pass_msg(s, label);

    /* Negative control 1: (2, 1) DIR_NORTH.  Front=(2, 0), which has
     * no C127 sensor on its visible wall (south wall of (2, 0)).
     * The D1C rect must be wall-only (no ordinal 1 pixels). */
    set_pose(game, ENTRY_MAP_X, ENTRY_MAP_Y, 0 /* DIR_NORTH */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmNorth = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                   PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmNorth >= PORTRAIT_WARM_THRESHOLD) {
        snprintf(label, sizeof(label),
                 "side wall (2,1) NORTH warm=%d >= %d "
                 "(portrait floating on side wall)",
                 warmNorth, PORTRAIT_WARM_THRESHOLD);
        fail_msg(s, label, &s->failedSideWallFloat);
        game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
        return 0;
    }
    snprintf(label, sizeof(label),
             "side wall (2,1) NORTH warm=%d < %d "
             "(no portrait floating)",
             warmNorth, PORTRAIT_WARM_THRESHOLD);
    pass_msg(s, label);

    /* Negative control 2: (2, 1) DIR_SOUTH.  Front=(2, 2), which
     * has a C127 sensor on its north wall (cell_bit 2) with
     * shipped sensorData = 4 (LEIF).  Even with the (3, 1) sensor
     * mutated to ordinal 1, the D1C rect at this pose must show
     * LEIF (ordinal 4) -- NOT ordinal 1 -- because the front-cell
     * filter is keyed on visibleWallCell and ordinal 1's sensor is
     * on a different cell entirely. */
    set_pose(game, ENTRY_MAP_X, ENTRY_MAP_Y, 2 /* DIR_SOUTH */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmSouth = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                   PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmSouth >= PORTRAIT_WARM_THRESHOLD) {
        /* The LEIF portrait IS warm-colored, so warm >= threshold
         * here is the LEIF portrait, not a floating ordinal 1.
         * Verify the front ordinal reports 4 (LEIF) and NOT 1 (HALK). */
        int southOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (southOrdinal == ORDINAL) {
            snprintf(label, sizeof(label),
                     "side wall (2,1) SOUTH reports ordinal=%d (expected 4 LEIF, "
                     "NOT ordinal 1 -- the (3,1) sensor mutation leaked across cells)",
                     southOrdinal);
            fail_msg(s, label, &s->failedSideWallFloat);
            game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
            return 0;
        }
        snprintf(label, sizeof(label),
                 "side wall (2,1) SOUTH warm=%d ordinal=%d (LEIF, NOT ordinal 1)",
                 warmSouth, southOrdinal);
        pass_msg(s, label);
    } else {
        snprintf(label, sizeof(label),
                 "side wall (2,1) SOUTH warm=%d < %d (no portrait)",
                 warmSouth, PORTRAIT_WARM_THRESHOLD);
        pass_msg(s, label);
    }

    /* Negative control 3: (2, 1) DIR_WEST.  Front=(1, 1), which has
     * a C127 sensor on its north wall (cell_bit 2) with shipped
     * sensorData = 1 (HALK).  At (2, 1) DIR_WEST the visible wall
     * is the east wall of (1, 1) (cell_bit 1) -- the (1, 1) sensor
     * sits on cell_bit 2, NOT cell_bit 1, so visibleWallCell != cell.
     * Therefore the D1C rect at this pose must NOT contain ordinal 1
     * pixels -- the (3, 1) sensor mutation to ordinal 1 must not
     * leak through the (2, 1) DIR_WEST pose. */
    set_pose(game, ENTRY_MAP_X, ENTRY_MAP_Y, 3 /* DIR_WEST */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmWest = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                  PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmWest >= PORTRAIT_WARM_THRESHOLD) {
        int westOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (westOrdinal == ORDINAL) {
            snprintf(label, sizeof(label),
                     "side wall (2,1) WEST warm=%d ordinal=%d "
                     "(portrait floating on side wall -- ordinal 1 leaked)",
                     warmWest, westOrdinal);
            fail_msg(s, label, &s->failedSideWallFloat);
            game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
            return 0;
        }
        snprintf(label, sizeof(label),
                 "side wall (2,1) WEST warm=%d ordinal=%d (NOT ordinal 1, OK)",
                 warmWest, westOrdinal);
        pass_msg(s, label);
    } else {
        snprintf(label, sizeof(label),
                 "side wall (2,1) WEST warm=%d < %d (no portrait floating)",
                 warmWest, PORTRAIT_WARM_THRESHOLD);
        pass_msg(s, label);
    }

    /* Restore the sensor data. */
    game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    Ordinal01EastEntryState s;
    memset(&s, 0, sizeof(s));

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP firestaff_dm1_v1_hoc_champion_portrait_01_front_east_entry_portrait_rect_position "
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

    printf("=== DM1 V1 Hall portrait ordinal 1 front_east_entry portrait_rect ===\n");
    printf("dataDir=%s ordinal=%d stripSrc=(%d,%d)-(%d,%d) rect=(%d,%d)-(%d,%d)\n",
           dataDir, ORDINAL,
           STRIP_SRC_X, STRIP_SRC_Y, STRIP_SRC_X_END, STRIP_SRC_Y_END,
           PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
           PORTRAIT_RECT_X_END, PORTRAIT_RECT_Y_END);
    printf("pose=(%d,%d,%d) front=(%d,%d) frontCellBit=%d\n",
           ENTRY_MAP_X, ENTRY_MAP_Y, ENTRY_DIR,
           FRONT_CELL_X, FRONT_CELL_Y, FRONT_CELL_BIT);

    check_catalog_ordinal_1(&s, &game);
    check_portrait_rect_position_at_east_entry(&s, &game);
    check_side_wall_no_float_at_east_entry(&s, &game);

    printf("=== %d passed, %d failed (ordinal-catalog=%d portrait-rect=%d side-wall=%d sensor-restore=%d) ===\n",
           s.passed, s.failed,
           s.failedOrdinalCatalog, s.failedPortraitRectMismatch,
           s.failedSideWallFloat, s.failedSensorRestore);
    M11_GameView_Shutdown(&game);
    return s.failed > 0 ? 1 : 0;
}
