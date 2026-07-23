#ifndef FIRESTAFF_DM1_V1_COMMAND_HIGHLIGHT_BOX_DISABLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_COMMAND_HIGHLIGHT_BOX_DISABLE_PC34_COMPAT_H

#include "firestaff/dm1/v1/command_highlight_box_enable_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F0363 reverses the F0362 inversion only when it is given the exact raw
 * PC34 layout receipt that opened it.  It owns no replacement pixels.
 */
typedef struct DM1_V1_CommandHighlightDisablePlanPc34Compat {
    int accepted;
    int zoneIndex;
    DM1_V1_CommandHighlightBoxPc34Compat box;
    DM1_V1_CommandHighlightRenderStepPc34Compat steps[
        DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34];
    int stepCount;
} DM1_V1_CommandHighlightDisablePlanPc34Compat;

int dm1_v1_command_highlight_box_disable_pc34(
    DM1_V1_CommandHighlightStatePc34Compat *state,
    const DM1_V1_CommandHighlightRenderPlanPc34Compat *enablePlan,
    DM1_V1_CommandHighlightDisablePlanPc34Compat *outPlan);

const char *dm1_v1_command_highlight_box_disable_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
