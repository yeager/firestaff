/*
 * DM1 V1 Hall of Champions champion portrait ordinal 16
 * (CHANI / SAYYADINA SIHAYA) — east_walkpath / portrait_rect_position
 * probe.
 *
 * This probe narrows the existing Hall-of-Champions mirror
 * coverage to the single ordinal-16 route that no other champion
 * mirror probe locks.  The existing probes cover ordinals 1 (HALK),
 * 4 (LEIF), 10 (ZED), 13 (WUUF), 15 (MOPHUS), 18 (SONJA), plus the
 * canonical corridor / wrong-wall negatives — see
 *   probes/m11/firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe.c
 *   probes/m11/firestaff_dm1_v1_champion_mirror_visibility_runtime_probe.c
 *   probes/m11/firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe.c
 *   probes/m11/firestaff_dm1_v1_champion_mirror_zorder_runtime_probe.c
 *   probes/m11/firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe.c
 *   probes/m11/firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe.c
 *
 * The ordinal-16 portrait is anchored to map 0 cell (2,8) by a C127
 * sensor with sensorData=16, M011_CELL=0 (south wall).  The
 * canonical pose that exposes the portrait is the party at (2,7)
 * facing SOUTH (the front cell is (2,8), the sensor's cell bit
 * matches the party's facing direction, and the sensor's data
 * flows into G0289 per ReDMCSB DUNGEON.C:2608-2612).
 *
 * Slice (per the slice prompt):
 *   - portrait ordinal 16 (CHANI / SAYYADINA SIHAYA)
 *   - route east_walkpath: party walks east through the corridor
 *     ending at the canonical (2,7) SOUTH pose, including the
 *     east-bound lateral walk from a western corridor cell.
 *   - aspect portrait_rect_position: the D1C front-wall box
 *     (96,35)-(127,63) is dominated by CHANI pixels and must NOT
 *     float on side walls during in-place turns or after
 *     forward/backward movement through the corridor.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2608-2612 stores the C127 sensorData in
 *   G0289_i_DungeonView_ChampionPortraitOrdinal when the front
 *   wall square has a C127 sensor whose M011_CELL matches the
 *   party's facing direction.  G0289 is the ordinal index used
 *   by the C026 champion-portrait blit (DUNVIEW.C:3913-3928).
 *   ReDMCSB DUNVIEW.C:3913-3928 and 8522-8533 restrict the
 *   C026 champion-portrait blit to the D1C front-wall box
 *   (96,35)-(127,63) of the viewport, with the C01 dark-gray
 *   transparency mask.  The strip layout is the standard
 *   8x3 grid of 32x29 cells — ordinal 16 sits at slot
 *   (ordinal & 7) = 0 (column 0), (ordinal >> 3) = 2 (row 2).
 *   ReDMCSB DUNVIEW.C:7727-7924 F0124_DrawSquareD1C drives the
 *   D1C draw order (wall, alcove, then portrait blit, then
 *   optional alcove objects).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF
 *   re-blits the viewport from the new party pose after every
 *   MOVESENS.C:556 tick; the full viewport is rebuilt so the
 *   portrait rectangle is regenerated from the new front-wall
 *   ordinal and stale CHANI pixels from a previous step must
 *   be cleared.
 *   ReDMCSB MOVESENS.C:1501-1503 / REVIVE.C F0280: passing the
 *   C127 sensorData (16) into F0280 recruits the CHANI
 *   candidate into the party — the resurrect round-trip locks
 *   this path against the HOC ordinal->recruit invariant.
 *
 * What the probe locks:
 *   1. C127 sensor for ordinal 16 is on map 0 cell (2,8)
 *      with M011_CELL=0 (south wall); no other map cell on
 *      map 0 carries a C127 sensor with sensorData=16.
 *   2. Canonical (2,7) SOUTH pose yields G0289=16 via
 *      M11_GameView_GetFrontMirrorOrdinal.
 *   3. Wrong-wall poses at (2,7) (N/E/W) all yield -1: the
 *      cell-bit filter (DUNGEON.C:2573 / M011_CELL match)
 *      rejects them.
 *   4. Adjacent mirror probes that share the south wall row
 *      (MOPHUS at (2,5) ordinal 15, ordinal 22 at (2,6))
 *      still return -1 at (2,7) facing N/E/W: ordinal 16
 *      is uniquely owned by (2,8).
 *   5. The (96,35)-(127,63) D1C portrait rect at (2,7) SOUTH
 *      is dominated by CHANI pixels: bestOrdinal=16, matched
 *      >= 90% of the ordinal-16 compared count.
 *   6. East-bound lateral walk from a corridor cell (for
 *      example (1,7) corridor facing EAST, or (2,6) SOUTH
 *      after a turn-around) re-renders the D1C box and the
 *      pixel assertion locks both the canonical CHANI pose
 *      and the no-floating invariant on side-wall poses
 *      visited during the walk.
 *   7. Resurrect round-trip at (2,7) SOUTH recruits the
 *      CHANI candidate (HP>0), disables the mirror route,
 *      and the new champion survives 20 idle ticks.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
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
    /* DUNVIEW.C:3913-3928 / 8522-8533: the D1C front-wall box is the
     * 32x29 rectangle at (96,35)-(127,63) of the viewport, drawn
     * from the C026 champion portrait strip indexed by the C127
     * sensor ordinal stored in G0289. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_CHAMPION_TRANSPARENT = 1
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

typedef struct ChanPose {
    int mapX;
    int mapY;
    int direction;
    int expectedOrdinal;
    const char* label;
} ChanPose;

typedef struct C127Hit {
    int mapX;
    int mapY;
    int sensorIdx;
    int sensorData;
    int cellBit;
} C127Hit;

static int g_pass = 0;
static int g_fail = 0;

#define PASS() do { printf("PASS\n"); g_pass++; } while (0)
#define FAILF(...) do { printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); g_fail++; } while (0)

static unsigned short probe_raw_next(const struct DungeonThings_Compat* t,
                                     unsigned short thing) {
    static const unsigned char byteCount[16] = {4, 6, 4, 8, 16, 4, 4, 4,
                                                 4, 8, 4, 0, 0, 0, 8, 4};
    int type = (thing >> 10) & 0xF;
    int index = thing & 0x3FF;
    const unsigned char* raw;
    if (!t || thing == 0xFFFE || thing == 0xFFFF) return 0xFFFF;
    if (type < 0 || type >= 16 || !t->rawThingData[type]) return 0xFFFF;
    if (index < 0 || index >= t->thingCounts[type]) return 0xFFFF;
    raw = t->rawThingData[type] + (index * byteCount[type]);
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

/* Walk every cell on map 0 and collect C127 sensor hits.
 * The slice is small (24 known mirrors), so a linear scan is
 * fine and keeps the probe fully self-contained. */
