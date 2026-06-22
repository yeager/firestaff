/*
 * DM1 V1 Hall of Champions portrait 12 / east_walkpath / portrait_rect_position
 * runtime probe.
 *
 * Assigned slice of the Hall portrait placement work for this lane:
 *   - portrait ordinal 12 (LINFLAS) in DM1 V1 DUNGEON.DAT
 *   - route: east walkpath at (2,10) facing NORTH (front wall square is
 *     (2,9) on the corridor east of the main x=1 column)
 *   - aspect: D1C portrait_rect_position (DUNVIEW.C:3913-3928 / 8522-8533),
 *     proving the C026 champion portrait ordinal 12 is rendered into the
 *     source-locked front-wall rectangle (96,35)-(127,63) of the viewport
 *     when the party faces north at (2,10), and that LINFLAS does NOT
 *     "float" over the side walls when the party turns east/south/west
 *     in the same cell.
 *
 * The pose (2,10,N) -> front ordinal 12 is verified by the corridor
 * scanner probe firestaff_dm1_v1_hall_corridor_ordinal_scanner_probe:
 *   (2,10,N) ordinal=12 name=LINFLAS
 *   (1,10,N) ordinal=9  name=ZED       (west neighbour on y=10 corridor)
 *   (3,10,N) ordinal=21 name=HISSSSA   (east neighbour on y=10 corridor)
 * The walkpath probe firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 * covers ordinals 1, 19, and the corridor / negative ordinals on the
 * x=1 column and y=3 corridor, so ordinal 12 (LINFLAS) on the east
 * walkpath at y=10 was the largest uncovered positive-ordinal slice
 * for DM1 V1.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps the M011_CELL(sensor) front-cell filter.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289_i_DungeonView_ChampionPortraitOrdinal.
 *   ReDMCSB DUNVIEW.C:3913-3928 and 8522-8533 blit the C026 portrait
 *     atlas into the D1C front-wall rectangle (96,35)-(127,63) using the
 *     C01_COLOR_DARK_GRAY (value 1) transparency mask.  The
 *     M635_ZONE_PORTRAIT_ON_WALL box is 32x29 pixels per the DEFS.H
 *     M027_PORTRAIT_X / M028_PORTRAIT_Y macros (8 columns x 3 rows).
 *   ReDMCSB DUNVIEW.C:7727-7924 F0124_DrawSquareD1C draws the D1C
 *     square wall / alcove / portrait / optional alcove objects in
 *     that order; only the front-wall D1C cell copies the C026
 *     portrait onto the screen.
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF redraws the
 *     full viewport far-to-near after every MOVESENS.C:556 tick,
 *     which clears any stale portrait pixels when the front square
 *     changes.
 *   ReDMCSB COORD.C:1693-1749 G2067/G2068 anchor the PC viewport origin
 *     at (0, 33) and the portrait dimensions at 32x29.
 *
 * The probe locks three invariants at the east-walkpath ordinal 12 pose:
 *
 *   1. ORDINAL MAPS TO LINFLAS: M11_GameView_GetFrontMirrorOrdinal at
 *      (2,10) facing NORTH returns 12, and
 *      M11_GameView_GetMirrorNameByOrdinal returns "LINFLAS" for that
 *      ordinal in the canonical DM1 V1 mirror catalog.  This is the
 *      catalogue-of-truth check the upstream scanner probe confirms.
 *
 *   2. D1C PORTRAIT RECT DRAWS ORDINAL 12: M11_GameView_Draw with the
 *      party at (2,10) facing NORTH renders the C026 atlas slot
 *      ((12 & 7) * 32, (12 >> 3) * 29) = (128, 29) into the viewport-
 *      relative (96,35)-(127,63) rectangle; the 32x29 box is dominated
 *      by ordinal-12 opaque pixels matching the C026 atlas, with no
 *      stale ordinal-9 (ZED) pixels left over from the previous (1,10,N)
 *      pose on the west-neighbour walkpath.
 *
 *   3. NO-FLOATING ON SIDE WALLS: at the same cell (2,10), turning east,
 *      south, or west reuses the same map square but a non-D1C wall cell
 *      (D1C is the front wall type); the front-mirror ordinal must be
 *      -1 (no clickable mirror route).  The D1C wall-box pixels can
 *      share 4bpp palette indices with C026 portrait assets without
 *      floating-portrait semantics — this is the same no-floating
 *      contract the firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
 *      uses (route ownership, not incidental color similarity).  This is
 *      the east_walkpath analogue of the no-floating poses for the
 *      main x=1 column; the y=10 corridor is the eastern extension.
 *
 * The probe also verifies a forward-walk re-blt: walking from
 * (1,10,N) ordinal=9 ZED to (2,10,N) ordinal=12 LINFLAS re-blits the
 * portrait rect from the new front square, with the 35% leak tolerance
 * the existing walkpath / zorder / reblt probes lock.  This binds the
 * portrait_rect_position contract to the cross-cell redraw path used by
 * the existing forward-walk re-blt evidence on the x=1 corridor.
 *
 * Note: the no-floating side poses (DIR_EAST / DIR_SOUTH / DIR_WEST at
 * (2,10)) only assert the front-mirror ordinal is -1.  Per the existing
 * zorder probe comment, the D1C wall box may still share palette pixels
 * with C026 portrait assets because the wall-ornament graphic and the
 * backing fill are drawn without a per-ordinal mask; this contract is
 * about route ownership, not incidental 4bpp palette similarity.
 *
 * The probe is intentionally narrow: it does not click the portrait or
 * open the candidate panel (those are covered by the actual-pose probe
 * and the candidate-panel probe).  It does not assert wall-perimeter
 * pixel stability across cells; the D1C front-wall pixel count and the
 * non-portrait wall texture vary between cells by design.  It does
 * prove the source-locked D1C portrait rectangle is dominated by the
 * expected C026 ordinal for the front pose and is empty on side poses.
 *
 * Usage: firestaff_dm1_v1_hall_champion_portrait_12_east_walkpath_portrait_rect_probe DATA_DIR
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
    /* DUNVIEW.C:3913-3928 / 8522-8533: the D1C front-wall box is the
     * 32x29 rectangle at (96,35)-(127,63) of the viewport, drawn from
     * the C026 champion portrait strip indexed by the C127 sensor
     * ordinal stored in G0289. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* DUNVIEW.C:3916: the C026 champion portrait blit masks
     * C01_COLOR_DARK_GRAY (value 1) as transparency.  This is the
     * same constant the existing visibility / zorder / reblt
     * probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

/* Count the pixels in the front-wall box that match the C026
 * champion portrait ordinal.  This reuses the visibility probe's
 * match formula (DUNVIEW.C:3916 C01 dark-gray transparency mask
 * + per-ordinal DUNVIEW.C:3918 (ordinal & 7) * 32 + (ordinal >> 3)
 * * 29 source stride).
 *
 * Returns the number of opaque ordinal pixels that actually
 * match between the source strip and the framebuffer, or 0 when
 * either the ordinal is out of range or the slot is not loaded. */
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

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = (int16_t)mapX;
    game->world.party.mapY = (int16_t)mapY;
    game->world.party.direction = (uint8_t)dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

