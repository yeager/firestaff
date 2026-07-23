#include "firestaff/dm1/v1/command_highlight_box_disable_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB WIP20210206 CLIKMENU.C F0362_COMMAND_HighlightBoxEnable "
    "MEDIA529:66-80; F0363_COMMAND_HighlightBoxDisable; "
    "DEFS.H G2012_HighlightedZone/G0341_B_HighlightBoxEnabled; "
    "IMAGE.C F0698_InvertBox:231-294. F0363 re-inverts only the "
    "G2012 rectangle previously published by F0362, then clears G0341.";

static int boxes_match(const DM1_V1_CommandHighlightBoxPc34Compat *left,
                       const DM1_V1_CommandHighlightBoxPc34Compat *right)
{
    return left && right && left->left == right->left &&
           left->right == right->right && left->top == right->top &&
           left->bottom == right->bottom;
}

int dm1_v1_command_highlight_box_disable_pc34(
    DM1_V1_CommandHighlightStatePc34Compat *state,
    const DM1_V1_CommandHighlightRenderPlanPc34Compat *enablePlan,
    DM1_V1_CommandHighlightDisablePlanPc34Compat *outPlan)
{
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));

    /* Never invert a fabricated or stale rectangle. */
    if (!state || !enablePlan || !state->highlightBoxEnabled ||
        !enablePlan->accepted || enablePlan->stepCount !=
            DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34 ||
        !boxes_match(&state->highlightedZone, &enablePlan->box)) {
        return 0;
    }

    outPlan->accepted = 1;
    outPlan->zoneIndex = enablePlan->zoneIndex;
    outPlan->box = enablePlan->box;
    outPlan->steps[0] = DM1_V1_COMMAND_HIGHLIGHT_ENABLE_SCREEN_UPDATE_PC34;
    outPlan->steps[1] = DM1_V1_COMMAND_HIGHLIGHT_INVERT_BOX_PC34;
    outPlan->steps[2] = DM1_V1_COMMAND_HIGHLIGHT_DISABLE_SCREEN_UPDATE_PC34;
    outPlan->steps[3] = DM1_V1_COMMAND_HIGHLIGHT_WAIT_VERTICAL_BLANK_PC34;
    outPlan->stepCount = DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34;
    state->highlightBoxEnabled = 0;
    return 1;
}

const char *dm1_v1_command_highlight_box_disable_source_evidence_pc34(void)
{
    return s_source_evidence;
}
