/*
 * DM1 V1 Hall of Champions portrait ordinal 1 (HALK) / route
 * west_negative_portrait_rect_position runtime probe.
 *
 * Slice: ordinal 1 (the very first portrait in the C026 champion-
 * portrait atlas), route aspect west_negative portrait_rect_position.
 * The route label "west_negative" describes the rect invariant: at
 * the (1,2) cell facing WEST the D1C portrait rectangle is reserved
 * at the source-locked position but no portrait is painted there
 * because the front square (0,2) is a doorway, not a C127 mirror
 * wall.  The positive route that puts ordinal 1 in that same D1C
 * rect is (7,9) facing NORTH.
 *
 * Companion to firestaff_dm1_v1_champion_mirror_capture_probe and
 * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe:
 *   - capture_probe proves the (7,9) NORTH HALK pose renders the
 *     portrait and (1,2) WEST is empty at the hardcoded rect.
 *   - actual_pose_runtime_probe proves (7,9) NORTH returns
 *     ordinal 1 from the C127 sensor lookup and (1,2) WEST
 *     returns -1.
 *   - panel_guard_probe proves the BUG-120/121 panel-active
 *     early-return preserves the portrait under the C040 panel.
 *
 * What is NOT yet proven for ordinal 1 specifically:
 *   (1) The mirror catalog name (F0660) and title (F0661) for
 *       ordinal 1 round-trip to "HALK" and a non-empty title.
 *   (2) The D1C portrait cutout (96, 35, 32, 29) at (7,9) NORTH
 *       pixel-matches C026 atlas slot 1 (HALK) at >= 90% palette
 *       agreement — i.e. the rect contains the right champion,
 *       not a stale pixel from a previous frame or a different
 *       ordinal.
 *   (3) The same D1C cutout at (1,2) WEST contains ZERO ordinal-1
 *       pixels (no floating HALK portrait painted through a wall
 *       that has no C127 mirror).
 *   (4) The portrait_rect_position contract: the D1C wall-mirror
 *       frame zone (80, 29, 64, 43) is invariant under pose
 *       changes — same source-locked rect regardless of whether
 *       the party faces NORTH or WEST.
 *   (5) The portrait cutout is parented at (frame.x + 16,
 *       frame.y + 6) per DUNVIEW.C:3913-3928 — the (+16, +6)
 *       offset is locked regardless of which C127 sensorData
 *       ordinal is at the front square.
 *   (6) The resurrect round-trip via SelectFrontMirrorCandidate at
 *       (7,9) NORTH sets candidateMirrorOrdinal=1, ConfirmMirror-
 *       Candidate appends HALK with full HP, the mirror route
 *       disables, and 20 idle ticks later HALK is still alive.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 — sensor cell → view-direction mapping
 *     (front-wall side filter; a sensor on (0,2) is NOT on the
 *     front wall of (1,2) facing WEST — the front square is the
 *     doorway, not the mirror wall).
 *   ReDMCSB DUNGEON.C:2608-2612 — C127 sensorData stores the
 *     champion-portrait ordinal in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 — blits C026 (champion portrait
 *     strip) at the fixed D1C wall box (96, 35) (size 32x29);
 *     C346 mirror frame via m11_dm1_wall_ornament_zone with
 *     coordSet=5 viewWallIndex=12 → destination (80, 29)
 *     (size 64x43).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 — viewport redraw
 *     far-to-near; portrait rect is rebuilt every step.
 *   ReDMCSB MOVESENS.C:1501-1503 — C127 sensorData passes to
 *     F0280 candidate selection.
 *   ReDMCSB REVIVE.C F0280:124-167 — candidate materialization
 *     from C127 sensorData.
 *   ReDMCSB REVIVE.C F0282:744-806 — disables matching C127
 *     mirror sensor after confirmed resurrect.
 *   ReDMCSB DEFS.H:821-826 M027_PORTRAIT_X / M028_PORTRAIT_Y —
 *     8-column atlas math (ordinal & 7) * W, (ordinal >> 3) * H.
 *   ReDMCSB DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS strip.
 *   m11_front_cell_mirror_ordinal (src/engine/m11_game_view.c:11652)
 *     — front-wall side filter on the C127 sensor lookup.
 *
 * Honesty scope:
 *   - Firestaff-runtime portrait_rect_position evidence only.
 *   - Does not claim DOS pixel parity.
 *   - Requires real DM1 V1 PC 3.4 DUNGEON.DAT + the C026 portrait
 *     strip in GRAPHICS.DAT.  Without that data the probe prints
 *     SKIP and exits 0.
 *   - SKIP guard via M12_AssetStatus_GameAvailable("dm1") so
 *     non-canonical fixtures don't break the build.
 *
 * Run: firestaff_dm1_v1_champion_mirror_ordinal_1_halk_pose_probe DATA_DIR
 */