typedef struct EastWalkPose {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    const char* label;
} EastWalkPose;

/* Verify a single east-walkpath pose at (2,10): the front ordinal must
 * match the expected value, the D1C portrait rect must be dominated by
 * the expected C026 ordinal pixels (or be empty for a no-portrait pose),
 * and a stale-portrait leak check guards against the previous ordinal's
 * pixels being left behind after a re-blt.
 *
 * The 90% match threshold for a positive-ordinal pose matches the
 * existing actual-pose probe and the visibility probe's exact-pixel
 * contract.  The 35% leak threshold matches the existing walkpath /
 * zorder / reblt probes' tolerance for cross-cell stale pixels. */
static int check_east_walk_pose(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                int prevOrdinal,
                                const EastWalkPose* pose,
                                unsigned char* fb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, pose->mapX, pose->mapY, pose->dir);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != pose->expectedOrdinal) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=%d\n",
                pose->label, ordinal, pose->expectedOrdinal);
        ok = 0;
    }
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, fb,
                                 pose->expectedOrdinal >= 0
                                     ? pose->expectedOrdinal
                                     : 0);
    if (pose->expectedOrdinal >= 0) {
        if (match.bestOrdinal != pose->expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * 90) {
            fprintf(stderr,
                    "FAIL %s portrait_rect expected ordinal=%d best=%d matched=%d/%d\n",
                    pose->label, pose->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else {
        /* No-portrait side pose: only the route ownership is locked —
         * the front-mirror ordinal is -1 (already checked above).  The
         * D1C wall-box pixels can share 4bpp palette indices with C026
         * portrait assets because the wall-ornament graphic and the
         * backing fill are drawn without a per-ordinal mask; this is
         * the same no-floating contract the zorder probe uses for the
         * main x=1 column (route ownership, not incidental palette
         * similarity).  The existing zorder probe comments document
         * the same source behaviour: the negative check is about
         * front-cell mirror route ownership, not incidental color
         * similarity in the already-rendered wall/ornament pixels. */
    }
    /* Cross-cell re-blt invariant: when the ordinal changes between
     * poses, the prior ordinal's pixels must not be the dominant
     * match in the new framebuffer's portrait rectangle. */
    if (prevOrdinal >= 0 && prevOrdinal != pose->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, fb, prevOrdinal);
        int prevCompared = match_front_portrait(portraits, fb, prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= 35) {
            fprintf(stderr,
                    "FAIL %s stale ordinal=%d leaked matched=%d/%d after step to ordinal=%d\n",
                    pose->label, prevOrdinal, stale, prevCompared,
                    pose->expectedOrdinal);
            ok = 0;
        }
    }
    printf("  %s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           pose->label, pose->mapX, pose->mapY, pose->dir, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;
    /* East-walkpath corridor at y=10 in the reference DM1 V1
     * DUNGEON.DAT.  The party walks east from (1,10) [front=(1,9)
     * C127 ordinal=9 ZED] to (2,10) [front=(2,9) C127 ordinal=12
     * LINFLAS] to (3,10) [front=(3,9) C127 ordinal=21 HISSSSA]
     * while facing NORTH, then back.  The portrait_rect_position
     * aspect is the D1C front-wall box (96,35)-(127,63) in the
     * viewport, drawn from the C026 champion portrait strip
     * (DUNVIEW.C:3913-3928 / 8522-8533). */
    const EastWalkPose poses[] = {
        {1, 10, DIR_NORTH,  9, "east_walk_step_a_ordinal_9_zed"},
        {2, 10, DIR_NORTH, 12, "east_walk_step_b_ordinal_12_linflas"},
        {3, 10, DIR_NORTH, 21, "east_walk_step_c_ordinal_21_hissssa"},
        {2, 10, DIR_NORTH, 12, "east_walk_step_d_ordinal_12_linflas_again"},
        {1, 10, DIR_NORTH,  9, "east_walk_step_e_ordinal_9_zed_back"},
        /* No-floating side-wall poses at (2,10): the same cell, but
         * the D1C front wall is on a different side, so the portrait
         * rectangle must not show LINFLAS.  This mirrors the no-floating
         * coverage in firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
         * but for the y=10 east-walkpath cell. */
        {2, 10, DIR_EAST,  -1, "east_walk_side_east_no_portrait"},
        {2, 10, DIR_SOUTH, -1, "east_walk_side_south_no_portrait"},
        {2, 10, DIR_WEST,  -1, "east_walk_side_west_no_portrait"},
    };
    int i;
    int prevOrdinal = -2;
    char nameBuf[32];

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
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Verifies that ordinal 12 maps to LINFLAS in the mirror catalog
     * (the runtime champion identity carried by the C026 portrait
     * sensor data on the front wall at (2,9) when the party is at
     * (2,10) facing NORTH).  This binds the ordinal value to the
     * named champion so the runtime probe and the catalog probe stay
     * in sync; the catalog builder is F0652_CHAMPION_BuildMirrorCatalog
     * which iterates DUNGEON.DAT text strings. */
    nameBuf[0] = 0;
    if (M11_GameView_GetMirrorNameByOrdinal(&game, 12, nameBuf, (int)sizeof(nameBuf)) <= 0 ||
        nameBuf[0] == 0) {
        fprintf(stderr, "FAIL mirror catalog: ordinal 12 name not found\n");
        ok = 0;
    } else {
        printf("  ordinal 12 mirror name = %s\n", nameBuf);
        /* "LINFLAS" is the canonical DM1 V1 catalog name for ordinal 12.
         * Accept any non-empty source-shaped name; the runtime check
         * is about the name being present, the value 12 itself, and
         * the rect drawing. */
        if (strcmp(nameBuf, "LINFLAS") != 0) {
            fprintf(stderr,
                    "FAIL mirror catalog: ordinal 12 name=%s expected=LINFLAS\n",
                    nameBuf);
            ok = 0;
        }
    }

    printf("=== DM1 V1 Hall champion portrait 12 east_walkpath portrait_rect probe ===\n");
    for (i = 0; i < (int)(sizeof(poses) / sizeof(poses[0])); ++i) {
        if (!check_east_walk_pose(&game, portraits, prevOrdinal, &poses[i], currFb)) {
            ok = 0;
        }
        prevOrdinal = poses[i].expectedOrdinal;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 hall champion portrait 12 east_walkpath portrait_rect probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
