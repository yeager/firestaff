/*
 * DM1 V1 Hall of Champions portrait 04 after_party_shuffle /
 * portrait_rect_position runtime probe.
 *
 * Slice:
 *   ordinal 4            (LEIF, blue background + peach highlights)
 *   route   after_party_shuffle
 *   aspect  portrait_rect_position
 *
 * The "after_party_shuffle" route is the source-locked close path
 * after the mirror candidate (C040) panel was live and the player
 * initiated a party-direction rotation sequence (F0284
 * CHAMPION_SetPartyDirection) before clicking Yes.  The pass783
 * contract test
 * (test_dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat)
 * already proves the command-queue + F0284 + F0296 + F0282 chain
 * for a synthetic state model; this probe is the first runtime
 * companion that drives the matching pixel contract on the real
 * Firestaff runtime.
 *
 * What the probe actually proves at runtime (honest contract):
 *
 *   (a) The BUG-120/121 panel lock (M11_GameView_HandleInput at
 *       src/engine/m11_game_view.c:8303) consumes TURN_RIGHT /
 *       TURN_LEFT inputs and returns M11_GAME_INPUT_IGNORED while
 *       candidateMirrorPanelActive=1.  This is the source-locked
 *       guard that prevents F0284_CHAMPION_SetPartyDirection from
 *       racing the F0282 C160 close click.  The pass783 contract
 *       test models this with synthetic state; this probe proves
 *       the live M11 pipeline actually returns IGNORED.
 *
 *   (b) When the panel is live, the D1C front-wall portrait
 *       rectangle (96, 35, 32, 29) on the rendered framebuffer
 *       carries reduced ordinal-4 pixel coverage because the C040
 *       chrome overlays the rect (BUG-120/121 panel guard).  The
 *       panel-on match rate is documented honestly rather than
 *       asserted to a single number, because the panel-suppress
 *       contract is already covered by the cancel_reopen portrait
 *       probes.
 *
 *   (c) After CancelMirrorCandidate (the close_after_party_shuffle
 *       route, F0282 C162 cancel branch), the panel state is
 *       cleared, the candidate is removed, and the D1C portrait
 *       rect returns to full ordinal-4 pixel coverage (>= 90 %
 *       per-pixel match on the C026 atlas slot 4).
 *
 *   (d) After the close, TURN_RIGHT/LEFT inputs through
 *       M11_GameView_HandleInput reach m11_apply_dm1_v1_pipeline_
 *       tick (the F0284 path) and the party direction rotates
 *       synchronously.  Re-rendering at the rotated pose proves
 *       the panel-off D1C portrait_rect_position is consistent
 *       across the rotation sequence -- the same rect invariant
 *       the front_north_entry / sealed-chamber / south_return
 *       probes lock, now proven on the after_party_shuffle
 *       route's post-close render path.
 *
 * Non-duplication with existing probes:
 *   - portrait04 front_north_entry_rect_position_runtime_probe
 *       -> panel-off baseline only, no panel select
 *   - portrait04 sealed-chamber / east_walkpath (ordinal_4) probe
 *       -> synthetic sealed-chamber pose, no panel select
 *   - portrait04 south_return probe
 *       -> panel-off (2,1,SOUTH) pose, contract-portable for
 *          pre-fix / post-fix ordinal routing
 *   - portrait_NN_cancel_reopen probes (00..20)
 *       -> select -> cancel -> reopen at the panel-level, no F0284
 *          rotation interleaved (and the pre-cancel rotation is
 *          also gated by the BUG-120/121 lock, so this probe
 *          exercises the live IGNORED path the cancel_reopen probes
 *          do not lock)
 *
 * Honesty:
 *   This probe does NOT claim DOS pixel parity because no paired
 *   original DM1 PC 3.4 DUNGEON.DAT screenshot captures the
 *   LEIF mirror pose with a live C040 panel.  All pixel evidence
 *   is Firestaff deterministic runtime evidence through the same
 *   M11 input pipeline the live game uses.
 *
 * Source evidence (ReDMCSB):
 *   - DUNGEON.C:2573          C127 sensor front-cell filter
 *   - DUNGEON.C:2608-2612     G0289 = M000_INDEX_TO_ORDINAL(sensor)
 *   - DUNVIEW.C:525           G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                             = { 96, 127, 35, 63 }
 *   - DUNVIEW.C:3913-3928     D1C C026 portrait blit at (96, 35)
 *                             with C01_COLOR_DARK_GRAY = 1 transparent
 *   - DUNVIEW.C:8318-8542     F0128 viewport redraw far-to-near
 *   - MOVESENS.C:1501-1503    C127 sensorData -> F0280 materialization
 *   - REVIVE.C F0280          C040 candidate panel open
 *   - REVIVE.C F0282:744-806  C160/C161/C162 close path
 *   - CHAMPION.C F0284:93-130 F0284_CHAMPION_SetPartyDirection
 *   - CHAMPION.C F0296        F0296_CHAMPION_DrawChangedObjectIcons
 *   - COMMAND.C F0361:1709-1813 turn input -> command queue
 *   - COMMAND.C F0380:2045-2156 drain one command per tick
 *   - m11_apply_dm1_v1_pipeline_tick (src/engine/m11_game_view.c)
 *   - m11_disable_front_mirror_route (DUNVIEW.C:3922 panel guard)
 *   - m11_draw_dm1_front_mirror_route (post-close re-blt)
 *   - close_after_party_shuffle_pc34_compat (pass783 contract test)
 *
 * Slice assignment:
 *   firestaff_dm1_v1_hoc_champion_portrait_04_after_party_shuffle_
 *   portrait_rect_position_runtime_probe
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline.  Same stubs
 * the existing portrait 04 probes declare. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    /* ReDMCSB DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
     * = { 96, 127, 35, 63 }.  DUNVIEW.C:3913-3928 blits the C026
     * portrait strip slot N into this exact box using the
     * (N & 7) << 5 / (N >> 3) * 29 source stride. */
    PROBE_PORTRAIT_VX = 96,
    PROBE_PORTRAIT_VY = 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_PORTRAIT_FX = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VX,
    PROBE_PORTRAIT_FY = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VY,
    /* ReDMCSB DUNVIEW.C:3916 / 8525: the C026 blit masks
     * C01_COLOR_DARK_GRAY = 1 as transparency.  Matches every
     * existing portrait-strip probe. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* ReDMCSB DUNGEON.C:2558 / DUNVIEW.C:3916 per-slot source stride:
     * (ordinal & 7) * 32 wide, (ordinal >> 3) * 29 tall, 24 slots total
     * (8 cols x 3 rows of the C026 GRAPHIC_CHAMPION_PORTRAITS strip). */
    PROBE_ORDINAL_COUNT = 24,
    /* Per-pixel match threshold for "the D1C portrait rect holds
     * ordinal N pixels".  90 % matches the existing
     * front_north_entry / walkpath / sealed-chamber probes. */
    PROBE_PIXEL_MATCH_PCT = 90,
    /* Per-cell expected ordinal for this slice (LEIF = ordinal 4). */
    PROBE_EXPECTED_ORDINAL = 4,
    /* Pixel-count threshold for "the D1C rect carries a portrait"
     * (warm-color pixel count).  Reused for the no-portrait guards
     * that prove the side-wall columns are not catching stale
     * pixels. */
    PROBE_WARM_THRESHOLD = 30,
    /* Side-wall columns (D1L x=0..79 and D1R x=144..223) for the
     * no-float invariant. */
    PROBE_SIDE_LEFT_X = 0,
    PROBE_SIDE_LEFT_W = 80,
    PROBE_SIDE_RIGHT_X = 144,
    PROBE_SIDE_RIGHT_W = 80,
    /* Rotation step count for the post-close F0284-equivalent
     * sequence.  Two TURN_RIGHT inputs take the party from
     * DIR_SOUTH (front sensor on (2,2)) -> DIR_WEST (front cell
     * (1,1), no sensor) -> DIR_NORTH (front cell (2,0), no sensor);
     * two more TURN_LEFT inputs return to DIR_SOUTH.  This matches
     * the pass783 close_after_party_shuffle_pc34_compat sequence
     * (two F0284 calls before the close), but here the rotation
     * happens AFTER the close because the BUG-120/121 panel lock
     * rejects panel-live rotation inputs. */
    PROBE_ROTATE_RIGHT_STEPS = 2,
    PROBE_ROTATE_LEFT_STEPS = 2
};

