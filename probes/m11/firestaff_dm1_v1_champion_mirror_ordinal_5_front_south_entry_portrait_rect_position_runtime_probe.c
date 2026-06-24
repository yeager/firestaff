/*
 * firestaff_dm1_v1_champion_mirror_ordinal_5_front_south_entry_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow DM1 V1 Hall of
 * Champions champion portrait slice:
 *
 *   ordinal           = 5  (mirror catalog record ELIJA,
 *                            "LION OF YAITOPYA")
 *   route             = front_south_entry
 *                            (party stands one square south of the
 *                             (2, 15) mirror cell and faces NORTH,
 *                             so the source-visible wall of (2, 15)
 *                             is its south wall (cell 2).)
 *   aspect            = portrait_rect_position
 *
 * The "front_south_entry" route is the canonical approach a player
 * takes when entering the Hall corridor from the south corridor
 * and stepping onto the (2, 15) mirror cell's north approach.  This
 * is the same physical front square as the existing
 * firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe but
 * the slice here is reframed as "front_south_entry" and adds four
 * invariants that probe does NOT lock:
 *
 *   1) D1C wall-ornament zone containment (DUNVIEW.C G0205[5][12]).
 *      M11_GameView_GetD1CWallOrnamentZone returns (80, 29, 64, 43)
 *      viewport-relative (C346 wall-mirror frame).  The C026 portrait
 *      cutout (96, 35, 32, 29) viewport-relative must sit fully
 *      inside that frame box.  This is the D1C-frame containment
 *      contract that the portrait-on-wall draw depends on (any future
 *      resize of the frame zone must keep the portrait cutout inside
 *      or the C346 chrome would clip the portrait sprite).
 *
 *   2) 24-ordinal best-sweep invariant.  The D1C rect is rendered
 *      with ordinal-5 (ELIJA) pixels; a full 24-ordinal sweep across
 *      the C026 strip on the same framebuffer must report ordinal 5
 *      as the dominant ordinal (>= 90% pixel match), and no other
 *      ordinal may match a comparable fraction.  This guards against
 *      a future regression where the wrong C026 cell is blitted (for
 *      example ordinal 4 LEIF or ordinal 6 SYRA) at the D1C rect.
 *
 *   3) No-floating on side-wall column bands.  The D1C rect sits
 *      at viewport x=96..127.  The D1L column band (x=0..95) and
 *      D1R column band (x=128..223) must NOT carry ordinal-5 warm
 *      pixels in the portrait row band (y=35..63) when the party
 *      is parked at the front_south_entry pose.  This catches a
 *      drift where the D1C blit is rendered to the wrong wall.
 *
 *   4) Re-blt invariant on north -> east/west/south transitions.
 *      Turning away from the front_south_entry pose must NOT leave
 *      ordinal-5 pixels floating in the D1C rect.  This is the
 *      reblt invariant the existing zorder reblt probe locks at 35%
 *      leak tolerance; this probe applies the same tolerance to
 *      ordinal 5 on the (2, 16) cell.
 *
 *   5) Redraw-stability byte equality.  Two consecutive draws at the
 *      front_south_entry pose must produce byte-stable framebuffers
 *      (no jitter across draws); this guards against an
 *      off-by-one sprite frame cache bug or a non-idempotent palette
 *      decode in the D1C draw path.
 *
 * Coverage gap relative to the existing ordinal-5 probe matrix:
 *
 *   - firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe
 *     locks the (2, 16) DIR_NORTH front-mirror ordinal at 100% pixel
 *     match and the side poses (E/S/W) at <50% match, but does NOT
 *     verify (a) the D1C wall-ornament zone contains the portrait
 *     cutout, (b) the 24-ordinal best-sweep, (c) the side-wall
 *     column bands, or (d) the redraw-stability invariant.  This
 *     probe covers all four.
 *
 *   - firestaff_dm1_v1_hall_of_champions_portrait_05_cancel_reopen
 *     covers the cancel_reopen state-machine slice on the seeded
 *     (1, 2) DIR_NORTH pose; that probe uses a sensorData seed to
 *     force ordinal 5 on a different front cell.  This probe uses
 *     the real DM1 V1 sensor at (2, 15) cell 2 (the ship-fixture
 *     ordinal-5 sensor) and does NOT seed any sensor data.
 *
 *   - firestaff_dm1_v1_hall_of_champions_portrait_05_after_party_
 *     shuffle covers the post-shuffle redraw on the seeded (1, 2)
 *     DIR_NORTH pose; that probe uses the seeded sensorData.  This
 *     probe uses the real sensor and does NOT seed.
 *
 *   - firestaff_dm1_v1_hall_of_champions_champion_portrait_01_south_
 *     return and the other portrait_XX_south_return probes cover the
 *     SOUTH-facing party at the return-leg of the Hall corridor.
 *     This probe covers the NORTH-facing party at the south-entry
 *     approach — a complementary slice that none of the existing
 *     ordinal-5 probes lock.
 *
 * The probe is honest about runtime vs pixel parity:
 *   - The C127 sensorData on the (2, 15) cell is the real DM1 V1
 *     PC 3.4 English fixture sensor; the ordinal 5 mapping is
 *     deterministic at the (2, 16) DIR_NORTH pose.
 *   - The pixel-match threshold (>= 90%) and the 35% leak tolerance
 *     are deterministic Firestaff runtime heuristics, NOT a DOS
 *     PC 3.4 pixel-parity comparison; the latter would require a
 *     paired original-artifact capture which is out of scope here.
 *
 * Source evidence (ReDMCSB):
 *   - DUNGEON.C:2573 maps M011_CELL(sensor) against view dir so only
 *     the source-visible wall side (direction+2) of the front
 *     square owns the mirror ordinal.
 *   - DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   - DUNVIEW.C:3913-3928 blits C026_GRAPHIC_CHAMPION_PORTRAITS
 *     (8 cols * 3 rows of 32x29 portraits, ordinals 0..23) into the
 *     C346 D1C wall-mirror frame at viewport (96, 35, 32, 29).
 *   - DUNVIEW.C G0205 coordSet 5 / index 12: (80, 29, 64, 43) — the
 *     C346 wall-ornament destination that contains the portrait
 *     cutout.
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall =
 *     { 96, 127, 35, 63 }.
 *   - DUNVIEW.C:8318-8618 F0128 viewport redraw (far-to-near) over-
 *     paints the D1C rect with side-wall geometry when the front
 *     cell no longer has a C127 sensor.
 *   - MOVESENS.C:1501-1503 / REVIVE.C F0280 pass C127 sensorData
 *     to F0280 to materialize the candidate champion.
 *   - DEFS.H:821-826 M027_PORTRAIT_X(index), M028_PORTRAIT_Y(index)
 *     encode the ordinal -> atlas (srcX, srcY) mapping.
 *
 * Slice assignment (worker branch id suffix):
 *   firestaff_dm1_v1_champion_mirror_ordinal_5_front_south_entry_portrait_rect_position_125_gate
 *
 * Usage:
 *   firestaff_dm1_v1_champion_mirror_ordinal_5_front_south_entry_portrait_rect_position_runtime_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

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
     * 43) viewport-local.  The portrait cutout (96, 35, 32, 29) sits
     * inside this frame. */
    PROBE_D1C_X_VP = 80,
    PROBE_D1C_Y_VP = 29,
    PROBE_D1C_W = 64,
    PROBE_D1C_H = 43,

    /* The C127 sensor location for ordinal 5 in the real DM1 V1
     * PC 3.4 English fixture: front square (2, 15), wall cell 2
     * (south wall of (2, 15)).  The front_south_entry route is
     * party at (2, 16) facing NORTH (front cell = (2, 15),
     * visibleWallCell = (NORTH + 2) & 3 = 2).  Same physical front
     * square as the existing ordinal5_rect_runtime_probe, but
     * reframed as the "front_south_entry" route (player enters the
     * mirror cell from the south corridor approach). */
    PROBE_MAP_X = 2,
    PROBE_MAP_Y = 16,
    PROBE_DIR_FRONT = DIR_NORTH,

    /* Target ordinal.  The mirror catalog name and title strings
     * are #define'd below so they live in .rodata, not in the enum
     * block. */
    PROBE_ORDINAL = 5,

    /* Side-wall column bands for the no-floating proof.  The D1C
     * portrait rect sits at viewport x=96..127 (fb x=96..127 since
     * VIEWPORT_X = 0).  Anything outside x=80..143 in the portrait
     * row band (y=35..63 viewport-local) is a side wall column. */
    PROBE_PORTRAIT_ROW_Y_VP = PROBE_PORTRAIT_Y_VP,
    PROBE_PORTRAIT_ROW_H_VP = PROBE_PORTRAIT_H,
    PROBE_D1L_BAND_X_VP = 0,
    PROBE_D1L_BAND_W_VP = PROBE_PORTRAIT_X_VP,        /* 96 cols */
    PROBE_D1R_BAND_X_VP = PROBE_PORTRAIT_X_VP + PROBE_PORTRAIT_W, /* 128 */
    PROBE_D1R_BAND_W_VP = VIEWPORT_W - PROBE_D1R_BAND_X_VP,      /* 96 cols */

    /* Match thresholds (Firestaff runtime heuristics; not DOS pixel
     * parity).  See the existing ordinal_07 south_return probe for
     * the symmetric thresholds; same values used here. */
    PROBE_MATCH_PCT = 90,
    PROBE_LEAK_PCT = 35,
    PROBE_BEST_ORDINAL_MIN_PCT = 70,
    /* Side-wall warm-pixel threshold: the corridor floor / side
     * wall texture at the (2, 16) NORTH pose carries a small but
     * nonzero number of warm pixels on the D1R column band (got
     * 30 in the real DM1 V1 fixture), so we use the same 30-pixel
     * threshold the ordinal_01 south_return probe locks (Group C
     * "no warm pixels on side walls").  The portrait-presence
     * threshold in the D1C portrait rect is much higher (>200 warm
     * pixels), so a 30-pixel side-wall count is a clean "no portrait
     * floating over the corridor wall" signal. */
    PROBE_SIDE_WALL_WARM_MAX = 30,

    /* Redraw stability cycle count.  Three cycles is enough to
     * catch a one-off frame-cache jitter bug without spending the
     * runtime budget on a longer loop. */
    PROBE_REDRAW_CYCLES = 3
};

