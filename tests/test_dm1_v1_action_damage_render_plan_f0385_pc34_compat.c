#include "firestaff/dm1/v1/action_damage_render_plan_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct StepLog {
    int count;
    int failAt;
    DM1_V1_ActionDamageRenderStepKindPc34Compat steps[
        DM1_V1_ACTION_DAMAGE_MAX_STEPS_PC34];
} StepLog;

static int log_step(
    void *context,
    const DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
    const DM1_V1_ActionDamageRenderStepPc34Compat *step)
{
    (void)plan;
    StepLog *log = (StepLog *)context;

    assert(plan != NULL);
    assert(step != NULL);
    assert(log != NULL);
    if (log->failAt == log->count) {
        return 0;
    }
    assert(log->count < DM1_V1_ACTION_DAMAGE_MAX_STEPS_PC34);
    log->steps[log->count] = step->kind;
    ++log->count;
    return 1;
}

static void assert_box(
    const DM1_V1_ActionDamageBoxPc34Compat *box,
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

static void assert_standard_prefix_and_suffix(
    const DM1_V1_ActionDamageRenderPlanPc34Compat *plan)
{
    (void)plan;
    assert(plan->steps[0].kind ==
           DM1_V1_ACTION_DAMAGE_STEP_ENABLE_SCREEN_UPDATE_PC34);
    assert(plan->steps[1].kind ==
           DM1_V1_ACTION_DAMAGE_STEP_CLEAR_ACTION_AREA_PC34);
    assert(plan->steps[plan->step_count - 1].kind ==
           DM1_V1_ACTION_DAMAGE_STEP_DISABLE_SCREEN_UPDATE_PC34);
}

static void test_full_damage_uses_source_named_draw_boundary(void)
{
    DM1_V1_ActionDamageRenderPlanPc34Compat plan;
    StepLog log = {0, -1, {0}};
    (void)log;

    assert(F0385_MENUS_DrawActionDamage(123, log_step, &log, &plan) == 1);

    assert(plan.valid == 1);
    assert(plan.damage == 123);
    assert(plan.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_FULL_PC34);
    assert_box(&plan.clear_box, 224, 319, 77, 121);
    assert_box(&plan.blit_box, 224, 319, 77, 121);
    assert(plan.has_damage_graphic == 1);
    assert(plan.graphic_index == DM1_V1_ACTION_DAMAGE_GRAPHIC_PC34);
    assert(plan.derived_bitmap_index == 0);
    assert(plan.bitmap_width == 96);
    assert(plan.bitmap_height == 45);
    assert(plan.bitmap_byte_width == 48);
    assert(plan.opaque_blit == 1);
    assert(plan.text_x == 265);
    assert(plan.text_y == DM1_V1_ACTION_DAMAGE_TEXT_Y_PC34);
    assert(strcmp(plan.text, "123") == 0);
    assert(plan.step_count == 5);
    assert_standard_prefix_and_suffix(&plan);
    assert(plan.steps[2].kind ==
           DM1_V1_ACTION_DAMAGE_STEP_BLIT_DAMAGE_GRAPHIC_PC34);
    assert(plan.steps[3].kind == DM1_V1_ACTION_DAMAGE_STEP_PRINT_TEXT_PC34);
    assert(log.count == plan.step_count);
    assert(log.steps[2] == DM1_V1_ACTION_DAMAGE_STEP_BLIT_DAMAGE_GRAPHIC_PC34);
}

static void test_medium_and_small_damage_select_derived_bitmaps(void)
{
    DM1_V1_ActionDamageRenderPlanPc34Compat medium;
    DM1_V1_ActionDamageRenderPlanPc34Compat small;

    assert(F0385_MENUS_DrawActionDamage(40, NULL, NULL, &medium) == 1);
    assert(medium.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_MEDIUM_PC34);
    assert_box(&medium.blit_box, 242, 305, 81, 117);
    assert(medium.derived_bitmap_index ==
           DM1_V1_ACTION_DAMAGE_DERIVED_MEDIUM_PC34);
    assert(medium.bitmap_width == 64);
    assert(medium.bitmap_byte_width == 32);
    assert(strcmp(medium.text, "40") == 0);
    assert(medium.text_x == 268);

    assert(F0385_MENUS_DrawActionDamage(15, NULL, NULL, &small) == 1);
    assert(small.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_SMALL_PC34);
    assert_box(&small.blit_box, 251, 292, 81, 117);
    assert(small.derived_bitmap_index ==
           DM1_V1_ACTION_DAMAGE_DERIVED_SMALL_PC34);
    assert(small.bitmap_width == 42);
    assert(small.bitmap_byte_width == 24);
    assert(strcmp(small.text, "15") == 0);
    assert(small.text_x == 268);
}

static void test_negative_damage_text_routes(void)
{
    DM1_V1_ActionDamageRenderPlanPc34Compat cantReach;
    DM1_V1_ActionDamageRenderPlanPc34Compat needAmmo;
    (void)needAmmo;

    assert(F0385_MENUS_DrawActionDamage(-1, NULL, NULL, &cantReach) == 1);
    assert(cantReach.render_kind ==
           DM1_V1_ACTION_DAMAGE_RENDER_CANT_REACH_PC34);
    assert(cantReach.has_damage_graphic == 0);
    assert(cantReach.text_x == 242);
    assert(strcmp(cantReach.text, "CAN'T REACH") == 0);
    assert(cantReach.step_count == 4);
    assert_standard_prefix_and_suffix(&cantReach);
    assert(cantReach.steps[2].kind ==
           DM1_V1_ACTION_DAMAGE_STEP_PRINT_TEXT_PC34);

    assert(F0385_MENUS_DrawActionDamage(-2, NULL, NULL, &needAmmo) == 1);
    assert(needAmmo.render_kind == DM1_V1_ACTION_DAMAGE_RENDER_NEED_AMMO_PC34);
    assert(needAmmo.has_damage_graphic == 0);
    assert(needAmmo.text_x == 248);
    assert(strcmp(needAmmo.text, "NEED AMMO") == 0);
}

static void test_invalid_damage_and_consumer_failure_are_rejected(void)
{
    DM1_V1_ActionDamageRenderPlanPc34Compat plan;
    (void)plan;
    StepLog log = {0, 2, {0}};
    (void)log;

    assert(F0385_MENUS_DrawActionDamage(32768, NULL, NULL, &plan) == 0);
    assert(F0385_MENUS_DrawActionDamage(1, NULL, NULL, NULL) == 0);
    assert(F0385_MENUS_DrawActionDamage(20, log_step, &log, &plan) == 0);
    assert(log.count == 2);
    assert(plan.valid == 1);
}

static void test_source_evidence_names_redmcsb_symbol(void)
{
    const char *evidence =
        DM1_V1_ActionDamageRenderPlan_SourceEvidencePc34Compat();
    (void)evidence;

    assert(evidence != NULL);
    assert(strstr(evidence, "ACTIDRAW.C") != NULL);
    assert(strstr(evidence, "F0385_MENUS_DrawActionDamage") != NULL);
}

int main(void)
{
    test_full_damage_uses_source_named_draw_boundary();
    test_medium_and_small_damage_select_derived_bitmaps();
    test_negative_damage_text_routes();
    test_invalid_damage_and_consumer_failure_are_rejected();
    test_source_evidence_names_redmcsb_symbol();

    puts("ok: DM1 F0385 action damage render callable");
    return 0;
}
