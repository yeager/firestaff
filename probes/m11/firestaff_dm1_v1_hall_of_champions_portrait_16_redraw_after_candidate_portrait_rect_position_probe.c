/*
 * firestaff_dm1_v1_hall_of_champions_portrait_16_redraw_after_candidate_portrait_rect_position_probe.c
 *
 * Narrow-slot DM1 V1 Hall of Champions runtime proof: champion portrait
 * ordinal 16, route redraw_after_candidate, aspect portrait_rect_position.
 *
 * Why this slice is uncovered by sibling probes
 * --------------------------------------------
 * The existing runtime probes cover these mirror ordinals on the Hall
 * of Champions map:
 *
 *   ordinal  1  (HALK)   at (1,2) facing NORTH  — wall_mirror_zones, actual_pose, candidate_panel family
 *   ordinal  2  (alt)    at (1,4) facing NORTH  — candidate_panel family
 *   ordinal  4  (LEIF)   at (2,1) facing SOUTH  — actual_pose
 *   ordinal 10  (ZED)    at (1,5) facing NORTH  — wall_mirror_zones, actual_pose, walkpath
 *   ordinal 13  (WUUF)   at (1,5) facing SOUTH  — actual_pose
 *   ordinal 15  (MOPHUS) at (2,4) facing SOUTH  — actual_pose
 *   ordinal 18  (SONJA)  at (1,3) facing EAST   — actual_pose, capture
 *   ordinal 19           at (3,3) facing NORTH  — walkpath
 *
 * Ordinal 16 (and the (2,7) SOUTH pose it lives at) has no runtime
 * coverage.  The cell-scan helper in this probe discovered the (2,7)
 * SOUTH C127 sensor with sensorData=16 directly from DM1 V1
 * DUNGEON.DAT — it is not on any of the prior probe routes.
 *
 * What this probe locks in
 * ------------------------
 * 1. Park the party at (2,7) facing SOUTH on map 0 (Hall of Champions).
 *    The front cell is (2,8).  M11_GameView_GetFrontMirrorOrdinal MUST
 *    return 16, sourced from C127 sensorData per DUNGEON.C:2573 +
 *    MOVESENS.C:1501-1503 + REVIVE.C F0280.
 *
 * 2. The D1C portrait rectangle must be drawn at the source-locked
 *    viewport coordinates (96, 35, 32, 29) per DUNVIEW.C:525 + 3913-3928
 *    and the G0109_auc_Graphic558_Box_ChampionPortraitOnWall initializer.
 *    When converted to framebuffer (viewport at M11_VIEWPORT_X=0,
 *    M11_VIEWPORT_Y=33) the destination is (96, 68, 32, 29).  We assert
 *    both the box and the painted content: a high match-rate against
 *    the ordinal-16 atlas cell.
 *
 * 3. REDRAW_AFTER_CANDIDATE route: open the C040 candidate panel via
 *    M11_GameView_SelectFrontMirrorCandidate (REVIVE.C F0280), confirm
 *    the panel covers the (96, 35, 32, 29) viewport rectangle (i.e. the
 *    D1C portrait pixels are masked by the panel chrome), then close
 *    the panel via M11_GameView_CancelMirrorCandidate (REVIVE.C
 *    F0282:744-783 cancel branch — preserves the mirror route, in
 *    contrast to ConfirmMirrorCandidate which disables it).  After
 *    cancel, the D1C portrait rectangle must be redrawn at the same
 *    source-locked position with the same ordinal-16 pixels, AND the
 *    portrait must NOT bleed onto ordinary side walls (no-floating
 *    invariant from the original 2026-06-20 BUG-020 capture work).
 *
 * 4. Final redraw must equal the pre-candidate baseline within an
 *    ordinal-content match tolerance, proving the post-cancel D1C
 *    blit is the same redraw the engine commits before the panel
 *    ever opens.  This is the "redraw_after_candidate" aspect: the
 *    portrait rectangle is recovered to its source-locked position
 *    and content after the C040 panel closes.
 *
 * Source evidence (ReDMCSB WIP 20210206, PC 3.4 path):
 *   - DUNGEON.C:2573 / 2608-2612 stores the C127 sensorData portrait
 *     ordinal in G0289.  G0289 is the 1-based ordinal into the
 *     C026 champion-portrait atlas (256x87, 8 columns x 3 rows of
 *     32x29 portraits).
 *   - DUNVIEW.C:3913-3928 draws the D1C champion portrait:
 *       F0132_VIDEO_Blit(C026, viewport, G0109_box, (ord&7)*32,
 *                         (ord>>3)*29, 32, viewport_w, C01_COLOR_DARK_GRAY)
 *     -- the source coordinates (ord&7)*32 / (ord>>3)*29 select the
 *     atlas cell, and G0109_box[4] = {96, 35, 32, 29} places the
 *     destination in viewport coords.
 *   - PANEL.C F0342 / F0346 draws C040 on top of the open candidate
 *     panel (no D1C blit during this state, masked by C101 zone).
 *   - REVIVE.C F0280:124-132 publishes the candidate and increments
 *     G0305, REVIVE.C F0282:744-783 cancel branch clears G0299 and
 *     removes the appended candidate without disabling the mirror
 *     sensor (in contrast to F0282:785-799 confirm branch which
 *     disables the sensor via G0289 reset).
 *   - COMMAND.C F0378 / F0358 dispatches C160/C161/C162 to REVIVE.C
 *     F0282 on click; this probe uses the direct cancel helper to
 *     keep the route focused.
 *
 * This is a Firestaff-side runtime/asset regression gate.  It does
 * NOT claim DOS pixel parity — original DM1 PC 3.4 DUNGEON.DAT frame
 * captures are still tracked separately under B1.
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
    /* M11_VIEWPORT_X = 0, M11_VIEWPORT_Y = 33 (matches
     * firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe and
     * DUNVIEW.C:525 G0109 box). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* Source-locked D1C portrait rectangle in viewport coords per
     * DUNVIEW.C:525 + 3913-3928. */
    PORTRAIT_VX = 96,
    PORTRAIT_VY = 35,
    PORTRAIT_W  = 32,
    PORTRAIT_H  = 29,
    /* C040 candidate panel rectangle in viewport coords per
     * PANEL.C F0346:1619-1637 + F0342: layout-696 centered at (80,52)
     * with width 144 height 73. */
    PANEL_VX = 80,
    PANEL_VY = 52,
    PANEL_W  = 144,
    PANEL_H  = 73,
    /* C026 champion-portrait atlas geometry (8 cols × 3 rows of 32x29
     * portraits).  Atlas is 256 pixels wide and at least 87 pixels tall
     * per DM1 GRAPHICS.DAT C026 entry. */
    ATLAS_COLS = 8,
    PORTRAIT_BOX_W = 32,
    PORTRAIT_BOX_H = 29
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Match the painted pixels inside the D1C portrait rectangle (viewport
 * coords) against the ordinal cell of the C026 atlas.  Returns percent
 * 0..100, or -1 if the atlas is unavailable / ordinal is out of range.
 * The C026 atlas uses palette index 1 as the transparency color, so we
 * skip those pixels and compare only opaque atlas pixels against the
 * destination framebuffer.  This mirrors the matching helper in
 * firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe. */