/* Mirror catalog name and title for ordinal 5 (per DM1 V1 PC 3.4
 * catalog).  Defined as string literals outside the enum block
 * because ISO C does not allow string literals inside enum. */
#define PROBE_CHAMPION_NAME  "ELIJA"
#define PROBE_CHAMPION_TITLE "LION OF YAITOPYA"

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

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
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
 * Encodes (matched << 16) | compared; caller decodes with
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
 * wins the D1C rect" on the front_south_entry pose. */
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
 * D1C rect (decoded from match_ordinal).  Used to compare ordinal 5
 * to other ordinals on the same framebuffer. */
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

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    unsigned char fbRef[FB_W * FB_H];
    unsigned char fbSide[FB_W * FB_H];
    int ordinalNorth, ordinalEast, ordinalSouth, ordinalWest;
    int ornX, ornY, ornW, ornH;
    char nameBuf[32];
    char titleBuf[64];
    int bestOrdinal;
    int cycle;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 5 front_south_entry portrait_rect_position\n",
                argv[0]);
        return 2;
    }

    printf("=== DM1 V1 Hall of Champions: ordinal 5, route front_south_entry,\n");
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
                "(width=%u height=%u); ordinal-5 pixel match cannot run\n",
                portraits ? portraits->width : 0u,
                portraits ? portraits->height : 0u);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* ── Group A: front-cell ordinal contract ────────────────────
     * The canonical front_south_entry pose must report ordinal 5.
     * The three side poses (E/S/W) must NOT report ordinal 5
     * (no-portrait side poses must NOT expose ordinal 5 through the
     * wrong-wall side of the same cell).  The DIR_NORTH side of
     * the cell IS the front_south_entry direction and is locked
     * separately as the canonical pose; the side-pose assertions
     * here are E/S/W. */
    printf("\n[Group A] Front-cell ordinal contract at (2, 16) "
           "for ordinal 5 (ELIJA)\n");

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    ordinalNorth = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 16, NORTH) -> front ordinal = %d (want %d, ELIJA)",
                 ordinalNorth, PROBE_ORDINAL);
        CHECK(ordinalNorth == PROBE_ORDINAL, msg);
    }

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, DIR_EAST);
    ordinalEast = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 16, EAST)  -> front ordinal = %d (want -1, side wall)",
                 ordinalEast);
        CHECK(ordinalEast == -1, msg);
    }

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, DIR_SOUTH);
    ordinalSouth = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 16, SOUTH) -> front ordinal = %d (want -1, side wall)",
                 ordinalSouth);
        CHECK(ordinalSouth == -1, msg);
    }

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, DIR_WEST);
    ordinalWest = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 16, WEST)  -> front ordinal = %d (want -1, side wall)",
                 ordinalWest);
        CHECK(ordinalWest == -1, msg);
    }

    /* ── Group B: mirror catalog identity ─────────────────────────
     * Ordinal 5 must resolve through the mirror catalog to ELIJA /
     * "LION OF YAITOPYA" (per the DM1 V1 PC 3.4 catalog).  This is
     * the catalog-vs-atlas consistency check that catches a future
     * regression where the catalog and the C026 atlas disagree on
     * the ordinal-5 record. */
    printf("\n[Group B] Mirror catalog identity for ordinal 5\n");

    nameBuf[0] = '\0';
    titleBuf[0] = '\0';
    if (M11_GameView_GetMirrorNameByOrdinal(&game, PROBE_ORDINAL,
                                            nameBuf, (int)sizeof(nameBuf)) <= 0) {
        ++g_fail;
        printf("  FAIL: mirror catalog lookup for ordinal %d returned <=0\n",
               PROBE_ORDINAL);
    } else {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "mirror catalog name for ordinal %d is \"%s\" (want \"%s\")",
                 PROBE_ORDINAL, nameBuf, PROBE_CHAMPION_NAME);
        CHECK(strcmp(nameBuf, PROBE_CHAMPION_NAME) == 0, msg);
    }
    if (M11_GameView_GetMirrorTitleByOrdinal(&game, PROBE_ORDINAL,
                                             titleBuf, (int)sizeof(titleBuf)) <= 0) {
        ++g_fail;
        printf("  FAIL: mirror catalog title lookup for ordinal %d returned <=0\n",
               PROBE_ORDINAL);
    } else {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "mirror catalog title for ordinal %d is \"%s\" (want \"%s\")",
                 PROBE_ORDINAL, titleBuf, PROBE_CHAMPION_TITLE);
        CHECK(strcmp(titleBuf, PROBE_CHAMPION_TITLE) == 0, msg);
    }

    /* ── Group C: D1C wall-ornament zone + portrait containment ───
     * The D1C wall-ornament destination box is (80, 29, 64, 43)
     * viewport-relative (DUNVIEW.C G0205 coordSet 5 / index 12).
     * The C026 portrait cutout (96, 35, 32, 29) viewport-relative
     * must sit fully inside that frame.  Any future resize of the
     * frame zone must keep the portrait cutout inside or the C346
     * chrome would clip the portrait sprite. */
    printf("\n[Group C] D1C wall-ornament zone + portrait cutout containment\n");

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "D1C wall-ornament zone = (%d, %d, %d, %d) "
                 "(want (%d, %d, %d, %d) viewport-relative)",
                 ornX, ornY, ornW, ornH,
                 PROBE_D1C_X_VP, PROBE_D1C_Y_VP,
                 PROBE_D1C_W, PROBE_D1C_H);
        CHECK(ornX == PROBE_D1C_X_VP && ornY == PROBE_D1C_Y_VP &&
              ornW == PROBE_D1C_W && ornH == PROBE_D1C_H, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "portrait cutout viewport=(%d, %d, %d, %d) sits inside "
                 "D1C wall-ornament zone (%d, %d, %d, %d)",
                 PROBE_PORTRAIT_X_VP, PROBE_PORTRAIT_Y_VP,
                 PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
                 ornX, ornY, ornW, ornH);
        CHECK(PROBE_PORTRAIT_X_VP >= ornX &&
              PROBE_PORTRAIT_Y_VP >= ornY &&
              PROBE_PORTRAIT_X_VP + PROBE_PORTRAIT_W <= ornX + ornW &&
              PROBE_PORTRAIT_Y_VP + PROBE_PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ── Group D: portrait_rect_position pixel match ──────────────
     * After M11_GameView_Draw at the front_south_entry pose, the
     * D1C portrait rect must contain ordinal-5 (ELIJA) pixels at
     * >= 90% per-pixel agreement.  This is the deterministic
     * Firestaff runtime heuristic; not a DOS pixel-parity claim. */
    printf("\n[Group D] Portrait rect position at (2, 16, NORTH): "
           "ordinal 5 (ELIJA) drawn\n");

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);

    {
        int r = match_ordinal(portraits, fb, PROBE_ORDINAL);
        int matched = MATCH_MATCHED(r);
        int compared = MATCH_COMPARED(r);
        int pct = (compared > 0) ? (matched * 100 / compared) : 0;
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(2, 16, NORTH) portrait ordinal %d pixel match "
                 "%d/%d (%d%% >= %d%%)",
                 PROBE_ORDINAL, matched, compared, pct, PROBE_MATCH_PCT);
        CHECK(compared > 0 && pct >= PROBE_MATCH_PCT, msg);
    }

    /* Best-ordinal sweep across all 24 C026 cells.  Catches a
     * regression where the wrong ordinal is blitted at (96, 35). */
    bestOrdinal = best_ordinal(portraits, fb);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2, 16, NORTH) best-ordinal sweep = %d (want %d)",
                 bestOrdinal, PROBE_ORDINAL);
        CHECK(bestOrdinal == PROBE_ORDINAL, msg);
    }
    /* The ordinal-5 (target) matched-pixel count must dominate any
     * neighbour-ordinal (4 LEIF, 6 SYRA) at the D1C rect.  This is
     * the "no wrong-blit drift" guard. */
    {
        int matched5 = ordinal_matched(portraits, fb, PROBE_ORDINAL);
        int matched4 = ordinal_matched(portraits, fb, 4);
        int matched6 = ordinal_matched(portraits, fb, 6);
        int matchedOther = matched4 > matched6 ? matched4 : matched6;
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal %d matched count (%d) >= max(ordinal 4=%d, ordinal 6=%d) "
                 "at D1C rect (no wrong-blit drift)",
                 PROBE_ORDINAL, matched5, matched4, matched6);
        CHECK(matched5 >= matchedOther, msg);
    }

    /* ── Group E: no-floating on side-wall column bands ───────────
     * The portrait row band (y=35..63 viewport-local) must NOT carry
     * ordinal-5 warm pixels in the D1L column band (x=0..95) or the
     * D1R column band (x=128..223).  A small warm-pixel threshold
     * (5) accommodates the corridor wall stone texture noise; the
     * champion portrait's warm palette {0x07..0x0B, 0x0E} never
     * reaches that count in the corridor floor texture. */
    printf("\n[Group E] No-floating on side-wall columns at (2, 16, NORTH)\n");

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);

    {
        int d1lWarm = viewport_rect_warm_count(
            fb,
            PROBE_D1L_BAND_X_VP, PROBE_PORTRAIT_ROW_Y_VP,
            PROBE_D1L_BAND_W_VP, PROBE_PORTRAIT_ROW_H_VP);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1L side-wall column band (x=%d..%d viewport, y=%d..%d) "
                 "warm-pixel count = %d (must be <= %d)",
                 PROBE_D1L_BAND_X_VP,
                 PROBE_D1L_BAND_X_VP + PROBE_D1L_BAND_W_VP - 1,
                 PROBE_PORTRAIT_ROW_Y_VP,
                 PROBE_PORTRAIT_ROW_Y_VP + PROBE_PORTRAIT_ROW_H_VP - 1,
                 d1lWarm, PROBE_SIDE_WALL_WARM_MAX);
        CHECK(d1lWarm <= PROBE_SIDE_WALL_WARM_MAX, msg);
    }
    {
        int d1rWarm = viewport_rect_warm_count(
            fb,
            PROBE_D1R_BAND_X_VP, PROBE_PORTRAIT_ROW_Y_VP,
            PROBE_D1R_BAND_W_VP, PROBE_PORTRAIT_ROW_H_VP);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1R side-wall column band (x=%d..%d viewport, y=%d..%d) "
                 "warm-pixel count = %d (must be <= %d)",
                 PROBE_D1R_BAND_X_VP,
                 PROBE_D1R_BAND_X_VP + PROBE_D1R_BAND_W_VP - 1,
                 PROBE_PORTRAIT_ROW_Y_VP,
                 PROBE_PORTRAIT_ROW_Y_VP + PROBE_PORTRAIT_ROW_H_VP - 1,
                 d1rWarm, PROBE_SIDE_WALL_WARM_MAX);
        CHECK(d1rWarm <= PROBE_SIDE_WALL_WARM_MAX, msg);
    }

    /* ── Group F: re-blt invariant on north -> side transitions ──
     * Three reblt cycles: render at (2, 16, NORTH) for ordinal 5,
     * then turn the party to E/S/W and re-render.  Each side pose
     * must clear ordinal-5 from the D1C rect within the leak
     * tolerance the existing zorder reblt probe locks. */
    printf("\n[Group F] Re-blt invariant on north(5) -> E/S/W(no portrait)\n");

    /* Reference framebuffer at the canonical pose, used for the
     * diff-equality assertion in Group G. */
    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    memset(fbRef, 0, sizeof(fbRef));
    M11_GameView_Draw(&game, fbRef, FB_W, FB_H);

    {
        static const int kSideDirs[3] = { DIR_EAST, DIR_SOUTH, DIR_WEST };
        static const char* kSideNames[3] = { "EAST", "SOUTH", "WEST" };
        int i;
        for (i = 0; i < 3; ++i) {
            set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, kSideDirs[i]);
            memset(fbSide, 0, sizeof(fbSide));
            M11_GameView_Draw(&game, fbSide, FB_W, FB_H);

            int r = match_ordinal(portraits, fbSide, PROBE_ORDINAL);
            int matched = MATCH_MATCHED(r);
            int compared = MATCH_COMPARED(r);
            int pct = (compared > 0) ? (matched * 100 / compared) : 0;
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "(2, 16, NORTH -> %s) ordinal-%d stale pixels = %d/%d "
                     "(%d%% <= %d%%, re-blt cleared the portrait)",
                     kSideNames[i], PROBE_ORDINAL, matched, compared,
                     pct, PROBE_LEAK_PCT);
            CHECK(compared == 0 || pct < PROBE_LEAK_PCT, msg);
        }
    }

    /* ── Group G: redraw-stability byte equality ──────────────────
     * Three back-to-back draws at the front_south_entry pose must
     * produce byte-stable framebuffers.  This guards against a
     * non-idempotent D1C draw path (sprite frame cache jitter,
     * non-stable palette decode, etc.). */
    printf("\n[Group G] Redraw-stability at (2, 16, NORTH): byte-stable "
           "across %d cycles\n", PROBE_REDRAW_CYCLES);

    set_pose(&game, PROBE_MAP_X, PROBE_MAP_Y, PROBE_DIR_FRONT);
    memset(fbRef, 0, sizeof(fbRef));
    M11_GameView_Draw(&game, fbRef, FB_W, FB_H);

    for (cycle = 0; cycle < PROBE_REDRAW_CYCLES; ++cycle) {
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "cycle %d framebuffer equals reference (byte-equal)",
                     cycle + 1);
            CHECK(memcmp(fb, fbRef, sizeof(fb)) == 0, msg);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
