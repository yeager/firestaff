#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const char* gameId;
    const char* statusLabel;
    const char* detailLabel;
    const char* pathLabel;
    const char* contractLabel;
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
        state->assetStatus.dm1Available = 1;
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
         "DM1 MEDIA + ENTRANCE + HOC RECEIPTS", 6},
        {"csb", "BOOT READY", "SWSH, TITLE, ENTRANCE, UTILITY", "CSB BOOT PATH",
         "CSB STARTUP CAPTURE RECEIPT", 6},
        {"dm2", "START MENU READY", "TITLE, SAVE MENU, FIRST HUD", "DM2 START MENU",
         "DM2 STARTUP HOST VIEW RECEIPT", 5},
        {"nexus", "TITLE MENU READY", "TITLE, WARNING, SAVE, CHAMPIONS", "NEXUS TITLE MENU",
         "NEXUS FULL START CONSUMER RECEIPT", 6},
        {"theron", "TRACK 02 READY", "TRACK 02, TITLE, STAGE, SOUL ROOM", "THERON TRACK 02",
         "THERON FULL START HOST VIEW RECEIPT", 6},
    };
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    int i;

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
        if (!expect(boot.statusLabel &&
                    strcmp(boot.statusLabel, expected[i].statusLabel) == 0,
                    "status label should match game startup path")) return 1;
        if (!expect(boot.detailLabel &&
                    strcmp(boot.detailLabel, expected[i].detailLabel) == 0,
                    "detail label should match game startup chain")) return 1;
        if (!expect(M12_StartupMenu_GetLaunchGate(&state, i, &gate) == 1,
                    "launch gate should build")) return 1;
        if (!expect(gate.canLaunch == 1,
                    "ready game launch gate should allow launch")) return 1;
        if (!expect(gate.rendererReady == 1 && gate.presentationReady == 1,
                    "ready game launch gate should expose renderer/presentation readiness")) return 1;
        if (!expect(gate.fullStartGraphicsReady == 1 &&
                    gate.startupContractReady == 1,
                    "ready game launch gate should expose full-start contract readiness")) return 1;
        if (!expect(gate.boot.startupStepReadyCount == expected[i].stepCount,
                    "launch gate should carry boot receipt progress")) return 1;
        if (!expect(gate.blockedLabel &&
                    strcmp(gate.blockedLabel, "READY TO LAUNCH") == 0,
                    "ready game launch gate should report ready label")) return 1;
        if (!expect(strcmp(M12_StartupMenu_GetEntryLaunchStatusLabel(&state, i),
                           "READY TO LAUNCH") == 0,
                    "ready game launch status label should come from launch gate")) return 1;
        if (!expect(strcmp(M12_StartupMenu_GetEntryLaunchDetailLabel(&state, i),
                           expected[i].pathLabel) == 0,
                    "ready game launch detail label should name boot path")) return 1;
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
        if (!expect(M12_StartupMenu_GetLaunchGate(&state, 0, &gate) == 1,
                    "auto-version launch gate should build")) return 1;
        if (!expect(gate.autoSelectedVersionIndex == 0,
                    "launch gate should find the first matched version by hash")) return 1;
        if (!expect(gate.canLaunch == 1 && gate.versionReady == 1,
                    "auto-version launch gate should allow verified data")) return 1;
        if (!expect(gate.fullStartGraphicsReady == 1 &&
                    gate.startupContractReady == 1,
                    "auto-version launch gate should publish full-start contract readiness")) return 1;
        if (!expect(gate.boot.versionReady == 1 &&
                    gate.boot.fullStartGraphicsReady == 1 &&
                    gate.boot.startupContractReady == 1,
                    "auto-version launch gate should normalize full-start boot readiness")) return 1;
        if (!expect(gate.boot.startupStepReadyCount == gate.boot.startupStepCount,
                    "auto-version launch gate should complete startup progress")) return 1;
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
