#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const char* gameId;
    const char* statusLabel;
    const char* detailLabel;
    const char* pathLabel;
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
        {"dm1", "FULL START READY", "SWSH, TITLE, ENTRANCE, HOC", "DM1 FULL START", 6},
        {"csb", "BOOT READY", "SWSH, TITLE, ENTRANCE, UTILITY", "CSB BOOT PATH", 6},
        {"dm2", "START MENU READY", "TITLE, SAVE MENU, FIRST HUD", "DM2 START MENU", 5},
        {"nexus", "TITLE MENU READY", "TITLE, WARNING, SAVE, CHAMPIONS", "NEXUS TITLE MENU", 6},
        {"theron", "TRACK 02 READY", "TRACK 02, TITLE, STAGE, SOUL ROOM", "THERON TRACK 02", 6},
    };
    M12_StartupMenuState state;
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
        if (!expect(boot.startupStepCount == expected[i].stepCount,
                    "startup step count should match the game boot path")) return 1;
        if (!expect(boot.startupStepReadyCount == expected[i].stepCount,
                    "startup ready count should match total when ready")) return 1;
        if (!expect(boot.nextStepLabel && strcmp(boot.nextStepLabel, "READY") == 0,
                    "ready boot receipt should report READY next step")) return 1;
        if (!expect(boot.startupPathLabel &&
                    strcmp(boot.startupPathLabel, expected[i].pathLabel) == 0,
                    "boot path label should match game")) return 1;
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

    puts("ok: all game launcher boot-readiness receipts expose full-start progress and labels");
    return 0;
}
