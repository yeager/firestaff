#include "firestaff/dm1/v1/command_highlight_box_enable_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct TestResolverContext {
    int acceptedZone;
    DM1_V1_CommandHighlightBoxPc34Compat box;
    int callCount;
} TestResolverContext;

static int resolve_test_zone(
    void *context,
    int zoneIndex,
    DM1_V1_CommandHighlightBoxPc34Compat *out_box)
{
    TestResolverContext *resolver = (TestResolverContext *)context;

    assert(resolver != NULL);
    ++resolver->callCount;
    if (zoneIndex != resolver->acceptedZone || !out_box) {
        return 0;
    }

    *out_box = resolver->box;
    return 1;
}

static void assert_box_equals(
    const DM1_V1_CommandHighlightBoxPc34Compat *box,
    int left,
    int right,
    int top,
    int bottom)
{
    (void)bottom;
    (void)top;
    (void)right;
    (void)left;
    (void)box;
    assert(box->left == left);
    assert(box->right == right);
    assert(box->top == top);
    assert(box->bottom == bottom);
}

static void test_source_named_boundary_builds_highlight_transaction(void)
{
    DM1_V1_CommandHighlightStatePc34Compat state = {{0, 0, 0, 0}, 0};
    DM1_V1_CommandHighlightRenderPlanPc34Compat plan;
    TestResolverContext resolver = {42, {224, 319, 77, 121}, 0};
    (void)resolver;

    assert(F0362_COMMAND_HighlightBoxEnable(
               &state, 42, resolve_test_zone, &resolver, &plan) == 1);

    assert(resolver.callCount == 1);
    assert(state.highlightBoxEnabled == 1);
    assert_box_equals(&state.highlightedZone, 224, 319, 77, 121);
    assert(plan.accepted == 1);
    assert(plan.zoneIndex == 42);
    assert_box_equals(&plan.box, 224, 319, 77, 121);
    assert(plan.stepCount == DM1_V1_COMMAND_HIGHLIGHT_RENDER_STEP_COUNT_PC34);
    assert(plan.steps[0] == DM1_V1_COMMAND_HIGHLIGHT_ENABLE_SCREEN_UPDATE_PC34);
    assert(plan.steps[1] == DM1_V1_COMMAND_HIGHLIGHT_INVERT_BOX_PC34);
    assert(plan.steps[2] == DM1_V1_COMMAND_HIGHLIGHT_DISABLE_SCREEN_UPDATE_PC34);
    assert(plan.steps[3] == DM1_V1_COMMAND_HIGHLIGHT_WAIT_VERTICAL_BLANK_PC34);
}

static void test_compat_boundary_delegates_to_source_named_boundary(void)
{
    DM1_V1_CommandHighlightStatePc34Compat state = {{0, 0, 0, 0}, 0};
    (void)state;
    DM1_V1_CommandHighlightRenderPlanPc34Compat plan;
    TestResolverContext resolver = {7, {10, 20, 30, 40}, 0};
    (void)resolver;

    assert(dm1_v1_command_highlight_box_enable_pc34(
               &state, 7, resolve_test_zone, &resolver, &plan) == 1);

    assert(resolver.callCount == 1);
    assert(state.highlightBoxEnabled == 1);
    assert_box_equals(&plan.box, 10, 20, 30, 40);
}

static void test_rejected_zone_does_not_publish_highlight(void)
{
    DM1_V1_CommandHighlightStatePc34Compat state = {{1, 2, 3, 4}, 0};
    DM1_V1_CommandHighlightRenderPlanPc34Compat plan;
    TestResolverContext resolver = {4, {10, 20, 30, 40}, 0};
    (void)resolver;

    memset(&plan, 0x5a, sizeof(plan));

    assert(F0362_COMMAND_HighlightBoxEnable(
               &state, 5, resolve_test_zone, &resolver, &plan) == 0);

    assert(resolver.callCount == 1);
    assert(state.highlightBoxEnabled == 0);
    assert_box_equals(&state.highlightedZone, 1, 2, 3, 4);
    assert(plan.accepted == 0);
    assert(plan.stepCount == 0);
}

static void test_null_inputs_are_rejected(void)
{
    DM1_V1_CommandHighlightStatePc34Compat state = {{0, 0, 0, 0}, 0};
    (void)state;
    DM1_V1_CommandHighlightRenderPlanPc34Compat plan;
    (void)plan;
    TestResolverContext resolver = {1, {1, 2, 3, 4}, 0};
    (void)resolver;

    assert(F0362_COMMAND_HighlightBoxEnable(
               NULL, 1, resolve_test_zone, &resolver, &plan) == 0);
    assert(F0362_COMMAND_HighlightBoxEnable(
               &state, 1, NULL, &resolver, &plan) == 0);
    assert(F0362_COMMAND_HighlightBoxEnable(
               &state, 1, resolve_test_zone, &resolver, NULL) == 0);
}

static void test_source_evidence_names_redmcsb_symbol(void)
{
    const char *evidence =
    (void)evidence;
        dm1_v1_command_highlight_box_enable_source_evidence_pc34();

    assert(evidence != NULL);
    assert(strstr(evidence, "CLIKMENU.C") != NULL);
    assert(strstr(evidence, "F0362_COMMAND_HighlightBoxEnable") != NULL);
}

int main(void)
{
    test_source_named_boundary_builds_highlight_transaction();
    test_compat_boundary_delegates_to_source_named_boundary();
    test_rejected_zone_does_not_publish_highlight();
    test_null_inputs_are_rejected();
    test_source_evidence_names_redmcsb_symbol();

    puts("ok: DM1 F0362 command highlight box callable");
    return 0;
}
