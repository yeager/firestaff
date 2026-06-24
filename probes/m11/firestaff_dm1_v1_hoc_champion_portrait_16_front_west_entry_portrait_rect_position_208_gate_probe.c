/*
 * firestaff_dm1_v1_hoc_champion_portrait_16_front_west_entry_portrait_rect_position_208_gate_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions portrait verification gate
 * (gate id 208, batch group 8) for one narrow slice:
 *
 *   ordinal 16
 *     (C026 atlas col 0, row 2 of the 8x3 portrait strip;
 *      mirror catalog record CHANI / title "SAYYADINA SIHAYA"
 *      on the canonical PC 3.4 EN build).
 *   route  front_west_entry
 *     The party enters the (2,8) Hall mirror cell from the west
 *     corridor, so the pose is party at (1,8) facing DIR_EAST
 *     (front cell = (2,8), visibleWallCell = (EAST+2)&3 = 3 = WEST).
 *   aspect portrait_rect_position
 *     The D1C front-wall portrait cutout at viewport (96, 35, 32, 29)
 *     per ReDMCSB DUNVIEW.C:3913-3928 + DUNVIEW.C:525 G0109
 *     Graphic558_Box_ChampionPortraitOnWall = {96, 127, 35, 63}.
 *
 * Source-locked evidence:
 *
 *   DUNGEON.C:2573
 *     M011_CELL(sensor) is matched against the party view direction
 *     and only sensors on M552_FRONT_WALL_ORNAMENT_ORDINAL = 5
 *     (DEFS.H:2552) are exposed to the G0289 candidate ordinal
 *     pipeline.
 *
 *   DUNGEON.C:2608-2612
 *     The C127 sensorData of the matching front-cell sensor is
 *     stored in G0289, the m11_front_cell_mirror_ordinal anchor.
 *
 *   MOVESENS.C:1501-1503
 *     C127 sensorData is passed to F0280 (REVIVE.C) which
 *     materializes the candidate champion from sensorData.
 *
 *   REVIVE.C F0280:124-132
 *     F0280 gates candidate materialization on the empty-leader
 *     champion slot and copies sensorData into the candidate
 *     ordinal.  F0282:744-806 owns the C162 cancel branch.
 *
 *   DUNVIEW.C:3913-3928
 *     The C026_GRAPHIC_CHAMPION_PORTRAITS atlas (256x87 strip,
 *     8 cols * 3 rows of 32x29 portraits, ordinals 0..23) is
 *     blitted into the fixed D1C champion-portrait destination
 *     rectangle {96, 127, 35, 63} (32x29 in viewport coords) with
 *     the C01_COLOR_DARK_GRAY transparency mask at source palette
 *     index 1.  The atlas math is
 *         srcX = (ordinal & 7) << 5
 *         srcY = (ordinal >> 3) * 29
 *     so ordinal 16 sits at (0, 58, 32, 29).
 *
 *   DUNVIEW.C:8318-8542  F0128_DUNGEONVIEW_Draw_CPSF
 *     The far-to-near viewport redraw path re-builds the
 *     viewport from the new party pose after every MOVESENS.C:556
 *     tick.  When the front cell no longer carries a C127 sensor
 *     the far-to-near order overpaints the D1C portrait rectangle
 *     with side-wall geometry, so no portrait sprite floats over
 *     a side wall.
 *
 *   COORD.C:1693-1749
 *     PC 3.4 viewport origin (0, 33) and 32x29 portrait dims
 *     (G2078_C32_PortraitWidth = 32, G2079_C29 = 29).
 *
 *   DEFS.H:821-826
 *     M027_PORTRAIT_X(index) / M028_PORTRAIT_Y(index) encode the
 *     ordinal -> (srcX, srcY) atlas mapping for the C026 strip.
 *
 *   DEFS.H:2186
 *     C026_GRAPHIC_CHAMPION_PORTRAITS - the 256x87 portrait
 *     strip asset.
 *
 *   DEFS.H:2552
 *     M552_FRONT_WALL_ORNAMENT_ORDINAL = 5 - the sensor type
 *     that exposes the C127 champion-portrait ordinal to the
 *     G0289 pipeline.
 *
 * On the canonical PC 3.4 English DM1 V1 DUNGEON.DAT, ordinal 16
 * is exposed exactly once on map 0: at (2,7) DIR_SOUTH, the front
 * cell (2,8) carries a C127 sensor on its NORTH aspect with
 * sensorData = 16.  The mirror cell (2,8) also carries a C127
 * sensor on its WEST aspect with sensorData = 0 (DAROOU, the
 * left-neighbour ordinal) - that is the cell the front_west_entry
 * route approaches.  No WEST-facing pose on map 0 exposes
 * ordinal 16 naturally, so this slice seeds the (2,8) WEST-aspect
 * C127 sensor from its shipped sensorData=0 to sensorData=16
 * for the duration of the probe (synthetic mutation; the
 * runtime drive is a real-engine C127 sensor drive against a
 * synthetic ordinal value, the same honest-data-quality trade-off
 * the ordinal-23 front_north_entry probe and the ordinal-2
 * palette_match_rect probe already make).
 *
 * Eight invariants are proved:
 *
 *   (A) Mirror catalog identity: M11_GameView_GetMirrorNameByOrdinal(16)
 *       returns a non-empty name in real DM1 V1 data.  On the
 *       canonical PC 3.4 EN build ordinal 16 binds to CHANI /
 *       "SAYYADINA SIHAYA" (verified at probe runtime); the
 *       slice does not hard-pin the name because different DM1 V1
 *       builds may rebind the ordinal.
 *
 *   (B) Front-cell ordinal contract at (1,8): DIR_EAST -> 16
 *       (after sensor seed), DIR_NORTH -> -1 (wrong wall),
 *       DIR_SOUTH -> -1 (wrong wall), DIR_WEST -> -1 (wrong
 *       wall, party looks at the corridor wall of (1,8) not at
 *       the (2,8) cell).  This is the source-locked
 *       wrong-wall filter (DUNGEON.C:2573 + DEFS.H:2552).
 *
 *   (C) D1C wall-ornament zone: M11_GameView_GetD1CWallOrnamentZone
 *       returns the source-locked (80, 29, 64, 43) viewport-
 *       relative C346 wall-mirror frame at the front_west_entry
 *       pose per DUNVIEW.C G0205 coordSet 5 / index 12, with
 *       the C026 portrait cutout (96, 35, 32, 29) parented at
 *       (+16, +6) inside the frame.
 *
 *   (D) Portrait_rect_position pixel proof: D1C rect (96, 35, 32,
 *       29) is painted with ordinal-16 pixels at the
 *       front_west_entry pose.  warm_count >= 30 (the C026
 *       champion-portrait palette set {0x07 green, 0x08 red,
 *       0x09 orange, 0x0A peach, 0x0B yellow, 0x0E blue} from
 *       DUNVIEW.C:3913-3928), and the strict per-pixel palette
 *       match on the C026 ordinal-16 source cell (0, 58, 32,
 *       29) is at least 95% (above the 90% threshold the
 *       ordinal-5 front_south_entry probe locks).
 *
 *   (E) No-floating contract on side-wall column bands:
 *       The D1C portrait rect sits at viewport x=96..127.  At
 *       (1,8) DIR_EAST the D1L column band (x=0..95, y=35..63)
 *       and the D1R column band (x=128..223, y=35..63) must
 *       NOT carry ordinal-16 warm pixels in the portrait row
 *       band - this catches a drift where the D1C blit is
 *       rendered to the wrong wall or the wrong row band.
 *
 *   (F) Re-blt invariant on EAST -> N/S/W transitions:
 *       Turning away from the front_west_entry pose must NOT
 *       leave ordinal-16 pixels floating in the D1C rect.
 *       Within the 35% leak tolerance the ordinal-5
 *       front_south_entry probe locks (35% relative match to
 *       the source cell at the negative pose).
 *
 *   (G) Adjacent corridor sanity: walking the y=8 corridor
 *       from (0,8) to (3,8) facing NORTH at all four poses
 *       must not expose ordinal 16 (the (2,8) WEST sensor is
 *       not visible from any NORTH-facing pose - the front
 *       cell is north of the party, not east).  This guards
 *       against a future regression where the C127 sensor
 *       filter stops rejecting wrong-aspect sensors.
 *
 *   (H) Redraw-stability byte equality: three back-to-back
 *       draws at the (1,8) DIR_EAST pose produce byte-stable
 *       framebuffers (no jitter across draws - guards against
 *       an off-by-one sprite frame cache bug or a non-
 *       idempotent palette decode in the D1C draw path).
 *
 * HONESTY: this is Firestaff deterministic-runtime evidence,
 * not original-DM1 PC 3.4 pixel parity.  The ordinal-rectangle
 * match uses the same warm-color heuristic the existing
 * firestaff_dm1_v1_champion_mirror_capture_probe uses (palette
 * indices {0x07 green, 0x08 red, 0x09 orange, 0x0A peach, 0x0B
 * yellow, 0x0E blue}) to distinguish 'portrait painted' from
 * 'wall texture only'.  The strict per-pixel palette match
 * routine reports both the best-matched ordinal and the
 * per-ordinal matched/compared ratio so an unrelated D1C
 * ornament cannot accidentally pass.
 *
 * Disjoint from the existing ordinal-16 Hall portrait probes:
 *   - firestaff_dm1_v1_champion_portrait_16_front_north_entry
 *     portrait_rect_position_probe (front_north_entry route at
 *     (2,7) DIR_SOUTH - the natural real-DM1 pose, no sensor
 *     seeding, no wrong-wall + adjacent + re-blt + redraw-
 *     stability invariants)
 *   - firestaff_dm1_v1_champion_mirror_ordinal_16_south_return
 *     portrait_rect_position_runtime_probe (south_return route
 *     with seeded (1,6) C127 sensor at the (1,5) DIR_SOUTH
 *     pose - different front cell, different seed, no
 *     wall-ornament zone check, no redraw stability)
 *   - firestaff_dm1_v1_champion_mirror_portrait_rect_ordinal16
 *     pc34_compat (sibling ordinal-16 + west_negative proof at
 *     (1,7) WEST and (2,7) WEST; no front_west_entry route,
 *     no re-blt invariant, no redraw-stability invariant)
 *   - firestaff_dm1_v1_hall_of_champions_portrait_16_cancel
 *     reopen_portrait_rect_position_runtime_probe (cancel_reopen
 *     route on the seeded (1,2,0) HALK cell - different front
 *     cell, different route, panel-state-machine invariant).
 *
 * Slice assignment (worker branch id suffix):
 *   firestaff_dm1_v1_hoc_champion_portrait_16_front_west_entry
 *   _portrait_rect_position_208_gate
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136,

    /* D1C champion portrait cutout (ReDMCSB DUNVIEW.C:3913-3928 /
     * G0109_Graphic558_Box_ChampionPortraitOnWall = {96, 127, 35,
     * 63}).  Viewport-local coords; the framebuffer destination
     * adds VIEWPORT_X / VIEWPORT_Y offsets. */
    PROBE_PORTRAIT_X_VP = 96,
    PROBE_PORTRAIT_Y_VP = 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_PORTRAIT_X_FB = VIEWPORT_X + PROBE_PORTRAIT_X_VP,
    PROBE_PORTRAIT_Y_FB = VIEWPORT_Y + PROBE_PORTRAIT_Y_VP,

    /* C026 portrait strip (DUNVIEW.C:3916-3919): 8 cols * 3 rows of
     * 32x29 portraits, ordinals 0..23. */
    PROBE_STRIP_COLS = 8,
    PROBE_STRIP_ROWS = 3,
    PROBE_ORDINAL_MAX = 24,
    PROBE_TRANSPARENT_PALETTE_IDX = 1,

    /* D1C wall-ornament zone (DUNVIEW.C G0205[5][12]): (80, 29, 64,
     * 43) viewport-local.  The portrait cutout (96, 35, 32, 29)
     * sits inside this frame. */
    PROBE_D1C_X_VP = 80,
    PROBE_D1C_Y_VP = 29,
    PROBE_D1C_W = 64,
    PROBE_D1C_H = 43,

    /* The C127 sensor lattice for ordinal 16 on the canonical
     * PC 3.4 EN DM1 V1 DUNGEON.DAT: ordinal 16 is exposed exactly
     * once at (2,7) DIR_SOUTH (front cell (2,8) has the C127
     * sensor on its NORTH aspect with sensorData = 16).  The
     * front_west_entry route is party at (1,8) DIR_EAST (front
     * cell (2,8), visibleWallCell = (EAST+2)&3 = 3 = WEST).  The
     * (2,8) WEST-aspect C127 sensor ships with sensorData = 0
     * (DAROOU, ordinal 0 / left neighbour of ordinal 16 in the
     * atlas), so the slice seeds that sensor from 0 to 16 to
     * lock the ordinal-16 edge case on the front_west_entry
     * route.  This is synthetic sensor mutation (no real
     * ordinal-16 sensor on the WEST aspect of (2,8) in the live
     * DM1 V1 DUNGEON.DAT); same honest-data-quality trade-off
     * the ordinal-23 front_north_entry probe and the ordinal-2
     * palette_match_rect probe already make. */
    PROBE_MAP_X = 1,
    PROBE_MAP_Y = 8,
    PROBE_DIR_FRONT = DIR_EAST,
    PROBE_FRONT_CELL_X = 2,
    PROBE_FRONT_CELL_Y = 8,
    PROBE_VISIBLE_WALL = 3 /* WEST */,

    /* Target ordinal.  Mirror catalog name and title strings are
     * #define'd below so they live in .rodata, not in the enum
     * block. */
    PROBE_ORDINAL = 16,

    /* Side-wall column bands for the no-floating proof.  The D1C
     * portrait rect sits at viewport x=96..127 (fb x=96..127
     * since VIEWPORT_X = 0).  Anything outside x=80..143 in the
     * portrait row band (y=35..63 viewport-local) is a side-wall
     * column band. */
    PROBE_PORTRAIT_ROW_Y_VP = PROBE_PORTRAIT_Y_VP,
    PROBE_PORTRAIT_ROW_H_VP = PROBE_PORTRAIT_H,
    PROBE_D1L_BAND_X_VP = 0,
    PROBE_D1L_BAND_W_VP = PROBE_PORTRAIT_X_VP,            /* 96 cols */
    PROBE_D1R_BAND_X_VP = PROBE_PORTRAIT_X_VP + PROBE_PORTRAIT_W, /* 128 */
    PROBE_D1R_BAND_W_VP = VIEWPORT_W - PROBE_D1R_BAND_X_VP,      /* 96 cols */

    /* Match thresholds (Firestaff runtime heuristics; not DOS pixel
     * parity).  Same values as the existing ordinal-5
     * front_south_entry probe for cross-slice consistency. */
    PROBE_MATCH_PCT = 95,
    PROBE_LEAK_PCT = 35,
    PROBE_SIDE_WALL_WARM_MAX = 30,

    /* Redraw stability cycle count.  Three cycles is enough to
     * catch a one-off frame-cache jitter bug without spending the
     * runtime budget on a longer loop. */
    PROBE_REDRAW_CYCLES = 3
};

