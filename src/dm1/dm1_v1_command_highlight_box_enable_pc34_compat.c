#include "firestaff/dm1/v1/command_highlight_box_enable_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB WIP20210206 CLIKMENU.C F0362_COMMAND_HighlightBoxEnable "
    "MEDIA529:66-80; F0665_F0362_sub:10-32; COORD.C F0638_GetZone; "
    "DEFS.H G2012_HighlightedZone/G0341_B_HighlightBoxEnabled. "
    "F0665 enables screen updates, copies the resolved inclusive zone, "
    "inverts it, enables highlight state, disables screen updates, then "
    "waits one vertical blank.";

int dm1_v1_command_highlight_box_enable_pc34(
    DM1_V1_CommandHighlightStatePc34Compat *state,
    int zoneIndex,
    DM1_V1_CommandHighlightZoneResolverPc34Compat resolveZone,
    void *resolverContext,
    DM1_V1_CommandHighlightRenderPlanPc34Compat *outPlan)
{
    DM1_V1_CommandHighlightBoxPc34Compat box;

    if (!state || !resolveZone || !outPlan) {
        return 0;
    }

    memset(outPlan, 0, sizeof(*outPlan));
    if (!resolveZone(resolverContext, zoneIndex, &box)) {
        return 0;
    }

    /* F0007 copies F0638's four source coordinates unchanged into G2012. */
    state->highlightedZone = box;
    state->highlightBoxEnabled = 1;

    outPlan->accepted = 1;
    outPlan->zoneIndex = zoneIndex;
    outPlan->box = box;
    outPlan->steps[0] = DM1_V1_COMMAND_HIGHLIGHT_ENABLE_SCREEN_UPDATE_PC34;
    outPlan->steps[1] = DM1_V1_COMMAND_HIGHLIGHT_INVERT_BOX_PC34;
    outPlan->steps[2] = DM1_V1_COMMAND_HIGHLIGHT_DISABLE_SCREEN_UPDATE_PC34;
    outPlan->steps[3] = DM1_V1_COMMAND_HIGHLIGHT_WAIT_VERTICAL_BLANK_PC34;
    outPlan->stepCount = DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34;
    return 1;
}

const char *dm1_v1_command_highlight_box_enable_source_evidence_pc34(void)
{
    return s_source_evidence;
}
