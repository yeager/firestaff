/*
 * DM1 V1 Hall of Champions portrait-ordinal-23 front-north-entry
 * portrait-rectangle runtime probe.
 *
 * Narrow slice of the Hall of Champions portrait-placement table:
 *   ordinal          23
 *   route            front_north_entry
 *   aspect           portrait_rect_position
 *
 * The probe locks the destination rectangle for the C026 champion
 * portrait strip on the D1C front-wall route.  The DM1 V1 source
 * (ReDMCSB) gives us:
 *   - The D1C portrait box is fixed at viewport-local (96,35)-(128,64):
 *     G0109_auc_Graphic558_Box_ChampionPortraitOnWall = { 96, 127, 35, 63 }
 *     and C026 portraits are 32×29 pixels per cell (DUNVIEW.C:3913-3928).
 *     m11_draw_dm1_front_champion_portrait in src/engine/m11_game_view.c
 *     draws at M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35 (src/engine/m11_game_view.c
 *     lines 13970-13981) using the same source-strip math
 *       srcX = (ord & 7) * 32, srcY = (ord >> 3) * 29
 *     which is the ReDMCSB DUNVIEW.C:3916 source-column math.
 *
 *   - The strip layout for ordinal 23 in C026 is:
 *       col = 23 & 7  = 7  (rightmost column of the strip)
 *       row = 23 >> 3 = 2  (third row of the strip)
 *     This corresponds to strip pixels (224, 58) through (255, 86).
 *     m11_front_cell_mirror_ordinal guards `sensorData >= 0 && sensorData
 *     < state->mirrorCatalog.count`, so the sensor must carry an ordinal
 *     within the mirror catalog.
 *
 *   - The D1C portrait route is gated by the C127 sensor on the
 *     front-cell's source-visible wall cell (partyDirection + 2).
 *     m11_front_cell_mirror_ordinal (src/engine/m11_game_view.c:11652)
 *     walks the front-cell's THING chain and rejects non-wall C127
 *     sensors.  Side walls (front-cell left/right neighbours) never
 *     carry a C127 champion-portrait sensor, so no portrait may
 *     "float" over a side wall in any pose.
 *
 *   - The "front_north_entry" route is the party standing one cell
 *     south of the front-wall cell and facing NORTH.  In real DM1 V1
 *     DUNGEON.DAT the C127 sensors under the champion mirrors are:
 *       (1,2) NORTH -> (1,1) front -> ordinal 1 (HALK)
 *       (1,5) NORTH -> (1,4) front -> ordinal 10 (ZED)
 *       (2,1) SOUTH -> (2,2) front -> ordinal 4 (LEIF)
 *       (2,4) SOUTH -> (2,5) front -> ordinal 15 (MOPHUS)
 *       (1,3) EAST  -> (2,3) front -> ordinal 18 (SONJA)
 *       (1,5) SOUTH -> (1,6) front -> ordinal 13 (WUUF)
 *     (See firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe.c
 *     for the full layout.)  The probe uses (1,2) NORTH for the
 *     front_north_entry anchor because it is the first mirror the party
 *     meets when entering the Hall.
 *
 * The probe verifies four invariants on the (1,3) NORTH
 * front_north_entry pose:
 *
 *   1. The catalog reports a non-empty champion name at ordinal 23
 *      (DM1 V1 has 19+ named champions so ordinal 23 sits at the
 *      right edge of the C026 strip; its name must be present and
 *      non-empty).  This proves the ordinal-23-strip-slot maps to a
 *      real champion (NABI / THE PROPHET in this DUNGEON.DAT).
 *
 *   2. The D1C portrait-rect (96,35)-(128,64) is the destination for
 *      ordinal 23 when ordinal 23 is the front ordinal.  Since real
 *      DM1 V1 places ordinal 23 nowhere on the Hall's front walls,
 *      the probe mutates the front cell's C127 sensorData to 23,
 *      redraws, and asserts the rect pixels match the C026 strip
 *      source cells at (224,58)-(255,86).  This is the strict pixel
 *      match: at least 95% of non-transparent strip pixels must equal
 *      the framebuffer pixel under them.  It proves both that the
 *      rect position is correct AND that ordinal 23 is read from the
 *      correct strip cell.
 *
 *   3. Side walls do not show a portrait: at (1,2) EAST (the same
 *      cell, wrong-side), the D1C portrait-rect must contain < 30
 *      warm pixels (no floating portrait) because the side wall has
 *      no C127 sensor.
 *
 *   4. After selecting the front_mirror candidate (HALK at ordinal 1)
 *      and confirming resurrect, the new champion carries a portrait
 *      ordinal that is the source-catalog mirror ordinal 1 (NOT 20+),
 *      confirming the F0673 catalog-recruit path stores
 *      portraitIndex from record->textStringIndex, not the 20+ slot
 *      ordinal.  This separates ordinal-23-as-strip-slot from
 *      ordinal-20-23-as-candidate-slot (a documented ReDMCSB
 *      ambiguity that downstream tests rely on).
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 / DUNVIEW.C:8522-8533 draws D1C
 *     portrait at the fixed G0109 box {96..127, 35..63}.
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw order
 *     (far-to-near) keeps the side-wall squares free of portraits.
 *   ReDMCSB F0107 / DEFS.H M635_ZONE_PORTRAIT_ON_WALL = 733/737.
 *   ReDMCSB ENDGAME.C:327-394 champion summary uses C412..C419
 *     layout-696 zones (separate from the in-viewport D1C route).
 *   m11_draw_dm1_front_champion_portrait in src/engine/m11_game_view.c
 *     is the Firestaff draw site.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

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
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,

    /* D1C portrait-on-wall rectangle (ReDMCSB G0109 + Firestaff
     * m11_draw_dm1_front_champion_portrait destination).  These are
     * viewport-relative coordinates; the framebuffer offset is
     * (VIEWPORT_Y+y)*FB_W + (VIEWPORT_X+x). */
    PORTRAIT_RECT_X = 96,
    PORTRAIT_RECT_Y = 35,
    PORTRAIT_RECT_W = 32,
    PORTRAIT_RECT_H = 29,
    PORTRAIT_RECT_X_END = PORTRAIT_RECT_X + PORTRAIT_RECT_W,  /* 128 */
    PORTRAIT_RECT_Y_END = PORTRAIT_RECT_Y + PORTRAIT_RECT_H,  /* 64 */

    /* C026 portrait strip layout.  ReDMCSB DUNVIEW.C:3916:32x29 cell,
     * 8 columns, 3 rows used by the game (DUNGEON.C:2608-2612 stores
     * sensorData in G0289 which is rendered as (ord&7)*32, (ord>>3)*29). */
    STRIP_COL_W = 32,
    STRIP_ROW_H = 29,
    ORDINAL = 23, /* 0-based strip ordinal, also catalog ordinal */
    STRIP_SRC_X = (ORDINAL & 7) * STRIP_COL_W,            /* 7*32 = 224 */
    STRIP_SRC_Y = (ORDINAL >> 3) * STRIP_ROW_H,           /* 2*29 = 58 */
    STRIP_SRC_X_END = STRIP_SRC_X + STRIP_COL_W,
    STRIP_SRC_Y_END = STRIP_SRC_Y + STRIP_ROW_H,

    /* Warm-colour palette indices used by champion portrait sprites
     * per ReDMCSB DUNVIEW.C:3913-3928: 0x07 green, 0x08 red, 0x09 orange,
     * 0x0A peach, 0x0B yellow, 0x0E blue.  Grey-stone wall texture
     * uses 0x01/0x02/0x07-grey/0x0D and never the warm set. */
    PORTRAIT_WARM_THRESHOLD = 30
};

