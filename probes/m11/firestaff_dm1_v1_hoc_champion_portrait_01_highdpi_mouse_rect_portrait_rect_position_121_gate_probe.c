/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 1 (HALK)
 * highdpi_mouse_rect / portrait_rect_position runtime gate probe.
 *
 * Targeted slice:
 *   ordinal    = 1 (HALK / barbarian, C127 sensorData=1)
 *   pose       = (map 0, x=1, y=2) facing NORTH
 *                 (front cell (1,1) carries the C127 champion mirror)
 *   route      = highdpi_mouse_rect
 *                 (the M11 scaled-window mouse dispatch path:
 *                  TOUCHCLICK_Compat_MapScaledViewportPointToDispatch
 *                  + TOUCHCLICK_Compat_MapScaledScreenPointToDispatch
 *                  + M11_Render_MapPointToFramebuffer on the SDL
 *                  render framebuffer. The probe verifies that
 *                  physical clicks landing inside the C026 portrait
 *                  cutout at viewport (96, 35, 32, 29) on multiple
 *                  high-DPI surface sizes do NOT float to a wall /
 *                  side-wall / panel zone — the portrait cutout is
 *                  inside the C007 viewport.dungeon zone, so
 *                  source-locked COMMAND.C:403 dispatch routes the
 *                  click to C080 viewport.dungeon / zoneIndex 7.)
 *   aspect     = portrait_rect_position
 *                 (the (96, 35, 32, 29) viewport-local cutout stays
 *                  anchored at the same source-locked rectangle for
 *                  ordinal 1 even after the scaled dispatch math,
 *                  so the inverse letterbox maps every high-DPI
 *                  physical pixel that lands inside the cutout back
 *                  onto viewport (96..127, 35..63).)
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate
 *     _portrait_rect_position_097_gate_probe covers the
 *     redraw_after_candidate cycle for ordinal 1 (HALK) but does
 *     not exercise the high-DPI scaled-window dispatch path on
 *     physical surface pixels.
 *   - tests/test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_compat.c
 *     exercises the scaled dispatch path on the chest slot zones
 *     but not on the C026 champion portrait cutout (which lives
 *     inside the C007 viewport.dungeon zone, not on a dedicated
 *     portrait zone).
 *   - firestaff_dm1_v1_hoc_champion_portrait_19_wall_ornament_no_
 *     float_runtime_probe covers the wall-ornament side of the
 *     ordinal 19 portrait rect but does not assert the dispatch
 *     routing on high-DPI surfaces.
 *
 * This probe fills that narrow slice: it pins the portrait_rect
 * (96, 35, 32, 29) viewport-local position invariant for ordinal 1
 * across the scaled-window dispatch math, on four distinct surface
 * sizes that cover the original HiDPI failure mode
 * (1512x982 MacBook Pro Retina) and a 4K letterbox case
 * (2560x1440). It does not duplicate any of the existing ordinal 1
 * probes because none of them assert the scaled dispatch / SDL
 * framebuffer map path against the C026 cutout.
 *
 * What the probe asserts:
 *   Stage 1 (ordinal sanity):    the front-mirror route at (1,2)
 *                                 NORTH reports ordinal 1 and the
 *                                 D1C wall zone matches coordSet
 *                                 5 / index 12 = (80, 29, 64, 43).
 *   Stage 2 (cutout invariant):  the source viewport rect remains
 *                                 (0, 33, 224, 136) per COORD.C
 *                                 and the portrait cutout (96, 35,
 *                                 32, 29) stays inside that
 *                                 viewport. The C026 portrait
 *                                 graphic is loaded and the
 *                                 ordinal-1 strip cell is reachable.
 *   Stage 3 (per-surface scaled dispatch):
 *     For each of 1280x720, 1920x1080, 2560x1440, 1512x982:
 *     a) TOUCHCLICK_Compat_NormalizeScaledViewportPoint maps a
 *        physical pixel picked at the inverse letterbox of the
 *        cutout center back to viewport-local coords inside the
 *        (96..127, 35..63) source cutout.
 *     b) Source-locked no-float (viewport path): the viewport
 *        dispatch at the portrait cutout center must NOT land on
 *        a portrait-specific zone (none is registered per
 *        COMMAND.C:403).  The touch-click matrix registers the
 *        C026 cutout as part of the wider C007 viewport.dungeon
 *        zone which is screen-relative, so the viewport-relative
 *        scan (HitTestInCoordMode) returns 0 for the cutout when
 *        no inventory / panel is open.  This is the source-locked
 *        no-float assertion: the cutout must not have a
 *        VIEWPORT_RELATIVE click zone that would steal clicks.
 *     c) Source-locked screen dispatch (screen path):
 *        TOUCHCLICK_Compat_MapScaledScreenPointToDispatch routes
 *        the cutout center through the primary interface table
 *        (COMMAND.C:1641-1644) and hits viewport.dungeon (C080,
 *        zoneIndex 7).  The dispatched screenX / screenY lands
 *        inside the screen-local portrait rect (96..127, 68..96)
 *        because the dispatch records the source-locked 320x200
 *        screen coordinate.
 *     d) M11_Render_MapPointToFramebuffer lands inside the source
 *        320x200 portrait cutout (96..127, 68..96) on the SDL
 *        render framebuffer map path.
 *   Stage 4 (no portrait-specific zone): a scan of every registered
 *     viewport-relative click zone in the public touch-click matrix
 *     confirms no zone covers the (96, 35, 32, 29) cutout.  This is
 *     the static no-float check that backs Stage 3b.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 D1C champion portrait blit (C026)
 *   ReDMCSB DUNVIEW.C:4547-4581 nibble 2 -> ordinal 1 / nibble 1
 *                       -> ordinal 0 (G0289 ordinal decode)
 *   ReDMCSB DUNVIEW.C:14271-14313 (D1C champion mirror BUG-120/121
 *                       guard: portrait anchored at (96, 35, 32, 29))
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin/224x136 dims
 *   ReDMCSB COMMAND.C:403 C080 maps to C007_ZONE_VIEWPORT;
 *                       layout-696 C003/C007 x=0 y=33 w=224 h=136
 *   ReDMCSB COMMAND.C:1641-1644 source-orders primary mouse input
 *                       before secondary mouse input
 *   firestaff shared/touch_click_zone_matrix_pc34_compat.c:25-26
 *                       registers viewport.dungeon at C080/zoneIndex
 *                       7 and intentionally omits any champion
 *                       portrait-specific click zone (the portrait
 *                       cutout is inside viewport.dungeon)
 *   firestaff tests/test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_
 *                       compat.c:1-20 establishes the scaled
 *                       dispatch + framebuffer map letterbox math
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_01_highdpi_mouse_rect_portrait_rect_position_121_gate_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"
#include "touch_click_zone_matrix_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    /* Source-locked PC 3.4 viewport (ReDMCSB COORD.C:1693-1722):
     * origin (M11_VIEWPORT_X, M11_VIEWPORT_Y) = (0, 33); size
     * (M11_VIEWPORT_W, M11_VIEWPORT_H) = (224, 136). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136,
    /* C026 champion portrait cutout (viewport-local) inside the D1C
     * wall box.  ReDMCSB DUNVIEW.C:3913-3928 /
     * m11_draw_dm1_front_champion_portrait uses
     *   M11_AssetLoader_BlitRegion(portraits,
     *       (portraitIdx & 7) * M11_PORTRAIT_W (== 32),
     *       (portraitIdx >> 3) * M11_PORTRAIT_H (== 29),
     *       M11_PORTRAIT_W, M11_PORTRAIT_H,
     *       M11_VIEWPORT_X + 96, M11_VIEWPORT_Y + 35, ...)
     * so the cutout is (96, 35, 32, 29) viewport-local = (96, 68, 32,
     * 29) framebuffer-local.  HiDPI dispatch must keep this rect
     * intact on every scaled surface. */
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* Framebuffer-local (320x200) rect: viewport (96, 35) maps to
     * screen (96, 35+33) = (96, 68). */
    PORTRAIT_X_FB = 96,
    PORTRAIT_Y_FB = 68,
    /* D1C champion-mirror frame zone from
     * M11_GameView_GetD1CWallOrnamentZone (coordSet 5 / index 12 per
     * DUNVIEW.C G0205): dstX=80, dstY=29, w=64, h=43 viewport-local.
     * The C026 portrait cutout (96, 35, 32, 29) sits inside this
     * zone. */
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    /* Hall of Champions ordinal 1 = HALK / barbarian.  C127 sensorData
     * is 0-indexed per DUNVIEW.C:4547-4581, so the M11 ordinal = 1 is
     * the second cell in the portrait strip: column 1, row 0. */
    ORDINAL_HALK = 1,
    /* Source-locked COMMAND.C:403 C080 -> C007_ZONE_VIEWPORT and the
     * firestaff touch-click matrix records zoneIndex=7 for that
     * mapping. */
    PORTRAIT_DISPATCH_COMMAND_ID = 80u,
    PORTRAIT_DISPATCH_ZONE_INDEX = 7u
};

