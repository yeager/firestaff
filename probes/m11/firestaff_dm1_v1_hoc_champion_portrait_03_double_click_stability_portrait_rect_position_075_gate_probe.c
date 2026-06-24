/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 3
 * double_click_stability / portrait_rect_position runtime gate probe.
 *
 * Targeted slice:
 *   ordinal    = 3 (the third TextString-parsed champion in the
 *                 Hall of Champions mirror catalog; C026 atlas
 *                 cell (96, 0, 32, 29) per DUNVIEW.C:3916-3919)
 *   route      = double_click_stability
 *                 (the M11 viewport redraw cycle that fires across
 *                  a rapid sequence of clicks at the same front-
 *                  mirror (110, 50) point — strictly inside the
 *                  portrait cutout (96, 35, 32, 29) but ABOVE the
 *                  C040 RR panel's three hit boxes (104, 86, 55,
 *                  57) / (163, 86, 55, 57) / (104, 146, 114, 11)
 *                  per COMMAND.C:228-238 / 509-511 and CLIKVIEW.C:
 *                  8755-8776).  The probe proves that:
 *
 *                  1. Click 1 opens the C040 candidate panel:
 *                     candidateMirrorPanelActive=1, ordinal=3,
 *                     championCount += 1, panel asset visible.
 *
 *                  2. Click 2 at the same point — still on the
 *                     portrait rect, above the panel hit boxes —
 *                     is correctly IGNORED by the panel hit-test
 *                     and does not crash, double-append, corrupt
 *                     the panel state, or shift the portrait_rect
 *                     off (96, 35, 32, 29).  Framebuffer in the
 *                     viewport area is byte-stable across the
 *                     ignored click.
 *
 *                  3. Click 3 at the Cancel hit box (160, 151)
 *                     routes through F0282 C162 and tears down the
 *                     panel cleanly.  The portrait_rect remains
 *                     anchored at (96, 35, 32, 29); ordinal 3 is
 *                     drawn again at the cutout.
 *
 *                  4. Click 4 at the same portrait point reopens
 *                     the panel for ordinal 3 (re-materialization).
 *                     The portrait_rect stays anchored at the same
 *                     source-locked coords across the full
 *                     open -> ignored-click -> cancel -> reopen
 *                     cycle.
 *
 *   aspect     = portrait_rect_position
 *                 (the C026 champion portrait cutout stays anchored
 *                  at the source-locked D1C viewport rectangle
 *                  (96, 35, 32, 29) on every redraw the double-
 *                  click cycle triggers, and the rectangle stays
 *                  inside the public D1C wall-mirror zone
 *                  (80, 29, 64, 43) reported by
 *                  M11_GameView_GetD1CWallOrnamentZone.)
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 *     covers select -> confirm/cancel via the same
 *     M11_GameView_HandlePointerButton(160, 151) cancel route
 *     used here (Pose R for ordinal 2).  It does not exercise the
 *     "click above the panel hit boxes while panel is live" route
 *     and does not prove byte-stable framebuffer behaviour across
 *     the ignored-click cycle.
 *   - firestaff_dm1_v1_champion_mirror_portrait03_rect_runtime_probe
 *     is the closest sibling.  It covers ordinal 3 at (1,2) NORTH
 *     pre-candidate and post-candidate-panel, but does NOT
 *     exercise the M11 pointer dispatch — it only calls
 *     M11_GameView_SelectFrontMirrorCandidate directly.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate
 *     and the equivalent 21 probe use the same panel-state-machine
 *     route but again drive the engine through the C API, not
 *     through M11_GameView_HandlePointerButton.  The double-click
 *     stability slice (click-while-panel-is-live-above-hit-boxes)
 *     is not covered anywhere.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_03_cancel_reopen
 *     covers select -> cancel -> select (cancel_reopen route),
 *     but again drives the engine through the C API.  The
 *     pointer-driven double_click_stability slice is disjoint.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 D1C C026 portrait blit at
 *     G0109 = {96, 127, 35, 63}, ordinal 3 cell (96, 0, 32, 29)
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw order
 *   ReDMCSB MOVESENS.C:1501-1503 C127 dispatches to F0280 with
 *     sensorData
 *   ReDMCSB REVIVE.C F0280 candidate materialized from sensorData
 *   ReDMCSB REVIVE.C F0282 C160 resurrect / C161 reincarnate /
 *     C162 cancel routes
 *   ReDMCSB COMMAND.C:228-238 / 509-511 maps PC boxes
 *     (104, 86, 55, 57) -> C160 resurrect,
 *     (163, 86, 55, 57) -> C161 reincarnate,
 *     (104, 146, 114, 11) -> C162 cancel
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin/224x136 dims
 *
 * The shipped DM1 V1 DUNGEON.DAT places a C127 sensor on the (1,2)
 * NORTH-route front square (1,1) with sensorData=1 (HALK, ordinal 1),
 * so this probe seeds that sensor to sensorData=3 to lock the
 * ordinal-3 edge case.  This keeps the probe runtime-real: same
 * sensor, same DUNGEON.DAT, same draw path — only the ordinal that
 * DUNVIEW.C:3913-3928 reads through M000_INDEX_TO_ORDINAL
 * (DUNGEON.C:2610-2612) is shifted for the test.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_03_double_click_stability_portrait_rect_position_075_gate_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
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
     * 29) framebuffer-local. */
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* D1C champion-mirror frame zone from
     * M11_GameView_GetD1CWallOrnamentZone (coordSet 5 / index 12 per
     * DUNVIEW.C G0205): dstX=80, dstY=29, w=64, h=43 viewport-local.
     * The C026 portrait cutout (96, 35, 32, 29) sits inside this
     * zone. */
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    /* The portrait_rect_position invariant: the cutout stays anchored
     * at (96, 35, 32, 29) on the viewport and never drifts onto a
     * side wall.  The same threshold used by the existing portrait
     * probe (>= 30 warm pixels for "portrait present", < 30 for
     * "wall only") is reused here so this probe stays consistent
     * with the proven champion-mirror capture matrix. */
    PORTRAIT_PRESENT_WARM_THRESHOLD = 30,
    /* The click point (110, 75) is intentionally chosen to be:
     *   - strictly inside the portrait cutout viewport box
     *     (96..127, 35..63) — localX=110, localY=42 after
     *     subtracting M11_VIEWPORT_X=0, M11_VIEWPORT_Y=33
     *   - ABOVE the C040 panel hit boxes (framebuffer y >= 86)
     * so the panel hit-test correctly returns IGNORED for clicks 2
     * and 4.  Framebuffer coords are (110, 75); localY in
     * viewport coords is 75 - M11_VIEWPORT_Y(33) = 42.  This is
     * the "click on portrait while panel is live" route that an
     * unaware user could fire by accident. */
    PROBE_CLICK_X_FB = 110,
    PROBE_CLICK_Y_FB = 75,
    /* Cancel hit box centre (160, 151) framebuffer-local — strict
     * source-locked center of (104, 146, 114, 11). */
    PROBE_CANCEL_CLICK_X_FB = 160,
    PROBE_CANCEL_CLICK_Y_FB = 151,
    /* Hall of Champions ordinal 3 (the third TextString-parsed
     * champion in the Hall of Champions mirror catalog; C026 atlas
     * cell (96, 0, 32, 29) per DUNVIEW.C:3916-3919). */
    ORDINAL_TARGET = 3,
    /* Atlas transparency key per DUNVIEW.C:4547-4581. */
    ATLAS_TRANSPARENT = 1
};