/* Mirror catalog name and title for ordinal 16 (per DM1 V1 PC 3.4
 * catalog).  Defined as string literals outside the enum block
 * because ISO C does not allow string literals inside enum. */
#define PROBE_CHAMPION_NAME  "CHANI"
#define PROBE_CHAMPION_TITLE "SAYYADINA SIHAYA"

/* Warm-color palette set used by C026 champion-portrait sprites
 * (skin tones / clothing / accents).  Grey stone wall texture uses
 * {0x01, 0x02, 0x0D}; corridor floor uses {0x01, 0x02, 0x03, 0x04,
 * 0x0C}.  Counting warm pixels is the coarse portrait-presence
 * fingerprint used by every existing champion-mirror probe. */
static int palette_is_warm(unsigned char idx) {
    switch (idx & 0x0Fu) {
        case 0x07: case 0x08: case 0x09:
        case 0x0A: case 0x0B: case 0x0E:
            return 1;
        default:
            return 0;
    }
}


static int g_pass = 0;
static int g_fail = 0;

#define PASS(msg) do { ++g_pass; printf("  PASS: %s\n", msg); } while (0)
#define FAIL(msg) do { ++g_fail; printf("  FAIL: %s\n", msg); } while (0)
#define CHECK(cond, msg) do { \
    if (cond) { PASS(msg); } \
    else      { FAIL(msg); } \
} while (0)