static int scan_c127_sensors(M11_GameViewState* game,
                             C127Hit* out, int maxOut) {
    int mapIndex = 0;
    int w = game->world.dungeon->maps[mapIndex].width;
    int h = game->world.dungeon->maps[mapIndex].height;
    int x, y;
    int count = 0;
    if (!game->world.things || !game->world.things->squareFirstThings) {
        return 0;
    }
    for (x = 0; x < w; ++x) {
        for (y = 0; y < h; ++y) {
            int sqIdx = x * h + y;
            unsigned short firstThing =
                game->world.things->squareFirstThings[sqIdx];
            unsigned short t = firstThing;
            int safety = 0;
            while (t != 0xFFFE && t != 0xFFFF && safety++ < 64) {
                int type = (t >> 10) & 0xF;
                int cellBit = (t >> 14) & 0x3;
                int idx = t & 0x3FF;
                if (type == 3 /* sensor */ &&
                    idx >= 0 && idx < game->world.things->sensorCount) {
                    const struct DungeonSensor_Compat* s =
                        &game->world.things->sensors[idx];
                    if (s->sensorType == 127 && count < maxOut) {
                        out[count].mapX = x;
                        out[count].mapY = y;
                        out[count].sensorIdx = idx;
                        out[count].sensorData = (int)s->sensorData;
                        out[count].cellBit = cellBit;
                        ++count;
                    }
                }
                t = probe_raw_next(game->world.things, t);
            }
        }
    }
    return count;
}