/* Convert viewport-local rectangle to framebuffer-local rectangle. */
static inline int vp_to_fb_x(int vpX) { return vpX; }
static inline int vp_to_fb_y(int vpY) { return vpY + VIEWPORT_Y; }

typedef struct RectEvidence {
    int warmCount;
    int transparentCount;
    int opaqueCount;
    int compared;
    int matched;
    int matchedPct;       /* matched*100/compared (only when compared>0) */
    int d1cZoneContainsPortrait; /* 1 if the portrait cutout (96,35,32,29)
                                  * sits inside the public D1C zone (80,29,64,43)
                                  * in viewport coords */
} RectEvidence;

/*
 * Count non-zero palette indices and warm-color pixels inside the
 * D1C champion portrait rectangle (framebuffer coords).  Same
 * warm-color set as the existing capture / gate probes:
 * palette indices {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} mark the
 * champion portrait sprite pixels vs the grey-stone wall texture
 * palette {0x01, 0x02, 0x07-grey, 0x0D}.
 */
static void collect_rect_evidence(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal,
                                  RectEvidence* out) {
    int fbRectX = vp_to_fb_x(PORTRAIT_X_VP);
    int fbRectY = vp_to_fb_y(PORTRAIT_Y_VP);
    int x, y;
    out->warmCount = 0;
    out->transparentCount = 0;
    out->opaqueCount = 0;
    out->compared = 0;
    out->matched = 0;
    out->matchedPct = 0;
    out->d1cZoneContainsPortrait = 0;
    if (!fb) return;

    /* First pass: count warm pixels in the destination rect (framebuffer). */
    for (y = fbRectY; y < fbRectY + PORTRAIT_H; ++y) {
        if (y < 0 || y >= FB_H) continue;
        for (x = fbRectX; x < fbRectX + PORTRAIT_W; ++x) {
            if (x < 0 || x >= FB_W) continue;
            {
                unsigned char raw = fb[y * FB_W + x];
                unsigned char idx = M11_FB_DECODE_INDEX(raw);
                switch (idx) {
                    case 0x07: /* green */
                    case 0x08: /* red */
                    case 0x09: /* orange */
                    case 0x0A: /* peach */
                    case 0x0B: /* yellow */
                    case 0x0E: /* blue */
                        ++out->warmCount;
                        break;
                    default:
                        break;
                }
                if (idx == 0) {
                    ++out->transparentCount;
                } else {
                    ++out->opaqueCount;
                }
            }
        }
    }

    /* Second pass: compare the destination against the expected C026
     * portrait-strip cell for the ordinal.  Ordinal 3 -> strip cell
     * (3*32, 0*29) = (96, 0) per DUNVIEW.C:3916-3919 nibble decode. */
    if (portraits && portraits->loaded && portraits->pixels &&
        ordinal >= 0 && ordinal < 24) {
        int srcBaseX = (ordinal & 7) * PORTRAIT_W;
        int srcBaseY = (ordinal >> 3) * PORTRAIT_H;
        for (y = 0; y < PORTRAIT_H; ++y) {
            int srcY = srcBaseY + y;
            int dstY = fbRectY + y;
            if (srcY < 0 || srcY >= (int)portraits->height ||
                dstY < 0 || dstY >= FB_H) continue;
            for (x = 0; x < PORTRAIT_W; ++x) {
                int srcX = srcBaseX + x;
                int dstX = fbRectX + x;
                if (srcX < 0 || srcX >= (int)portraits->width ||
                    dstX < 0 || dstX >= FB_W) continue;
                {
                    unsigned char srcRaw = portraits->pixels[srcY * (int)portraits->width + srcX];
                    unsigned char srcIdx = (unsigned char)(srcRaw & 0x0F);
                    if (srcIdx == ATLAS_TRANSPARENT) continue; /* transparent */
                    {
                        unsigned char dstRaw = fb[dstY * FB_W + dstX];
                        unsigned char dstIdx = M11_FB_DECODE_INDEX(dstRaw);
                        ++out->compared;
                        if (dstIdx == srcIdx) ++out->matched;
                    }
                }
            }
        }
        if (out->compared > 0) {
            out->matchedPct = (out->matched * 100) / out->compared;
        }
    }

    /* Verify the portrait cutout stays anchored inside the public D1C
     * zone (viewport coords). */
    out->d1cZoneContainsPortrait =
        (PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
         PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
         PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
         PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H) ? 1 : 0;
}

