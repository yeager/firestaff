#include "firestaff/dm1/v1/command_highlight_box_enable_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int s_assertions;
static int s_failures;

#define CHECK(condition) do { \
    ++s_assertions; \
    if (!(condition)) { \
        ++s_failures; \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    } \
} while (0)

typedef struct ZoneFixture {
    int calls;
} ZoneFixture;

static int resolve_zone(void *context,
                        int zoneIndex,
                        DM1_V1_CommandHighlightBoxPc34Compat *outBox)
{
    ZoneFixture *fixture = (ZoneFixture *)context;

    ++fixture->calls;
    if (zoneIndex != 68 || !outBox) {
        return 0;
    }

    /* ReDMCSB C068_ZONE_TURN_LEFT geometry. */
    outBox->left = 234;
    outBox->right = 252;
    outBox->top = 125;
    outBox->bottom = 145;
    return 1;
}

int main(void)
{
    DM1_V1_CommandHighlightStatePc34Compat state;
    DM1_V1_CommandHighlightRenderPlanPc34Compat plan;
    ZoneFixture fixture;
    static const DM1_V1_CommandHighlightRenderStepPc34Compat expectedSteps[] = {
        DM1_V1_COMMAND_HIGHLIGHT_ENABLE_SCREEN_UPDATE_PC34,
        DM1_V1_COMMAND_HIGHLIGHT_INVERT_BOX_PC34,
        DM1_V1_COMMAND_HIGHLIGHT_DISABLE_SCREEN_UPDATE_PC34,
        DM1_V1_COMMAND_HIGHLIGHT_WAIT_VERTICAL_BLANK_PC34
    };
    int i;

    memset(&state, 0, sizeof(state));
    memset(&fixture, 0, sizeof(fixture));
    CHECK(strstr(dm1_v1_command_highlight_box_enable_source_evidence_pc34(),
                 "CLIKMENU.C F0362_COMMAND_HighlightBoxEnable") != NULL);
    CHECK(!dm1_v1_command_highlight_box_enable_pc34(
        NULL, 68, resolve_zone, &fixture, &plan));
    CHECK(!dm1_v1_command_highlight_box_enable_pc34(
        &state, 68, NULL, &fixture, &plan));
    CHECK(!dm1_v1_command_highlight_box_enable_pc34(
        &state, 68, resolve_zone, &fixture, NULL));

    CHECK(dm1_v1_command_highlight_box_enable_pc34(
        &state, 68, resolve_zone, &fixture, &plan));
    CHECK(fixture.calls == 1);
    CHECK(state.highlightBoxEnabled == 1);
    CHECK(state.highlightedZone.left == 234 && state.highlightedZone.right == 252);
    CHECK(state.highlightedZone.top == 125 && state.highlightedZone.bottom == 145);
    CHECK(plan.accepted == 1 && plan.zoneIndex == 68);
    CHECK(plan.box.left == 234 && plan.box.right == 252 &&
          plan.box.top == 125 && plan.box.bottom == 145);
    CHECK(plan.stepCount == 4);
    for (i = 0; i < plan.stepCount; ++i) {
        CHECK(plan.steps[i] == expectedSteps[i]);
    }

    CHECK(!dm1_v1_command_highlight_box_enable_pc34(
        &state, 69, resolve_zone, &fixture, &plan));
    CHECK(fixture.calls == 2);
    CHECK(state.highlightBoxEnabled == 1);
    CHECK(state.highlightedZone.left == 234 && state.highlightedZone.right == 252 &&
          state.highlightedZone.top == 125 && state.highlightedZone.bottom == 145);
    CHECK(plan.accepted == 0 && plan.stepCount == 0);

    printf("test_dm1_v1_command_highlight_box_enable_pc34_compat: %d assertions, %d failures\n",
           s_assertions, s_failures);
    return s_failures == 0 ? 0 : 1;
}