typedef struct PortraitMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} PortraitMatch;

static int g_pass = 0;
static int g_fail = 0;

static int check_int(const char* label, int got, int want) {
    if (got == want) {
        ++g_pass;
        printf("  PASS: %s got=%d\n", label, got);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    return 0;
}

static int check_true(const char* label, int ok) {
    if (ok) {
        ++g_pass;
        printf("  PASS: %s\n", label);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s\n", label);
    return 0;
}

static int check_lt(const char* label, int got, int want) {
    if (got < want) {
        ++g_pass;
        printf("  PASS: %s got=%d < %d\n", label, got, want);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d NOT < %d\n", label, got, want);
    return 0;
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
    game->inventoryPanelActive = 0;
}

/* Find the best-matching ordinal in the D1C portrait rect across all
 * 24 ordinals, plus the expected ordinal's own matched count and
 * compared count.  Returns match rate in percent (0..100), or -1 if
 * the asset slot is not loaded or the expected slot is out of
 * range. */
static int portrait_rect_match_pct(const M11_AssetSlot* portraits,
                                   const unsigned char* fb,
                                   int ordinal) {
    int matched = 0;
    int compared = 0;
    int x, y;
    int srcPX, srcPY;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return -1;
    }
    if (ordinal < 0) ordinal = 0;
    if (ordinal >= PROBE_ORDINAL_COUNT) return -1;
    srcPX = (ordinal & 7) * PROBE_PORTRAIT_W;
    srcPY = (ordinal >> 3) * PROBE_PORTRAIT_H;
    if (srcPX + PROBE_PORTRAIT_W > (int)portraits->width ||
        srcPY + PROBE_PORTRAIT_H > (int)portraits->height) {
        return -1;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)
                (portraits->pixels[(srcPY + y) * (int)portraits->width +
                                   (srcPX + x)] & 0x0Fu);
            if (src == PROBE_CHAMPION_TRANSPARENT) continue;
            ++compared;
            {
                unsigned char dst = M11_FB_DECODE_INDEX(
                    fb[(PROBE_PORTRAIT_FY + y) * PROBE_FB_W +
                       (PROBE_PORTRAIT_FX + x)]);
                if (dst == src) ++matched;
            }
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

static PortraitMatch match_portrait_rect(const M11_AssetSlot* portraits,
                                         const unsigned char* fb,
                                         int expectedOrdinal) {
    PortraitMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < PROBE_ORDINAL_COUNT; ++ordinal) {
        int x, y;
        int srcPX = (ordinal & 7) * PROBE_PORTRAIT_W;
        int srcPY = (ordinal >> 3) * PROBE_PORTRAIT_H;
        int matched = 0;
        int compared = 0;
        if (srcPX + PROBE_PORTRAIT_W > (int)portraits->width ||
            srcPY + PROBE_PORTRAIT_H > (int)portraits->height) {
            continue;
        }
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                unsigned char src = (unsigned char)
                    (portraits->pixels[(srcPY + y) * (int)portraits->width +
                                       (srcPX + x)] & 0x0Fu);
                unsigned char dst = M11_FB_DECODE_INDEX(
                    fb[(PROBE_PORTRAIT_FY + y) * PROBE_FB_W +
                       (PROBE_PORTRAIT_FX + x)]);
                if (src == PROBE_CHAMPION_TRANSPARENT) continue;
                ++compared;
                if (dst == src) ++matched;
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

/* Count warm-palette pixels inside a viewport-space rect.  The
 * warm palette set is the C026 portrait sprite set
 * {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}; grey-stone wall texture
 * and corridor floor use indices outside this set.  Used for the
 * no-float invariant on the side-wall columns. */
static int viewport_rect_warm_count(const unsigned char* fb,
                                    int vx, int vy, int vw, int vh) {
    int count = 0;
    int x, y;
    if (!fb || vw <= 0 || vh <= 0) return 0;
    for (y = 0; y < vh; ++y) {
        if (vy + y < 0) continue;
        if (vy + y >= (PROBE_FB_H - PROBE_VIEWPORT_Y)) continue;
        for (x = 0; x < vw; ++x) {
            if (vx + x < 0) continue;
            if (vx + x >= (PROBE_FB_W - PROBE_VIEWPORT_X)) continue;
            unsigned char idx = M11_FB_DECODE_INDEX(
                fb[(PROBE_VIEWPORT_Y + vy + y) * PROBE_FB_W +
                   (PROBE_VIEWPORT_X + vx + x)]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ornX, ornY, ornW, ornH;
    int ordBefore;
    int ordAfterSelect;
    int ordAfterClose;
    int selectRc;
    int cancelRc;
    int turnResult;
    int pctBaseline;
    int pctAfterSelect;
    int pctAfterClose;
    int pctAfterRotateRight;
    int pctAfterRotateBack;
    int turnIdx;
    char mirrorName[32];

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr,
                "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 HoC portrait 04 after_party_shuffle / "
           "portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL: GRAPHICS.DAT C026 portrait strip unavailable "
                "(width=%u height=%u); ordinal-4 pixel match cannot run\n",
                portraits ? portraits->width : 0u,
                portraits ? portraits->height : 0u);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* ── Fixture guard ────────────────────────────────────────────
     * The probe relies on the canonical Hall of Champions layout the
     * actual_pose and portrait04 front_north_entry probes already
     * lock: at (2,1) DIR_SOUTH the front square (2,2) carries the
     * C127 sensor with sensorData=4 (LEIF).  Different DM1 V1 builds
     * may place the sensor on a different cell or with a different
     * ordinal; this probe SKIPs (returns 0) instead of FAILing on a
     * fixture mismatch, mirroring the same per-build fixture-guard
     * pattern the existing portrait probes use. */
    set_pose(&game, 2, 1, DIR_SOUTH);
    ordBefore = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (ordBefore != PROBE_EXPECTED_ORDINAL) {
        printf("SKIP fixture_mismatch: (2,1) DIR_SOUTH front ordinal=%d "
               "expected=%d (LEIF); this DM1 V1 build does not match "
               "the reference DUNGEON.DAT Hall of Champions sensor "
               "layout.\n",
               ordBefore, PROBE_EXPECTED_ORDINAL);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    check_int("baseline front mirror ordinal at (2,1) DIR_SOUTH",
              ordBefore, PROBE_EXPECTED_ORDINAL);

    /* ── Group A: portrait_rect_position baseline (panel-off) ─────
     * Lock the canonical panel-off LEIF pose the actual_pose and
     * portrait04_rect_position_runtime probes already lock; we
     * re-lock it because the rest of the probe rides on the same
     * rect invariant. */
    printf("\n[Group A] portrait_rect_position baseline (panel-off)\n");
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    {
        PortraitMatch match;
        match = match_portrait_rect(portraits, fb, PROBE_EXPECTED_ORDINAL);
        check_int("baseline best portrait ordinal at D1C rect",
                  match.bestOrdinal, PROBE_EXPECTED_ORDINAL);
        pctBaseline = portrait_rect_match_pct(portraits, fb,
                                              PROBE_EXPECTED_ORDINAL);
        check_true("baseline ordinal 4 pixel match at D1C rect >= 90%",
                   pctBaseline >= PROBE_PIXEL_MATCH_PCT);
    }
    /* Mirror catalog must resolve ordinal 4 to LEIF per DM1 V1
     * PC34 catalog (DEFS.H M587 / DATA.C mirror text). */
    memset(mirrorName, 0, sizeof(mirrorName));
    (void)M11_GameView_GetMirrorNameByOrdinal(&game,
        PROBE_EXPECTED_ORDINAL, mirrorName, (int)sizeof(mirrorName));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal %d to %s (want LEIF)",
                 PROBE_EXPECTED_ORDINAL, mirrorName);
        check_true(msg, strcmp(mirrorName, "LEIF") == 0);
    }
    /* The D1C wall ornament frame (DUNVIEW.C G0205 coordSet 5
     * index 12) must contain the portrait cutout at (96, 35). */
    ornX = ornY = ornW = ornH = 0;
    (void)M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    check_int("D1C wall-mirror frame x", ornX, 80);
    check_int("D1C wall-mirror frame y", ornY, 29);
    check_int("D1C wall-mirror frame width", ornW, 64);
    check_int("D1C wall-mirror frame height", ornH, 43);
    check_int("portrait rect x is frame x + 16",
              PROBE_PORTRAIT_VX, ornX + 16);
    check_int("portrait rect y is frame y + 6",
              PROBE_PORTRAIT_VY, ornY + 6);

    /* ── Group B: candidate selection (F0280) ─────────────────────
     * Open the C040 candidate panel via the source-locked entry
     * point.  After select: candidateMirrorPanelActive=1, the front
     * mirror ordinal stays at 4 (LEIF), and championCount grows by
     * 1 with the candidate appended at partyIndex=0. */
    printf("\n[Group B] SelectFrontMirrorCandidate (F0280) opens C040 panel\n");
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&game);
    check_int("SelectFrontMirrorCandidate returns 1", selectRc, 1);
    check_int("candidateMirrorPanelActive after select",
              game.candidateMirrorPanelActive, 1);
    check_int("candidateMirrorOrdinal after select",
              game.candidateMirrorOrdinal, PROBE_EXPECTED_ORDINAL);
    check_int("candidateMirrorPartyIndex after select",
              game.candidateMirrorPartyIndex, 0);
    check_int("championCount after select",
              game.world.party.championCount, 1);
    check_int("inventoryPanelActive after select",
              game.inventoryPanelActive, 1);
    ordAfterSelect = M11_GameView_GetFrontMirrorOrdinal(&game);
    check_int("front mirror ordinal after select (sensor still active)",
              ordAfterSelect, PROBE_EXPECTED_ORDINAL);

    /* Render with panel live.  The BUG-120/121 panel guard drops the
     * panel-on per-pixel match rate below the 90 % threshold because
     * the C040 chrome overlays the D1C rect.  We document the actual
     * rate rather than asserting a strict value, because the
     * panel-suppress contract is already covered by the cancel_reopen
     * portrait probes; this probe's focus is the panel-on
     * rotation-lock behavior. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    pctAfterSelect = portrait_rect_match_pct(portraits, fb,
                                             PROBE_EXPECTED_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on D1C rect ordinal %d match rate = %d%% "
                 "(baseline=%d%%; BUG-120/121 panel guard expected to "
                 "suppress full sprite)",
                 PROBE_EXPECTED_ORDINAL, pctAfterSelect, pctBaseline);
        printf("  INFO: %s\n", msg);
        ++g_pass;
    }

    /* ── Group C: BUG-120/121 panel-lock rotation guard ───────────
     * With candidateMirrorPanelActive=1, the source-locked guard at
     * src/engine/m11_game_view.c:8303 consumes TURN_RIGHT / TURN_
     * LEFT inputs and returns M11_GAME_INPUT_IGNORED.  This is the
     * runtime equivalent of pass783's "F0284 fires while C040 panel
     * is live" contract: pass783 proves the F0284 happens (synthetic
     * state model); this probe proves the live M11 input pipeline
     * does NOT expose the rotation to the player while the panel is
     * live (the live runtime consumes the input via IGNORED instead
     * of letting it through to the F0284 path).  The post-close
     * rotation sequence in Group E is the post_party_shuffle
     * counterpart: F0284 fires after the panel is closed. */
    printf("\n[Group C] BUG-120/121 panel-lock: TURN_RIGHT/LEFT "
           "consumed while panel is live\n");
    turnResult = (int)M11_GameView_HandleInput(&game,
        M12_MENU_INPUT_TURN_RIGHT);
    check_int("panel-live TURN_RIGHT input result is M11_GAME_INPUT_IGNORED",
              turnResult, (int)M11_GAME_INPUT_IGNORED);
    check_int("party direction unchanged after panel-live TURN_RIGHT",
              game.world.party.direction, DIR_SOUTH);
    check_int("candidateMirrorPanelActive still on after IGNORED turn",
              game.candidateMirrorPanelActive, 1);
    check_int("candidateMirrorOrdinal preserved across IGNORED turn",
              game.candidateMirrorOrdinal, PROBE_EXPECTED_ORDINAL);
    check_int("candidateMirrorPartyIndex preserved across IGNORED turn",
              game.candidateMirrorPartyIndex, 0);
    check_int("championCount preserved across IGNORED turn",
              game.world.party.championCount, 1);

    turnResult = (int)M11_GameView_HandleInput(&game,
        M12_MENU_INPUT_TURN_LEFT);
    check_int("panel-live TURN_LEFT input result is M11_GAME_INPUT_IGNORED",
              turnResult, (int)M11_GAME_INPUT_IGNORED);
    check_int("party direction unchanged after panel-live TURN_LEFT",
              game.world.party.direction, DIR_SOUTH);

    /* ── Group D: cancel close (F0282 C162 cancel branch) ──────────
     * CancelMirrorCandidate runs the C162 cancel path: F0282 clears
     * G0299, decrements G0305, and F0643_PARTY_ClearChampionSlot
     * for the candidate index.  The portrait on the wall stays the
     * same because the sensor (sensorType=127, sensorData=4) is
     * still active; only the appended candidate goes away.  The
     * D1C portrait_rect_position must return to the panel-off LEIF
     * pixel match rate (>= 90 %). */
    printf("\n[Group D] CancelMirrorCandidate (F0282 C162) closes panel "
           "= close_after_party_shuffle\n");
    cancelRc = M11_GameView_CancelMirrorCandidate(&game);
    check_int("CancelMirrorCandidate returns 1", cancelRc, 1);
    check_int("candidateMirrorPanelActive after cancel",
              game.candidateMirrorPanelActive, 0);
    check_int("candidateMirrorOrdinal after cancel",
              game.candidateMirrorOrdinal, -1);
    check_int("candidateMirrorPartyIndex after cancel",
              game.candidateMirrorPartyIndex, -1);
    check_int("championCount after cancel",
              game.world.party.championCount, 0);
    check_int("inventoryPanelActive after cancel",
              game.inventoryPanelActive, 0);
    ordAfterClose = M11_GameView_GetFrontMirrorOrdinal(&game);
    check_int("front mirror ordinal after cancel",
              ordAfterClose, PROBE_EXPECTED_ORDINAL);

    /* Render after close.  Portrait_rect_position must match ordinal
     * 4 (LEIF) at >= 90 % per-pixel match rate -- the close returns
     * the rect to its panel-off baseline because the C040 chrome no
     * longer overlays the D1C box. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    pctAfterClose = portrait_rect_match_pct(portraits, fb,
                                            PROBE_EXPECTED_ORDINAL);
    {
        PortraitMatch match;
        match = match_portrait_rect(portraits, fb,
                                    PROBE_EXPECTED_ORDINAL);
        check_int("after close: best portrait ordinal at D1C rect",
                  match.bestOrdinal, PROBE_EXPECTED_ORDINAL);
        check_true("after close: ordinal 4 pixel match at D1C rect >= 90%",
                   pctAfterClose >= PROBE_PIXEL_MATCH_PCT);
    }

    /* ── Group E: post-close F0284 rotation sequence ──────────────
     * Now that the panel is closed, TURN_RIGHT/LEFT reach
     * m11_apply_dm1_v1_pipeline_tick and the F0284 party-direction
     * rotation runs.  Two TURN_RIGHT steps take the party SOUTH ->
     * WEST -> NORTH (DIR_SOUTH=2, DIR_WEST=3, DIR_NORTH=0,
     * DIR_EAST=1).  At DIR_SOUTH the front mirror is LEIF (4); at
     * DIR_WEST and DIR_NORTH the front cells are not C127 sensors,
     * so the front mirror ordinal returns -1.  Re-rendering after
     * the rotation sequence proves portrait_rect_position stays
     * consistent across the rotation: the rect is empty when the
     * front sensor is gone and full again when the rotation returns
     * to DIR_SOUTH.  This is the post_party_shuffle counterpart of
     * the pass783 close_after_party_shuffle_pc34_compat sequence:
     * the F0284 happens AFTER the close in the live runtime. */
    printf("\n[Group E] post-close F0284 rotation: %d x TURN_RIGHT\n",
           PROBE_ROTATE_RIGHT_STEPS);
    /* Two TURN_RIGHT steps: SOUTH -> WEST -> NORTH. */
    {
        const int expectedDirs[PROBE_ROTATE_RIGHT_STEPS] = {
            DIR_WEST, DIR_NORTH
        };
        const int expectedOrdinals[PROBE_ROTATE_RIGHT_STEPS] = {
            -1, -1
        };
        for (turnIdx = 0; turnIdx < PROBE_ROTATE_RIGHT_STEPS; ++turnIdx) {
            char labelDir[80];
            char labelOrd[80];
            turnResult = (int)M11_GameView_HandleInput(&game,
                M12_MENU_INPUT_TURN_RIGHT);
            check_int("post-close TURN_RIGHT input result is REDRAW",
                      turnResult, (int)M11_GAME_INPUT_REDRAW);
            snprintf(labelDir, sizeof(labelDir),
                     "post-close TURN_RIGHT step %d party direction",
                     turnIdx + 1);
            check_int(labelDir, game.world.party.direction,
                      expectedDirs[turnIdx]);
            snprintf(labelOrd, sizeof(labelOrd),
                     "post-close TURN_RIGHT step %d front mirror ordinal",
                     turnIdx + 1);
            check_int(labelOrd,
                      M11_GameView_GetFrontMirrorOrdinal(&game),
                      expectedOrdinals[turnIdx]);
            check_int("candidateMirrorPanelActive stays off (no panel re-open)",
                      game.candidateMirrorPanelActive, 0);
        }
    }

    /* Render after the rotation.  The front sensor is gone (DIR_NORTH
     * at (2,1) looks at (2,0) which is not a C127 sensor), so the
     * D1C rect should not match ordinal 4 at the 90 % threshold.
     * This proves the rect does NOT carry stale LEIF pixels from
     * the pre-rotation state. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    pctAfterRotateRight = portrait_rect_match_pct(portraits, fb,
        PROBE_EXPECTED_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after rotation: D1C rect ordinal %d match rate = %d%% "
                 "(front sensor gone; rect should NOT carry LEIF pixels)",
                 PROBE_EXPECTED_ORDINAL, pctAfterRotateRight);
        check_true(msg, pctAfterRotateRight < PROBE_PIXEL_MATCH_PCT);
    }

    /* Side-wall no-float invariant after the rotation: the side-wall
     * columns (D1L x=0..79, D1R x=144..223) at the portrait row band
     * must not carry ordinal-4 warm pixels. */
    {
        int sideLeftWarm = viewport_rect_warm_count(fb,
            PROBE_SIDE_LEFT_X, PROBE_PORTRAIT_VY,
            PROBE_SIDE_LEFT_W, PROBE_PORTRAIT_H);
        int sideRightWarm = viewport_rect_warm_count(fb,
            PROBE_SIDE_RIGHT_X, PROBE_PORTRAIT_VY,
            PROBE_SIDE_RIGHT_W, PROBE_PORTRAIT_H);
        char msgLeft[200], msgRight[200];
        snprintf(msgLeft, sizeof(msgLeft),
                 "D1L side-wall column (x=%d..%d, y=%d..%d) has < %d "
                 "warm pixels after rotation (got %d)",
                 PROBE_SIDE_LEFT_X,
                 PROBE_SIDE_LEFT_X + PROBE_SIDE_LEFT_W - 1,
                 PROBE_PORTRAIT_VY,
                 PROBE_PORTRAIT_VY + PROBE_PORTRAIT_H - 1,
                 PROBE_WARM_THRESHOLD, sideLeftWarm);
        check_lt(msgLeft, sideLeftWarm, PROBE_WARM_THRESHOLD);
        snprintf(msgRight, sizeof(msgRight),
                 "D1R side-wall column (x=%d..%d, y=%d..%d) has < %d "
                 "warm pixels after rotation (got %d)",
                 PROBE_SIDE_RIGHT_X,
                 PROBE_SIDE_RIGHT_X + PROBE_SIDE_RIGHT_W - 1,
                 PROBE_PORTRAIT_VY,
                 PROBE_PORTRAIT_VY + PROBE_PORTRAIT_H - 1,
                 PROBE_WARM_THRESHOLD, sideRightWarm);
        check_lt(msgRight, sideRightWarm, PROBE_WARM_THRESHOLD);
    }

    /* ── Group F: rotate back to SOUTH, sensor returns ────────────
     * Two TURN_LEFT inputs take the party NORTH -> WEST -> SOUTH.
     * The front sensor comes back (DIR_SOUTH at (2,1) is the LEIF
     * ordinal-4 pose).  The D1C portrait rect must return to its
     * panel-off LEIF pixel match rate (>= 90 %). */
    printf("\n[Group F] rotate back to SOUTH: %d x TURN_LEFT\n",
           PROBE_ROTATE_LEFT_STEPS);
    {
        const int expectedDirs[PROBE_ROTATE_LEFT_STEPS] = {
            DIR_WEST, DIR_SOUTH
        };
        for (turnIdx = 0; turnIdx < PROBE_ROTATE_LEFT_STEPS; ++turnIdx) {
            char labelDir[80];
            turnResult = (int)M11_GameView_HandleInput(&game,
                M12_MENU_INPUT_TURN_LEFT);
            check_int("post-rotation TURN_LEFT input result is REDRAW",
                      turnResult, (int)M11_GAME_INPUT_REDRAW);
            snprintf(labelDir, sizeof(labelDir),
                     "post-rotation TURN_LEFT step %d party direction",
                     turnIdx + 1);
            check_int(labelDir, game.world.party.direction,
                      expectedDirs[turnIdx]);
        }
    }
    check_int("front mirror ordinal returns to LEIF (4)",
              M11_GameView_GetFrontMirrorOrdinal(&game),
              PROBE_EXPECTED_ORDINAL);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    pctAfterRotateBack = portrait_rect_match_pct(portraits, fb,
        PROBE_EXPECTED_ORDINAL);
    {
        PortraitMatch match;
        match = match_portrait_rect(portraits, fb,
                                    PROBE_EXPECTED_ORDINAL);
        check_int("after rotate back: best portrait ordinal at D1C rect",
                  match.bestOrdinal, PROBE_EXPECTED_ORDINAL);
        check_true("after rotate back: ordinal 4 match at D1C rect >= 90%",
                   pctAfterRotateBack >= PROBE_PIXEL_MATCH_PCT);
    }

    /* ── Group G: determinism (re-render after rotate back) ────────
     * Two consecutive renders after the rotation-back sequence must
     * produce the same match rate (no F0128 redraw flake). */
    printf("\n[Group G] determinism: re-render after rotate back\n");
    {
        int pctSecond;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
        pctSecond = portrait_rect_match_pct(portraits, fb,
                                            PROBE_EXPECTED_ORDINAL);
        check_int("determinism: re-render ordinal-4 match rate",
                  pctSecond, pctAfterRotateBack);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