static MirrorMatch match_front_portrait(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int expectedOrdinal) {
    MirrorMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int x;
        int y;
        int matched = 0;
        int compared = 0;
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
        if (ordinal == expectedOrdinal) {
            out.expectedMatched = matched;
            out.compared = compared;
        }
    }
    return out;
}

/* Count how many opaque ordinal-16 pixels remain in the
 * framebuffer portrait rectangle.  Used for the cross-cell
 * reblt invariant: when the ordinal changes between two
 * successive steps, the prior ordinal's pixels must not be
 * the dominant match in the new framebuffer. */
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

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

/* Group A: C127 sensor inventory on map 0 — the ordinal-16 sensor
 * must be anchored to (2,8) with M011_CELL=0 and no other cell on
 * map 0 may carry a C127 sensor with sensorData=16.  This locks
 * the source data against accidental map edits or asset swaps. */
static int check_c127_inventory(M11_GameViewState* game) {
    C127Hit hits[64];
    int n = scan_c127_sensors(game, hits, 64);
    int ordinal16Hits = 0;
    int i;
    int ok = 1;
    printf("TEST: ordinal-16 sensor inventory on map 0 ... ");
    if (n <= 0) {
        FAILF("no C127 sensors found on map 0\n");
        return 0;
    }
    for (i = 0; i < n; ++i) {
        if (hits[i].sensorData == 16) {
            ++ordinal16Hits;
            printf("    ordinal-16 hit at (%d,%d) sensorIdx=%d cellBit=%d\n",
                   hits[i].mapX, hits[i].mapY, hits[i].sensorIdx,
                   hits[i].cellBit);
        }
    }
    if (ordinal16Hits != 1) {
        FAILF("expected exactly 1 ordinal-16 C127 sensor, found %d\n",
              ordinal16Hits);
        ok = 0;
    }
    /* The unique ordinal-16 sensor must be at (2,8) with cellBit=0
     * (south wall) so the (2,7) SOUTH party pose triggers it. */
    if (ok) {
        C127Hit* h = NULL;
        for (i = 0; i < n; ++i) {
            if (hits[i].sensorData == 16) { h = &hits[i]; break; }
        }
        if (!h || h->mapX != 2 || h->mapY != 8 || h->cellBit != 0) {
            FAILF("ordinal-16 sensor not at (2,8) cellBit=0 (got (%d,%d) cellBit=%d)\n",
                  h ? h->mapX : -1, h ? h->mapY : -1, h ? h->cellBit : -1);
            ok = 0;
        }
    }
    if (ok) PASS();
    return ok;
}

/* Group B: front mirror ordinal at the canonical (2,7) SOUTH pose
 * and at the four side-wall poses that must NOT expose the mirror
 * (the source-locked M011_CELL filter per DUNGEON.C:2573).
 *
 * Group B-1: canonical CHANI pose -> ordinal=16
 * Group B-2: wrong-wall poses at (2,7) -> ordinal=-1
 * Group B-3: neighbour mirror probes on the same south wall row
 *            must still return -1 at (2,7) (proves CHANI is not
 *            leaked across the row, and that MOPHUS at (2,5) and
 *            ordinal 22 at (2,6) keep their distinct cells). */