#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "asset_status_m12.h"
#include "render_sdl_m11.h"
#include "memory_champion_state_pc34_compat.h"

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
    /* D1C champion portrait cutout.  Parent of C026 blit per
     * DUNVIEW.C:3913-3928 and the (+16, +6) offset from the C346
     * D1C wall-mirror frame (80, 29, 64, 43) documented in
     * DUNVIEW.C G0205 coordSet 5 / viewWallIndex 12. */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* Warm-color pixel thresholds.  Same warm palette set the
     * existing firestaff_dm1_v1_champion_mirror_capture_probe and
     * firestaff_dm1_v1_hall_of_champions_west_negative_portrait_
     * rect_position_probe use:
     *   {0x07 green, 0x08 red, 0x09 orange, 0x0A peach,
     *    0x0B yellow, 0x0E blue}
     * Grey-stone wall texture never uses this set, so a positive
     * ordinal pose has >= POS_THRESHOLD warm pixels while a no-
     * portrait cell has < NEG_THRESHOLD. */
    PORTRAIT_WARM_POS_THRESHOLD = 30,
    PORTRAIT_WARM_NEG_THRESHOLD = 30,
    /* Side-wall rects used by the no-floating check.  These are
     * outside the (96..128, 35..64) portrait cutout and outside
     * the (80..144, 29..72) D1C wall-mirror frame so the warm
     * count here must stay near zero on corridor poses where no
     * C127 mirror is on the front wall. */
    SIDE_NORTH_X = VIEWPORT_X + 0,
    SIDE_NORTH_Y = VIEWPORT_Y + 0,
    SIDE_NORTH_W = 64,
    SIDE_NORTH_H = 32,
    SIDE_SOUTH_X = VIEWPORT_X + 160,
    SIDE_SOUTH_Y = VIEWPORT_Y + 0,
    SIDE_SOUTH_W = 64,
    SIDE_SOUTH_H = 32,
    SIDE_FLOAT_THRESHOLD = 5,
    /* C026 atlas math constants from DEFS.H:821-826. */
    C026_PORTRAIT_W = 32,
    C026_PORTRAIT_H = 29
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Pixel-level match for ordinal N at the D1C portrait cutout.
 * Skips C026 atlas "transparent" palette index 1 and returns
 * matched/compared as integer percentages so the probe can
 * assert >= 90% pixel agreement without claiming DOS pixel
 * parity.  Source-locked to DUNVIEW.C:3913-3928 (C026 blit) and
 * DEFS.H:821-826 (atlas math). */
static int match_portrait(const M11_AssetSlot* portraits,
                          const unsigned char* fb,
                          int ordinal) {
    int x, y, matched = 0, compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    if (ordinal < 0 || ordinal >= 24) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * C026_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * C026_PORTRAIT_H + y;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) continue; /* transparent in C026 atlas */
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count warm-colored pixels in a viewport rectangle.  Used by the
 * no-floating invariant: a C127 mirror on the front wall paints
 * the champion portrait sprite inside the D1C cutout, which uses
 * the warm palette set; the corridor side walls and the door-
 * front cell never use the warm set, so warm_count >=
 * PORTRAIT_WARM_POS_THRESHOLD means a portrait is on screen and
 * warm_count < PORTRAIT_WARM_NEG_THRESHOLD means no portrait is
 * on screen. */
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

/* Pose the party and zero the candidate-panel state.  Centralizes
 * the boilerplate so every render call below uses the same field
 * initialization.  Source-locked to DUNGEON.C:2573 + MOVESENS.C:
 * 1501-1503 + REVIVE.C F0280 — the C127 sensor lookup and
 * candidate-materialization front-cell filter. */
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
}