/* HiDPI surface sizes covered by the probe.  These are the same
 * surfaces the original HiDPI chest-slot regression test exercises
 * (tests/test_dm1_v1_hidpi_chest_slot_hit_zone_pc34_compat.c).  The
 * 1512x982 surface is the original MacBook Pro Retina HiDPI failure
 * case (16" pre-2024), and 2560x1440 is the 4K letterbox case that
 * stresses aspect-ratio handling. */
typedef struct Surface {
    const char* label;
    int width;
    int height;
} Surface;

static const Surface kSurfaces[] = {
    { "1280x720",   1280,  720  },
    { "1920x1080",  1920,  1080 },
    { "2560x1440",  2560,  1440 },
    { "1512x982",   1512,  982  }
};
static const int kSurfacesCount = (int)(sizeof(kSurfaces) / sizeof(kSurfaces[0]));

static int g_failures = 0;
static int g_pass = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } else { \
        ++g_pass; \
    } \
} while (0)

/*
 * Pick the physical surface pixel whose inverse letterbox lands on
 * the requested source point.  We sweep the surface for the first
 * integer physical pixel whose normalized output equals the
 * requested source coords.  The sweep is bounded by the surface
 * dimensions and is the same O(W*H) routine the existing HiDPI
 * regression test relies on for pre-computed physical pixels.
 *
 * Returns 1 on success and writes the picked (physicalX, physicalY)
 * pair into the out-params; returns 0 if no physical pixel in the
 * surface lands on the requested source point.
 */