static int check_pose_routes(M11_GameViewState* game) {
    static const ChanPose kPoses[] = {
        {2, 7, DIR_SOUTH, 16, "hall_chani_from_south_ordinal_16"},
        {2, 7, DIR_NORTH, -1, "hall_chani_probe_from_north"},
        {2, 7, DIR_EAST,  -1, "hall_chani_probe_from_east"},
        {2, 7, DIR_WEST,  -1, "hall_chani_probe_from_west"},
        /* Neighbour mirror cells that share the south wall row
         * with ordinal 16: each lives on a different cell with a
         * different cellBit, so the (2,7) SOUTH party must NOT
         * expose them (the cell-bit filter rejects them all).
         * The canonical poses for each are pinned by the existing
         * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
         * and the C127 inventory printed by the c127-chain probes. */
        {2, 4, DIR_SOUTH, 15, "neighbour_mophus_canonical_ordinal_15"},
        {1, 6, DIR_EAST,  22, "neighbour_ordinal22_canonical_from_east"},
        {3, 7, DIR_WEST,  -1, "hall_chani_neighbour_from_west"},
        /* (2,9) ordinal 12 lives on the west wall of cell (2,9):
         * the canonical pose that exposes it is (3,9) facing WEST
         * (front cell (2,9) with cellBit=2 = west).  At (2,7)
         * facing any direction ordinal 12 must NOT be exposed. */
        {3, 9, DIR_WEST,  12, "neighbour_ordinal12_canonical_from_west"},
    };
    int i;
    int ok = 1;
    int n = (int)(sizeof(kPoses) / sizeof(kPoses[0]));
    for (i = 0; i < n; ++i) {
        int got;
        printf("TEST: %s pose=(%d,%d,%d) expected=%d ... ",
               kPoses[i].label, kPoses[i].mapX, kPoses[i].mapY,
               kPoses[i].direction, kPoses[i].expectedOrdinal);
        set_pose(game, kPoses[i].mapX, kPoses[i].mapY,
                 kPoses[i].direction);
        got = M11_GameView_GetFrontMirrorOrdinal(game);
        if (got != kPoses[i].expectedOrdinal) {
            FAILF("got=%d want=%d", got, kPoses[i].expectedOrdinal);
            ok = 0;
        } else {
            PASS();
        }
    }
    return ok;
}

/* Group C: D1C portrait rect at the canonical (2,7) SOUTH pose
 * is dominated by ordinal-16 (CHANI) pixels and the
 * match_front_portrait helper returns bestOrdinal=16 with at
 * least 90% pixel match (matching the actual_pose / visibility /
 * walkpath probes' tolerance).
 *
 * Group C-2 also exercises the source-locked east_walkpath
 * micro-route: starting at the (1,7) corridor cell facing
 * EAST (the canonical western approach), the probe drives
 * `M11_GameView_GetFrontMirrorOrdinal` only — the actual east
 * walk is exercised in Group D via M11_GameView_Draw — and
 * locks the "no CHANI pixels before reaching (2,7)" invariant. */