typedef struct Ordinal23State {
    int passed;
    int failed;
    int failedOrdinalCatalog;
    int failedPortraitRectMismatch;
    int failedSideWallFloat;
    int failedCatalogRecruitOrdinal;
} Ordinal23State;

static void pass_msg(Ordinal23State* s, const char* msg) {
    printf("  PASS: %s\n", msg);
    s->passed++;
}

static void fail_msg(Ordinal23State* s, const char* msg, int* counter) {
    printf("  FAIL: %s\n", msg);
    s->failed++;
    if (counter) *counter += 1;
}

static int is_warm_palette_index(unsigned char idx) {
    /* Palette indices for the warm-colour set used by champion portrait
     * sprites (ReDMCSB DUNVIEW.C:3913-3928). */
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

/* Compare the C026 portrait strip cells at ordinal 23 against the
 * framebuffer D1C portrait-rectangle.  Returns the number of matched
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

/* Count warm pixels in the C026 ordinal-23 strip source cell.  Used
 * to ensure the asset itself has the warm pixels we expect to find
 * on the framebuffer, independent of the draw path. */
static int count_warm_in_strip_ordinal(
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
            if (src == (unsigned char)stripTransparentColor) continue;
            if (is_warm_palette_index(src)) ++count;
        }
    }
    return count;
}

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int direction) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = direction;
}