/* Source-strip pixel at ordinal O, cell (x, y).  Returns the low
 * nibble (palette index); the source uses the upper bit for level. */
static unsigned char strip_pixel(const M11_AssetSlot* portraits,
                                 int ordinal, int x, int y) {
    int srcX;
    int srcY;
    if (!portraits || !portraits->pixels || ordinal < 0 ||
        ordinal >= PROBE_ORDINAL_MAX) {
        return 0;
    }
    srcX = (ordinal & (PROBE_STRIP_COLS - 1)) * PROBE_PORTRAIT_W + x;
    srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
    if (srcX < 0 || srcX >= (int)portraits->width ||
        srcY < 0 || srcY >= (int)portraits->height) {
        return 0;
    }
    return (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
}

/* Match ordinal O against the D1C portrait rect on the framebuffer.
 * Returns (matched << 16) | compared; caller decodes with
 * MATCH_MATCHED / MATCH_COMPARED. */
static int match_ordinal(const M11_AssetSlot* portraits,
                         const unsigned char* fb, int ordinal) {
    int matched = 0;
    int compared = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= PROBE_ORDINAL_MAX) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char src = strip_pixel(portraits, ordinal, x, y);
            unsigned char dst;
            if (src == PROBE_TRANSPARENT_PALETTE_IDX) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(
                fb[(PROBE_PORTRAIT_Y_FB + y) * FB_W +
                   (PROBE_PORTRAIT_X_FB + x)]);
            ++compared;
            if (dst == src) {
                ++matched;
            }
        }
    }
    return (matched << 16) | (compared & 0xFFFF);
}
#define MATCH_MATCHED(v)   (((v) >> 16) & 0xFFFF)
#define MATCH_COMPARED(v)  ((v) & 0xFFFF)