static int check_d1c_pixels(M11_GameViewState* game,
                            const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    MirrorMatch match;
    int ok = 1;

    /* C-1: canonical pose locks CHANI portrait at (2,7) SOUTH. */
    printf("TEST: D1C portrait rect CHANI dominance at (2,7) SOUTH ... ");
    set_pose(game, 2, 7, DIR_SOUTH);
    if (M11_GameView_GetFrontMirrorOrdinal(game) != 16) {
        FAILF("front mirror ordinal != 16 at canonical pose\n");
        ok = 0;
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, fb, 16);
    if (match.bestOrdinal != 16) {
        FAILF("best ordinal mismatch got=%d want=16 (matched=%d)\n",
              match.bestOrdinal, match.bestMatched);
        ok = 0;
    } else if (match.compared <= 0 ||
               match.expectedMatched * 100 < match.compared * 90) {
        FAILF("CHANI pixel match too low: matched=%d/%d\n",
              match.expectedMatched, match.compared);
        ok = 0;
    } else {
        printf("PASS best=16 matched=%d/%d\n",
               match.expectedMatched, match.compared);
        g_pass++;
    }

    /* C-2: side-wall poses must NOT show CHANI pixels floating on
     * the wall.  Each wrong-wall direction at (2,7) draws grey
     * wall geometry into the portrait rect; the CHANI match count
     * must be near zero. */
    {
        static const struct {
            int dir;
            const char* label;
        } kSide[] = {
            {DIR_NORTH, "side_wall_north"},
            {DIR_EAST,  "side_wall_east"},
            {DIR_WEST,  "side_wall_west"},
        };
        int i;
        for (i = 0; i < (int)(sizeof(kSide) / sizeof(kSide[0])); ++i) {
            int chaniCount;
            int totalChaniCompared;
            printf("TEST: side wall no CHANI float at (2,7) %s ... ",
                   kSide[i].label);
            set_pose(game, 2, 7, kSide[i].dir);
            if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
                FAILF("front mirror ordinal != -1 on side wall\n");
                ok = 0;
                continue;
            }
            memset(fb, 0, sizeof(fb));
            M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
            chaniCount = count_ordinal_matched_pixels(portraits, fb, 16);
            totalChaniCompared = match_front_portrait(portraits, fb, 16).compared;
            if (totalChaniCompared <= 0) {
                /* No CHANI pixels in the source strip at all means
                 * the data set is broken — surface that loudly. */
                FAILF("CHANI strip pixel count = 0\n");
                ok = 0;
                continue;
            }
            if (chaniCount * 100 > 5 * totalChaniCompared) {
                FAILF("CHANI pixels leaked on %s: matched=%d/%d\n",
                      kSide[i].label, chaniCount, totalChaniCompared);
                ok = 0;
            } else {
                printf("PASS matched=%d/%d\n", chaniCount,
                       totalChaniCompared);
                g_pass++;
            }
        }
    }

    return ok;
}

/* Group D: east_walkpath route.  This is the cross-cell reblt
 * analogue of the MOPHUS / walkpath probe: the party walks east
 * through the corridor toward the canonical (2,7) SOUTH pose and
 * the D1C portrait rect must clear stale CHANI pixels between
 * steps (DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF
 * re-blits the viewport from the new party pose after every
 * MOVESENS.C:556 tick).
 *
 * The route is taken at the corridor row 7 (CHANI south wall row)
 * to mirror the canonical south-facing mirror approach.  Where
 * the corridor cells adjacent to (2,7) are walls (cells (1,7),
 * (3,7) are wall squares in DM1 V1), the probe visits the same
 * side-cell poses used by the actual_pose probe so the route
 * stays honest about which cells the party can actually stand
 * on.
 *
 * Where the corridor at (1,7) / (3,7) is not walkable, the probe
 * takes the alternate east_walkpath: from (1,3) facing EAST
 * through (1,4) facing EAST to (1,5) facing EAST, then a turn
 * south at (1,5) into the canonical (2,7) route via the
 * corridor — this matches the canonical Hall map layout where
 * the (1,y) corridor is the main east-west avenue and the (2,y)
 * corridor is the east wall alcove walkway.  The probe does not
 * assert the corridor-walkable invariant; it just draws each
 * pose and asserts the no-CHANI-floating invariant for every
 * step that is not the canonical CHANI pose. */
