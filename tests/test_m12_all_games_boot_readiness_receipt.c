#include "menu_startup_m12.h"
#include "nexus_v1_launcher.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const char* gameId;
    const char* statusLabel;
    const char* detailLabel;
    const char* pathLabel;
    const char* contractLabel;
    const char* captureLabel;
    int stepCount;
} ExpectedBootReceipt;

static int expect(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static void mark_game_ready(M12_StartupMenuState* state, int slot, const char* gameId) {
    if (!state || slot < 0 || slot >= M12_CONFIG_GAME_COUNT || !gameId) {
        return;
    }
    state->assetStatus.originalFileCandidateFound = 1;
    if (strcmp(gameId, "dm1") == 0) {
        M12_AssetRequiredFileStatus* graphics;
        M12_AssetRequiredFileStatus* dungeon;
        state->assetStatus.dm1Available = 1;
        state->assetStatus.requiredFileCounts[slot] = 2U;
        graphics = &state->assetStatus.requiredFiles[slot][0];
        memset(graphics, 0, sizeof(*graphics));
        graphics->gameId = "dm1";
        graphics->roleId = "graphics";
        graphics->label = "GRAPHICS.DAT";
        graphics->required = 1;
        graphics->matched = 1;
        snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
                 "/tmp/firestaff-test/dm1/GRAPHICS.DAT");
        snprintf(graphics->matchedHash, sizeof(graphics->matchedHash),
                 "fa6b1aa29e191418713bf2cda93d962e");
        dungeon = &state->assetStatus.requiredFiles[slot][1];
        memset(dungeon, 0, sizeof(*dungeon));
        dungeon->gameId = "dm1";
        dungeon->roleId = "dungeon";
        dungeon->label = "DUNGEON.DAT";
        dungeon->required = 1;
        dungeon->matched = 1;
        snprintf(dungeon->matchedPath, sizeof(dungeon->matchedPath),
                 "/tmp/firestaff-test/dm1/DUNGEON.DAT");
        snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash),
                 "766450c940651fc021c92fe5d0d0b3a6");
    } else if (strcmp(gameId, "csb") == 0) {
        state->assetStatus.csbAvailable = 1;
    } else if (strcmp(gameId, "dm2") == 0) {
        state->assetStatus.dm2Available = 1;
    } else if (strcmp(gameId, "nexus") == 0) {
        state->assetStatus.nexusAvailable = 1;
    } else if (strcmp(gameId, "theron") == 0) {
        state->assetStatus.theronAvailable = 1;
    }
    state->assetStatus.versions[slot][0].gameId = gameId;
    state->assetStatus.versions[slot][0].versionId = "synthetic-ready";
    state->assetStatus.versions[slot][0].label = "Synthetic ready";
    state->assetStatus.versions[slot][0].shortLabel = "READY";
    state->assetStatus.versions[slot][0].matched = 1;
    state->entries[slot].available = 1;
    state->gameOptions[slot].versionIndex = 0;
}