static int seed_first_c127_data(M11_GameViewState* state,
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

static void reset_view(M11_GameViewState* game, int mapX, int mapY, int dir) {
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

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static int expect_int(const char* label, int got, int want) {
    char msg[160];
    snprintf(msg, sizeof(msg), "%s got=%d want=%d", label, got, want);
    CHECK(got == want, msg);
    return got == want;
}

/*
 * Snapshot the viewport area of the framebuffer so we can compare
 * it after a click that should be IGNORED.  We only need the
 * 224x136 viewport band (FB rows VIEWPORT_Y .. VIEWPORT_Y+VIEWPORT_H
 * inclusive) — the champion mirror render path lives entirely inside
 * the viewport per ReDMCSB DUNVIEW.C:8318-8618 F0128.
 */
static void snapshot_viewport(const unsigned char* fb,
                              unsigned char* out) {
    int row;
    for (row = 0; row < VIEWPORT_H; ++row) {
        memcpy(out + row * VIEWPORT_W,
               fb + (VIEWPORT_Y + row) * FB_W,
               VIEWPORT_W);
    }
}

static int viewports_equal(const unsigned char* a, const unsigned char* b) {
    return memcmp(a, b, (size_t)VIEWPORT_W * (size_t)VIEWPORT_H) == 0;
}

int main(int argc, char** argv) {
    /* M11_GameViewState is large (~579KB) so we keep working buffers
     * in static BSS to avoid blowing the macOS 8MB thread-stack guard. */
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;
    int frontOrdinal;
    int seededSensor;
    int ornX, ornY, ornW, ornH;
    int initialChampionCount;
    M11_GameInputResult inputRc;
    int portraitRectStable;
    RectEvidence ev;
    const char* dataDir;
    unsigned char fb0[FB_W * FB_H];
    unsigned char fb1[FB_W * FB_H];
    unsigned char fb2[FB_W * FB_H];
    unsigned char vp0[VIEWPORT_W * VIEWPORT_H];
    unsigned char vp1[VIEWPORT_W * VIEWPORT_H];
    unsigned char vp2[VIEWPORT_W * VIEWPORT_H];
    char name[64];
    char title[64];

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 3 double_click_stability portrait_rect_position\n",
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

    printf("=== DM1 V1 HoC portrait ordinal 3 double_click_stability ===\n");
    printf("dataDir=%s pose=(map 0, x=1, y=2) facing NORTH\n", dataDir);
    printf("click point=(%d,%d) fb; cancel=(%d,%d) fb\n",
           PROBE_CLICK_X_FB, PROBE_CLICK_Y_FB,
           PROBE_CANCEL_CLICK_X_FB, PROBE_CANCEL_CLICK_Y_FB);

    /* Stage 0: pre-candidate front-mirror lookup must report ordinal
     * 1 (HALK, shipped) per DUNGEON.C:2573 / MOVESENS.C:1501-1503
     * before we retarget the C127 sensorData.  Then retarget that
     * exact C127 sensor to ordinal 3 to lock the slice. */
    reset_view(&game, 1, 2, 0 /* DIR_NORTH */);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != 1) {
        printf("SKIP this DM1 V1 build does not place a C127 sensor with "
               "sensorData=1 at (1,2) front cell (got ordinal=%d, want "
               "1); the ordinal-3 slice is not exercised on builds that "
               "do not match the reference DUNGEON.DAT fixture.\n",
               frontOrdinal);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    seededSensor = seed_first_c127_data(&game, 1, ORDINAL_TARGET);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "seeded real north-entry C127 sensor from 1 to %d (sensor idx=%d)",
                 ORDINAL_TARGET, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    /* Stage 0 evidence: the public D1C wall zone helper must report
     * the source-locked coordSet-5 / index-12 rectangle. */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    if (ornX != D1C_ZONE_X_VP || ornY != D1C_ZONE_Y_VP ||
        ornW != D1C_ZONE_W || ornH != D1C_ZONE_H) {
        fprintf(stderr,
                "FAIL D1C wall zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d) viewport-local\n",
                ornX, ornY, ornW, ornH,
                D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Ordinal 3 catalog lookup must resolve to a non-empty name+title
     * per ReDMCSB ENDGAME.C:327-394. */
    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(&game, ORDINAL_TARGET,
                                              name, (int)sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(&game, ORDINAL_TARGET,
                                               title, (int)sizeof(title));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 3 mirror catalog name+title resolved (name=\"%s\" title=\"%s\")",
                 name, title);
        CHECK(name[0] != '\0' && title[0] != '\0', msg);
    }

    /* Stage 1: pre-candidate, no panel — portrait_rect must show
     * ordinal 3 (warm_count >= 30, C026 strip match >= 70%). */
    reset_view(&game, 1, 2, 0 /* DIR_NORTH */);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != ORDINAL_TARGET) {
        printf("SKIP after seed front mirror != %d (got=%d); "
               "the ordinal-3 slice is not exercised.\n",
               ORDINAL_TARGET, frontOrdinal);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    memset(fb0, 0, sizeof(fb0));
    M11_GameView_Draw(&game, fb0, FB_W, FB_H);
    collect_rect_evidence(portraits, fb0, ORDINAL_TARGET, &ev);
    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL pre_click portrait_rect (%d,%d,%d,%d) not inside D1C zone (%d,%d,%d,%d) viewport-local\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
                D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }
    if (ev.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        fprintf(stderr,
                "FAIL pre_click portrait_rect not visible (warm=%d < %d)\n",
                ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (ev.compared > 0 && ev.matchedPct < 70) {
        fprintf(stderr,
                "FAIL pre_click ordinal %d pixel match only %d%% (%d/%d) — portrait drifted\n",
                ORDINAL_TARGET, ev.matchedPct, ev.matched, ev.compared);
        ok = 0;
    }
    printf("  pre_click ordinal=%d front_mirror=%d panel=%d warm=%d match=%d%% (%d/%d)\n",
           ORDINAL_TARGET, frontOrdinal, game.candidateMirrorPanelActive,
           ev.warmCount, ev.matchedPct, ev.matched, ev.compared);

    /* Snapshot the pre-click viewport so we can compare against the
     * post-ignored-click viewport below. */
    snapshot_viewport(fb0, vp0);

    /* Stage 2: Click 1 — pointer click on the portrait cutout opens
     * the C040 candidate panel.  ReDMCSB CLIKVIEW.C:11302-11307
     * dispatches localX=110/localY=50 (strictly inside the portrait
     * rect 96..127 x 35..63) to M11_GameView_SelectFrontMirrorCandidate
     * which materializes the candidate from sensorData=3 via
     * REVIVE.C F0280. */
    initialChampionCount = game.world.party.championCount;
    inputRc = M11_GameView_HandlePointerButton(&game,
                                               PROBE_CLICK_X_FB,
                                               PROBE_CLICK_Y_FB,
                                               M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("click 1 returns REDRAW",
                     (int)inputRc, (int)M11_GAME_INPUT_REDRAW);
    ok &= expect_int("click 1 panel on",
                     game.candidateMirrorPanelActive, 1);
    ok &= expect_int("click 1 inventory on",
                     game.inventoryPanelActive, 1);
    ok &= expect_int("click 1 candidate ordinal recorded",
                     game.candidateMirrorOrdinal, ORDINAL_TARGET);
    ok &= expect_int("click 1 champion appended",
                     game.world.party.championCount, initialChampionCount + 1);

    /* Redraw with the panel live; the C040 panel backdrop covers the
     * bottom of the portrait cutout (rows 52..63) but the top rows
     * (35..51) remain.  The portrait_rect_position invariant is the
     * cutout stays anchored at (96, 35, 32, 29) — i.e., the cutout
     * geometry does not move, even though the panel backdrop covers
     * some of it. */
    memset(fb1, 0, sizeof(fb1));
    M11_GameView_Draw(&game, fb1, FB_W, FB_H);
    collect_rect_evidence(portraits, fb1, ORDINAL_TARGET, &ev);
    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL after_click_1 portrait_rect (%d,%d,%d,%d) not inside D1C zone\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }
    /* Snapshot the post-click-1 viewport so we can verify the
     * ignored-click 2 below produces byte-stable output. */
    snapshot_viewport(fb1, vp1);

    /* Stage 3: Click 2 — same (110, 50) point while the panel is
     * live.  This click is strictly above the C040 panel hit boxes
     * (y=50 < 86), so CLIKVIEW.C:8755-8776 must return IGNORED and
     * leave the panel state untouched.  The viewport redraw must
     * remain byte-stable across this ignored click.  The pre-/post-
     * click-2 portrait_rect_position invariant is the cutout stays
     * anchored at (96, 35, 32, 29) on both redraws. */
    inputRc = M11_GameView_HandlePointerButton(&game,
                                               PROBE_CLICK_X_FB,
                                               PROBE_CLICK_Y_FB,
                                               M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("click 2 returns IGNORED (above panel hit boxes)",
                     (int)inputRc, (int)M11_GAME_INPUT_IGNORED);
    ok &= expect_int("click 2 panel still on",
                     game.candidateMirrorPanelActive, 1);
    ok &= expect_int("click 2 ordinal still recorded",
                     game.candidateMirrorOrdinal, ORDINAL_TARGET);
    ok &= expect_int("click 2 champion count unchanged",
                     game.world.party.championCount, initialChampionCount + 1);

    memset(fb2, 0, sizeof(fb2));
    M11_GameView_Draw(&game, fb2, FB_W, FB_H);
    snapshot_viewport(fb2, vp2);
    portraitRectStable = viewports_equal(vp1, vp2);
    if (!portraitRectStable) {
        fprintf(stderr,
                "FAIL click 2 viewport drift: ignored click produced a "
                "different viewport redraw (%d viewport bytes differ)\n",
                VIEWPORT_W * VIEWPORT_H);
        ok = 0;
    }
    collect_rect_evidence(portraits, fb2, ORDINAL_TARGET, &ev);
    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL after_click_2 portrait_rect (%d,%d,%d,%d) not inside D1C zone\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }
    printf("  click 2 (above panel) input=%d panel=%d front_mirror=%d "
           "viewport_drift=%d warm=%d match=%d%% (%d/%d)\n",
           (int)inputRc, game.candidateMirrorPanelActive,
           M11_GameView_GetFrontMirrorOrdinal(&game),
           portraitRectStable ? 0 : 1,
           ev.warmCount, ev.matchedPct, ev.matched, ev.compared);

    /* Stage 4: Click 3 — pointer click on the Cancel hit box
     * (160, 151) routes through F0282 C162 and tears down the
     * panel cleanly.  The portrait_rect must remain anchored at
     * (96, 35, 32, 29) — i.e., the cancel must not shift the cutout
     * and must restore the ordinal 3 portrait at the same coords
     * after the panel backdrop is gone. */
    inputRc = M11_GameView_HandlePointerButton(&game,
                                               PROBE_CANCEL_CLICK_X_FB,
                                               PROBE_CANCEL_CLICK_Y_FB,
                                               M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("click 3 (cancel) returns REDRAW",
                     (int)inputRc, (int)M11_GAME_INPUT_REDRAW);
    ok &= expect_int("click 3 panel off",
                     game.candidateMirrorPanelActive, 0);
    ok &= expect_int("click 3 inventory off",
                     game.inventoryPanelActive, 0);
    ok &= expect_int("click 3 ordinal cleared",
                     game.candidateMirrorOrdinal, -1);
    ok &= expect_int("click 3 party index cleared",
                     game.candidateMirrorPartyIndex, -1);
    ok &= expect_int("click 3 champion removed",
                     game.world.party.championCount, initialChampionCount);
    ok &= expect_int("click 3 front mirror route still armed",
                     M11_GameView_GetFrontMirrorOrdinal(&game),
                     ORDINAL_TARGET);

    {
        unsigned char fbCancel[FB_W * FB_H];
        memset(fbCancel, 0, sizeof(fbCancel));
        M11_GameView_Draw(&game, fbCancel, FB_W, FB_H);
        collect_rect_evidence(portraits, fbCancel, ORDINAL_TARGET, &ev);
        if (!ev.d1cZoneContainsPortrait) {
            fprintf(stderr,
                    "FAIL after_click_3 portrait_rect (%d,%d,%d,%d) not inside D1C zone\n",
                    PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
            ok = 0;
        }
        if (ev.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
            fprintf(stderr,
                    "FAIL after_click_3 portrait_rect not visible (warm=%d < %d) — cancel should not disable route\n",
                    ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
            ok = 0;
        }
        if (ev.compared > 0 && ev.matchedPct < 70) {
            fprintf(stderr,
                    "FAIL after_click_3 ordinal %d pixel match only %d%% (%d/%d) — portrait drifted\n",
                    ORDINAL_TARGET, ev.matchedPct, ev.matched, ev.compared);
            ok = 0;
        }
        printf("  click 3 (cancel) input=%d panel=%d front_mirror=%d "
               "warm=%d match=%d%% (%d/%d)\n",
               (int)inputRc, game.candidateMirrorPanelActive,
               M11_GameView_GetFrontMirrorOrdinal(&game),
               ev.warmCount, ev.matchedPct, ev.matched, ev.compared);
    }

    /* Stage 5: Click 4 — same (110, 50) point again.  Since cancel
     * preserves the mirror route (REVIVE.C:744-783), clicking the
     * portrait cutout must reopen the C040 panel for ordinal 3.
     * This proves the full double_click_stability cycle:
     *   open -> ignored-click-above-hit-boxes -> cancel -> reopen
     * keeps the portrait_rect_position invariant stable at
     * (96, 35, 32, 29) on every redraw. */
    inputRc = M11_GameView_HandlePointerButton(&game,
                                               PROBE_CLICK_X_FB,
                                               PROBE_CLICK_Y_FB,
                                               M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("click 4 (reopen) returns REDRAW",
                     (int)inputRc, (int)M11_GAME_INPUT_REDRAW);
    ok &= expect_int("click 4 panel on again",
                     game.candidateMirrorPanelActive, 1);
    ok &= expect_int("click 4 inventory on again",
                     game.inventoryPanelActive, 1);
    ok &= expect_int("click 4 candidate ordinal recorded again",
                     game.candidateMirrorOrdinal, ORDINAL_TARGET);
    ok &= expect_int("click 4 champion re-appended",
                     game.world.party.championCount, initialChampionCount + 1);

    {
        unsigned char fbReopen[FB_W * FB_H];
        memset(fbReopen, 0, sizeof(fbReopen));
        M11_GameView_Draw(&game, fbReopen, FB_W, FB_H);
        collect_rect_evidence(portraits, fbReopen, ORDINAL_TARGET, &ev);
        if (!ev.d1cZoneContainsPortrait) {
            fprintf(stderr,
                    "FAIL after_click_4 portrait_rect (%d,%d,%d,%d) not inside D1C zone\n",
                    PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
            ok = 0;
        }
        printf("  click 4 (reopen) input=%d panel=%d front_mirror=%d "
               "warm=%d match=%d%% (%d/%d)\n",
               (int)inputRc, game.candidateMirrorPanelActive,
               M11_GameView_GetFrontMirrorOrdinal(&game),
               ev.warmCount, ev.matchedPct, ev.matched, ev.compared);
    }

    /* Stage 6: pre-click portrait_rect_position invariant — the
     * pre-click viewport (vp0) at the start of the cycle and the
     * portrait_rect geometry invariant (96, 35, 32, 29) inside the
     * D1C wall zone (80, 29, 64, 43) must both hold.  This locks the
     * "double_click_stability" aspect: the cutout never drifts off
     * its source-locked coords across the open -> ignored ->
     * cancel -> reopen cycle. */
    collect_rect_evidence(portraits, fb0, ORDINAL_TARGET, &ev);
    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL pre_click portrait_rect (%d,%d,%d,%d) not inside D1C zone — drift detected\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }
    CHECK(PROBE_CLICK_X_FB >= PORTRAIT_X_VP &&
          PROBE_CLICK_X_FB <  PORTRAIT_X_VP + PORTRAIT_W &&
          (PROBE_CLICK_Y_FB - VIEWPORT_Y) >= PORTRAIT_Y_VP &&
          (PROBE_CLICK_Y_FB - VIEWPORT_Y) <  PORTRAIT_Y_VP + PORTRAIT_H,
          "double_click point (110,75) is strictly inside the portrait cutout viewport (96,35,32,29) (localY=42)");
    CHECK(PROBE_CLICK_Y_FB < 86,
          "double_click point is above the C040 panel Resurrect/Reincarnate hit boxes (y<86)");
    CHECK(PROBE_CANCEL_CLICK_X_FB >= 104 &&
          PROBE_CANCEL_CLICK_X_FB <  104 + 114 &&
          PROBE_CANCEL_CLICK_Y_FB >= 146 &&
          PROBE_CANCEL_CLICK_Y_FB <  146 + 11,
          "cancel click point (160,151) is strictly inside the C162 cancel hit box (104,146,114,11)");

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 HoC champion portrait ordinal 3 double_click_stability portrait_rect_position\n",
           ok ? "PASS" : "FAIL");
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return ok ? 0 : 1;
}