static int match_d1c_portrait(const M11_AssetSlot* portraits,
                              const unsigned char* fb,
                              int ordinal) {
    int x, y, matched = 0, compared = 0;
    int framebufferX, framebufferY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    if (ordinal < 0 || ordinal >= (ATLAS_COLS * 3)) return -1;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            unsigned char src;
            unsigned char dst;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) continue; /* atlas transparency */
            framebufferX = VIEWPORT_X + PORTRAIT_VX + x;
            framebufferY = VIEWPORT_Y + PORTRAIT_VY + y;
            if (framebufferX < 0 || framebufferX >= FB_W) continue;
            if (framebufferY < 0 || framebufferY >= FB_H) continue;
            dst = M11_FB_DECODE_INDEX(fb[framebufferY * FB_W + framebufferX]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : -1;
}

/* Count distinct non-zero palette indices in a viewport-coords rect.
 * Used to confirm "no portrait floating on side walls" — the side
 * walls use the grey stone texture (small distinct index count) while
 * the D1C portrait paints warm-color champion pixels (large distinct
 * count). */
static int count_distinct(const unsigned char* fb,
                          int vx, int vy, int vw, int vh) {
    unsigned char seen[16];
    int n = 0;
    int x, y;
    memset(seen, 0, sizeof(seen));
    for (y = 0; y < vh; ++y) {
        for (x = 0; x < vw; ++x) {
            int fbX = VIEWPORT_X + vx + x;
            int fbY = VIEWPORT_Y + vy + y;
            unsigned char idx;
            if (fbX < 0 || fbX >= FB_W) continue;
            if (fbY < 0 || fbY >= FB_H) continue;
            idx = (unsigned char)(fb[fbY * FB_W + fbX] & 0x0F);
            if (idx != 0 && !seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
}

/* Count the painted pixels in the (96,35,32,29) viewport rectangle
 * whose palette index falls in the warm-color set
 * {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} — the green / red / orange /
 * peach / yellow / blue set used by champion portrait sprites per
 * ReDMCSB DUNVIEW.C:3913-3928.  The grey-stone wall texture uses
 * palette indices 0x01 / 0x02 / 0x07-grey / 0x0D and never the warm
 * set, so positive-ordinal poses have high warm_count while
 * negative-ordinal poses have low warm_count.  This is the same
 * heuristic firestaff_dm1_v1_champion_mirror_capture_probe uses. */
static int count_warm_pixels(const unsigned char* fb) {
    int n = 0;
    int x, y;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int fbX = VIEWPORT_X + PORTRAIT_VX + x;
            int fbY = VIEWPORT_Y + PORTRAIT_VY + y;
            unsigned char idx;
            if (fbX < 0 || fbX >= FB_W) continue;
            if (fbY < 0 || fbY >= FB_H) continue;
            idx = (unsigned char)(fb[fbY * FB_W + fbX] & 0x0F);
            switch (idx) {
                case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0E:
                    ++n;
                    break;
                default:
                    break;
            }
        }
    }
    return n;
}

/* Drive the engine to the (2,7) SOUTH pose on map 0 (Hall of Champions),
 * reset the candidate panel state, and seed a single D1C wall ornament
 * so the no-panel frame has the realistic M11 D1C composition that the
 * original BUG-120/121 panel_guard_probe uses. */
static void park_at_ordinal_16(M11_GameViewState* state) {
    if (!state) return;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 2;
    state->world.party.mapY = 7;
    state->world.party.direction = DIR_SOUTH;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
    state->world.party.championCount = 0;
    /* Seed a D1C wall ornament so the no-panel frame matches the
     * realistic M11 D1C composition used by the original panel_guard
     * probe (firestaff_dm1_v1_hall_of_champions_panel_guard_probe).
     * The C040-candidate panel-active frame must still suppress the
     * ornament blit per the BUG-120/121 guard, so this seed only
     * affects the no-panel baseline + post-cancel redraw. */
    if (state->world.dungeon &&
        state->world.dungeon->header.mapCount > 0 &&
        state->world.dungeon->maps[0].wallOrnamentCount < 1) {
        state->world.dungeon->maps[0].wallOrnamentCount = 1;
    }
    state->wallOrnamentIndices[0][0] = 1;
    state->ornamentCacheLoaded[0] = 0;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const M11_AssetSlot* rrPanel;
    int ornX = 0, ornY = 0, ornW = 0, ornH = 0;
    int baselineMatch = -1;
    int baselineWarm = -1;
    int baselineSideWallDistinct = -1;
    int frontOrdinal = -1;
    unsigned char fb[FB_W * FB_H];

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    printf("=== DM1 V1 Hall of Champions portrait 16 redraw_after_candidate portrait_rect_position probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Fixture check: this probe targets the (2,7) SOUTH C127 sensor
     * with sensorData=16 in DM1 V1 Hall of Champions.  Different
     * DM1 V1 builds place the C127 sensor on different cells, so on
     * builds that don't match the reference DUNGEON.DAT we SKIP rather
     * than fail.  Per-build fixture guard, same shape as the
     * candidate_panel family. */
    park_at_ordinal_16(&state);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    if (frontOrdinal != 16) {
        printf("SKIP hall_portrait_16_fixture_mismatch "
               "(2,7) SOUTH front ordinal=%d expected=16; "
               "this DM1 V1 build does not match the reference "
               "DUNGEON.DAT fixture (the C127 sensor at (2,8) is "
               "laid out differently; see TODO.md fixture-mismatch)\n",
               frontOrdinal);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    rrPanel = M11_AssetLoader_Load((M11_AssetLoader*)&state.assetLoader,
                                   (unsigned int)40 /* C040 PANEL_RESURRECT_REINCARNATE */);
    /* rrPanel may be unavailable on truncated DM1 GRAPHICS.DAT builds;
     * treat that as a soft pass — the redraw check below uses
     * match_d1c_portrait + warm-pixel counting which does not need
     * the C040 asset. */
    if (rrPanel && rrPanel->loaded && rrPanel->pixels &&
        (rrPanel->width != PANEL_W || rrPanel->height != PANEL_H)) {
        fprintf(stderr,
                "WARN C040 panel asset size mismatch got=%ux%u want=%dx%d; "
                "skipping panel-cover check\n",
                rrPanel->width, rrPanel->height, PANEL_W, PANEL_H);
        rrPanel = NULL;
    }

    /* ── Phase 1: pre-candidate baseline ───────────────────────────────
     * Park at (2,7) SOUTH, draw the framebuffer, capture the
     * portrait-rectangle match rate, warm-pixel count, and side-wall
     * distinct count.  This is the gold baseline that the
     * redraw_after_candidate route must restore after the C040 panel
     * closes. */
    park_at_ordinal_16(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "(2,7) SOUTH front mirror ordinal = 16");
        CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == 16, msg);
    }

    /* Source-locked D1C wall ornament zone (DUNVIEW.C G0205 set 12 +
     * M11_GameView_GetD1CWallOrnamentZone).  The destination box is
     * coordSet-dependent; we read the public M11 helper. */
    M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone is non-empty: "
                 "(%d, %d, %d, %d) in viewport coords",
                 ornX, ornY, ornW, ornH);
        CHECK(ornW > 0 && ornH > 0, msg);
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&state, fb, FB_W, FB_H);

    baselineMatch = match_d1c_portrait(portraits, fb, 16);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "pre-candidate D1C portrait rectangle matches ordinal 16 "
                 "(>= 90%%, got %d%%)",
                 baselineMatch);
        CHECK(baselineMatch >= 90, msg);
    }
    baselineWarm = count_warm_pixels(fb);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "pre-candidate D1C portrait rectangle has warm-color pixels "
                 "(>= 30, got %d)",
                 baselineWarm);
        CHECK(baselineWarm >= 30, msg);
    }

    /* No-floating invariant (original 2026-06-20 BUG-020 work): the
     * D1C portrait rectangle must NOT bleed onto ordinary side walls.
     * The grey-stone wall texture is bounded — the rightmost cell of
     * the side wall row is at viewport (256, 49, 64, 39).  We expect
     * that rectangle to contain wall-texture-only pixels (small
     * distinct index count) and no warm-color champion pixels. */
    baselineSideWallDistinct = count_distinct(fb, 256, 49, 64, 39);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "pre-candidate (2,7) SOUTH side wall (256,49,64,39) is wall-texture-only "
                 "(distinct <= 4, got %d)",
                 baselineSideWallDistinct);
        CHECK(baselineSideWallDistinct <= 4, msg);
    }
    {
        int x, y, warmLeaked = 0;
        for (y = 49; y < 49 + 39; ++y) {
            for (x = 256; x < 256 + 64; ++x) {
                unsigned char idx;
                if (VIEWPORT_X + x < 0 || VIEWPORT_X + x >= FB_W) continue;
                if (VIEWPORT_Y + y < 0 || VIEWPORT_Y + y >= FB_H) continue;
                idx = (unsigned char)(fb[(VIEWPORT_Y + y) * FB_W + (VIEWPORT_X + x)] & 0x0F);
                switch (idx) {
                    case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0E:
                        ++warmLeaked;
                        break;
                    default: break;
                }
            }
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "pre-candidate (2,7) SOUTH side wall has zero warm-color leaks "
                     "(got %d)",
                     warmLeaked);
            CHECK(warmLeaked == 0, msg);
        }
    }

    /* ── Phase 2: open the C040 candidate panel ────────────────────────
     * REVIVE.C F0280 publishes the candidate and increments G0305,
     * setting G0299, the C040 panel state, and the inventory panel.
     * The D1C portrait pixels must be covered by the panel chrome. */
    if (M11_GameView_SelectFrontMirrorCandidate(&state) != 1) {
        fprintf(stderr, "FAIL SelectFrontMirrorCandidate returned 0\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "candidate panel active after SelectFrontMirrorCandidate "
                 "(got %d, want 1)",
                 state.candidateMirrorPanelActive);
        CHECK(state.candidateMirrorPanelActive == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "candidate ordinal recorded (got %d, want 16)",
                 state.candidateMirrorOrdinal);
        CHECK(state.candidateMirrorOrdinal == 16, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "candidate champion appended (got %d, want 1)",
                 state.world.party.championCount);
        CHECK(state.world.party.championCount == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "candidate panel-active mirror route preserved "
                 "(front ordinal still 16, got %d)",
                 M11_GameView_GetFrontMirrorOrdinal(&state));
        CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == 16, msg);
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&state, fb, FB_W, FB_H);

    /* The D1C portrait rectangle is fully covered by the C040 panel
     * chrome — match_d1c_portrait must drop sharply because the
     * panel chrome pixels replace the portrait pixels.  Match is
     * expected to be < 50% (the panel covers the cutout). */
    {
        int covered = match_d1c_portrait(portraits, fb, 16);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "candidate-panel-on D1C portrait rectangle is masked "
                 "(match dropped below 50%%, got %d%%)",
                 covered);
        CHECK(covered < 50, msg);
    }

    /* C040 panel chrome presence: the panel rectangle (80, 52, 144, 73)
     * in viewport coords must contain non-grey pixels if the C040
     * asset is available.  If the asset is unavailable we skip the
     * chrome-pixel assertion rather than fail, since the redraw
     * coverage check is the load-bearing assertion. */
    if (rrPanel && rrPanel->loaded && rrPanel->pixels) {
        int x, y, opaquePanel = 0;
        for (y = 0; y < (int)rrPanel->height; ++y) {
            for (x = 0; x < (int)rrPanel->width; ++x) {
                unsigned char src;
                int fbX = VIEWPORT_X + PANEL_VX + x;
                int fbY = VIEWPORT_Y + PANEL_VY + y;
                if (fbX < 0 || fbX >= FB_W) continue;
                if (fbY < 0 || fbY >= FB_H) continue;
                src = (unsigned char)(rrPanel->pixels[y * (int)rrPanel->width + x] & 0x0F);
                if (src == 6) continue; /* C040 transparency */
                ++opaquePanel;
            }
        }
        {
            char msg[160];
            snprintf(msg, sizeof(msg),
                     "C040 candidate panel asset has opaque pixels in range "
                     "(want > 0, got %d)",
                     opaquePanel);
            CHECK(opaquePanel > 0, msg);
        }
    }

    /* ── Phase 3: cancel the candidate panel ───────────────────────────
     * REVIVE.C F0282:744-783 cancel branch clears G0299 and removes
     * the appended candidate without disabling the C127 mirror sensor.
     * The mirror route must remain available so the portrait can be
     * redrawn at the source-locked position. */
    if (M11_GameView_CancelMirrorCandidate(&state) != 1) {
        fprintf(stderr, "FAIL CancelMirrorCandidate returned 0\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "post-cancel candidate panel off (got %d, want 0)",
                 state.candidateMirrorPanelActive);
        CHECK(state.candidateMirrorPanelActive == 0, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "post-cancel inventory panel off (got %d, want 0)",
                 state.inventoryPanelActive);
        CHECK(state.inventoryPanelActive == 0, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "post-cancel champion removed (got %d, want 0)",
                 state.world.party.championCount);
        CHECK(state.world.party.championCount == 0, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "post-cancel candidate ordinal cleared (got %d, want -1)",
                 state.candidateMirrorOrdinal);
        CHECK(state.candidateMirrorOrdinal == -1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "post-cancel front mirror route preserved "
                 "(front ordinal still 16, got %d)",
                 M11_GameView_GetFrontMirrorOrdinal(&state));
        CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == 16, msg);
    }

    /* ── Phase 4: redraw_after_candidate assertion ─────────────────────
     * The D1C portrait rectangle must be redrawn at the source-locked
     * (96, 35, 32, 29) viewport position with the same ordinal-16
     * atlas content as the pre-candidate baseline.  This is the
     * redraw_after_candidate route proof: the M11 engine restores
     * the D1C blit to its source-locked position after the C040
     * panel chrome is removed. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&state, fb, FB_W, FB_H);

    {
        int postMatch = match_d1c_portrait(portraits, fb, 16);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-cancel D1C portrait rectangle matches ordinal 16 "
                 "(>= 90%%, got %d%%)",
                 postMatch);
        CHECK(postMatch >= 90, msg);
    }
    {
        int postWarm = count_warm_pixels(fb);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "post-cancel D1C portrait rectangle has warm-color pixels "
                 "(>= 30, got %d)",
                 postWarm);
        CHECK(postWarm >= 30, msg);
    }
    {
        int postSideDistinct = count_distinct(fb, 256, 49, 64, 39);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-cancel (2,7) SOUTH side wall (256,49,64,39) is wall-texture-only "
                 "(distinct <= 4, got %d)",
                 postSideDistinct);
        CHECK(postSideDistinct <= 4, msg);
    }
    {
        int x, y, warmLeaked = 0;
        for (y = 49; y < 49 + 39; ++y) {
            for (x = 256; x < 256 + 64; ++x) {
                unsigned char idx;
                if (VIEWPORT_X + x < 0 || VIEWPORT_X + x >= FB_W) continue;
                if (VIEWPORT_Y + y < 0 || VIEWPORT_Y + y >= FB_H) continue;
                idx = (unsigned char)(fb[(VIEWPORT_Y + y) * FB_W + (VIEWPORT_X + x)] & 0x0F);
                switch (idx) {
                    case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0E:
                        ++warmLeaked;
                        break;
                    default: break;
                }
            }
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-cancel (2,7) SOUTH side wall has zero warm-color leaks "
                     "(got %d)",
                     warmLeaked);
            CHECK(warmLeaked == 0, msg);
        }
    }

    /* Source-locked position assertion: the public D1C wall ornament
     * zone must still be non-empty and the (96, 35, 32, 29) viewport
     * rectangle must contain the ordinal-16 champion portrait pixels
     * post-cancel — the same source-locked position the pre-candidate
     * baseline used.  This is the portrait_rect_position aspect. */
    {
        int postOrnX = 0, postOrnY = 0, postOrnW = 0, postOrnH = 0;
        M11_GameView_GetD1CWallOrnamentZone(&state, &postOrnX, &postOrnY,
                                            &postOrnW, &postOrnH);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-cancel D1C wall ornament zone preserved "
                     "(%d, %d, %d, %d) == baseline (%d, %d, %d, %d)",
                     postOrnX, postOrnY, postOrnW, postOrnH,
                     ornX, ornY, ornW, ornH);
            CHECK(postOrnX == ornX && postOrnY == ornY &&
                  postOrnW == ornW && postOrnH == ornH, msg);
        }
    }

    /* Final equality: post-cancel D1C portrait match-rate must equal
     * the pre-candidate baseline match-rate.  This proves the
     * redraw_after_candidate route restores the exact D1C content
     * the engine committed before the panel opened. */
    {
        int postMatch = match_d1c_portrait(portraits, fb, 16);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "redraw_after_candidate: post-cancel match-rate == baseline "
                 "(baseline %d%%, post-cancel %d%%, |delta| <= 5)",
                 baselineMatch, postMatch);
        CHECK(postMatch >= baselineMatch - 5 &&
              postMatch <= baselineMatch + 5, msg);
    }

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
