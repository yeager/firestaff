#include "m11_game_view.h"
#include "theron_v1_srm_classifier.h"
#include "theron_v1_startup_save_resume.h"
#include "theron_v1_track02.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char *G2159_puc_Bitmap_Source;
unsigned char *G2160_puc_Bitmap_Destination;

static int fail(const char *message, const M11_GameViewState *view) {
    fprintf(stderr, "FAIL: %s (phase=%d continue=%d detail=%s)\n",
            message, view ? view->theronState.startup_phase : -1,
            view ? view->theronState.save_resume_continue_focus : -1,
            view ? view->inspectDetail : "");
    return 1;
}

int main(int argc, char **argv) {
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    Theron_V1PceBramReceipt receipt;
    Theron_V1PceBramBodyReceipt body;
    Theron_V1_World *world;
    int result = 0;

    if (argc != 4) {
        fprintf(stderr, "usage: %s DATA_DIR TQJP02.bin EMPTY_JP_BRAM\n", argv[0]);
        return 2;
    }
    if (theron_v1_pce_bram_classify_path(argv[3], &receipt) !=
            THERON_V1_PCE_BRAM_READY ||
        !receipt.save_body_layout_proven || receipt.save_body_campaign_valid ||
        receipt.serialized_campaign_byte != 0u ||
        !theron_v1_pce_bram_decode_original_body_path(argv[3], &body) ||
        !body.layout_verified || !body.semantics_verified ||
        body.ram_267c_campaign_byte != 0u) {
        fputs("FAIL: authentic empty JP BRAM has the expected zero-progress body\n",
              stderr);
        return 1;
    }

    memset(&spec, 0, sizeof(spec));
    spec.title = "THERON'S QUEST";
    spec.gameId = "theron";
    spec.sourceId = "theron";
    spec.dataDir = argv[1];
    spec.verifiedAssetPath = argv[2];
    spec.verifiedAssetMd5 = THERON_TRACK02_MD5_JP_BIN;
    spec.savePath = argv[3];
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        result = fail("M11 starts with authentic JP Track 02 and the captured empty BRAM",
                      &view);
        goto done;
    }
    world = (Theron_V1_World *)view.theronWorld;
    if (!world || view.theronState.save_resume_claim !=
                      THERON_V1_STARTUP_RESUME_NONE ||
        view.theronState.save_resume_continue_focus ||
        world->campaign_completion_mask != 0u ||
        world->progression.quest_items_collected != 0u ||
        view.theronState.startup_phase != THERON_STARTUP_PHASE_TITLE) {
        result = fail("zero-progress JP BRAM is not admitted as Continue at title",
                      &view);
        goto done;
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase != THERON_STARTUP_PHASE_STAGE_SELECT ||
        view.theronState.selected_dungeon != 1 ||
        view.theronState.save_resume_continue_focus) {
        result = fail("empty JP BRAM does not change the fresh Akutuba route or expose Continue",
                      &view);
        goto done;
    }
    (void)M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP);
    if (view.theronState.startup_phase != THERON_STARTUP_PHASE_STAGE_SELECT ||
        view.theronState.selected_dungeon != 1 ||
        view.theronState.save_resume_claim != THERON_V1_STARTUP_RESUME_NONE ||
        view.theronState.save_resume_continue_focus ||
        world->campaign_completion_mask != 0u ||
        world->progression.quest_items_collected != 0u) {
        result = fail("empty-slot navigation cannot restore progress or select unavailable Continue",
                      &view);
        goto done;
    }
    puts("PASS: authentic empty JP BRAM is rejected as Continue by native M11 startup");

done:
    M11_GameView_Shutdown(&view);
    return result;
}
