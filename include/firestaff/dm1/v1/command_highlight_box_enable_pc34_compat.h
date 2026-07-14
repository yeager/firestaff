#ifndef FIRESTAFF_DM1_V1_COMMAND_HIGHLIGHT_BOX_ENABLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_COMMAND_HIGHLIGHT_BOX_ENABLE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* PC 3.4 F0638 zone geometry uses inclusive screen coordinates. */
typedef struct DM1_V1_CommandHighlightBoxPc34Compat {
    int left;
    int right;
    int top;
    int bottom;
} DM1_V1_CommandHighlightBoxPc34Compat;

typedef struct DM1_V1_CommandHighlightStatePc34Compat {
    DM1_V1_CommandHighlightBoxPc34Compat highlightedZone;
    int highlightBoxEnabled;
} DM1_V1_CommandHighlightStatePc34Compat;

typedef int (*DM1_V1_CommandHighlightZoneResolverPc34Compat)(
    void *context,
    int zoneIndex,
    DM1_V1_CommandHighlightBoxPc34Compat *out_box);

typedef enum DM1_V1_CommandHighlightRenderStepPc34Compat {
    DM1_V1_COMMAND_HIGHLIGHT_ENABLE_SCREEN_UPDATE_PC34 = 0,
    DM1_V1_COMMAND_HIGHLIGHT_INVERT_BOX_PC34,
    DM1_V1_COMMAND_HIGHLIGHT_DISABLE_SCREEN_UPDATE_PC34,
    DM1_V1_COMMAND_HIGHLIGHT_WAIT_VERTICAL_BLANK_PC34,
    DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34
} DM1_V1_CommandHighlightRenderStepPc34Compat;

typedef struct DM1_V1_CommandHighlightRenderPlanPc34Compat {
    int accepted;
    int zoneIndex;
    DM1_V1_CommandHighlightBoxPc34Compat box;
    DM1_V1_CommandHighlightRenderStepPc34Compat steps[
        DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34];
    int stepCount;
} DM1_V1_CommandHighlightRenderPlanPc34Compat;

/*
 * Implements the MEDIA529 PC 3.4 F0362 route: resolve a source layout zone,
 * retain its exact inclusive rectangle, and emit its inversion transaction.
 */
int dm1_v1_command_highlight_box_enable_pc34(
    DM1_V1_CommandHighlightStatePc34Compat *state,
    int zoneIndex,
    DM1_V1_CommandHighlightZoneResolverPc34Compat resolveZone,
    void *resolverContext,
    DM1_V1_CommandHighlightRenderPlanPc34Compat *outPlan);

const char *dm1_v1_command_highlight_box_enable_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