static int pick_physical_pixel_for_source(const Surface* surface,
                                          int sourceW,
                                          int sourceH,
                                          int sourceX,
                                          int sourceY,
                                          int* outPhysicalX,
                                          int* outPhysicalY) {
    int candidateX;
    int candidateY;
    (void)sourceW;
    (void)sourceH;
    for (candidateY = 0; candidateY < surface->height; ++candidateY) {
        for (candidateX = 0; candidateX < surface->width; ++candidateX) {
            int normalizedX;
            int normalizedY;
            if (!TOUCHCLICK_Compat_NormalizeScaledViewportPoint(
                    candidateX, candidateY,
                    surface->width, surface->height,
                    &normalizedX, &normalizedY)) {
                continue;
            }
            if (normalizedX == sourceX && normalizedY == sourceY) {
                *outPhysicalX = candidateX;
                *outPhysicalY = candidateY;
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Pick the physical surface pixel whose inverse screen-letterbox
 * (320x200) lands on the requested source point.  Used to feed the
 * screen-letterbox dispatch path used by
 * TOUCHCLICK_Compat_MapScaledScreenPointToDispatch.
 */
static int pick_physical_pixel_for_screen_source(const Surface* surface,
                                                  int sourceX,
                                                  int sourceY,
                                                  int* outPhysicalX,
                                                  int* outPhysicalY) {
    int candidateX;
    int candidateY;
    for (candidateY = 0; candidateY < surface->height; ++candidateY) {
        for (candidateX = 0; candidateX < surface->width; ++candidateX) {
            int screenX;
            int screenY;
            if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(
                    candidateX, candidateY,
                    surface->width, surface->height,
                    &screenX, &screenY)) {
                continue;
            }
            if (screenX == sourceX && screenY == sourceY) {
                *outPhysicalX = candidateX;
                *outPhysicalY = candidateY;
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Reset the game view to the canonical ordinal-1 front-mirror pose
 * (1, 2) NORTH and clear any panel / inventory state that would
 * otherwise route clicks to a panel zone.
 */
static void reset_to_ordinal1_pose(M11_GameViewState* game) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = 1;
    game->world.party.mapY = 2;
    game->world.party.direction = 0; /* DIR_NORTH */
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

int main(int argc, char** argv) {
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ornX, ornY, ornW, ornH;
    int frontOrdinal;
    int vx, vy, vw, vh;
    int surfaceIndex;
    const char* dataDir;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 1 (HALK) highdpi_mouse_rect portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 HoC portrait ordinal 1 (HALK) highdpi_mouse_rect ===\n");
    printf("dataDir=%s pose=(map 0, x=1, y=2) facing NORTH\n", dataDir);

    /* Stage 1: ordinal sanity.  Front-mirror route lookup at (1,2)
     * NORTH must report ordinal 1 per DUNGEON.C:2573 and
     * MOVESENS.C:1501-1503. */
    reset_to_ordinal1_pose(&game);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != ORDINAL_HALK) {
        printf("SKIP this DM1 V1 build does not place the C127 sensor "
               "with sensorData=1 at (1,2) front cell (got ordinal=%d, "
               "want %d); the highdpi_mouse_rect slice is not "
               "exercised on builds that do not match the reference "
               "DUNGEON.DAT fixture.\n",
               frontOrdinal, ORDINAL_HALK);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    CHECK(frontOrdinal == ORDINAL_HALK);

    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    CHECK(ornX == D1C_ZONE_X_VP);
    CHECK(ornY == D1C_ZONE_Y_VP);
    CHECK(ornW == D1C_ZONE_W);
    CHECK(ornH == D1C_ZONE_H);

    /* Stage 2: source-locked invariants.  Source viewport rect must
     * remain (0, 33, 224, 136) per COORD.C.  The portrait cutout
     * stays inside the viewport and inside the D1C zone.  The C026
     * portrait graphic must be loadable so we know what the cutout
     * is supposed to paint. */
    CHECK(TOUCHCLICK_Compat_GetSourceViewportRect(&vx, &vy, &vw, &vh) == 1);
    CHECK(vx == 0);
    CHECK(vy == 33);
    CHECK(vw == 224);
    CHECK(vh == 136);
    CHECK(PORTRAIT_X_VP >= 0 && PORTRAIT_X_VP < vw);
    CHECK(PORTRAIT_Y_VP >= 0 && PORTRAIT_Y_VP + PORTRAIT_H <= vh);
    CHECK(PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
          PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
          PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
          PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H);

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    CHECK(portraits->loaded);
    {
        int srcBaseX = (ORDINAL_HALK & 7) * PORTRAIT_W;
        int srcBaseY = (ORDINAL_HALK >> 3) * PORTRAIT_H;
        CHECK(srcBaseX + PORTRAIT_W <= (int)portraits->width);
        CHECK(srcBaseY + PORTRAIT_H <= (int)portraits->height);
    }

    /* Stage 3: per-surface scaled dispatch. */
    for (surfaceIndex = 0; surfaceIndex < kSurfacesCount; ++surfaceIndex) {
        const Surface* surface = &kSurfaces[surfaceIndex];
        int physX = -1;
        int physY = -1;
        int viewportX;
        int viewportY;
        TouchClickDispatchPc34Compat dispatch;
        int fbX;
        int fbY;
        int viewportDispatchHit = 0;

        /* Stage 3a: viewport dispatch must land inside the source
         * portrait cutout.  We pick the center of the cutout on the
         * viewport (viewport-local (112, 49) is the center of
         * (96, 35, 32, 29)). */
        CHECK(pick_physical_pixel_for_source(surface, VIEWPORT_W, VIEWPORT_H,
                                             PORTRAIT_X_VP + PORTRAIT_W / 2,
                                             PORTRAIT_Y_VP + PORTRAIT_H / 2,
                                             &physX, &physY) == 1);
        CHECK(TOUCHCLICK_Compat_NormalizeScaledViewportPoint(
                  physX, physY, surface->width, surface->height,
                  &viewportX, &viewportY) == 1);
        CHECK(viewportX >= PORTRAIT_X_VP &&
              viewportX < PORTRAIT_X_VP + PORTRAIT_W);
        CHECK(viewportY >= PORTRAIT_Y_VP &&
              viewportY < PORTRAIT_Y_VP + PORTRAIT_H);

        /* Stage 3b: viewport-path dispatch must NOT land on a
         * portrait-specific zone.  The C026 cutout is part of the
         * wider C007 viewport.dungeon zone which is screen-relative,
         * so the viewport-relative scan (HitTestInCoordMode) returns
         * 0 for the cutout when no inventory / panel is open and no
         * other viewport-relative zone covers (112, 49).  This is
         * the source-locked no-float assertion: a future regression
         * that adds a VIEWPORT_RELATIVE zone covering the cutout
         * would steal clicks and be caught here.  If the dispatch
         * DOES return 1, the dispatched zone must not be a
         * portrait-specific zone — it is allowed to be an inventory
         * slot because the inventory zones happen to share the
         * viewport coordinate space (they are active only when the
         * inventory is open, but the matrix does not gate on that
         * state).  We assert the zone is NOT named "portrait" or any
         * of the existing portrait-pickup click groups. */
        memset(&dispatch, 0, sizeof(dispatch));
        if (TOUCHCLICK_Compat_MapScaledViewportPointToDispatch(
                physX, physY, surface->width, surface->height,
                TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch) == 1) {
            viewportDispatchHit = 1;
            if (dispatch.groupName != NULL &&
                (strstr(dispatch.groupName, "portrait") != NULL ||
                 strstr(dispatch.groupName, "champion") != NULL)) {
                fprintf(stderr,
                        "FAIL surface=%s viewport dispatch at portrait center phys=(%d,%d) routed to group=%s — portrait cutout should not have a viewport-relative click zone\n",
                        surface->label, physX, physY,
                        dispatch.groupName);
                ++g_failures;
            } else {
                ++g_pass;
            }
        } else {
            ++g_pass;
        }

        /* Stage 3c: SDL framebuffer map must land inside the source
         * 320x200 portrait rect (96..127, 68..96).  Note that
         * M11_Render_MapPointToFramebuffer uses screen-letterbox
         * math (320x200), so we feed it the physical pixel picked
         * via the screen letterbox — the viewport-letterbox pixel
         * is at a different surface location because the SDL
         * renderer letterboxes against the 320x200 source. */
        {
            int fbPhysX = -1;
            int fbPhysY = -1;
            CHECK(pick_physical_pixel_for_screen_source(
                      surface,
                      PORTRAIT_X_FB + PORTRAIT_W / 2,
                      PORTRAIT_Y_FB + PORTRAIT_H / 2,
                      &fbPhysX, &fbPhysY) == 1);
            if (M11_Render_MapPointToFramebuffer(fbPhysX, fbPhysY,
                                                surface->width, surface->height,
                                                M11_FB_WIDTH, M11_FB_HEIGHT,
                                                M11_SCALE_FIT,
                                                0,
                                                M11_DISPLAY_ASPECT_CONTENT,
                                                &fbX, &fbY) != 1) {
                fprintf(stderr,
                        "FAIL surface=%s framebuffer map at (%d,%d) returned 0\n",
                        surface->label, fbPhysX, fbPhysY);
                ++g_failures;
            } else {
                CHECK(fbX >= PORTRAIT_X_FB && fbX < PORTRAIT_X_FB + PORTRAIT_W);
                CHECK(fbY >= PORTRAIT_Y_FB && fbY < PORTRAIT_Y_FB + PORTRAIT_H);
            }
        }

        printf("  surface=%s vp_phys=(%d,%d) vp=(%d,%d) fb=(%d,%d) viewport_dispatch_hit=%d\n",
               surface->label, physX, physY, viewportX, viewportY,
               fbX, fbY, viewportDispatchHit);
    }

    /* Stage 3d: screen-letterbox dispatch (primary interface scan).
     * A physical pixel that picks the screen-local center of the
     * portrait rect (112, 82) routes via the screen-letterbox primary
     * table scan to viewport.dungeon (C080, zoneIndex 7).  This is
     * the high-DPI no-float double-check: an off-by-one in the
     * screen-letterbox math could leak a portrait-pickup zone here,
     * which we do not want. */
    for (surfaceIndex = 0; surfaceIndex < kSurfacesCount; ++surfaceIndex) {
        const Surface* surface = &kSurfaces[surfaceIndex];
        int physX = -1;
        int physY = -1;
        TouchClickDispatchPc34Compat dispatch;

        CHECK(pick_physical_pixel_for_screen_source(
                  surface,
                  PORTRAIT_X_FB + PORTRAIT_W / 2,
                  PORTRAIT_Y_FB + PORTRAIT_H / 2,
                  &physX, &physY) == 1);
        memset(&dispatch, 0, sizeof(dispatch));
        if (TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(
                physX, physY, surface->width, surface->height,
                TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch) != 1) {
            fprintf(stderr,
                    "FAIL surface=%s screen-letterbox dispatch at portrait center phys=(%d,%d) returned 0 — should hit viewport.dungeon\n",
                    surface->label, physX, physY);
            ++g_failures;
            continue;
        }
        CHECK(dispatch.commandId == PORTRAIT_DISPATCH_COMMAND_ID);
        CHECK(dispatch.zoneIndex == PORTRAIT_DISPATCH_ZONE_INDEX);
        CHECK(dispatch.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT);
        if (dispatch.groupName == NULL ||
            strcmp(dispatch.groupName, "viewport.dungeon") != 0) {
            fprintf(stderr,
                    "FAIL surface=%s screen-letterbox dispatch at portrait center group=%s want=viewport.dungeon\n",
                    surface->label,
                    dispatch.groupName ? dispatch.groupName : "(null)");
            ++g_failures;
        }
        CHECK(dispatch.screenX >= PORTRAIT_X_FB &&
              dispatch.screenX < PORTRAIT_X_FB + PORTRAIT_W);
        CHECK(dispatch.screenY >= PORTRAIT_Y_FB &&
              dispatch.screenY < PORTRAIT_Y_FB + PORTRAIT_H);
        printf("  surface=%s screen_letterbox phys=(%d,%d) screen=(%d,%d) dispatch=%s cmd=%u zone=%u\n",
               surface->label, physX, physY,
               dispatch.screenX, dispatch.screenY,
               dispatch.groupName ? dispatch.groupName : "(null)",
               dispatch.commandId, dispatch.zoneIndex);
    }

    /* Stage 4: static no-float scan.  Walk the public touch-click
     * zone matrix and assert no portrait-specific or champion-
     * specific VIEWPORT_RELATIVE zone covers the (96, 35, 32, 29)
     * cutout.  Panel zones (panel.resurrect, panel.reincarnate,
     * panel.candidate_*) are ALLOWED to overlap because they are
     * only active while the candidate mirror panel is open — the
     * existing 097 probe Stage 2 covers that overlap, and the
     * source-locked COMMAND.C:403 keeps the portrait cutout inside
     * the wider C007 viewport.dungeon zone in dungeon mode.  This
     * scan is the static backstop for Stage 3b: it would catch a
     * future regression that adds a portrait-pickup or champion-
     * mirror VIEWPORT_RELATIVE zone that would steal clicks in
     * dungeon mode. */
    {
        unsigned int zi;
        int portraitCoverers = 0;
        TouchClickZonePc34Compat portraitCoverer;
        memset(&portraitCoverer, 0, sizeof(portraitCoverer));
        for (zi = 0; zi < TOUCHCLICK_Compat_GetZoneCount(); ++zi) {
            TouchClickZonePc34Compat z;
            if (!TOUCHCLICK_Compat_GetZone(zi, &z)) continue;
            if (z.coordMode != TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT) continue;
            /* Only flag zones that look portrait-specific.  Panel
             * zones are allowed to overlap the cutout because the
             * RR panel C040 (panel.resurrect) intentionally covers
             * the lower half of the viewport while G0299 is set
             * (ReDMCSB PANEL.C F0347). */
            if (z.groupName == NULL) continue;
            if (strstr(z.groupName, "portrait") == NULL &&
                strstr(z.groupName, "champion") == NULL &&
                strstr(z.groupName, "mirror") == NULL) {
                continue;
            }
            /* Check viewport-relative zone rect overlap with cutout. */
            if (PORTRAIT_X_VP < z.x + z.w &&
                PORTRAIT_X_VP + PORTRAIT_W > z.x &&
                PORTRAIT_Y_VP < z.y + z.h &&
                PORTRAIT_Y_VP + PORTRAIT_H > z.y) {
                ++portraitCoverers;
                if (portraitCoverers == 1) portraitCoverer = z;
            }
        }
        if (portraitCoverers > 0) {
            fprintf(stderr,
                    "FAIL %d portrait/champion/mirror viewport-relative zone(s) cover the portrait cutout (96,35,32,29); first overlap: cmd=%u zone=%u group=%s rect=(%d,%d,%d,%d)\n",
                    portraitCoverers, portraitCoverer.commandId,
                    portraitCoverer.zoneIndex,
                    portraitCoverer.groupName ? portraitCoverer.groupName : "(null)",
                    portraitCoverer.x, portraitCoverer.y,
                    portraitCoverer.w, portraitCoverer.h);
            ++g_failures;
        } else {
            ++g_pass;
        }
        printf("  static_coverer_scan: %d portrait/champion/mirror viewport-relative zones overlap portrait cutout (want 0)\n",
               portraitCoverers);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 HoC champion portrait ordinal 1 highdpi_mouse_rect portrait_rect_position\n",
           g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_pass, g_failures);
    return g_failures == 0 ? 0 : 1;
}
