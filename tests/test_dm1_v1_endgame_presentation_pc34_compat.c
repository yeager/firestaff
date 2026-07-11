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

int main(void) {
    DM1_V1_EndgamePresentationInputPc34 input;
    DM1_V1_EndgamePresentationDecisionPc34 output;

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

    return failures ? 1 : 0;
}
