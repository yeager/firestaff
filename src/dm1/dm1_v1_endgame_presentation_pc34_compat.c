#include "dm1_v1_endgame_presentation_pc34_compat.h"

#include "dm1_v1_endgame_layout_pc34_compat.h"

static int dm1_v1_endgame_point_in_rect_pc34(int x, int y,
                                               const DM1_V1_EndgameRectPc34* rect) {
    return rect && x >= rect->x && y >= rect->y &&
           x < rect->x + rect->w && y < rect->y + rect->h;
}

void dm1_v1_endgame_presentation_decide_pc34(
    const DM1_V1_EndgamePresentationInputPc34* input,
    DM1_V1_EndgamePresentationDecisionPc34* output) {
    DM1_V1_EndgameRectPc34 restartBox;
    DM1_V1_EndgameRectPc34 quitBox;

    if (!output) return;
    output->presentationReady = 0;
    output->controlsVisible = 0;
    output->action = DM1_V1_ENDGAME_PRESENTATION_ACTION_NONE_PC34;
    if (!input || !input->gameWon) return;

    /* ReDMCSB ENDGAME.C F0446 lines 939-961 holds the final F0444
     * handoff until victory text and its delay have completed. */
    if (input->finalHandoffReady ||
        (!input->endgameCalledWithTrue && input->finalDelayTicks <= 0 &&
         input->fuseDelayTicks <= 0 && input->fuseDelayRemainingTicks <= 0 &&
         input->textMessageDelayTicks <= 0)) {
        output->presentationReady = 1;
    }
    if (!output->presentationReady) return;

    /* ReDMCSB ENDGAME.C F0444 lines 485-549 installs G0446 only while
     * G0524_B_RestartGameAllowed is set. */
    output->controlsVisible = input->restartAllowed ? 1 : 0;
    if (input->backRequested) {
        output->action = DM1_V1_ENDGAME_PRESENTATION_ACTION_RETURN_TO_MENU_PC34;
        return;
    }
    if (!input->pointerPressed || !output->controlsVisible) return;

    (void)dm1_v1_endgame_restart_box_pc34(0, &restartBox);
    if (dm1_v1_endgame_point_in_rect_pc34(input->pointerX, input->pointerY,
                                           &restartBox)) {
        output->action = DM1_V1_ENDGAME_PRESENTATION_ACTION_RESTART_PC34;
        return;
    }
    (void)dm1_v1_endgame_quit_box_pc34(0, &quitBox);
    if (dm1_v1_endgame_point_in_rect_pc34(input->pointerX, input->pointerY,
                                           &quitBox)) {
        output->action = DM1_V1_ENDGAME_PRESENTATION_ACTION_RETURN_TO_MENU_PC34;
    }
}

const char* dm1_v1_endgame_presentation_evidence_pc34(void) {
    return "ReDMCSB ENDGAME.C F0444 lines 440-590 presents THE END, then "
           "installs restart/quit input only with G0524_B_RestartGameAllowed; "
           "F0446 lines 939-961 delays the F0444(TRUE) handoff.";
}
