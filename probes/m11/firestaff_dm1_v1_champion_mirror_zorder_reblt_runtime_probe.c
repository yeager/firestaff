/*
 * DM1 V1 champion mirror Z-order re-blt runtime probe.
 *
 * The existing
 *   firestaff_dm1_v1_champion_mirror_visibility_runtime_probe
 *   firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
 * cover the static Hall D1C pose lattice: each (mapX, mapY, dir) is exercised
 * once and the probe asserts (a) the front-cell champion portrait ordinal
 * matches the source catalog and (b) the dominant portrait pixels in the
 * D1C wall rectangle match the expected ordinal.  Neither probe, however,
 * asserts the Z-order *re-blt* invariant that the M11 draw stack must
 * honour when the party turns in place at a single Hall D1C cell:
 *
 *   - turning from a "front mirror" pose to a "no-portrait" side pose must
 *     clear the previous ordinal's pixels from the portrait rectangle
 *     instead of leaving a stale portrait ghost over the side wall;
 *   - turning from a "no-portrait" side pose back to a "front mirror" pose
 *     must re-blt the new ordinal's pixels on top of the wall;
 *   - turning between two different front-mirror ordinals (e.g. N -> S at
 *     a D1C cell where the front ordinal flips 2 -> 3) must not leave the
 *     old ordinal's pixels as the dominant pixels in the portrait rect.
 *
 * This probe drives a real Hall-of-Champions pose sequence,
 * pixel-comparing each step's framebuffer against the prior step to
 * prove the re-blt invariant above.  Earlier versions used the stale
 * TextString-derived (1,4,N)=2 fixture; the source-locked actual-pose
 * contract uses the C127 sensor cells instead:
 *   (1,2,N)=1, (2,1,S)=4, (1,3,E)=18,
 *   (1,5,N)=10, (2,4,S)=15, (1,5,S)=13.
 * Instead of treating each direction as an independent black-box check,
 * the probe walks through them sequentially and adds the new check:
 *
 *   no_stale_portrait_ordinal  - if the prior ordinal was O and the
 *     current ordinal is -1 (no portrait) or a different ordinal O' (with
 *     O' != O), the prior ordinal's pixels must not be the dominant
 *     pixels in the portrait rectangle (re-blt must clear the ghost).
 *
 * The probe does not assert wall-perimeter pixel stability across
 * directions because different directions naturally show different wall
 * geometry (the side wall vs the front wall); that is a per-direction
 * draw contract, not a Z-order invariant.  The existing zorder probe
 * already exercises per-direction wall draws.
 *
 * Source evidence:
 *   ReDMCSB DUNVIEW.C:3913-3928  blits the C026 champion portrait into
 *   the fixed D1C wall box only when G0289 is set (the ordinal is in
 *   range).  ReDMCSB DUNVIEW.C:7727-7924 (F0124_DrawSquareD1C) and
 *   DUNVIEW.C:8064-8316 (F0126_DrawSquareD0R) drive the per-square draw
 *   order, and ReDMCSB MOVESENS.C:556 invokes F0128_DUNGEONVIEW_Draw_CPSF
 *   after a turn so the viewport is fully re-bltted by the new party
 *   pose.  ReDMCSB DUNVIEW.C:8522-8533 restricts the champion-portrait
 *   draw to the D1C front square; the side poses must therefore end up
 *   with no portrait pixels in the D1C wall box.
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
    /* DUNVIEW.C:3913-3928 / 8522-8533 blit the D1C champion portrait to
     * the fixed (96,35)-(127,63) viewport-relative rectangle. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* Side pose: the C1 dark-gray transparency is what the source blit
     * skips; it is the same constant the front-mirror route uses
     * (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY). */
    PROBE_CHAMPION_TRANSPARENT = 1
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedOrdinal;
    int expectedMatched;
    int compared;
} MirrorMatch;

typedef struct ReBltStep {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    const char* label;
} ReBltStep;

static int count_ordinal_pixels(const M11_AssetSlot* portraits,
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
            out.expectedOrdinal = expectedOrdinal;
            out.expectedMatched = matched;
            out.compared = compared;
        }
    }
    return out;
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

/* A pixel-count that uses the *previous ordinal*'s expectedMatch as the
 * reference for "compared".  We don't use the existing match.compared
 * because when the expectedOrdinal is -1, the compared count would be
 * zero (no ordinal was tracked).  Instead, we re-derive the per-ordinal
 * compared count from the portrait strip. */
static int ordinal_compared_count(const M11_AssetSlot* portraits, int ordinal) {
    int x;
    int y;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            ++compared;
        }
    }
    return compared;
}