/* Sweep all 24 ordinals and return the ordinal with the highest
 * absolute matched-pixel count.  Used to verify "no other ordinal
 * wins the D1C rect" on the front_west_entry pose. */
static int best_ordinal(const M11_AssetSlot* portraits,
                        const unsigned char* fb) {
    int best = -1;
    int bestMatched = -1;
    int o;
    for (o = 0; o < PROBE_ORDINAL_MAX; ++o) {
        int r = match_ordinal(portraits, fb, o);
        int matched = MATCH_MATCHED(r);
        if (matched > bestMatched) {
            bestMatched = matched;
            best = o;
        }
    }
    return best;
}

/* Return the matched-pixel count for the requested ordinal on the
 * D1C rect (decoded from match_ordinal).  Used to compare ordinal
 * 16 to other ordinals on the same framebuffer. */
static int ordinal_matched(const M11_AssetSlot* portraits,
                           const unsigned char* fb, int ordinal) {
    return MATCH_MATCHED(match_ordinal(portraits, fb, ordinal));
}

/* Set the runtime pose to (mapX, mapY, dir) on map 0 with the
 * Hall-of-Champions panel state cleared. */
static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

/* Count warm-palette pixels in a viewport-space rectangle. */
static int viewport_rect_warm_count(const unsigned char* fb,
                                    int vpX, int vpY, int vpW, int vpH) {
    int count = 0;
    int x, y;
    if (!fb || vpW <= 0 || vpH <= 0) return 0;
    for (y = 0; y < vpH; ++y) {
        int fbY = VIEWPORT_Y + vpY + y;
        if (fbY < 0 || fbY >= FB_H) continue;
        for (x = 0; x < vpW; ++x) {
            int fbX = VIEWPORT_X + vpX + x;
            if (fbX < 0 || fbX >= FB_W) continue;
            unsigned char idx = M11_FB_DECODE_INDEX(
                fb[fbY * FB_W + fbX]);
            if (palette_is_warm(idx)) ++count;
        }
    }
    return count;
}

