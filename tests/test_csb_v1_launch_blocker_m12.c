#include "menu_hit_m12.h"
#include "menu_input_m12.h"
#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int force_csb_available(M12_StartupMenuState* state) {
    int versionIndex = M12_AssetStatus_FindVersionIndex("csb", "st20-21-en");
    if (!state || versionIndex < 0 ||
        versionIndex >= M12_ASSET_MAX_VERSIONS_PER_GAME) {
        return 0;
    }
    state->entries[1].title = "CHAOS STRIKES BACK";
    state->entries[1].gameId = "csb";
    state->entries[1].kind = M12_MENU_ENTRY_GAME;
    state->entries[1].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[1].available = 1;
    /* Suppress the NO GAME DATA FOUND popup that m12_show_no_game_data_popup()
     * raised during M12_StartupMenu_InitWithDataDir() because the test data
     * dir (/dev/shm/firestaff-test-no-assets) is empty. The fixture manually
     * arranges CSB availability, so we treat it as if a candidate was found,
     * then return the view to MAIN so the card-click path is reachable. */
    state->assetStatus.originalFileCandidateFound = 1;
    state->view = M12_MENU_VIEW_MAIN;
    state->activatedIndex = -1;
    state->launchRequested = 0;
    state->quickResumeLaunchRequested = 0;
    state->messageLine1 = "";
    state->messageLine2 = "";
    state->messageLine3 = "";
    state->messageIsMissingGameData = 0;
    state->messageGameId[0] = '\0';
    state->assetStatus.csbAvailable = 1;
    state->assetStatus.dm1Available = 0;
    state->assetStatus.versions[1][versionIndex].gameId = "csb";
    state->assetStatus.versions[1][versionIndex].versionId = "st20-21-en";
    state->assetStatus.versions[1][versionIndex].label = "Atari ST 2.0/2.1 English";
    state->assetStatus.versions[1][versionIndex].shortLabel = "ST 2.1 EN";
    state->assetStatus.versions[1][versionIndex].matched = 1;
    for (int i = 0; i < (int)M12_ASSET_MAX_VERSIONS_PER_GAME; ++i) {
        if (i != versionIndex) {
            state->assetStatus.versions[1][i].matched = 0;
        }
    }
    state->assetStatus.requiredFileCounts[1] = 2;
    state->assetStatus.requiredFiles[1][0].gameId = "csb";
    state->assetStatus.requiredFiles[1][0].roleId = "graphics";
    state->assetStatus.requiredFiles[1][0].label = "GRAPHICS.DAT";
    state->assetStatus.requiredFiles[1][0].required = 1;
    state->assetStatus.requiredFiles[1][0].matched = 1;
    state->assetStatus.requiredFiles[1][1].gameId = "csb";
    state->assetStatus.requiredFiles[1][1].roleId = "dungeon";
    state->assetStatus.requiredFiles[1][1].label = "DUNGEON.DAT";
    state->assetStatus.requiredFiles[1][1].required = 1;
    state->assetStatus.requiredFiles[1][1].matched = 1;
    state->gameOptions[1].versionIndex = versionIndex;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    return 1;
}

static int force_csb_version_only_missing_required(M12_StartupMenuState* state) {
    if (!force_csb_available(state)) return 0;
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.requiredFiles[1][0].matched = 0;
    state->assetStatus.requiredFiles[1][1].matched = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
    state->gameOptions[1].presentationModeIndex = M12_PRESENTATION_V21_UPSCALED;
    return 1;
}

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static int render_smoke_nonblank(const M12_StartupMenuState* state, const char* label) {
    const int w = M12_ModernMenu_NativeWidth();
    const int h = M12_ModernMenu_NativeHeight();
    const size_t bytes = (size_t)w * (size_t)h * 4u;
    unsigned char* rgba = (unsigned char*)malloc(bytes);
    int distinct;
    if (!rgba) {
        fprintf(stderr, "FAIL: %s render buffer allocation\n", label);
        return 0;
    }
    memset(rgba, 0, bytes);
    M12_ModernMenu_Render(state, rgba, w, h);
    distinct = M12_ModernMenu_CountDistinctColors(rgba, w, h, 128);
    free(rgba);
    if (distinct < 3) {
        fprintf(stderr, "FAIL: %s render should produce nonblank startup/menu pixels\n", label);
        return 0;
    }
    return 1;
}

