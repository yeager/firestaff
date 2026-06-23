/*
 * DM1 V1 candidate-panel spell-cast suppression runtime probe.
 *
 * ReDMCSB source anchors:
 *   COMMAND.C:2303-2311 in F0380_COMMAND_ProcessQueue_CPSC gates the
 *   C100 spell-area and C111 action-area routes on !G0299 while the
 *   C040 resurrect/reincarnate candidate panel is live.
 *   COMMAND.C:2474-2476 in F0380_COMMAND_ProcessQueue_CPSC stores the
 *   C165..C198 rename command range only after the earlier G0299-owned
 *   panel routes have had first ownership.
 *   REVIVE.C:272-276 in F0280_CHAMPION_AddCandidateChampionToParty sets
 *   G0299 to the appended candidate ordinal, and REVIVE.C:744-799 in
 *   F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel
 *   clears G0299 only on cancel or confirm.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static void set_hall_start_north(M11_GameViewState* game) {
    /* Real DM1 V1 DUNGEON.DAT: (1,2) NORTH front=(1,1) has C127
     * sensor idx=15 data=1 (HALK).  v2.7.22 anchored the front-cell
     * mirror ordinal to the C127 sensorData (ReDMCSB DUNGEON.C:2573
     * + MOVESENS.C:1501-1503 + REVIVE.C F0280).  The OLD corridor
     * pose (1,4) NORTH had only a TextString and no C127 sensor —
     * it returns -1 under the v2.7.22 contract.  The OLD ordinal
     * 2 (HALK-by-old-assumption) is also stale: real (1,2) is
     * ordinal 1, not 2.  Use (1,2) NORTH ordinal 1. */
    game->world.party.mapIndex = 0;
    game->world.party.mapX = 1;
    game->world.party.mapY = 2;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

static int open_game(const char* dataDir,
                     M12_StartupMenuState* menu,
                     M11_GameViewState* game) {
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    return M11_GameView_OpenSelectedMenuEntry(game, menu);
}

static int prepare_candidate_with_spell(M11_GameViewState* game,
                                        int expectedMirrorOrdinal) {
    int ok = 1;
    set_hall_start_north(game);
    ok &= expect_int("front mirror ordinal",
                     M11_GameView_GetFrontMirrorOrdinal(game),
                     expectedMirrorOrdinal);

    /* ReDMCSB COMMAND.C:2303-2311 suppresses spell/action area routes while
     * G0299 is nonzero; preloading a valid Firestaff spell-buffer state makes
     * any missing suppression observable as a cast/clear mutation. */
    ok &= expect_int("open spell panel",
                     M11_GameView_OpenSpellPanel(game), 1);
    ok &= expect_int("enter rune 1",
                     M11_GameView_EnterRune(game, 0), 1);
    ok &= expect_int("enter rune 2",
                     M11_GameView_EnterRune(game, 0), 1);
    ok &= expect_int("spell panel preselect",
                     game->spellPanelOpen, 1);
    ok &= expect_int("spell buffer preselect",
                     game->spellBuffer.runeCount, 2);

    /* ReDMCSB REVIVE.C:272-276 sets G0299 when the candidate is appended;
     * Firestaff mirrors that as candidateMirrorPanelActive plus the
     * candidate ordinal/party-index fields. */
    ok &= expect_int("select candidate",
                     M11_GameView_SelectFrontMirrorCandidate(game), 1);
    ok &= expect_int("candidate panel live",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("candidate ordinal live",
                     game->candidateMirrorOrdinal, expectedMirrorOrdinal);
    ok &= expect_int("candidate party index",
                     game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("candidate appended",
                     game->world.party.championCount, 1);
    ok &= expect_int("candidate inventory live",
                     game->inventoryPanelActive, 1);
    return ok;
}

static int check_spell_cast_input_ignored(M11_GameViewState* game,
                                          int expectedMirrorOrdinal) {
    M11_GameInputResult result;
    uint16_t manaBefore;
    uint32_t tickBefore;
    int ok = 1;

    manaBefore = game->world.party.champions[0].mana.current;
    tickBefore = game->world.gameTick;

    /* ReDMCSB COMMAND.C:2303-2311 requires !G0299 before C100 can reach
     * spell handling; this verifies the Firestaff M12 spell-cast input route
     * is consumed as a no-op while C040 owns the panel. */
    result = M11_GameView_HandleInput(game, M12_MENU_INPUT_SPELL_CAST);
    ok &= expect_int("spell cast ignored",
                     (int)result, (int)M11_GAME_INPUT_IGNORED);
    ok &= expect_int("spell panel still open",
                     game->spellPanelOpen, 1);
    ok &= expect_int("spell buffer preserved",
                     game->spellBuffer.runeCount, 2);
    ok &= expect_int("mana unchanged",
                     game->world.party.champions[0].mana.current, manaBefore);
    ok &= expect_int("tick unchanged",
                     (int)game->world.gameTick, (int)tickBefore);
    ok &= expect_int("candidate panel still live",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("candidate ordinal preserved",
                     game->candidateMirrorOrdinal, expectedMirrorOrdinal);
    ok &= expect_int("candidate party index preserved",
                     game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("candidate still appended",
                     game->world.party.championCount, 1);
    return ok;
}

static int check_spell_clear_input_ignored(M11_GameViewState* game,
                                           int expectedMirrorOrdinal) {
    M11_GameInputResult result;
    int ok = 1;

    /* ReDMCSB COMMAND.C:2474-2476 covers C165..C198 rename commands after
     * the candidate-panel-owned dispatch; Firestaff's clear/recant input must
     * likewise remain below the G0299 candidate-panel gate. */
    result = M11_GameView_HandleInput(game, M12_MENU_INPUT_SPELL_CLEAR);
    ok &= expect_int("spell clear ignored",
                     (int)result, (int)M11_GAME_INPUT_IGNORED);
    ok &= expect_int("spell panel still open after clear",
                     game->spellPanelOpen, 1);
    ok &= expect_int("spell buffer preserved after clear",
                     game->spellBuffer.runeCount, 2);
    ok &= expect_int("candidate panel still live after clear",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("candidate ordinal preserved after clear",
                     game->candidateMirrorOrdinal, expectedMirrorOrdinal);
    ok &= expect_int("candidate still appended after clear",
                     game->world.party.championCount, 1);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    if (!open_game(dataDir, &menu, &game)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        return 1;
    }

    /* v2.7.22: real DM1 V1 Hall of Champions C127 mirror positions
     * use the ReDMCSB DUNGEON.C:2573/2610-2612 front-wall side filter.
     * HALK is visible from (1,2) NORTH ordinal=1; ZED's raw sensor is
     * on (1,4) cell 0, so the source-visible pose is (1,3) SOUTH
     * ordinal=10, not the wrong-wall (1,5) NORTH view.  Use HALK for
     * the candidate-panel suppression regression. */
    if (prepare_candidate_with_spell(&game, 1)) {
        ok &= check_spell_cast_input_ignored(&game, 1);
        ok &= check_spell_clear_input_ignored(&game, 1);
    } else {
        ok = 0;
    }

    printf("probe=firestaff_dm1_v1_candidate_panel_spell_cast_suppression\n");
    printf("sourceEvidence=COMMAND.C:2303-2311 F0380 gates C100/C111 on !G0299; COMMAND.C:2474-2476 leaves C165..C198 behind panel dispatch; REVIVE.C:272-276 and 744-799 set/clear G0299\n");
    printf("candidatePanelSpellCastSuppressionOk=%u\n", ok ? 1u : 0u);

    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
