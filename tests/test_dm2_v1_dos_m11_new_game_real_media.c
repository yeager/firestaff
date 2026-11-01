/* Opt-in M11 regression for DM2 DOS's source-owned NEW GAME handoff. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_startup_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    DM2_V1_StartupMenuPointerLayout layout;
    DM2_V1_BootRuntimeReceipt runtime;
    uint8_t framebuffer[M11_FB_WIDTH * M11_FB_HEIGHT];
    int draw_count;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 77;
    }
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.title = "DUNGEON MASTER II";
    /* Preserve the retail ZIP as the selected source owner.  M11 resolves
     * GRAPHICS.DAT and DUNGEON.DAT through this virtual archive path in RAM;
     * it must not depend on a pre-extracted directory. */
    spec.dataDir = archive;
    /* Match a normal launcher start: IBMIOP owns INTRO.MVE before SKULL's
     * startup menu becomes interactive.  The test fast-forwards only the
     * host clock; M11 still admits and presents every source frame in order. */
    spec.launcherOptionsBound = 1;
    spec.presentationWidth = M11_FB_WIDTH;
    spec.presentationHeight = M11_FB_HEIGHT;
    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        fputs("FAIL: DM2 DOS retail archive did not enter M11\n", stderr);
        return 1;
    }
    view.bootProbeFastForward = 1;
    for (draw_count = 0; draw_count < 220 &&
                         !view.dm2DosMveIntroComplete; ++draw_count) {
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    }
    /* The retail MVE contains 217 display boundaries.  After its last page
     * has been presented, one further draw must release IBMIOP and render
     * SKULL's real GDAT menu rather than leaving a black frame or starting a
     * dungeon without its source menu. */
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (!view.dm2DosMveIntroComplete || view.dm2DosMveIntroActive ||
        view.dm2DosMveIntroRejected || !view.dm2State.startup_menu_active ||
        framebuffer[0] == 0u) {
        fprintf(stderr,
                "FAIL: DM2 DOS INTRO.MVE did not hand off to SKULL's real menu "
                "(complete=%d active=%d rejected=%d failure=%d menu=%d pixel0=%u)\n",
                view.dm2DosMveIntroComplete, view.dm2DosMveIntroActive,
                view.dm2DosMveIntroRejected, view.dm2DosMveFailureCode,
                view.dm2State.startup_menu_active,
                (unsigned int)framebuffer[0]);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(&layout, 0, sizeof(layout));
    if (!dm2_v1_boot_startup_menu_pointer_layout(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &layout) ||
        !layout.valid || layout.new_game.w <= 0 || layout.new_game.h <= 0) {
        fputs("FAIL: DM2 DOS real GDAT lacks a New Game rectangle\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    /* Exercise the normal DOS mouse path using the actual 0xD7 rectangle
     * decoded from retail GDAT. M11 must map this source-space click through
     * SKULL's pointer event, not an invented menu row or keyboard shortcut. */
    if (M11_GameView_HandlePointerButton(
            &view,
            layout.new_game.x + layout.new_game.w / 2,
            layout.new_game.y + layout.new_game.h / 2,
            DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        view.dm2State.startup_menu_active || !view.dm2State.level_loaded ||
        view.world.party.championCount < 1) {
        fputs("FAIL: DM2 DOS mouse New Game did not commit the source runtime\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    {
        const int hero_count = dm2_v1_runtime_get_champion_count();
        DM2_V1_RuntimeSourceHeroStateReceipt hero;
        if (hero_count < 1 || hero_count > DM2_MAX_HEROES ||
            !dm2_v1_runtime_get_source_hero_state(0, &hero) ||
            hero.cur_hp < 0 || hero.cur_hp > hero.max_hp ||
            hero.name1[0] == '\0' || hero.name2[0] == '\0') {
            fputs("FAIL: DM2 DOS source hero HP/identity receipt is incomplete\n",
                  stderr);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        printf("PASS: DM2 DOS source hero receipt has identity and HP %d/%d\n",
               hero.cur_hp, hero.max_hp);
    }
    {
        static const int formation_x[4] = { 263, 285, 285, 263 };
        static const int formation_y[4] = {  72,  72,  94,  94 };
        DM2_V1_RuntimeSourceHeroStateReceipt hero_before;
        DM2_V1_RuntimeSourceHeroStateReceipt hero_after;
        int facing = dm2_v1_runtime_get_party_dir();
        int hero_count = dm2_v1_runtime_get_champion_count();
        int source_relative;
        int target_relative = -1;
        int hero_index = -1;
        int slot;

        for (int i = 0; i < hero_count && i < DM2_MAX_HEROES; ++i) {
            DM2_V1_RuntimeSourceHeroStateReceipt candidate;
            if (dm2_v1_runtime_get_source_hero_state((uint8_t)i,
                                                       &candidate) &&
                candidate.cur_hp > 0) {
                hero_index = i;
                hero_before = candidate;
                break;
            }
        }
        if (hero_index < 0 || facing < 0 || facing > 3) {
            fputs("FAIL: DM2 DOS formation has no live source champion/facing\n",
                  stderr);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        source_relative = (hero_before.party_position - facing) & 3;
        for (slot = 1; slot <= 4; ++slot) {
            int candidate_relative = (source_relative + slot) & 3;
            int candidate_absolute = (candidate_relative + facing) & 3;
            int occupied = 0;
            for (int i = 0; i < hero_count && i < DM2_MAX_HEROES; ++i) {
                DM2_V1_RuntimeSourceHeroStateReceipt candidate;
                if (dm2_v1_runtime_get_source_hero_state((uint8_t)i,
                                                           &candidate) &&
                    candidate.cur_hp > 0 &&
                    candidate.party_position == candidate_absolute) {
                    occupied = 1;
                    break;
                }
            }
            if (!occupied) {
                target_relative = candidate_relative;
                break;
            }
        }
        if (target_relative < 0) {
            fputs("FAIL: DM2 DOS formation has no empty destination slot\n",
                  stderr);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        if (M11_GameView_HandlePointerButton(
                &view, formation_x[source_relative],
                formation_y[source_relative], DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                M11_GAME_INPUT_REDRAW ||
            !dm2_v1_runtime_get_source_hero_state((uint8_t)hero_index,
                                                   &hero_after) ||
            hero_after.party_position != hero_before.party_position) {
            fputs("FAIL: first retail formation click changed the party position\n",
                  stderr);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        if (M11_GameView_HandlePointerButton(
                &view, formation_x[target_relative],
                formation_y[target_relative], DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                M11_GAME_INPUT_REDRAW ||
            !dm2_v1_runtime_get_source_hero_state((uint8_t)hero_index,
                                                   &hero_after) ||
            hero_after.party_position != ((target_relative + facing) & 3)) {
            fputs("FAIL: retail DM2 formation clicks did not move the source champion\n",
                  stderr);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        puts("PASS: DM2 DOS retail formation hitboxes move a champion into an empty slot");
    }
    memset(&runtime, 0, sizeof(runtime));
    if (!dm2_v1_runtime_weather_chain_started() ||
        !dm2_v1_boot_runtime_capture(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &runtime) ||
        !runtime.runtime_ready || runtime.current_level != 0 ||
        runtime.outdoor) {
        fputs("FAIL: DM2 DOS New Game did not retain map-0 indoor scene ownership\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (!view.dm2LastRuntimeFrameAccepted ||
        !view.dm2LastRuntimeRealAssetsReady ||
        !view.dm2LastRuntimeNoCoreFallbacks ||
        view.dm2LastRuntimeFallbackDrawCount != 0) {
        fprintf(stderr,
                "FAIL: DM2 DOS map-0 indoor frame was not admitted "
                "(accepted=%d real=%d noFallbacks=%d fallbackDraws=%d)\n",
                view.dm2LastRuntimeFrameAccepted,
                view.dm2LastRuntimeRealAssetsReady,
                view.dm2LastRuntimeNoCoreFallbacks,
                view.dm2LastRuntimeFallbackDrawCount);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    M11_GameView_Shutdown(&view);
    puts("PASS: DM2 DOS real-GDAT mouse New Game admits map-0 indoor runtime");
    return 0;
}
