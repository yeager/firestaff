#include "m11_game_view.h"
#include "theron_v1_startup_save_resume.h"
#include "theron_v1_track02.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int fail(const char *message, const M11_GameViewState *view) {
    fprintf(stderr, "FAIL: %s (phase=%d dungeon=%d cursor=%d detail=%s)\n",
            message, view ? view->theronState.startup_phase : -1,
            view ? view->theronState.selected_dungeon : -1,
            view ? view->theronState.startup_cursor : -1,
            view ? view->inspectDetail : "");
    return 1;
}

int main(int argc, char **argv) {
    static const char *const expected_roster_names[] = {
        "THERON", "MARA", "LINOS", "HEXA", "HAKAR", "TIRAN", "DOTAN",
        "PENTAI"
    };
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    Theron_V1_World *world;
    int result = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s DATA_DIR TQJP02.bin\n", argv[0]);
        return 2;
    }

    memset(&spec, 0, sizeof(spec));
    spec.title = "THERON'S QUEST";
    spec.gameId = "theron";
    spec.sourceId = "theron";
    spec.dataDir = argv[1];
    spec.verifiedAssetPath = argv[2];
    spec.verifiedAssetMd5 = THERON_TRACK02_MD5_JP_BIN;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        result = fail("M11 starts a fresh game from authentic Japanese Track 02",
                      &view);
        goto done;
    }
    world = (Theron_V1_World *)view.theronWorld;
    if (!world || view.theronState.startup_phase != THERON_STARTUP_PHASE_TITLE ||
        view.theronState.save_resume_claim != THERON_V1_STARTUP_RESUME_NONE ||
        M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase != THERON_STARTUP_PHASE_STAGE_SELECT ||
        view.theronState.selected_dungeon != 1 ||
        M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase != THERON_STARTUP_PHASE_SOUL_ROOM ||
        view.theronState.level_loaded ||
        view.theronState.startup_roster_name_status !=
            THERON_TRACK02_SIGNAL_OK ||
        view.theronState.startup_roster_name_count !=
            (int)(sizeof(expected_roster_names) /
                  sizeof(expected_roster_names[0])) ||
        M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase != THERON_STARTUP_PHASE_READY ||
        view.theronState.selected_mirrors_mask == 0) {
        result = fail("authentic JP title selects Ak-Tu-Ba and admits the source roster through Soul Room",
                      &view);
        goto done;
    }
    for (size_t i = 0u;
         i < sizeof(expected_roster_names) / sizeof(expected_roster_names[0]);
         ++i) {
        if (strcmp(view.theronState.startup_roster_names[i],
                   expected_roster_names[i]) != 0) {
            result = fail("authentic JP roster order matches Track 02 source names",
                          &view);
            goto done;
        }
    }

    for (int i = 0;
         i <= THERON_STARTUP_HERO_MIRROR_COUNT &&
         view.theronState.startup_cursor != THERON_STARTUP_HERO_MIRROR_COUNT;
         ++i) {
        if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW) break;
    }
    if (view.theronState.startup_cursor !=
            THERON_STARTUP_HERO_MIRROR_COUNT ||
        M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase != THERON_STARTUP_PHASE_IN_DUNGEON ||
        !view.theronState.level_loaded || world->current_dungeon != 1 ||
        world->current_level != 0 ||
        !world->levels[0][0].source_header_verified ||
        world->party.champion_count <= 0 ||
        strcmp(world->party.champions[0].name, "THERON") != 0 ||
        world->progression.quest_items_collected != 0u) {
        result = fail("authentic JP new game admits source-verified Ak-Tu-Ba level 0 without fabricated quest progress",
                      &view);
        goto done;
    }

    puts("PASS: authentic JP M11 fresh campaign reaches source-verified Ak-Tu-Ba level 0");

done:
    M11_GameView_Shutdown(&view);
    return result;
}
