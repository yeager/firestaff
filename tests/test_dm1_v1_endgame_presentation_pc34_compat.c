#include <stdio.h>
#include <string.h>

#include "dm1_v1_endgame_presentation_pc34_compat.h"

static int failures;

static void check(const char* name, int condition) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

static DM1_V1_EndgamePresentationInputPc34 ready_input(void) {
    DM1_V1_EndgamePresentationInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.gameWon = 1;
    input.finalHandoffReady = 1;
    return input;
}

static DM1_V1_EndgameFinalPresentationInputPc34 final_ready_input(void) {
    DM1_V1_EndgameFinalPresentationInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.gameWon = 1;
    input.finalPresentationReady = 1;
    input.endgameCalledWithTrue = 1;
    input.replayCursor = 45;
    input.replayEventCount = 45;
    input.replayFrameRemainingTicks = 0;
    input.fuseDelayRemainingTicks = 0;
    input.textMessageDelayTicks = 1560;
    input.finalDelayTicks = 600;
    input.requestedMusicTrackId = 2;
    input.expectedVictoryMusicId = 2;
    input.musicPlayRequestCount = 1;
    input.assetsAvailable = 1;
    input.f0444MaterialBound = 1;
    input.theEndGraphicId = 6;
    input.championMirrorGraphicId = 346;
    input.championPortraitsGraphicId = 26;
    input.theEndX = 120;
    input.theEndY = 95;
    input.theEndW = 80;
    input.theEndH = 14;
    input.creditsPaletteSize = 16;
    input.creditsPaletteFirstEntry = 0x009;
    input.creditsPaletteLastEntry = 0xFFC;
    return input;
}

int main(void) {
    DM1_V1_EndgamePresentationInputPc34 input;
    DM1_V1_EndgamePresentationDecisionPc34 output;
    DM1_V1_EndgameFinalPresentationInputPc34 finalInput;
    DM1_V1_EndgameFinalPresentationReceiptPc34 receipt;

    memset(&input, 0, sizeof(input));
    dm1_v1_endgame_presentation_decide_pc34(&input, &output);
    check("non-win is not presentable", !output.presentationReady);

    input = ready_input();
    input.finalHandoffReady = 0;
    input.endgameCalledWithTrue = 1;
    input.fuseDelayRemainingTicks = 1;
    dm1_v1_endgame_presentation_decide_pc34(&input, &output);
    check("F0446 delay blocks presentation", !output.presentationReady);

    input = ready_input();
    dm1_v1_endgame_presentation_decide_pc34(&input, &output);
    check("handoff makes presentation ready", output.presentationReady);
    check("restart controls remain hidden", !output.controlsVisible);

    input.restartAllowed = 1;
    input.pointerPressed = 1;
    input.pointerX = 110;
    input.pointerY = 150;
    dm1_v1_endgame_presentation_decide_pc34(&input, &output);
    check("restart controls become visible", output.controlsVisible);
    check("restart click requests restart",
          output.action == DM1_V1_ENDGAME_PRESENTATION_ACTION_RESTART_PC34);

    input.pointerX = 170;
    input.pointerY = 178;
    dm1_v1_endgame_presentation_decide_pc34(&input, &output);
    check("quit click returns to menu",
          output.action == DM1_V1_ENDGAME_PRESENTATION_ACTION_RETURN_TO_MENU_PC34);

    input.pointerPressed = 0;
    input.backRequested = 1;
    dm1_v1_endgame_presentation_decide_pc34(&input, &output);
    check("back returns only from presentable endgame",
          output.action == DM1_V1_ENDGAME_PRESENTATION_ACTION_RETURN_TO_MENU_PC34);
    check("source evidence is present",
          strstr(dm1_v1_endgame_presentation_evidence_pc34(), "F0444") != NULL);

    finalInput = final_ready_input();
    check("final presentation receipt builds",
          dm1_v1_endgame_final_presentation_receipt_pc34(
              &finalInput, &receipt) == 1);
    check("final presentation receipt is valid", receipt.valid);
    check("terminal save state is locked", receipt.saveTerminalStateLocked);
    check("gameplay input is blocked", receipt.gameplayInputBlocked);
    check("F0445 replay is drained", receipt.f0445ReplayDrained);
    check("final delay is drained", receipt.finalDelayDrained);
    check("text messages are complete", receipt.textMessagesCompleted);
    check("original C2 music was requested", receipt.originalMusicRequested);
    check("original GRAPHICS.DAT route is required", receipt.originalGraphicsDatRoute);
    check("F0444 source material is bound", receipt.f0444MaterialBound);
    check("credits palette route is source-bound", receipt.creditsPaletteRoute);
    check("THE END graphic id is C006", receipt.theEndGraphicId == 6);
    check("endgame champion mirror id is C346",
          receipt.championMirrorGraphicId == 346);
    check("portrait atlas id is C026", receipt.championPortraitsGraphicId == 26);
    check("THE END G0012 rect is preserved",
          receipt.theEndX == 120 && receipt.theEndY == 95 &&
          receipt.theEndW == 80 && receipt.theEndH == 14);
    check("credits palette endpoints are preserved",
          receipt.creditsPaletteFirstEntry == 0x009 &&
          receipt.creditsPaletteLastEntry == 0xFFC);

    finalInput = final_ready_input();
    finalInput.f0444MaterialBound = 0;
    check("unbound F0444 material prevents valid final receipt",
          dm1_v1_endgame_final_presentation_receipt_pc34(
              &finalInput, &receipt) == 1 && !receipt.valid &&
          !receipt.originalGraphicsDatRoute);

    finalInput = final_ready_input();
    finalInput.assetsAvailable = 0;
    check("missing original assets prevents valid final receipt",
          dm1_v1_endgame_final_presentation_receipt_pc34(
              &finalInput, &receipt) == 1 && !receipt.valid &&
          !receipt.originalGraphicsDatRoute);

    finalInput = final_ready_input();
    finalInput.requestedMusicTrackId = 1;
    check("wrong music track prevents valid final receipt",
          dm1_v1_endgame_final_presentation_receipt_pc34(
              &finalInput, &receipt) == 1 && !receipt.valid &&
          !receipt.originalMusicRequested);

    return failures ? 1 : 0;
}