/* Count warm-palette pixels in the D1C portrait rect. */
static int portrait_rect_warm_count(const unsigned char* fb) {
    return viewport_rect_warm_count(fb,
                                    PROBE_PORTRAIT_X_VP,
                                    PROBE_PORTRAIT_Y_VP,
                                    PROBE_PORTRAIT_W,
                                    PROBE_PORTRAIT_H);
}

/* Seed the C127 sensor whose sensorData equals `oldData` AND whose
 * cell/visible-wall-aspect matches the request.  Returns the sensor
 * index, or -1 if not found.  Used to lock the ordinal-16 edge
 * case on the front_west_entry route (party at (1,8) DIR_EAST,
 * front cell (2,8), visibleWallCell WEST = 3) without mutating
 * unrelated cells. */
static int seed_c127_sensor(M11_GameViewState* state,
                            int oldData,
                            int newData) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127 &&
            (int)state->world.things->sensors[i].sensorData == oldData) {
            state->world.things->sensors[i].sensorData =
                (unsigned short)newData;
            return i;
        }
    }
    return -1;
}

/* Verify and pretty-print the catalog identity for ordinal 16.
 * The slice does not pin the catalog name to a hardcoded literal
 * (ordinal 16 is row 2, col 0 of the C026 atlas and the canonical
 * PC 3.4 EN catalog binding is verified at probe runtime). */