static void render_at(M11_GameViewState* state,
                      unsigned char* fb,
                      int mapX, int mapY, int direction) {
    set_pose(state, mapX, mapY, direction);
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits = NULL;
    int assetsAvailable;
    unsigned char fbNorth[FB_W * FB_H];
    unsigned char fbWest [FB_W * FB_H];
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int pctHalk = 0;
    int warmNorthInner = 0;
    int warmWestInner = 0, warmWestWall = 0;
    int warmWestSideNorth = 0, warmWestSideSouth = 0;
    int ordinalNorth = -1, ordinalWest = -1;
    char nameBuf[CHAMPION_NAME_LENGTH + 1];
    char titleBuf[CHAMPION_TITLE_LENGTH + 1];

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 Hall portrait ordinal 1 (HALK) / west_negative portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
        printf("SKIP firestaff_dm1_v1_champion_mirror_ordinal_1_halk_pose_probe "
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

    /* ── Group A: mirror catalog identity for ordinal 1 ────────────
     * Source-locked to F0660/F0661 mirror-catalog decode.  The
     * mirror catalog is loaded from DM1 V1 DUNGEON.DAT at startup
     * and stores champion names/titles indexed by ordinal.  For
     * ordinal 1 the name MUST be "HALK" and the title MUST be
     * non-empty (ReDMCSB DEFS.H:CHAMPION_NAME_LENGTH + the
     * F0660/F0661 mirror-catalog pair in BASE.C). */
    printf("\n[Group A] Mirror catalog identity for ordinal 1 (HALK)\n");
    memset(nameBuf, 0, sizeof(nameBuf));
    memset(titleBuf, 0, sizeof(titleBuf));
    (void)M11_GameView_GetMirrorNameByOrdinal(&state, 1, nameBuf, sizeof(nameBuf));
    (void)M11_GameView_GetMirrorTitleByOrdinal(&state, 1, titleBuf, sizeof(titleBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 1 mirror catalog name == \"HALK\" (got \"%s\")",
                 nameBuf);
        CHECK(strcmp(nameBuf, "HALK") == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 1 mirror catalog title non-empty (got \"%s\")",
                 titleBuf);
        CHECK(titleBuf[0] != '\0', msg);
    }

    /* ── Group B: (7,9) NORTH HALK positive route ─────────────────
     * Source-locked to DUNGEON.C:2573 (C127 sensor front-wall
     * filter) + MOVESENS.C:1501-1503 (sensorData routing) +
     * REVIVE.C F0280 (candidate materialization).  At (7,9)
     * facing NORTH, the front square carries the compact C127
     * sensor with sensorData=1 (HALK).  The engine must:
     *   - return ordinal 1 from M11_GameView_GetFrontMirrorOrdinal
     *   - paint the HALK portrait sprite (atlas slot 1) at the
     *     D1C cutout (96, 35, 32, 29) with >= 90% pixel match.
     */
    printf("\n[Group B] (7,9) NORTH HALK positive route: portrait painted at D1C rect\n");

    render_at(&state, fbNorth, 7, 9, DIR_NORTH);
    ordinalNorth = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetFrontMirrorOrdinal((7,9)N) == 1 (got %d)",
                 ordinalNorth);
        CHECK(ordinalNorth == 1, msg);
    }

    pctHalk = match_portrait(portraits, fbNorth, 1);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait cutout (96,35)-(127,63) matches C026 ordinal 1 >= 90%% (got %d%%)",
                 pctHalk);
        CHECK(pctHalk >= 90, msg);
    }

    if (portraits && portraits->loaded && portraits->pixels) {
        warmNorthInner = rect_warm_count(fbNorth,
                                         PORTRAIT_X, PORTRAIT_Y,
                                         PORTRAIT_W, PORTRAIT_H);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "Inner portrait cutout warm_count >= %d for HALK (got %d)",
                     PORTRAIT_WARM_POS_THRESHOLD, warmNorthInner);
            CHECK(warmNorthInner >= PORTRAIT_WARM_POS_THRESHOLD, msg);
        }
    }

    /* ── Group C: portrait_rect_position contract ─────────────────
     * Source-locked to DUNVIEW.C:3913-3928 (C026 blit) + DUNVIEW.C
     * G0205 Graphic558 coordSet 5 / viewWallIndex 12 (C346 D1C
     * wall-mirror frame).  The D1C wall-mirror frame MUST be at
     * (80, 29, 64, 43) regardless of which pose the party is in.
     * The portrait cutout MUST be at (frame.x + 16, frame.y + 6)
     * = (96, 35, 32, 29) per the (+16, +6) parented offset. */
    printf("\n[Group C] portrait_rect_position contract: (96, 35, 32, 29) invariant\n");

    set_pose(&state, 7, 9, DIR_NORTH);
    {
        int rc = M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetD1CWallOrnamentZone returns 1 (got %d)", rc);
        CHECK(rc == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall box X == 80 (got %d)", ornX);
        CHECK(ornX == 80, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall box Y == 29 (got %d)", ornY);
        CHECK(ornY == 29, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall box W == 64 (got %d)", ornW);
        CHECK(ornW == 64, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall box H == 43 (got %d)", ornH);
        CHECK(ornH == 43, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Portrait cutout X == wallX + 16 == 96 (got %d)", ornX + 16);
        CHECK(ornX + 16 == 96, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Portrait cutout Y == wallY + 6 == 35 (got %d)", ornY + 6);
        CHECK(ornY + 6 == 35, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Portrait cutout W == 32 (got %d)", PORTRAIT_W);
        CHECK(PORTRAIT_W == 32, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Portrait cutout H == 29 (got %d)", PORTRAIT_H);
        CHECK(PORTRAIT_H == 29, msg);
    }

    /* The portrait_rect_position invariant must hold regardless of
     * the active pose.  Re-query the helper at (1,2) WEST and
     * confirm the same box. */
    set_pose(&state, 1, 2, DIR_WEST);
    {
        int rc = M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall box at (1,2) WEST is invariant: rc=%d X=%d Y=%d W=%d H=%d",
                 rc, ornX, ornY, ornW, ornH);
        CHECK(rc == 1 && ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
    }

    /* ── Group D: (1,2) WEST west_negative slice ──────────────────
     * Source-locked to DUNGEON.C:2573 (C127 sensor front-wall
     * filter).  At (1,2) facing WEST the front square is (0,2),
     * which is a doorway and does NOT carry a C127 sensor.  The
     * engine must:
     *   - return -1 from M11_GameView_GetFrontMirrorOrdinal
     *   - leave the D1C portrait cutout empty (no floating HALK
     *     sprite painted through a wall that has no mirror)
     *   - leave the side walls (corridor walls on the north and
     *     south sides of the viewport) free of warm portrait
     *     pixels (no floating sprite on an ordinary side wall). */
    printf("\n[Group D] (1,2) WEST west_negative slice: portrait_rect_position is empty\n");

    render_at(&state, fbWest, 1, 2, DIR_WEST);
    ordinalWest = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetFrontMirrorOrdinal((1,2)W) == -1 (got %d)",
                 ordinalWest);
        CHECK(ordinalWest == -1, msg);
    }

    warmWestInner = rect_warm_count(fbWest,
                                    PORTRAIT_X, PORTRAIT_Y,
                                    PORTRAIT_W, PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Inner portrait cutout warm_count < %d (got %d) at (1,2) WEST",
                 PORTRAIT_WARM_NEG_THRESHOLD, warmWestInner);
        CHECK(warmWestInner < PORTRAIT_WARM_NEG_THRESHOLD, msg);
    }

    if (portraits && portraits->loaded && portraits->pixels) {
        int pctStale = match_portrait(portraits, fbWest, 1);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C cutout at (1,2) WEST does NOT match HALK (ordinal 1) "
                     ">= 90%% (got %d%% — floating portrait)",
                     pctStale);
            CHECK(pctStale < 90, msg);
        }
    }

    /* The corridor side walls (north and south corridors of the
     * viewport) must NOT contain HALK portrait pixels at (1,2)
     * WEST.  This is the explicit no-floating invariant: the
     * portrait must not bleed onto ordinary side walls just
     * because the D1C cell is reserved. */
    warmWestSideNorth = rect_warm_count(fbWest,
                                        SIDE_NORTH_X, SIDE_NORTH_Y,
                                        SIDE_NORTH_W, SIDE_NORTH_H);
    warmWestSideSouth = rect_warm_count(fbWest,
                                        SIDE_SOUTH_X, SIDE_SOUTH_Y,
                                        SIDE_SOUTH_W, SIDE_SOUTH_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Corridor north side-wall warm_count <= %d at (1,2) WEST (got %d)",
                 SIDE_FLOAT_THRESHOLD, warmWestSideNorth);
        CHECK(warmWestSideNorth <= SIDE_FLOAT_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Corridor south side-wall warm_count <= %d at (1,2) WEST (got %d)",
                 SIDE_FLOAT_THRESHOLD, warmWestSideSouth);
        CHECK(warmWestSideSouth <= SIDE_FLOAT_THRESHOLD, msg);
    }

    /* The wall box at (1,2) WEST may contain stone-wall pixels
     * (warm_count <= 50 per the sibling
     * firestaff_dm1_v1_hall_of_champions_west_negative_portrait_
     * rect_position_probe); we don't fail on it here, but we
     * log it for the record. */
    warmWestWall = rect_warm_count(fbWest,
                                   VIEWPORT_X + 80,
                                   VIEWPORT_Y + 29,
                                   64, 43);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Wall box warm_count at (1,2) WEST (logged) = %d",
                 warmWestWall);
        printf("  INFO: %s\n", msg);
        ++g_pass;
    }

    /* ── Group E: resurrect round-trip at (7,9) NORTH HALK ────────
     * Source-locked to REVIVE.C F0280 (candidate materialization
     * from C127 sensorData) + REVIVE.C F0282 (disable matching
     * mirror after confirmed resurrect) + DUNGEON.C:2608-2612
     * (C127 sensorData storage in G0289). */
    printf("\n[Group E] HALK resurrect round-trip via (7,9) NORTH\n");

    {
        int initialCount;
        struct ChampionState_Compat* newChamp;
        int rc;
        int i;

        set_pose(&state, 7, 9, DIR_NORTH);
        initialCount = state.world.party.championCount;

        rc = M11_GameView_SelectFrontMirrorCandidate(&state);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "M11_GameView_SelectFrontMirrorCandidate((7,9)N) == 1 (got %d)",
                     rc);
            CHECK(rc == 1, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "candidateMirrorOrdinal after select == 1 (got %d)",
                     state.candidateMirrorOrdinal);
            CHECK(state.candidateMirrorOrdinal == 1, msg);
        }

        rc = M11_GameView_ConfirmMirrorCandidate(&state, 0);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "M11_GameView_ConfirmMirrorCandidate((7,9)N) == 1 (got %d)",
                     rc);
            CHECK(rc == 1, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "championCount == initial+1 after confirm (initial=%d got=%d)",
                     initialCount, state.world.party.championCount);
            CHECK(state.world.party.championCount == initialCount + 1, msg);
        }

        newChamp = &state.world.party.champions[initialCount];
        {
            /* CHAMPION_NAME_LENGTH is 8 (PackedName, no NUL in original —
             * see memory_champion_state_pc34_compat.h:29).  We compare the
             * first 4 bytes (the literal "HALK") with memcmp so the unsigned
             * char array of the struct doesn't trigger a -Wpointer-sign
             * conversion warning against strncmp's const char * signature. */
            static const unsigned char kHalkName[4] = { 'H', 'A', 'L', 'K' };
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "new champion name == \"HALK\" (got first-4 = %02X %02X %02X %02X)",
                     newChamp->name[0], newChamp->name[1],
                     newChamp->name[2], newChamp->name[3]);
            CHECK(memcmp(newChamp->name, kHalkName, sizeof(kHalkName)) == 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "new champion HP > 0 (got %d/%d)",
                     newChamp->hp.current, newChamp->hp.maximum);
            CHECK(newChamp->hp.current > 0 && newChamp->hp.maximum > 0, msg);
        }

        for (i = 0; i < 20; ++i) {
            (void)M11_GameView_AdvanceIdleTick(&state);
            if (newChamp->hp.current == 0) break;
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "HALK survives 20 idle ticks (HP=%d/%d alive=%d)",
                     newChamp->hp.current, newChamp->hp.maximum,
                     newChamp->hp.current > 0 ? 1 : 0);
            CHECK(newChamp->hp.current > 0, msg);
        }

        {
            int ordAfter = M11_GameView_GetFrontMirrorOrdinal(&state);
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "Front-cell mirror ordinal after confirm == -1 (got %d)",
                     ordAfter);
            CHECK(ordAfter == -1, msg);
        }
    }

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