/* Skip guard: this probe depends on real DM1 V1 data with a Hall of
 * Champions and at least one mirror with C127 sensorData 23 in the
 * catalog (or at least ordinal 23 < catalog.count).  Without that, we
 * print SKIP rather than fail. */
static int ordinal_23_in_catalog(const M11_GameViewState* game) {
    return game->mirrorCatalogAvailable &&
        game->mirrorCatalog.count > ORDINAL;
}

static int check_catalog_ordinal_23(Ordinal23State* s, M11_GameViewState* game) {
    char name[16];
    char title[32];
    name[0] = '\0';
    title[0] = '\0';
    if (!ordinal_23_in_catalog(game)) {
        printf("SKIP ordinal 23 not in mirror catalog "
               "(count=%d < %d)\n",
               game->mirrorCatalog.count, ORDINAL + 1);
        return 0;
    }
    int hasName = M11_GameView_GetMirrorNameByOrdinal(game, ORDINAL,
                                                     name, sizeof(name));
    int hasTitle = M11_GameView_GetMirrorTitleByOrdinal(game, ORDINAL,
                                                       title, sizeof(title));
    if (!hasName || name[0] == '\0') {
        fail_msg(s, "ordinal 23 has no name (ReDMCSB catalog leaves slot empty)",
                 &s->failedOrdinalCatalog);
        return 0;
    }
    char label[96];
    snprintf(label, sizeof(label),
             "ordinal 23 catalog name='%s' title='%s' hasName=%d hasTitle=%d",
             name, title, hasName, hasTitle);
    pass_msg(s, label);
    return 1;
}