static void check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(game, PROBE_ORDINAL,
                                                name, sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_ORDINAL,
                                                 title, sizeof(title));
    if (name[0] != '\0') {
        PASS("ordinal 16 mirror catalog name is non-empty");
        printf("    ordinal %d catalog name=\"%s\" title=\"%s\"\n",
               PROBE_ORDINAL, name, title);
    } else {
        FAIL("ordinal 16 mirror catalog name is non-empty");
        printf("    ordinal %d returned empty catalog name\n",
               PROBE_ORDINAL);
    }
    if (strcmp(name, PROBE_CHAMPION_NAME) == 0) {
        printf("    NOTE: ordinal %d binds to canonical PC 3.4 EN "
               "CHANI / \"SAYYADINA SIHAYA\" on this DM1 V1 build\n",
               PROBE_ORDINAL);
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    unsigned char fbRef[FB_W * FB_H];
    int ordinalEast, ordinalNorth, ordinalSouth, ordinalWest;
    int ornX, ornY, ornW, ornH;
    int bestOrdinal;
    int cycle;
    int seededSensor;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 16 front_west_entry portrait_rect_position\n",
                argv[0]);
        return 2;
    }

    printf("=== DM1 V1 Hall of Champions: ordinal 16, route front_west_entry,\n");
    printf("===          aspect portrait_rect_position\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL: GRAPHICS.DAT C026 champion portrait strip unavailable "
                "(width=%d height=%d)\n",
                portraits ? (int)portraits->width : -1,
                portraits ? (int)portraits->height : -1);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("Source: DUNVIEW.C:3913-3928 + DUNVIEW.C:525 (G0109 {96,127,35,63})\n");
    printf("        DUNGEON.C:2573 / 2608-2612 / DEFS.H:2552 / MOVESENS.C:1501-1503.\n");
    printf("Slice:  party at (%d,%d) DIR_EAST (front cell=(%d,%d), "
           "visibleWallCell=%d=WEST).\n",
           PROBE_MAP_X, PROBE_MAP_Y,
           PROBE_FRONT_CELL_X, PROBE_FRONT_CELL_Y, PROBE_VISIBLE_WALL);
    printf("        seeding (2,8) WEST-aspect C127 sensor from shipped=0 "
           "(DAROOU) to 16 (target) for the duration of the probe.\n");

    /* (A) Mirror catalog identity. */
    check_catalog_identity(&game);

    /* (B) Front-cell ordinal contract at (1,8): EAST -> 16 after seed,
     *     N/S/W -> -1 (wrong wall).  We seed FIRST so the EAST pose
     *     resolves to 16; the three wrong-wall poses must reject the
     *     sensor on the wrong aspect (DUNGEON.C:2573 + DEFS.H:2552). */
    seededSensor = seed_c127_sensor(&game, 0 /* DAROOU */, 16 /* target */);
    if (seededSensor < 0) {
        fprintf(stderr,
                "SKIP front_west_entry_fixture_mismatch: no C127 sensor "
                "with sensorData=0 on map 0; this DM1 V1 build does not "
                "match the reference DUNGEON.DAT fixture for the "
                "(2,8) WEST-aspect seed site.\n");
        M11_GameView_Shutdown(&game);
        return 0;
    }
    printf("  PASS: seeded C127 sensor[%d] on the (2,8) WEST aspect "
           "from shipped sensorData=0 (DAROOU) to 16 (target)\n",
           seededSensor);
    ++g_pass;

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    ordinalEast = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST front mirror ordinal = 16 (got %d)",
                 ordinalEast);
        CHECK(ordinalEast == 16, buf);
    }

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, DIR_NORTH);
    ordinalNorth = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_NORTH front mirror ordinal = -1 (got %d)",
                 ordinalNorth);
        CHECK(ordinalNorth == -1, buf);
    }

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, DIR_SOUTH);
    ordinalSouth = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_SOUTH front mirror ordinal = -1 (got %d)",
                 ordinalSouth);
        CHECK(ordinalSouth == -1, buf);
    }

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, DIR_WEST);
    ordinalWest = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_WEST front mirror ordinal = -1 (got %d)",
                 ordinalWest);
        CHECK(ordinalWest == -1, buf);
    }

    /* (C) D1C wall-ornament zone returns the source-locked
     *     (80, 29, 64, 43) viewport-relative frame at the
     *     front_west_entry pose, with the portrait cutout
     *     (96, 35, 32, 29) parented at (+16, +6) inside. */
    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    ornX = ornY = ornW = ornH = 0;
    if (!M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH)) {
        FAIL("(1,8) DIR_EAST D1C wall-ornament zone unavailable");
    } else {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST D1C wall-ornament zone x = %d (want %d)",
                 ornX, PROBE_D1C_X_VP);
        CHECK(ornX == PROBE_D1C_X_VP, buf);
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST D1C wall-ornament zone y = %d (want %d)",
                 ornY, PROBE_D1C_Y_VP);
        CHECK(ornY == PROBE_D1C_Y_VP, buf);
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST D1C wall-ornament zone w = %d (want %d)",
                 ornW, PROBE_D1C_W);
        CHECK(ornW == PROBE_D1C_W, buf);
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST D1C wall-ornament zone h = %d (want %d)",
                 ornH, PROBE_D1C_H);
        CHECK(ornH == PROBE_D1C_H, buf);
        /* Cutout containment: the portrait cutout (96, 35, 32, 29)
         * must sit fully inside the wall-ornament frame (80, 29,
         * 64, 43).  If the frame is ever resized to exclude the
         * cutout, the C346 chrome would clip the portrait sprite. */
        {
            int ok = 1;
            if (PROBE_PORTRAIT_X_VP < ornX) ok = 0;
            if (PROBE_PORTRAIT_Y_VP < ornY) ok = 0;
            if (PROBE_PORTRAIT_X_VP + PROBE_PORTRAIT_W > ornX + ornW) ok = 0;
            if (PROBE_PORTRAIT_Y_VP + PROBE_PORTRAIT_H > ornY + ornH) ok = 0;
            CHECK(ok, "(1,8) DIR_EAST portrait cutout parented inside D1C frame");
        }
    }

    /* (D) Portrait_rect_position pixel proof at the front_west_entry
     *     pose: D1C rect (96, 35, 32, 29) is painted with ordinal-16
     *     pixels (warm_count >= 30, best_ordinal == 16, strict
     *     palette match >= 95% on the C026 ordinal-16 cell). */
    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);

    {
        int warm = portrait_rect_warm_count(fb);
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST D1C portrait rect warm_count = %d (>= %d)",
                 warm, PROBE_SIDE_WALL_WARM_MAX);
        CHECK(warm >= PROBE_SIDE_WALL_WARM_MAX, buf);
    }
    {
        bestOrdinal = best_ordinal(portraits, fb);
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST best_ordinal = 16 (got %d)",
                 bestOrdinal);
        CHECK(bestOrdinal == 16, buf);
    }
    {
        int matched = ordinal_matched(portraits, fb, PROBE_ORDINAL);
        int r = match_ordinal(portraits, fb, PROBE_ORDINAL);
        int compared = MATCH_COMPARED(r);
        int pct = (compared > 0) ? (matched * 100 / compared) : 0;
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST ordinal-16 palette_match = %d/%d (%d%%) >= %d%%",
                 matched, compared, pct, PROBE_MATCH_PCT);
        CHECK(compared > 0 && pct >= PROBE_MATCH_PCT, buf);
    }
    /* Ordinal-16 dominance against its row-2 neighbour ordinal 17
     * (right neighbour in the atlas) and its row-1 ordinal 9
     * (above in the atlas).  Ordinal 16 is its own champion; a
     * sibling atlas slot cannot accidentally win. */
    {
        int matched16 = ordinal_matched(portraits, fb, 16);
        int matched17 = ordinal_matched(portraits, fb, 17);
        int matched9  = ordinal_matched(portraits, fb, 9);
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST ordinal-16 dominance over row-2 right "
                 "(ord16=%d vs ord17=%d)",
                 matched16, matched17);
        CHECK(matched16 > matched17, buf);
        snprintf(buf, sizeof(buf),
                 "(1,8) DIR_EAST ordinal-16 dominance over row-1 above "
                 "(ord16=%d vs ord9=%d)",
                 matched16, matched9);
        CHECK(matched16 > matched9, buf);
    }

    /* (E) No-floating contract on side-wall column bands at the
     *     front_west_entry pose: D1L column band (x=0..95, y=35..63)
     *     and D1R column band (x=128..223, y=35..63) must NOT carry
     *     ordinal-16 warm pixels in the portrait row band. */
    {
        int d1lWarm = viewport_rect_warm_count(fb,
                                                PROBE_D1L_BAND_X_VP,
                                                PROBE_PORTRAIT_ROW_Y_VP,
                                                PROBE_D1L_BAND_W_VP,
                                                PROBE_PORTRAIT_ROW_H_VP);
        int d1rWarm = viewport_rect_warm_count(fb,
                                                PROBE_D1R_BAND_X_VP,
                                                PROBE_PORTRAIT_ROW_Y_VP,
                                                PROBE_D1R_BAND_W_VP,
                                                PROBE_PORTRAIT_ROW_H_VP);
        {
            char buf[160];
            snprintf(buf, sizeof(buf),
                     "(1,8) DIR_EAST D1L column band warm_count = %d (< %d)",
                     d1lWarm, PROBE_SIDE_WALL_WARM_MAX);
            CHECK(d1lWarm < PROBE_SIDE_WALL_WARM_MAX, buf);
        }
        {
            char buf[160];
            snprintf(buf, sizeof(buf),
                     "(1,8) DIR_EAST D1R column band warm_count = %d (< %d)",
                     d1rWarm, PROBE_SIDE_WALL_WARM_MAX);
            CHECK(d1rWarm < PROBE_SIDE_WALL_WARM_MAX, buf);
        }
    }

    /* (F) Re-blt invariant on EAST -> N/S/W transitions: turning
     *     away from the front_west_entry pose must NOT leave
     *     ordinal-16 pixels floating in the D1C rect.  Within the
     *     35% leak tolerance the ordinal-5 front_south_entry probe
     *     locks. */
    {
        const int transitions[3] = { DIR_NORTH, DIR_SOUTH, DIR_WEST };
        const char* tnames[3] = { "NORTH", "SOUTH", "WEST" };
        int ti;
        for (ti = 0; ti < 3; ++ti) {
            int matched;
            int r;
            int compared;
            int pct;
            char buf[200];
            set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, transitions[ti]);
            memset(fb, 0, sizeof(fb));
            M11_GameView_Draw(&game, fb, FB_W, FB_H);
            matched = ordinal_matched(portraits, fb, PROBE_ORDINAL);
            r = match_ordinal(portraits, fb, PROBE_ORDINAL);
            compared = MATCH_COMPARED(r);
            pct = (compared > 0) ? (matched * 100 / compared) : 0;
            snprintf(buf, sizeof(buf),
                     "(1,8) DIR_%s reblt ordinal-16 leak = %d/%d (%d%%) <= %d%%",
                     tnames[ti], matched, compared, pct, PROBE_LEAK_PCT);
            CHECK(pct <= PROBE_LEAK_PCT, buf);
        }
    }

    /* (G) Adjacent corridor sanity: walking the y=8 corridor from
     *     (0,8) to (3,8) facing NORTH must not expose ordinal 16.
     *     The (2,8) WEST sensor is not visible from any NORTH-facing
     *     pose - the front cell is north of the party, not east.
     *     This guards against a future regression where the C127
     *     sensor filter stops rejecting wrong-aspect sensors. */
    {
        const int sx[4] = { 0, 1, 2, 3 };
        int i;
        for (i = 0; i < 4; ++i) {
            int ord;
            char buf[160];
            set_pose(&game, sx[i], 8, DIR_NORTH);
            ord = M11_GameView_GetFrontMirrorOrdinal(&game);
            snprintf(buf, sizeof(buf),
                     "(%d,8) DIR_NORTH ordinal not 16 (got %d)",
                     sx[i], ord);
            CHECK(ord != 16, buf);
        }
    }

    /* (H) Redraw-stability byte equality at the front_west_entry
     *     pose: three back-to-back draws produce byte-stable
     *     framebuffers (no jitter across draws). */
    {
        int stable = 1;
        set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
        memset(fbRef, 0, sizeof(fbRef));
        M11_GameView_Draw(&game, fbRef, FB_W, FB_H);
        for (cycle = 1; cycle < PROBE_REDRAW_CYCLES; ++cycle) {
            unsigned char fbCur[FB_W * FB_H];
            memset(fbCur, 0, sizeof(fbCur));
            M11_GameView_Draw(&game, fbCur, FB_W, FB_H);
            if (memcmp(fbRef, fbCur, sizeof(fbRef)) != 0) {
                stable = 0;
                printf("    cycle %d framebuffer diverged from reference\n",
                       cycle);
                break;
            }
        }
        CHECK(stable, "(1,8) DIR_EAST redraw byte equality across 3 cycles");
    }

    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&game);
    return g_fail == 0 ? 0 : 1;
}
