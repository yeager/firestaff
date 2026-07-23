#include "dm1_v1_endgame_presentation_pc34_compat.h"

#include "dm1_v1_endgame_layout_pc34_compat.h"
#include <string.h>

enum {
    DM1_V1_ENDGAME_THE_END_GRAPHIC_PC34 = 6,
    DM1_V1_ENDGAME_CHAMPION_MIRROR_GRAPHIC_PC34 = 346,
    DM1_V1_ENDGAME_CHAMPION_PORTRAITS_GRAPHIC_PC34 = 26,
    DM1_V1_ENDGAME_VICTORY_MUSIC_TRACK_PC34 = 2,
    DM1_V1_ENDGAME_CREDITS_PALETTE_SIZE_PC34 = 16,
    DM1_V1_ENDGAME_CREDITS_PALETTE_FIRST_PC34 = 0x009,
    DM1_V1_ENDGAME_CREDITS_PALETTE_LAST_PC34 = 0xFFC
};

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

int dm1_v1_endgame_final_presentation_receipt_pc34(
    const DM1_V1_EndgameFinalPresentationInputPc34* input,
    DM1_V1_EndgameFinalPresentationReceiptPc34* output) {
    if (!output) return 0;
    memset(output, 0, sizeof(*output));
    output->sourceEvidence = dm1_v1_endgame_presentation_evidence_pc34();
    if (!input || !input->gameWon) return 1;

    output->finalScreenReady =
        (input->finalPresentationReady && input->endgameCalledWithTrue) ? 1 : 0;
    output->gameplayInputBlocked = input->gameWon ? 1 : 0;
    output->controlsVisible =
        (output->finalScreenReady && input->restartAllowed) ? 1 : 0;
    output->f0445ReplayDrained =
        (input->replayEventCount > 0 &&
         input->replayCursor >= input->replayEventCount &&
         input->replayFrameRemainingTicks <= 0) ? 1 : 0;
    output->finalDelayDrained =
        (input->finalDelayTicks > 0 &&
         input->fuseDelayRemainingTicks <= 0) ? 1 : 0;
    output->textMessagesCompleted =
        (input->textMessageDelayTicks > 0 &&
         input->fuseDelayRemainingTicks <= 0) ? 1 : 0;
    output->saveTerminalStateLocked =
        (output->finalScreenReady && output->f0445ReplayDrained &&
         output->finalDelayDrained) ? 1 : 0;
    output->originalMusicRequested =
        (input->expectedVictoryMusicId == DM1_V1_ENDGAME_VICTORY_MUSIC_TRACK_PC34 &&
         input->requestedMusicTrackId == input->expectedVictoryMusicId &&
         input->musicPlayRequestCount > 0) ? 1 : 0;
    output->originalGraphicsDatRoute =
        (input->assetsAvailable && input->f0444MaterialBound &&
         input->theEndGraphicId == DM1_V1_ENDGAME_THE_END_GRAPHIC_PC34 &&
         input->championMirrorGraphicId ==
             DM1_V1_ENDGAME_CHAMPION_MIRROR_GRAPHIC_PC34 &&
         input->championPortraitsGraphicId ==
             DM1_V1_ENDGAME_CHAMPION_PORTRAITS_GRAPHIC_PC34 &&
         input->theEndX == 120 && input->theEndY == 95 &&
         input->theEndW == 80 && input->theEndH == 14) ? 1 : 0;
    output->f0444MaterialBound = input->f0444MaterialBound ? 1 : 0;
    output->creditsPaletteRoute =
        (input->creditsPaletteSize ==
             DM1_V1_ENDGAME_CREDITS_PALETTE_SIZE_PC34 &&
         input->creditsPaletteFirstEntry ==
             DM1_V1_ENDGAME_CREDITS_PALETTE_FIRST_PC34 &&
         input->creditsPaletteLastEntry ==
             DM1_V1_ENDGAME_CREDITS_PALETTE_LAST_PC34) ? 1 : 0;

    output->theEndGraphicId = input->theEndGraphicId;
    output->championMirrorGraphicId = input->championMirrorGraphicId;
    output->championPortraitsGraphicId = input->championPortraitsGraphicId;
    output->theEndX = input->theEndX;
    output->theEndY = input->theEndY;
    output->theEndW = input->theEndW;
    output->theEndH = input->theEndH;
    output->creditsPaletteSize = input->creditsPaletteSize;
    output->creditsPaletteFirstEntry = input->creditsPaletteFirstEntry;
    output->creditsPaletteLastEntry = input->creditsPaletteLastEntry;
    output->valid = output->finalScreenReady &&
                    output->gameplayInputBlocked &&
                    output->saveTerminalStateLocked &&
                    output->originalMusicRequested &&
                    output->originalGraphicsDatRoute &&
                    output->creditsPaletteRoute;
    return 1;
}

const char* dm1_v1_endgame_presentation_evidence_pc34(void) {
    return "ReDMCSB ENDGAME.C F0444 lines 440-590 presents THE END, then "
           "installs restart/quit input only with G0524_B_RestartGameAllowed; "
           "F0446 lines 924-961 plays C2_MUSIC_GAME_WON, prints ordered "
           "victory TextString messages, delays, clears restart, and delays "
           "the F0444(TRUE) handoff; DATA.C G0012/G0015/G0016/G0019 and "
           "DEFS.H C006/C026/C346 provide the original endgame graphics and "
           "credits palette route.";
}