int main(void) {
    static const ExpectedBootReceipt expected[] = {
        {"dm1", "FULL START READY", "SWSH, TITLE, ENTRANCE, HOC", "DM1 FULL START",
         "DM1 MEDIA + ENTRANCE + HOC RECEIPTS", "DM1 HOC RENDER CAPTURE PROOF", 7},
        {"csb", "BOOT READY", "SWSH, TITLE, ENTRANCE, UTILITY", "CSB BOOT PATH",
         "CSB STARTUP CAPTURE RECEIPT", "CSB TITLE + HUD CAPTURE PROOF", 7},
        {"dm2", "START MENU READY", "TITLE, SAVE MENU, FIRST HUD", "DM2 START MENU",
         "DM2 STARTUP HOST VIEW RECEIPT", "DM2 TITLE TIMING CAPTURE PROOF", 6},
        {"nexus", "TITLE MENU READY", "TITLE, WARNING, SAVE, CHAMPIONS", "NEXUS TITLE MENU",
         "NEXUS HOST-CALLER/FULL-START PACKAGE RECEIPTS", "NEXUS TIMING CAPTURE PROOF", 7},
        {"theron", "TRACK 02 READY", "TRACK 02, TITLE, STAGE, SOUL ROOM", "THERON TRACK 02",
         "THERON FULL START HOST VIEW RECEIPT", "THERON TRACK 02 REAL GRAPHICS PROOF", 7},
    };
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    int i;
    const unsigned int fullMask = M12_STARTUP_BOOT_STEP_DATA |
                                  M12_STARTUP_BOOT_STEP_VERSION |
                                  M12_STARTUP_BOOT_STEP_STARTUP_MENU |
                                  M12_STARTUP_BOOT_STEP_FULL_GRAPHICS |
                                  M12_STARTUP_BOOT_STEP_CONTRACT |
                                  M12_STARTUP_BOOT_STEP_CAPTURE;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    state.view = M12_MENU_VIEW_MAIN;
    for (i = 0; i < M12_CONFIG_GAME_COUNT; ++i) {
        M12_StartupBootReadiness boot;
        M12_StartupLaunchGate gate;
        mark_game_ready(&state, i, expected[i].gameId);
        if (!expect(M12_StartupMenu_GetBootReadiness(&state, i, &boot) == 1,
                    "boot readiness receipt should build")) return 1;
        if (!expect(boot.handled == 1, "game entry should be handled")) return 1;
        if (!expect(boot.supported == 1, "game should be supported by startup receipt")) return 1;
        if (!expect(boot.dataReady == 1, "game data should be ready")) return 1;
        if (!expect(boot.versionReady == 1, "selected version should be ready")) return 1;
        if (!expect(boot.fullStartGraphicsReady == 1, "full-start graphics should be ready")) return 1;
        if (!expect(boot.startupContractExpected == 1,
                    "full-start contract should be expected")) return 1;
        if (!expect(boot.startupContractReady == 1,
                    "full-start contract should be ready with verified startup")) return 1;
        if (!expect(boot.packagedCaptureExpected == 1,
                    "packaged capture proof should be expected")) return 1;
        if (!expect(boot.packagedCaptureReady == 1,
                    "packaged capture proof should be ready with verified startup")) return 1;
        if (strcmp(expected[i].gameId, "dm1") == 0) {
            if (!expect(boot.dm1HoCRealAssetCaptureReady == 1 &&
                        boot.dm1HoCMacWindowCaptureReady == 1 &&
                        boot.dm1HoCReleaseAppCaptureReady == 1 &&
                        boot.dm1HoCHostCaptureRouteReady == 1 &&
                        boot.dm1HoCReleaseCaptureOwnershipReady == 1 &&
                        boot.dm1HoCLaunchPathReady == 1 &&
                        boot.dm1HoCReceiptOnlyConsumerReady == 1 &&
                        boot.dm1HoCNoHostFallbackVisualsReady == 1 &&
                        boot.dm1HoCLowerLevelHelpersReady == 1 &&
                        boot.dm1HoCHostDrawUsesOwnedReceiptReady == 1 &&
                        boot.dm1HoCHostDrawConsumesBackingAssetReady == 1 &&
                        boot.dm1HoCHostDrawRejectsBackingFallbackReady == 1 &&
                        boot.dm1HoCHoCAssetCaptureReady == 1 &&
                        boot.dm1HoCHostWindowCaptureReady == 1 &&
                        boot.dm1HoCOpenedEntranceFrameReady == 1 &&
                        boot.dm1HoCHallMirrorOverlayReady == 1 &&
                        boot.dm1HoCBlockedEnterUntilChampionReady == 1 &&
                        boot.dm1HoCRenderCommandCount == 3,
                        "DM1 M12 boot readiness should consume HoC release/app capture ownership receipt")) return 1;
        }
        if (!expect(boot.expectedStepMask == fullMask,
                    "boot receipt should expose the full expected startup proof mask")) return 1;
        if (!expect(boot.readyStepMask == fullMask,
                    "boot receipt should expose the full ready startup proof mask")) return 1;
        if (!expect(boot.blockedStepMask == 0u,
                    "ready boot receipt should have no blocked startup proof steps")) return 1;
        if (!expect(boot.startupStepCount == expected[i].stepCount,
                    "startup step count should match the game boot path")) return 1;
        if (!expect(boot.startupStepReadyCount == expected[i].stepCount,
                    "startup ready count should match total when ready")) return 1;
        if (!expect(boot.nextStepLabel && strcmp(boot.nextStepLabel, "READY") == 0,
                    "ready boot receipt should report READY next step")) return 1;
        if (!expect(boot.startupPathLabel &&
                    strcmp(boot.startupPathLabel, expected[i].pathLabel) == 0,
                    "boot path label should match game")) return 1;
        if (!expect(boot.startupContractLabel &&
                    strcmp(boot.startupContractLabel, expected[i].contractLabel) == 0,
                    "startup contract label should match game-owned receipt")) return 1;
        if (!expect(boot.packagedCaptureLabel &&
                    strcmp(boot.packagedCaptureLabel, expected[i].captureLabel) == 0,
                    "packaged capture label should match game startup proof")) return 1;
        if (!expect(boot.activeProofLabel &&
                    strcmp(boot.activeProofLabel, expected[i].captureLabel) == 0,
                    "active proof label should name packaged capture when ready")) return 1;
        if (!expect(strcmp(M12_StartupMenu_GetEntryCaptureProofLabel(&state, i),
                           expected[i].captureLabel) == 0,
                    "public capture proof label should match game startup proof")) return 1;
        if (!expect(boot.statusLabel &&
                    strcmp(boot.statusLabel, expected[i].statusLabel) == 0,
                    "status label should match game startup path")) return 1;
        if (!expect(boot.detailLabel &&
                    strcmp(boot.detailLabel, expected[i].detailLabel) == 0,
                    "detail label should match game startup chain")) return 1;
        if (strcmp(expected[i].gameId, "nexus") == 0) {
            if (!expect(boot.packagedCaptureRoute ==
                            NEXUS_V1_STARTUP_CAPTURE_TITLE,
                        "Nexus M12 boot receipt should expose package capture route")) return 1;
            if (!expect(boot.packagedCaptureFirstDrawKind ==
                            NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND,
                        "Nexus M12 boot receipt should expose first capture draw kind")) return 1;
            if (!expect(boot.packagedCaptureCommandCount == 1,
                        "Nexus M12 boot receipt should expose capture command count")) return 1;
            if (!expect(boot.packagedCaptureWarningFrames == 48 &&
                        boot.packagedCaptureTitleReadyFrame == 102 &&
                        boot.packagedCaptureTitleFrameMax == 102 &&
                        boot.packagedCaptureTitlePromptVisible == 1,
                        "Nexus M12 boot receipt should expose title/warning timing")) return 1;
        }
        if (!expect(M12_StartupMenu_GetLaunchGate(&state, i, &gate) == 1,
                    "launch gate should build")) return 1;
        if (!expect(gate.canLaunch == 1,
                    "ready game launch gate should allow launch")) return 1;
        if (!expect(gate.rendererReady == 1 && gate.presentationReady == 1,
                    "ready game launch gate should expose renderer/presentation readiness")) return 1;
        if (!expect(gate.fullStartGraphicsReady == 1 &&
                    gate.startupContractReady == 1 &&
                    gate.packagedCaptureReady == 1,
                    "ready game launch gate should expose full-start capture readiness")) return 1;
        if (!expect(gate.boot.startupStepReadyCount == expected[i].stepCount,
                    "launch gate should carry boot receipt progress")) return 1;
        if (!expect(gate.blockedLabel &&
                    strcmp(gate.blockedLabel, "READY TO LAUNCH") == 0,
                    "ready game launch gate should report ready label")) return 1;
        if (!expect(strcmp(M12_StartupMenu_GetEntryLaunchStatusLabel(&state, i),
                           "READY TO LAUNCH") == 0,
                    "ready game launch status label should come from launch gate")) return 1;
        if (!expect(strcmp(M12_StartupMenu_GetEntryLaunchDetailLabel(&state, i),
                           expected[i].captureLabel) == 0,
                    "ready game launch detail label should name active capture proof")) return 1;
    }
    if (!expect(strcmp(M12_StartupMenu_GetDataStatusValue(&state), "5 GAMES READY") == 0,
                "scan feedback should count launch-gated full-start-ready games")) return 1;

    state.activatedIndex = 0;
    state.gameOptions[0].versionIndex = 1;
    state.assetStatus.versions[0][1].gameId = "dm1";
    state.assetStatus.versions[0][1].versionId = "synthetic-missing";
    state.assetStatus.versions[0][1].label = "Synthetic missing";
    state.assetStatus.versions[0][1].shortLabel = "MISS";
    state.assetStatus.versions[0][1].matched = 0;
    {
        M12_StartupBootReadiness boot;
        M12_StartupLaunchGate gate;
        if (!expect(M12_StartupMenu_GetBootReadiness(&state, 0, &boot) == 1,
                    "selected missing-version boot receipt should build")) return 1;
        if (!expect(boot.versionReady == 0 && boot.fullStartGraphicsReady == 0,
                    "raw boot receipt should report selected missing version")) return 1;
        if (!expect((boot.readyStepMask & M12_STARTUP_BOOT_STEP_DATA) != 0u &&
                    (boot.readyStepMask & M12_STARTUP_BOOT_STEP_VERSION) == 0u,
                    "raw boot receipt should expose data-ready/version-blocked mask")) return 1;
        if (!expect((boot.blockedStepMask & M12_STARTUP_BOOT_STEP_VERSION) != 0u &&
                    (boot.blockedStepMask & M12_STARTUP_BOOT_STEP_CAPTURE) != 0u,
                    "raw boot receipt should expose blocked version/capture proof bits")) return 1;
        if (!expect(boot.activeProofLabel &&
                    strcmp(boot.activeProofLabel, "SELECTED VERSION") == 0,
                    "raw boot receipt active proof should name selected version")) return 1;
        if (!expect(M12_StartupMenu_GetLaunchGate(&state, 0, &gate) == 1,
                    "auto-version launch gate should build")) return 1;
        if (!expect(gate.autoSelectedVersionIndex == 0,
                    "launch gate should find the first matched version by hash")) return 1;
        if (!expect(gate.canLaunch == 1 && gate.versionReady == 1,
                    "auto-version launch gate should allow verified data")) return 1;
        if (!expect(gate.fullStartGraphicsReady == 1 &&
                    gate.startupContractReady == 1 &&
                    gate.packagedCaptureReady == 1,
                    "auto-version launch gate should publish full-start capture readiness")) return 1;
        if (!expect(gate.boot.versionReady == 1 &&
                    gate.boot.fullStartGraphicsReady == 1 &&
                    gate.boot.startupContractReady == 1 &&
                    gate.boot.packagedCaptureReady == 1,
                    "auto-version launch gate should normalize full-start boot readiness")) return 1;
        if (!expect(gate.boot.startupStepReadyCount == gate.boot.startupStepCount,
                    "auto-version launch gate should complete startup progress")) return 1;
        if (!expect(gate.boot.readyStepMask == fullMask &&
                    gate.boot.blockedStepMask == 0u,
                    "auto-version launch gate should normalize startup proof masks")) return 1;
        intent = M12_StartupMenu_GetLaunchIntent(&state);
        if (!expect(intent.valid == 1 && intent.options.versionIndex == 0,
                    "launch intent should use the auto-selected matched version")) return 1;
    }

    state.settings.rendererBackendIndex = M12_RENDERER_BACKEND_OPENGL;
    if (!expect(M12_StartupMenu_RendererBackendAvailable(
                    state.settings.rendererBackendIndex) == 0,
                "test renderer backend should be unavailable")) return 1;
    if (!expect(strcmp(M12_StartupMenu_GetDataStatusValue(&state), "FILES FOUND") == 0,
                "scan feedback should not call data-only games ready when launch gate blocks")) return 1;

    puts("ok: all game launcher boot-readiness receipts expose full-start progress and labels");
    return 0;
}