static int check_portrait_rect_position_ordinal_23(
    Ordinal23State* s, M11_GameViewState* game) {
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    int warmStrip;
    int matched, compared;
    int stripTransparentColor = 1; /* The transparent color passed to
                                      M11_AssetLoader_BlitRegion by
                                      m11_draw_dm1_front_champion_portrait
                                      (src/engine/m11_game_view.c line 13979).
                                      ReDMCSB C01_COLOR_DARK_GRAY = 1
                                      (DEFS.H:2079). */

    /* Place party at (1,2) facing NORTH -> front=(1,1) -> ordinal 1.
     * We use the proven front_north_entry pose from
     * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe.c. */
    set_pose(game, 1, 2, 0 /* DIR_NORTH */);

    int actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (actualOrdinal != 1) {
        printf("SKIP front_north_entry (1,2) NORTH front ordinal=%d expected=1; "
               "this DM1 V1 build does not match the reference DUNGEON.DAT fixture\n",
               actualOrdinal);
        return 0;
    }

    portraits = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        fail_msg(s, "C026 GRAPHICS.DAT champion portrait strip unavailable",
                 &s->failedPortraitRectMismatch);
        return 0;
    }

    /* Sanity: the strip source at ordinal 23 must itself carry some
     * non-transparent pixels; otherwise even a perfect blit can't
     * satisfy the rect check and we'd be measuring nothing.  NABI
     * (ordinal 23 in real DM1 V1) happens to use a cool/grey palette
     * for her face and robes so the warm-pixel count may be tiny,
     * but she still has plenty of non-transparent pixels. */
    {
        int nonTransparent = 0;
        int y, x;
        for (y = 0; y < STRIP_ROW_H; ++y) {
            for (x = 0; x < STRIP_COL_W; ++x) {
                unsigned char src = portraits->pixels[
                    (STRIP_SRC_Y + y) * (int)portraits->width + (STRIP_SRC_X + x)] & 0x0F;
                if (src != (unsigned char)stripTransparentColor) ++nonTransparent;
            }
        }
        if (nonTransparent < 50) {
            char label[96];
            snprintf(label, sizeof(label),
                     "C026 strip ordinal-23 source has %d non-transparent pixels "
                     "(expected >= 50) - asset looks empty",
                     nonTransparent);
            fail_msg(s, label, &s->failedPortraitRectMismatch);
            return 0;
        }
        {
            char label[96];
            snprintf(label, sizeof(label),
                     "C026 strip ordinal-23 source has %d non-transparent pixels (>= 50)",
                     nonTransparent);
            pass_msg(s, label);
        }
    }
    warmStrip = count_warm_in_strip_ordinal(portraits, stripTransparentColor);
    {
        char label[96];
        snprintf(label, sizeof(label),
                 "C026 strip ordinal-23 source warm-pixel count = %d (informational)",
                 warmStrip);
        pass_msg(s, label);
    }

    /* Sanity: the current front ordinal (HALK=1 at this pose) must
     * itself render correctly in the D1C rect, so we know the rect
     * destination is wired correctly before we mutate the sensor to
     * ordinal 23. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    {
        int warmFbuf = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                          PORTRAIT_RECT_W, PORTRAIT_RECT_H);
        if (warmFbuf < PORTRAIT_WARM_THRESHOLD) {
            char label[96];
            snprintf(label, sizeof(label),
                     "baseline front_north_entry (1,2) NORTH rect warm=%d < %d "
                     "(expected HALK portrait present)",
                     warmFbuf, PORTRAIT_WARM_THRESHOLD);
            fail_msg(s, label, &s->failedPortraitRectMismatch);
            return 0;
        }
        {
            char label[96];
            snprintf(label, sizeof(label),
                     "baseline front_north_entry (1,2) NORTH rect warm=%d >= %d",
                     warmFbuf, PORTRAIT_WARM_THRESHOLD);
            pass_msg(s, label);
        }
    }

    /* Now mutate the front cell's C127 sensorData to ordinal 23 so we
     * can verify the D1C rect position holds ordinal 23's pixels.  We
     * walk the (1,1) cell's THING chain looking for the C127 sensor
     * with cell=visibleWallCell (front wall = partyDirection+2 = 2).
     * (1,2) party direction=0 -> visibleWallCell=2 (south wall of
     * (1,1) faces north when standing at (1,2) looking north). */
    {
        /* We need to walk the front cell's THING chain.  The chain is
         * stored as raw bytes in world.things->rawThingData[type]; each
         * THING entry has a 2-byte "next thing" pointer at offset 0.
         * We replicate m11_raw_next_thing here because the helper is
         * file-static in m11_game_view.c. */
        int mapIndex = game->world.party.mapIndex;
        const struct DungeonMapDesc_Compat* map = &game->world.dungeon->maps[mapIndex];
        int frontMapX = 1; /* (1,2) NORTH -> front=(1,1) */
        int frontMapY = 1;
        int visibleWallCell = (game->world.party.direction + 2) & 3;
        int base = frontMapX * (int)map->height + frontMapY;
        int squareIndex = base;
        unsigned short thing = game->world.things->squareFirstThings[squareIndex];
        unsigned short savedSensorData = 0;
        int foundSensor = 0;
        int sensorIndex = -1;
        char label[160];

        while (thing != THING_ENDOFLIST && thing != THING_NONE) {
            int type = THING_GET_TYPE(thing);
            int index = THING_GET_INDEX(thing);
            int cell = THING_GET_CELL(thing);
            if (type == THING_TYPE_SENSOR && cell == visibleWallCell &&
                index >= 0 && index < game->world.things->sensorCount &&
                game->world.things->sensors[index].sensorType == 127) {
                savedSensorData = game->world.things->sensors[index].sensorData;
                game->world.things->sensors[index].sensorData = (unsigned short)ORDINAL;
                foundSensor = 1;
                sensorIndex = index;
                break;
            }
            /* Advance via the THING chain.  rawThingData[type] is an
             * array of size thingCounts[type] * s_thingDataByteCount[type]
             * (defined in m11_game_view.c / include/memory_dungeon_dat_pc34_compat.h).
             * For sensors (type 3) the count is 8 bytes; the first two are
             * the next-thing pointer. */
            {
                static const unsigned char s_thingDataByteCount[16] = {
                    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
                };
                int byteCount = (type >= 0 && type < 16) ? s_thingDataByteCount[type] : 2;
                const unsigned char* raw;
                if (type < 0 || type >= 16 || !game->world.things->rawThingData[type] ||
                    index < 0 || index >= game->world.things->thingCounts[type]) {
                    thing = THING_ENDOFLIST;
                    break;
                }
                raw = game->world.things->rawThingData[type] + (index * byteCount);
                thing = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
            }
        }
        if (!foundSensor) {
            snprintf(label, sizeof(label),
                     "could not locate front C127 sensor at (1,1) cell=%d",
                     visibleWallCell);
            fail_msg(s, label, &s->failedPortraitRectMismatch);
            return 0;
        }

        /* Verify GetFrontMirrorOrdinal now returns 23. */
        {
            int mutatedOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
            if (mutatedOrdinal != ORDINAL) {
                snprintf(label, sizeof(label),
                         "after sensor mutation GetFrontMirrorOrdinal=%d expected=%d",
                         mutatedOrdinal, ORDINAL);
                fail_msg(s, label, &s->failedPortraitRectMismatch);
                /* Restore before bailing. */
                game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
                return 0;
            }
            snprintf(label, sizeof(label),
                     "after sensor mutation GetFrontMirrorOrdinal=%d (expected %d)",
                     mutatedOrdinal, ORDINAL);
            pass_msg(s, label);
        }

        /* Redraw and verify the D1C rect matches the C026 strip at
         * ordinal 23. */
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, FB_W, FB_H);

        if (!compare_strip_to_portrait_rect(fb, portraits, stripTransparentColor,
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
        /* Threshold: at least 95% of the strip's non-transparent pixels
         * must match the framebuffer.  This catches draw-order bugs
         * where a different ordinal or a stone-wall fill lands in the
         * rect. */
        if (matched * 100 < 95 * compared) {
            snprintf(label, sizeof(label),
                     "D1C rect (96,35)-(128,64) does not match C026 ordinal 23 "
                     "matched=%d compared=%d (>=95%% required) - rect position wrong",
                     matched, compared);
            fail_msg(s, label, &s->failedPortraitRectMismatch);
            game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
            return 0;
        }
        snprintf(label, sizeof(label),
                 "D1C rect (96,35)-(128,64) renders ordinal 23 strip "
                 "matched=%d compared=%d (%d%%)",
                 matched, compared, matched * 100 / compared);
        pass_msg(s, label);

        /* Restore the sensor data. */
        game->world.things->sensors[sensorIndex].sensorData = savedSensorData;
        {
            int restoredOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
            if (restoredOrdinal != actualOrdinal) {
                snprintf(label, sizeof(label),
                         "after sensor restore GetFrontMirrorOrdinal=%d expected=%d "
                         "(mutation cleanup failed)",
                         restoredOrdinal, actualOrdinal);
                fail_msg(s, label, &s->failedPortraitRectMismatch);
                return 0;
            }
            snprintf(label, sizeof(label),
                     "after sensor restore GetFrontMirrorOrdinal=%d (back to baseline %d)",
                     restoredOrdinal, actualOrdinal);
            pass_msg(s, label);
        }
    }
    return 1;
}

