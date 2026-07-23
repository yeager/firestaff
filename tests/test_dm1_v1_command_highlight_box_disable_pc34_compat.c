#include "firestaff/dm1/v1/command_highlight_box_disable_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static void set_enabled_plan(DM1_V1_CommandHighlightRenderPlanPc34Compat *plan)
{
    memset(plan, 0, sizeof(*plan));
    plan->accepted = 1;
    plan->zoneIndex = 68; /* C068_ZONE_TURN_LEFT. */
    plan->box.left = 234;
    plan->box.right = 252;
    plan->box.top = 125;
    plan->box.bottom = 145;
    plan->stepCount = DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34;
}

int main(void)
{
    DM1_V1_CommandHighlightStatePc34Compat state;
    DM1_V1_CommandHighlightRenderPlanPc34Compat enablePlan;
    DM1_V1_CommandHighlightDisablePlanPc34Compat disablePlan;
    int i;

    memset(&state, 0, sizeof(state));
    set_enabled_plan(&enablePlan);
    CHECK(strstr(dm1_v1_command_highlight_box_disable_source_evidence_pc34(),
                 "F0363_COMMAND_HighlightBoxDisable") != NULL);
    CHECK(!dm1_v1_command_highlight_box_disable_pc34(NULL, &enablePlan,
                                                      &disablePlan));
    CHECK(!dm1_v1_command_highlight_box_disable_pc34(&state, NULL,
                                                      &disablePlan));
    CHECK(!dm1_v1_command_highlight_box_disable_pc34(&state, &enablePlan,
                                                      NULL));

    state.highlightBoxEnabled = 1;
    state.highlightedZone = enablePlan.box;
    CHECK(dm1_v1_command_highlight_box_disable_pc34(&state, &enablePlan,
                                                     &disablePlan));
    CHECK(!state.highlightBoxEnabled);
    CHECK(disablePlan.accepted && disablePlan.zoneIndex == 68);
    CHECK(disablePlan.box.left == 234 && disablePlan.box.right == 252 &&
          disablePlan.box.top == 125 && disablePlan.box.bottom == 145);
    CHECK(disablePlan.stepCount ==
          DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34);
    for (i = 0; i < disablePlan.stepCount; ++i) {
        CHECK(disablePlan.steps[i] ==
              (DM1_V1_CommandHighlightRenderStepPc34Compat)i);
    }

    /* A second release and any drifted raw C0xx layout receipt are rejected. */
    CHECK(!dm1_v1_command_highlight_box_disable_pc34(&state, &enablePlan,
                                                      &disablePlan));
    state.highlightBoxEnabled = 1;
    state.highlightedZone = enablePlan.box;
    enablePlan.box.right = 253;
    CHECK(!dm1_v1_command_highlight_box_disable_pc34(&state, &enablePlan,
                                                      &disablePlan));
    CHECK(state.highlightBoxEnabled);
    CHECK(!disablePlan.accepted && disablePlan.stepCount == 0);

    printf("test_dm1_v1_command_highlight_box_disable_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