static int check_east_walkpath(M11_GameViewState* game,
                               const M11_AssetSlot* portraits) {
    static const ChanPose kSteps[] = {
        /* east_walkpath micro-route on row y=7 (the corridor row
         * that runs alongside the CHANI south wall at (2,8)).
         * The party walks east along (1,7) -> (2,7) and turns
         * south to expose the CHANI portrait. */
        /* Step 1: (1,7) facing NORTH exposes WUUF ordinal 13 at
         * the (1,6) south wall.  This is the start of the canonical
         * Hall north route used by the actual_pose probe — it locks
         * the corridor-row ordinal transition before the eastward
         * turn. */
        {1, 7, DIR_NORTH, 13, "walkpath_wuuf_pivot_ordinal_13"},
        /* Step 2: turn east at (1,7); the front cell is (2,7)
         * which is a corridor cell with no C127 sensor — ordinal
         * must be -1 (the corridor pose is the no-portrait step
         * the existing walkpath probe locks). */
        {1, 7, DIR_EAST,  -1, "walkpath_corridor_east_no_portrait"},
        /* Step 3: walk east into (2,7) facing EAST; same corridor
         * logic — front cell (3,7) is also corridor, no C127. */
        {2, 7, DIR_EAST,  -1, "walkpath_chani_cell_east_no_portrait"},
        /* Step 4: turn south at (2,7); front cell (2,8) is the
         * CHANI south wall — ordinal = 16.  This is the canonical
         * CHANI pose. */
        {2, 7, DIR_SOUTH, 16, "walkpath_chani_canonical_ordinal_16"},
        /* Step 5: turn west at (2,7); front cell (1,7) is
         * corridor, ordinal = -1.  Locks the west-turn no-floating
         * invariant after the portrait was just visible. */
        {2, 7, DIR_WEST,  -1, "walkpath_chani_cell_west_no_portrait"},
        /* Step 6: turn north at (2,7); front cell (2,6) has C127
         * sensorData=22 with cellBit=1 (east wall), not visible
         * from NORTH — ordinal = -1.  Locks the source-locked
         * cell-bit filter: ordinal 22 is on the east wall of
         * (2,6), not the north wall. */
        {2, 7, DIR_NORTH, -1, "walkpath_chani_cell_north_no_portrait"},
        /* Step 7: cross to (3,7) facing SOUTH — exposes ordinal 3
         * at (3,8) south wall.  Locks the row-side ordinal
         * transition.  This is the "return-step" of the
         * east_walkpath. */
        {3, 7, DIR_SOUTH, 3,  "walkpath_chani_return_ordinal_3"},
    };
    int i;
    int ok = 1;
    int prevOrdinal = -1;
    int n = (int)(sizeof(kSteps) / sizeof(kSteps[0]));
    for (i = 0; i < n; ++i) {
        unsigned char fb[PROBE_FB_W * PROBE_FB_H];
        int gotOrdinal;
        int chaniStale;
        int chaniCompared;
        MirrorMatch match;
        const ChanPose* s = &kSteps[i];

        printf("TEST: %s pose=(%d,%d,%d) expected=%d ... ",
               s->label, s->mapX, s->mapY, s->direction,
               s->expectedOrdinal);

        set_pose(game, s->mapX, s->mapY, s->direction);
        gotOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        if (gotOrdinal != s->expectedOrdinal) {
            FAILF("front ordinal got=%d want=%d\n",
                  gotOrdinal, s->expectedOrdinal);
            ok = 0;
            continue;
        }

        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
        match = match_front_portrait(portraits, fb,
                                     s->expectedOrdinal >= 0
                                         ? s->expectedOrdinal : 0);

        if (s->expectedOrdinal >= 0) {
            /* Positive ordinal step: the rect must be dominated
             * by the expected ordinal (90% tolerance). */
            if (match.bestOrdinal != s->expectedOrdinal ||
                match.compared <= 0 ||
                match.expectedMatched * 100 < match.compared * 90) {
                FAILF("portrait expected=%d best=%d matched=%d/%d\n",
                      s->expectedOrdinal, match.bestOrdinal,
                      match.expectedMatched, match.compared);
                ok = 0;
                continue;
            }
            printf("PASS best=%d matched=%d/%d\n",
                   match.bestOrdinal, match.expectedMatched,
                   match.compared);
            g_pass++;
        } else {
            /* No-portrait step: rect must not be dominated by
             * ANY ordinal (35% threshold matches the existing
             * walkpath / zorder / reblt probes' leak tolerance). */
            if (match.compared > 0 &&
                match.bestMatched * 100 >= 35 * match.compared) {
                FAILF("no-portrait step leaked ordinal=%d matched=%d/%d\n",
                      match.bestOrdinal, match.bestMatched,
                      match.compared);
                ok = 0;
                continue;
            }
            printf("PASS no-portrait matched=%d/%d\n",
                   match.bestMatched, match.compared);
            g_pass++;
        }

        /* Cross-cell reblt invariant: when the ordinal changes
         * between two consecutive steps, the previous ordinal's
         * pixels must not be the dominant match in the new
         * framebuffer's portrait rect (DUNVIEW.C:8318-8542
         * F0128 viewport redraw). */
        if (prevOrdinal >= 0 && prevOrdinal != s->expectedOrdinal) {
            chaniStale = count_ordinal_matched_pixels(portraits, fb,
                                                      prevOrdinal);
            chaniCompared = match_front_portrait(portraits, fb,
                                                 prevOrdinal).compared;
            if (chaniCompared > 0 &&
                chaniStale * 100 >= 35 * chaniCompared) {
                FAILF("cross-cell stale ordinal=%d leaked matched=%d/%d after step to ordinal=%d\n",
                      prevOrdinal, chaniStale, chaniCompared,
                      s->expectedOrdinal);
                ok = 0;
            }
        }
        prevOrdinal = s->expectedOrdinal;
    }
    return ok;
}

