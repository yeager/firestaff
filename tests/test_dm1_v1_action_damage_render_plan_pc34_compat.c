#include "firestaff/dm1/v1/action_damage_render_plan_pc34_compat.h"

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

typedef struct ConsumeTrace {
    int count;
    DM1_V1_ActionDamageRenderStepKindPc34Compat kinds[5];
} ConsumeTrace;

static int record_step(void *context,
                       const DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
                       const DM1_V1_ActionDamageRenderStepPc34Compat *step)
{
    ConsumeTrace *trace = (ConsumeTrace *)context;
    CHECK(plan != NULL);
    CHECK(step != NULL);
    trace->kinds[trace->count++] = step->kind;
    return 1;
}

static int stop_after_first(void *context,
                            const DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
                            const DM1_V1_ActionDamageRenderStepPc34Compat *step)
{
    (void)context;
    (void)plan;
    (void)step;
    return 0;
}

static void check_steps(const DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
                        const DM1_V1_ActionDamageRenderStepKindPc34Compat *expected,
                        int expected_count)
{
    int i;
    CHECK(plan->step_count == expected_count);
    for (i = 0; i < expected_count; ++i) {
        CHECK(plan->steps[i].kind == expected[i]);
    }
}

int main(void)
{
    DM1_V1_ActionDamageRenderPlanPc34Compat plan;
    ConsumeTrace trace;
    static const DM1_V1_ActionDamageRenderStepKindPc34Compat negative_steps[] = {
        DM1_V1_ACTION_DAMAGE_STEP_ENABLE_SCREEN_UPDATE_PC34,
        DM1_V1_ACTION_DAMAGE_STEP_CLEAR_ACTION_AREA_PC34,
        DM1_V1_ACTION_DAMAGE_STEP_PRINT_TEXT_PC34,
        DM1_V1_ACTION_DAMAGE_STEP_DISABLE_SCREEN_UPDATE_PC34
    };
    static const DM1_V1_ActionDamageRenderStepKindPc34Compat positive_steps[] = {
        DM1_V1_ACTION_DAMAGE_STEP_ENABLE_SCREEN_UPDATE_PC34,
        DM1_V1_ACTION_DAMAGE_STEP_CLEAR_ACTION_AREA_PC34,
        DM1_V1_ACTION_DAMAGE_STEP_BLIT_DAMAGE_GRAPHIC_PC34,
        DM1_V1_ACTION_DAMAGE_STEP_PRINT_TEXT_PC34,
        DM1_V1_ACTION_DAMAGE_STEP_DISABLE_SCREEN_UPDATE_PC34
    };

    CHECK(strstr(DM1_V1_ActionDamageRenderPlan_SourceEvidencePc34Compat(),
                 "ACTIDRAW.C F0385") != NULL);
    CHECK(!DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(0, NULL));
    CHECK(!DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(32768, &plan));

    CHECK(DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(-1, &plan));
    CHECK(plan.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_CANT_REACH_PC34);
    CHECK(strcmp(plan.text, "CAN'T REACH") == 0);
    CHECK(plan.text_x == 242 && plan.text_y == 100);
    CHECK(!plan.has_damage_graphic);
    check_steps(&plan, negative_steps, 4);

    CHECK(DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(-2, &plan));
    CHECK(plan.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_NEED_AMMO_PC34);
    CHECK(strcmp(plan.text, "NEED AMMO") == 0);
    CHECK(plan.text_x == 248 && plan.text_y == 100);
    check_steps(&plan, negative_steps, 4);

    CHECK(DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(0, &plan));
    CHECK(plan.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_SMALL_PC34);
    CHECK(plan.derived_bitmap_index == 3 && plan.bitmap_width == 42 &&
          plan.bitmap_height == 37 && plan.bitmap_byte_width == 24);
    CHECK(strcmp(plan.text, "0") == 0 && plan.text_x == 271);
    CHECK(plan.blit_box.left == 251 && plan.blit_box.right == 292 &&
          plan.blit_box.top == 81 && plan.blit_box.bottom == 117);
    check_steps(&plan, positive_steps, 5);

    CHECK(DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(16, &plan));
    CHECK(plan.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_MEDIUM_PC34);
    CHECK(plan.derived_bitmap_index == 2 && plan.bitmap_width == 64 &&
          plan.bitmap_height == 37 && plan.bitmap_byte_width == 32);
    CHECK(strcmp(plan.text, "16") == 0 && plan.text_x == 268);
    CHECK(plan.blit_box.left == 242 && plan.blit_box.right == 305 &&
          plan.blit_box.top == 81 && plan.blit_box.bottom == 117);

    CHECK(DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(41, &plan));
    CHECK(plan.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_FULL_PC34);
    CHECK(plan.derived_bitmap_index == 0 && plan.bitmap_width == 96 &&
          plan.bitmap_height == 45 && plan.bitmap_byte_width == 48);
    CHECK(strcmp(plan.text, "41") == 0 && plan.text_x == 268);
    CHECK(plan.graphic_index == 14 && plan.opaque_blit);
    CHECK(plan.blit_box.left == 224 && plan.blit_box.right == 319 &&
          plan.blit_box.top == 77 && plan.blit_box.bottom == 121);

    memset(&trace, 0, sizeof(trace));
    CHECK(DM1_V1_ActionDamageRenderPlan_ConsumePc34Compat(
        &plan, record_step, &trace));
    CHECK(trace.count == plan.step_count);
    {
        int i;
        for (i = 0; i < plan.step_count; ++i) {
            CHECK(trace.kinds[i] == plan.steps[i].kind);
        }
    }
    CHECK(!DM1_V1_ActionDamageRenderPlan_ConsumePc34Compat(
        &plan, stop_after_first, NULL));
    CHECK(!DM1_V1_ActionDamageRenderPlan_ConsumePc34Compat(
        &plan, NULL, NULL));

    printf("test_dm1_v1_action_damage_render_plan_pc34_compat: %d assertions, %d failures\n",
           s_assertions, s_failures);
    return s_failures == 0 ? 0 : 1;
}
