/* Opt-in real PC-DOS SKSave resume -> M11 DM2 Fireball regression. */

#include "m11_game_view.h"
#include "dm2_v1_spell.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *corpus = getenv("FIRESTAFF_DM2_SKSAVE_CORPUS");
    char save_path[1024];
    M11_GameViewState view;
    M11_GameLaunchSpec spec;

    if (!root || !root[0] || !corpus || !corpus[0]) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR and FIRESTAFF_DM2_SKSAVE_CORPUS are not set");
        return 0;
    }
    if (snprintf(save_path, sizeof(save_path), "%s/sksave0.dat", corpus) >=
            (int)sizeof(save_path)) {
        fputs("FAIL: DOS save path is too long\n", stderr);
        return 1;
    }
    memset(&view, 0, sizeof(view));
    memset(&spec, 0, sizeof(spec));
    spec.title = "Dungeon Master II PC-DOS SKSave";
    spec.gameId = "dm2";
    spec.dataDir = root;
    spec.savePath = save_path;
    spec.sourceId = "dos-sksave-real";
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.launcherOptionsBound = 1;

    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec) ||
        !view.dm2BootProfile || !view.dm2State.level_loaded) {
        fprintf(stderr, "FAIL: DOS SKSave did not publish an M11 DM2 session\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (!M11_GameView_OpenSpellPanel(&view)) {
        fputs("FAIL: DOS M11 did not open the DM2 spell panel\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    view.spellBuffer.runes[0] = DM2_RUNE_YA;
    view.spellBuffer.runes[1] = DM2_RUNE_FUL;
    view.spellBuffer.runes[2] = DM2_RUNE_IR;
    view.spellBuffer.runeCount = 3;
    if (!M11_GameView_CastSpell(&view) || view.spellPanelOpen) {
        fputs("FAIL: DOS M11 DM2 Fireball did not commit through the source owner\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    {
        int step_seen = 0;
        for (int i = 0; i < 4; ++i) {
            DM2_V1_ProceedTimersReceipt timers;
            dm2_v1_runtime_tick();
            memset(&timers, 0, sizeof(timers));
            if (dm2_v1_runtime_last_proceed_timers_receipt(&timers) &&
                timers.type_tally[0x1e] > 0) {
                step_seen = 1;
                break;
            }
        }
        if (!step_seen) {
            fputs("FAIL: DOS M11 Fireball never reached DM2_STEP_MISSILE\n",
                  stderr);
            M11_GameView_Shutdown(&view);
            return 1;
        }
    }
    puts("PASS: authentic DOS SKSave resume reaches M11 Fireball and DM2_STEP_MISSILE");
    M11_GameView_Shutdown(&view);
    return 0;
}