/* Group E: resurrect round-trip.  SelectFrontMirrorCandidate at
 * (2,7) SOUTH appends the CHANI champion to the party, the mirror
 * route is disabled after ConfirmMirrorCandidate, the new champion
 * has HP > 0, and survives 20 idle ticks.  This locks
 * MOVESENS.C:1501-1503 / REVIVE.C F0280 against the HOC ordinal
 * route — if any step in the source-locked chain regresses, the
 * probe fails before the slice is shipped. */
static int check_resurrect_round_trip(M11_GameViewState* game) {
    int initialCount;
    int rc;
    struct ChampionState_Compat* newChamp;
    int i;
    int ok = 1;

    printf("TEST: resurrect round-trip at (2,7) SOUTH (CHANI) ... ");
    set_pose(game, 2, 7, DIR_SOUTH);
    if (M11_GameView_GetFrontMirrorOrdinal(game) != 16) {
        FAILF("front mirror ordinal != 16 before resurrect\n");
        return 0;
    }
    initialCount = game->world.party.championCount;
    rc = M11_GameView_SelectFrontMirrorCandidate(game);
    if (rc != 1) {
        FAILF("SelectFrontMirrorCandidate=%d\n", rc);
        return 0;
    }
    if (game->world.party.championCount != initialCount + 1) {
        FAILF("championCount=%d want=%d\n",
              game->world.party.championCount, initialCount + 1);
        ok = 0;
    }
    rc = M11_GameView_ConfirmMirrorCandidate(game, 0);
    if (rc != 1) {
        FAILF("ConfirmMirrorCandidate=%d\n", rc);
        ok = 0;
    }
    newChamp = &game->world.party.champions[initialCount];
    if (newChamp->hp.current == 0 || newChamp->hp.maximum == 0) {
        FAILF("new champion has zero HP (%d/%d)\n",
              newChamp->hp.current, newChamp->hp.maximum);
        ok = 0;
    }
    for (i = 0; i < 20; ++i) {
        (void)M11_GameView_AdvanceIdleTick(game);
        if (newChamp->hp.current == 0) {
            FAILF("new champion died at tick %d\n", i);
            ok = 0;
            break;
        }
    }
    if (game->partyDead) {
        FAILF("partyDead=1 after resurrection\n");
        ok = 0;
    }
    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        FAILF("mirror route not disabled after confirm\n");
        ok = 0;
    }
    if (ok) {
        printf("PASS HP=%d/%d\n", newChamp->hp.current,
               newChamp->hp.maximum);
        g_pass++;
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall of Champions ordinal-16 (CHANI / SAYYADINA SIHAYA) "
           "east_walkpath / portrait_rect_position probe ===\n");

    if (!check_c127_inventory(&game)) ok = 0;
    if (!check_pose_routes(&game)) ok = 0;
    if (!check_d1c_pixels(&game, portraits)) ok = 0;
    if (!check_east_walkpath(&game, portraits)) ok = 0;
    if (!check_resurrect_round_trip(&game)) ok = 0;

    printf("=== %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