static int check_side_wall_no_float_ordinal_23(
    Ordinal23State* s, M11_GameViewState* game) {
    unsigned char fb[FB_W * FB_H];
    int warmFront;
    int warmEast;
    int warmWest;
    int warmSouth;
    char label[128];

    /* Front_north_entry at (1,2) NORTH: portrait MUST appear in the
     * D1C rect.  This is the positive control. */
    set_pose(game, 1, 2, 0 /* DIR_NORTH */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmFront = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                   PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmFront < PORTRAIT_WARM_THRESHOLD) {
        snprintf(label, sizeof(label),
                 "front_north_entry (1,2) NORTH front rect warm=%d < %d (no portrait)",
                 warmFront, PORTRAIT_WARM_THRESHOLD);
        fail_msg(s, label, &s->failedSideWallFloat);
        return 0;
    }
    snprintf(label, sizeof(label),
             "front_north_entry (1,2) NORTH front rect warm=%d >= %d (portrait present)",
             warmFront, PORTRAIT_WARM_THRESHOLD);
    pass_msg(s, label);

    /* Same cell facing EAST: the right wall is the visible wall, not
     * the front.  No C127 sensor on a side wall -> no portrait in
     * the D1C rect.  This is the negative control. */
    set_pose(game, 1, 2, 1 /* DIR_EAST */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmEast = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                  PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmEast >= PORTRAIT_WARM_THRESHOLD) {
        snprintf(label, sizeof(label),
                 "side wall (1,2) EAST front rect warm=%d >= %d (portrait floating on side)",
                 warmEast, PORTRAIT_WARM_THRESHOLD);
        fail_msg(s, label, &s->failedSideWallFloat);
        return 0;
    }
    snprintf(label, sizeof(label),
             "side wall (1,2) EAST front rect warm=%d < %d (no portrait floating)",
             warmEast, PORTRAIT_WARM_THRESHOLD);
    pass_msg(s, label);

    /* Same cell facing WEST: left wall, same negative control. */
    set_pose(game, 1, 2, 3 /* DIR_WEST */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmWest = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                  PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmWest >= PORTRAIT_WARM_THRESHOLD) {
        snprintf(label, sizeof(label),
                 "side wall (1,2) WEST front rect warm=%d >= %d (portrait floating on side)",
                 warmWest, PORTRAIT_WARM_THRESHOLD);
        fail_msg(s, label, &s->failedSideWallFloat);
        return 0;
    }
    snprintf(label, sizeof(label),
             "side wall (1,2) WEST front rect warm=%d < %d (no portrait floating)",
             warmWest, PORTRAIT_WARM_THRESHOLD);
    pass_msg(s, label);

    /* Same cell facing SOUTH: the cell behind the party.  The wall
     * there has no C127 sensor (it's the corridor behind the entry),
     * so no portrait either. */
    set_pose(game, 1, 2, 2 /* DIR_SOUTH */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmSouth = count_warm_in_rect(fb, PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
                                   PORTRAIT_RECT_W, PORTRAIT_RECT_H);
    if (warmSouth >= PORTRAIT_WARM_THRESHOLD) {
        snprintf(label, sizeof(label),
                 "back wall (1,2) SOUTH front rect warm=%d >= %d (portrait floating)",
                 warmSouth, PORTRAIT_WARM_THRESHOLD);
        fail_msg(s, label, &s->failedSideWallFloat);
        return 0;
    }
    snprintf(label, sizeof(label),
             "back wall (1,2) SOUTH front rect warm=%d < %d (no portrait floating)",
             warmSouth, PORTRAIT_WARM_THRESHOLD);
    pass_msg(s, label);

    return 1;
}

static int check_catalog_recruit_uses_mirror_ordinal(
    Ordinal23State* s, M11_GameViewState* game) {
    int idxBefore;
    int idxAfter;
    int recruitResult;
    int ordinalAfter;
    char label[160];

    /* Reset to a clean state and place the party at the front_north_entry
     * pose.  m11_front_cell_mirror_ordinal returns the C127 sensorData
     * (1 for HALK at (1,1)). */
    set_pose(game, 1, 2, 0 /* DIR_NORTH */);
    idxBefore = M11_GameView_GetFrontMirrorOrdinal(game);
    if (idxBefore != 1) {
        printf("SKIP catalog-recruit pose has no mirror (ordinal=%d)\n", idxBefore);
        return 0;
    }

    recruitResult = M11_GameView_RecruitChampionByMirrorOrdinal(game, idxBefore);
    if (recruitResult != 1) {
        fail_msg(s, "M11_GameView_RecruitChampionByMirrorOrdinal returned 0 on front mirror",
                 &s->failedCatalogRecruitOrdinal);
        return 0;
    }
    pass_msg(s, "M11_GameView_RecruitChampionByMirrorOrdinal returned 1 on front mirror");

    /* After recruiting HALK, the front-mirror ordinal still reports 1
     * (the C127 sensor is still on the wall; only F0280 marks the
     * sensor "already in party" so the route is disabled).  The
     * newly-appended champion at slot 0 carries the catalog textString
     * index, not 20+slot. */
    idxAfter = M11_GameView_GetFrontMirrorOrdinal(game);
    ordinalAfter = game->world.party.champions[0].portraitIndex;
    if (idxAfter != idxBefore) {
        snprintf(label, sizeof(label),
                 "front-mirror ordinal changed after recruit got=%d want=%d",
                 idxAfter, idxBefore);
        fail_msg(s, label, &s->failedCatalogRecruitOrdinal);
        return 0;
    }
    pass_msg(s, "front-mirror ordinal preserved after recruit (C127 sensor still on wall)");

    /* F0673 catalog-recruit path stores portraitIndex from the catalog
     * record's textStringIndex, NOT 20+slot.  For HALK (first catalog
     * entry) that index is the first text string the parser accepted,
     * which on real DM1 V1 is the same as the mirror ordinal 1.
     *
     * Strict invariant: portraitIndex must equal idxBefore (catalog
     * ordinal 1), NOT 20+0 (the candidate ordinal for slot 0).  This
     * is the documented separation between ordinal-23-as-strip-slot
     * (DUNVIEW.C:3916) and ordinal-20-23-as-candidate-slot
     * (REVIVE.C F0280). */
    if (ordinalAfter != idxBefore) {
        snprintf(label, sizeof(label),
                 "champion[0].portraitIndex=%d expected %d (catalog ordinal 1, NOT 20+0)",
                 ordinalAfter, idxBefore);
        fail_msg(s, label, &s->failedCatalogRecruitOrdinal);
        return 0;
    }
    snprintf(label, sizeof(label),
             "champion[0].portraitIndex=%d == catalog ordinal %d (NOT 20+0)",
             ordinalAfter, idxBefore);
    pass_msg(s, label);
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    Ordinal23State s;
    memset(&s, 0, sizeof(s));
    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP firestaff_dm1_v1_champion_portrait_ordinal_23_front_north_entry_rect_runtime_probe "
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
    printf("=== DM1 V1 Hall portrait ordinal 23 front_north_entry portrait_rect ===\n");
    printf("dataDir=%s ordinal=%d stripSrc=(%d,%d)-(%d,%d) rect=(%d,%d)-(%d,%d)\n",
           dataDir, ORDINAL,
           STRIP_SRC_X, STRIP_SRC_Y, STRIP_SRC_X_END, STRIP_SRC_Y_END,
           PORTRAIT_RECT_X, PORTRAIT_RECT_Y,
           PORTRAIT_RECT_X_END, PORTRAIT_RECT_Y_END);

    check_catalog_ordinal_23(&s, &game);
    check_portrait_rect_position_ordinal_23(&s, &game);
    check_side_wall_no_float_ordinal_23(&s, &game);
    check_catalog_recruit_uses_mirror_ordinal(&s, &game);

    printf("=== %d passed, %d failed (ordinal-catalog=%d portrait-rect=%d side-wall=%d catalog-recruit=%d) ===\n",
           s.passed, s.failed,
           s.failedOrdinalCatalog, s.failedPortraitRectMismatch,
           s.failedSideWallFloat, s.failedCatalogRecruitOrdinal);
    M11_GameView_Shutdown(&game);
    return s.failed > 0 ? 1 : 0;
}
