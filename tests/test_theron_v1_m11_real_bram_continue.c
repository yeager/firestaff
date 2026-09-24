#include "m11_game_view.h"
#include "theron_v1_startup_save_resume.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int fail(const char* message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(int argc, char** argv) {
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    Theron_V1_World* world;
    int failures = 0;

    if (argc != 4) {
        fprintf(stderr, "usage: %s DATA_DIR TRACK02 BRAM\n", argv[0]);
        return 2;
    }

    memset(&spec, 0, sizeof(spec));
    spec.title = "THERON'S QUEST";
    spec.gameId = "theron";
    spec.sourceId = "theron";
    spec.dataDir = argv[1];
    spec.verifiedAssetPath = argv[2];
    spec.verifiedAssetMd5 = "f23601102138f87c33025877767ebf76";
    spec.savePath = argv[3];
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        fprintf(stderr, "M11 startup detail: %s\n", view.inspectDetail);
        return fail("M11 starts with authenticated US Track 02 and Backup RAM");
    }
    world = (Theron_V1_World*)view.theronWorld;
    if (!world || view.theronState.save_resume_claim !=
                      THERON_V1_STARTUP_RESUME_SRM ||
        view.theronState.save_resume_srm_active_slot != 0 ||
        view.theronState.save_resume_srm_import_status !=
            THERON_V1_SRM_PROGRESS_IMPORT_OK) {
        ++failures;
        fprintf(stderr, "FAIL: M11 admits the authenticated original Backup RAM slot\n");
    }
    if (view.theronState.startup_phase != THERON_STARTUP_PHASE_TITLE ||
        M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase !=
            THERON_STARTUP_PHASE_STAGE_SELECT) {
        ++failures;
        fprintf(stderr, "FAIL: authenticated M11 title opens stage selection\n");
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) !=
            M11_GAME_INPUT_REDRAW ||
        !view.theronState.save_resume_continue_focus) {
        ++failures;
        fprintf(stderr, "FAIL: Continue receives focus in the authentic startup menu\n");
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
        M11_GAME_INPUT_REDRAW) {
        ++failures;
        fprintf(stderr, "FAIL: explicit M11 Continue action is handled\n");
    }
    if (!strstr(view.inspectDetail, "continued original Backup RAM slot=0")) {
        ++failures;
        fprintf(stderr, "FAIL: M11 Continue receipt identifies the authentic slot (detail=%s)\n",
                view.inspectDetail);
    }
    if (view.theronState.startup_phase !=
            THERON_STARTUP_PHASE_STAGE_SELECT ||
        view.theronState.level_loaded) {
        ++failures;
        fprintf(stderr, "FAIL: Continue remains at the source-backed stage boundary before an explicit dungeon-entry action\n");
    }
    if (world->party.champions[0].max_health != 175 ||
        world->party.champions[0].max_stamina != 1500 ||
        world->party.champions[0].max_mana != 50 ||
        world->timer_count != 0) {
        ++failures;
        fprintf(stderr, "FAIL: M11 Continue applies only authenticated Theron save fields (max=%u/%u/%u objects=%u timers=%u)\n",
                world->party.champions[0].max_health,
                world->party.champions[0].max_stamina,
                world->party.champions[0].max_mana,
                world->object_count, world->timer_count);
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase !=
            THERON_STARTUP_PHASE_STAGE_SELECT ||
        view.theronState.selected_dungeon != 2) {
        ++failures;
        fprintf(stderr,
                "FAIL: authentic Continue skips the completed first chapter and focuses unlocked dungeon 2 (phase=%d dungeon=%d)\n",
                view.theronState.startup_phase,
                view.theronState.selected_dungeon);
    } else if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                   M11_GAME_INPUT_REDRAW ||
               view.theronState.startup_phase !=
                   THERON_STARTUP_PHASE_SOUL_ROOM ||
               view.theronState.selected_dungeon != 2 ||
               view.theronState.level_loaded) {
        ++failures;
        fprintf(stderr,
                "FAIL: authentic Continue enters the source-backed next chapter's Soul Room before forcefield admission (phase=%d dungeon=%d level_loaded=%d)\n",
                view.theronState.startup_phase,
                view.theronState.selected_dungeon,
                view.theronState.level_loaded);
    }
    M11_GameView_Shutdown(&view);
    if (failures) {
        fprintf(stderr, "Theron authenticated M11 Continue FAILED (%d checks)\n",
                failures);
        return 1;
    }
    puts("Theron authenticated M11 Continue checks passed");
    return 0;
}