int main(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    M12_StartupBootReadiness boot;
    M12_StartupLaunchGate gate;
    int changed;
    const int gridLeft = 42 + 390 + 44;
    const int cardW = (1920 - gridLeft - 48 - 22 * 2) / 3;
    const int csbCardCenterX = gridLeft + 1 * (cardW + 22) + cardW / 2;
    const int hitCardH = ((1080 - 130) - 40 - 22) / 2;
    const int cardCenterY = 40 + hitCardH / 2;
    const int platformCardCenterX = 160 + 250;
    const int choiceCardCenterY = 280 + 125;
    const int originalCardCenterX = 210 + 240;

    M12_StartupMenu_InitWithDataDir(&state, "/dev/shm/firestaff-test-no-assets", NULL);
    state.languageExplicit = 1;
    state.settings.languageIndex = 0;
    if (!expect(force_csb_available(&state),
                "CSB readiness fixture should bind a catalogued Atari version")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, csbCardCenterX, cardCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB card direct click should change menu state")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS, "CSB card direct click should enter game options")) return 1;
    if (!expect(state.activatedIndex == 1, "CSB direct click should activate CSB")) return 1;
    if (!expect(M12_StartupMenu_GetBootReadiness(&state, 1, &boot) == 1,
                "CSB boot readiness receipt should build")) return 1;
    if (!expect(boot.fullStartGraphicsReady == 0,
                "CSB M12 readiness should leave full startup proof to M11")) return 1;
    if (!expect(boot.startupContractExpected == 1 && boot.startupContractReady == 0,
                "CSB M12 readiness should not invent the M11 startup receipt")) return 1;
    if (!expect(boot.packagedCaptureExpected == 1 && boot.packagedCaptureReady == 0,
                "CSB M12 readiness should not invent the M11 capture proof")) return 1;
    if (!expect(boot.startupContractLabel &&
                strcmp(boot.startupContractLabel, "CSB STARTUP CAPTURE RECEIPT") == 0,
                "CSB boot readiness should name the startup receipt contract")) return 1;
    if (!expect(boot.packagedCaptureLabel &&
                strcmp(boot.packagedCaptureLabel, "CSB TITLE + HUD CAPTURE PROOF") == 0,
                "CSB boot readiness should name the packaged capture proof")) return 1;
    if (!expect(boot.startupStepCount > 3,
                "CSB boot readiness should expose the full boot chain step count")) return 1;
    if (!expect(boot.startupStepReadyCount == 3,
                "CSB boot readiness should count only data, version, and title handoff")) return 1;
    if (!expect(boot.nextStepLabel && strstr(boot.nextStepLabel, "TITLE START") != NULL,
                "CSB boot readiness next step should belong to M11")) return 1;
    if (!expect(boot.startupPathLabel && strcmp(boot.startupPathLabel, "CSB BOOT PATH") == 0,
                "CSB boot readiness should name the CSB path")) return 1;
    if (!expect(boot.statusLabel && strcmp(boot.statusLabel, "TITLE START AVAILABLE") == 0,
                "CSB boot status should describe the M11 title handoff")) return 1;
    if (!expect(boot.detailLabel &&
                strcmp(boot.detailLabel, "MENU AND CAPTURE PROOFS NOT READY") == 0,
                "CSB boot detail should leave later proofs to M11")) return 1;
    if (!expect(M12_StartupMenu_GetLaunchGate(&state, 1, &gate) == 1,
                "CSB launch gate should build")) return 1;
    if (!expect(gate.canLaunch == 1,
                "CSB launch gate should allow the verified M11 title attempt")) return 1;
    if (!expect(gate.boot.fullStartGraphicsReady == 0,
                "CSB launch gate should carry unproven M11 startup readiness")) return 1;
    if (!expect(strcmp(M12_StartupMenu_GetEntryLaunchStatusLabel(&state, 1),
                       "TITLE START AVAILABLE") == 0,
                "CSB card status should identify the title handoff")) return 1;
    if (!expect(strcmp(M12_StartupMenu_GetEntryLaunchDetailLabel(&state, 1),
                       "VERIFIED TITLE START; MENU AND CAPTURE STILL GATED") == 0,
                "CSB card detail should keep M11 menu/capture proof gated")) return 1;
    if (!render_smoke_nonblank(&state, "CSB options")) return 1;

    if (M12_ModernMenu_HitTest(&state, platformCardCenterX, choiceCardCenterY).kind !=
            M12_HIT_GAMEOPT_ROW) {
        fprintf(stderr, "FAIL: CSB platform control should be hit-testable\n");
        return 1;
    }
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.gameCardFlowStage == 1,
                "CSB Atari ST platform selection should open presentation choices")) return 1;
    changed = M12_ModernMenu_HandlePointer(&state, originalCardCenterX, choiceCardCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB Launch direct click should be handled")) return 1;
    if (!expect(state.launchRequested == 1, "CSB hash-matched assets should request runtime launch")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE, "CSB launch blocker should show message")) return 1;
    if (!expect(state.messageLine1 && strcmp(state.messageLine1, "RUNTIME NOT READY") != 0,
                "CSB launch transition should not report the old runtime blocker")) return 1;
    if (!render_smoke_nonblank(&state, "CSB ready message")) return 1;

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1, "CSB launch intent is valid when version is matched")) return 1;
    if (!expect(intent.gameId && strcmp(intent.gameId, "csb") == 0, "CSB intent should still identify CSB for diagnostics")) return 1;

    puts("ok: CSB launcher fixture renders options/title-handoff views and exposes a launch intent");
    puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 launch state and LOADSAVE.C F0435 new-game load boundary");

    M12_StartupMenu_InitWithDataDir(&state, "/dev/shm/firestaff-test-no-assets", NULL);
    state.languageExplicit = 1;
    state.settings.languageIndex = 0;
    if (!expect(force_csb_version_only_missing_required(&state),
                "CSB missing-data fixture should retain its catalogued version")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, csbCardCenterX, cardCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB version-only fixture should still open options for regression coverage")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS, "CSB version-only fixture enters game options")) return 1;
    if (!expect(state.gameOptions[1].presentationModeIndex == M12_PRESENTATION_V21_UPSCALED,
                "CSB version-only fixture keeps the V2.1 presentation selection")) return 1;
    if (!expect(M12_StartupMenu_GetBootReadiness(&state, 1, &boot) == 1,
                "CSB missing-data boot readiness receipt should build")) return 1;
    if (!expect(boot.fullStartGraphicsReady == 0,
                "CSB missing-data boot readiness should not report full startup ready")) return 1;
    if (!expect(boot.startupContractExpected == 1 && boot.startupContractReady == 0,
                "CSB missing-data boot readiness should expect but not satisfy startup receipt contract")) return 1;
    if (!expect(boot.packagedCaptureExpected == 1 && boot.packagedCaptureReady == 0,
                "CSB missing-data boot readiness should expect but not satisfy packaged capture proof")) return 1;
    if (!expect(boot.startupStepCount == 7,
                "CSB missing-data boot readiness should retain full boot chain count")) return 1;
    if (!expect(boot.startupStepReadyCount == 1,
                "CSB missing-data boot readiness should only count the matched version")) return 1;
    if (!expect(boot.nextStepLabel && strcmp(boot.nextStepLabel, "REQUIRED GAME DATA") == 0,
                "CSB missing-data boot readiness should name required data as next step")) return 1;
    if (!expect(boot.statusLabel && strcmp(boot.statusLabel, "DATA MISSING") == 0,
                "CSB missing-data status label should stay explicit")) return 1;
    if (!expect(M12_StartupMenu_GetLaunchGate(&state, 1, &gate) == 1,
                "CSB missing-data launch gate should build")) return 1;
    if (!expect(gate.canLaunch == 0,
                "CSB missing-data launch gate should block launch")) return 1;
    if (!expect(gate.dataReady == 0 && gate.versionReady == 1,
                "CSB missing-data launch gate should expose data/version split")) return 1;
    if (!expect(gate.blockedLabel && strcmp(gate.blockedLabel, "DATA MISSING") == 0,
                "CSB missing-data launch gate should expose blocker label")) return 1;
    if (!expect(strcmp(M12_StartupMenu_GetEntryLaunchStatusLabel(&state, 1),
                       "DATA MISSING") == 0,
                "CSB missing-data card status should use launch gate blocker")) return 1;
    if (!expect(strstr(M12_StartupMenu_GetEntryLaunchDetailLabel(&state, 1),
                       "FILES ARE MISSING") != NULL,
                "CSB missing-data card detail should use launch gate detail")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, platformCardCenterX, choiceCardCenterY, 1, NULL);
    if (!expect(changed == 1 && state.view == M12_MENU_VIEW_MESSAGE &&
                state.gameCardFlowStage == 0,
                "CSB missing data must block platform selection before presentation")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS,
                "CSB missing-data fixture should return to game options")) return 1;
    state.gameCardFlowStage = 1;
    state.gameOptions[1].presentationModeIndex = M12_PRESENTATION_V21_UPSCALED;
    changed = M12_ModernMenu_HandlePointer(&state, originalCardCenterX, choiceCardCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB V2.1 launch click should be handled")) return 1;
    if (!expect(state.launchRequested == 0,
                "CSB V2.1 version match must not bypass missing required-file gating")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE,
                "CSB V2.1 missing required data shows a message instead of launching")) return 1;
    if (!expect(state.messageIsMissingGameData == 1,
                "CSB missing-data popup should expose visual missing-data context")) return 1;
    if (!expect(strcmp(state.messageGameId, "csb") == 0,
                "CSB missing-data popup should carry the CSB game id for card art")) return 1;
    if (!expect(state.activatedIndex == 1,
                "CSB missing-data popup should bind the CSB card art index")) return 1;
    if (!expect(state.messageLine2 && strstr(state.messageLine2, "GRAPHICS.DAT") &&
                strstr(state.messageLine2, "DUNGEON.DAT"),
                "CSB V2.1 missing-data popup names both required V1 runtime files")) return 1;
    if (!expect(state.messageLine3 &&
                strstr(state.messageLine3, "/dev/shm/firestaff-test-no-assets"),
                "CSB V2.1 missing-data popup names the searched data directory")) return 1;

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 0,
                "CSB V2.1 version-only launch intent is invalid without required files")) return 1;

    puts("ok: CSB V2 presentation selection does not bypass CSB required-file launch gating");
    puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 selects CSB media before LOADSAVE.C F0435 dungeon load");
    return 0;
}