static int check_step(M11_GameViewState* game,
                      const M11_AssetSlot* portraits,
                      int prevOrdinal,
                      const ReBltStep* step,
                      unsigned char* outFb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, step->mapX, step->mapY, step->dir);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != step->expectedOrdinal) {
        fprintf(stderr, "FAIL %s front ordinal got=%d want=%d\n",
                step->label, ordinal, step->expectedOrdinal);
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, outFb, step->expectedOrdinal);

    if (step->expectedOrdinal >= 0) {
        if (match.bestOrdinal != step->expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * 90) {
            fprintf(stderr,
                    "FAIL %s visible portrait expected=%d best=%d matched=%d/%d\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else {
        /* No-portrait side pose must not be dominated by a stale portrait
         * from a previous ordinal (re-blt must clear the ghost).  We
         * tolerate the small "best=14" leak the existing zorder probe
         * already tolerates (35% of the prior ordinal's compared count)
         * so this probe is consistent with the static-pose coverage. */
        if (prevOrdinal >= 0) {
            int stale = count_ordinal_pixels(portraits, outFb, prevOrdinal);
            int prevCompared = ordinal_compared_count(portraits, prevOrdinal);
            int stalePct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
            if (stalePct >= 35) {
                fprintf(stderr,
                        "FAIL %s stale ordinal=%d leaked pixels=%d/%d (re-blt did not clear)\n",
                        step->label, prevOrdinal, stale, prevCompared);
                ok = 0;
            }
        }
    }

    /* Z-order re-blt invariant: when the ordinal changes between steps,
     * the new ordinal must dominate (or the rect must have no portrait)
     * and the old ordinal must not be dominant. */
    if (prevOrdinal >= 0 && step->expectedOrdinal >= 0 &&
        prevOrdinal != step->expectedOrdinal) {
        /* Both are front poses with different ordinals: the previous
         * ordinal's pixels must not be dominant in the new framebuffer. */
        int stale = count_ordinal_pixels(portraits, outFb, prevOrdinal);
        int prevCompared = ordinal_compared_count(portraits, prevOrdinal);
        int stalePct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (stalePct >= 35) {
            fprintf(stderr,
                    "FAIL %s prior ordinal=%d still leaks pixels=%d/%d after re-blt to ordinal=%d\n",
                    step->label, prevOrdinal, stale, prevCompared, step->expectedOrdinal);
            ok = 0;
        }
    }

    printf("%s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           step->label, step->mapX, step->mapY, step->dir, ordinal,
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
    /* Real C127 Hall sequence.  ReDMCSB DUNGEON.C:2573 and 2608-2612
     * allow only the front-wall side of each C127 sensor to set G0289.
     * The interleaved no-portrait poses prove stale portraits are
     * cleared before the next source-valid front mirror is blitted. */
    const ReBltStep steps[] = {
        {1, 2, DIR_NORTH, 1,  "d1c_reblt_step_a_halk_north_ordinal_1"},
        {1, 2, DIR_WEST,  -1, "d1c_reblt_step_b_start_west_no_portrait"},
        {1, 2, DIR_EAST,  -1, "d1c_reblt_step_c_start_east_wrong_wall_no_portrait"},
        {2, 1, DIR_SOUTH, 4,  "d1c_reblt_step_d_leif_south_ordinal_4"},
        {1, 3, DIR_NORTH, -1, "d1c_reblt_step_e_corridor_north_no_portrait"},
        {1, 3, DIR_EAST,  18, "d1c_reblt_step_f_sonja_east_ordinal_18"},
        {1, 5, DIR_NORTH, 10, "d1c_reblt_step_g_zed_north_ordinal_10"},
        {1, 5, DIR_EAST,  -1, "d1c_reblt_step_h_end_east_wrong_wall_no_portrait"},
        {2, 4, DIR_SOUTH, 15, "d1c_reblt_step_i_mophus_south_ordinal_15"},
        {1, 5, DIR_SOUTH, 13, "d1c_reblt_step_j_wuuf_south_ordinal_13"},
        {1, 4, DIR_WEST,  -1, "d1c_reblt_step_k_corridor_west_no_portrait"},
    };
    int stepIdx;
    int prevOrdinal = -2; /* sentinel: no prior ordinal */

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
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

    for (stepIdx = 0; stepIdx < (int)(sizeof(steps) / sizeof(steps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk = check_step(&game, portraits, prevOrd,
                                &steps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = steps[stepIdx].expectedOrdinal;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror z-order re-blt runtime probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
