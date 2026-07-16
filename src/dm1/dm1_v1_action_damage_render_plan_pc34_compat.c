#include "firestaff/dm1/v1/action_damage_render_plan_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB ACTIDRAW.C F0385_MENUS_DrawActionDamage:70-198, PC34 "
    "MEDIA009: F0077; G0578=C0_FALSE; M524_FillScreenBox(G0001, black); "
    "negative -1 prints CAN'T REACH at 242,100 and every other negative "
    "prints NEED AMMO at 248,100; positive damage >40 uses native graphic "
    "C014/G0499/C048, >15 uses derived C002 at 64x37/G0502/C032, otherwise "
    "derived C003 at 42x37/G0503/C024; numeric text begins at 274,100 and "
    "moves left three pixels for every digit; F0078 closes the sequence. "
    "This is a render plan and consumer contract only, without bitmap, cache, "
    "framebuffer, or M11 state behavior.";

static const DM1_V1_ActionDamageBoxPc34Compat s_action_area_box = {
    224, 319, 77, 121
};
static const DM1_V1_ActionDamageBoxPc34Compat s_full_damage_box = {
    224, 319, 77, 121
};
static const DM1_V1_ActionDamageBoxPc34Compat s_medium_damage_box = {
    242, 305, 81, 117
};
static const DM1_V1_ActionDamageBoxPc34Compat s_small_damage_box = {
    251, 292, 81, 117
};

const char *DM1_V1_ActionDamageRenderPlan_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}

static void dm1_v1_action_damage_add_step(
    DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
    DM1_V1_ActionDamageRenderStepKindPc34Compat kind)
{
    plan->steps[plan->step_count].kind = kind;
    ++plan->step_count;
}

static int dm1_v1_action_damage_write_decimal(int damage, char text[16])
{
    int count = snprintf(text, 16, "%d", damage);
    return count > 0 ? count : 0;
}

int DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(
    int damage,
    DM1_V1_ActionDamageRenderPlanPc34Compat *out_plan)
{
    int digit_count;

    if (!out_plan || damage < INT16_MIN ||
        damage > DM1_V1_ACTION_DAMAGE_MAX_PC34) {
        return 0;
    }

    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->valid = 1;
    out_plan->damage = damage;
    out_plan->clear_box = s_action_area_box;
    out_plan->text_y = DM1_V1_ACTION_DAMAGE_TEXT_Y_PC34;
    out_plan->text_foreground_color = DM1_V1_ACTION_DAMAGE_COLOR_CYAN_PC34;
    out_plan->text_background_color = DM1_V1_ACTION_DAMAGE_COLOR_BLACK_PC34;

    dm1_v1_action_damage_add_step(
        out_plan, DM1_V1_ACTION_DAMAGE_STEP_ENABLE_SCREEN_UPDATE_PC34);
    dm1_v1_action_damage_add_step(
        out_plan, DM1_V1_ACTION_DAMAGE_STEP_CLEAR_ACTION_AREA_PC34);

    if (damage < 0) {
        out_plan->render_kind =
            damage == DM1_V1_ACTION_DAMAGE_CANT_REACH_PC34
                ? DM1_V1_ACTION_DAMAGE_RENDER_CANT_REACH_PC34
                : DM1_V1_ACTION_DAMAGE_RENDER_NEED_AMMO_PC34;
        out_plan->text_x =
            out_plan->render_kind == DM1_V1_ACTION_DAMAGE_RENDER_CANT_REACH_PC34
                ? 242
                : 248;
        (void)snprintf(out_plan->text, sizeof(out_plan->text), "%s",
                       out_plan->render_kind ==
                               DM1_V1_ACTION_DAMAGE_RENDER_CANT_REACH_PC34
                           ? "CAN'T REACH"
                           : "NEED AMMO");
        dm1_v1_action_damage_add_step(
            out_plan, DM1_V1_ACTION_DAMAGE_STEP_PRINT_TEXT_PC34);
    } else {
        out_plan->has_damage_graphic = 1;
        out_plan->graphic_index = DM1_V1_ACTION_DAMAGE_GRAPHIC_PC34;
        out_plan->opaque_blit = 1;
        if (damage > DM1_V1_ACTION_DAMAGE_MEDIUM_MAX_PC34) {
            out_plan->render_kind = DM1_V1_ACTION_DAMAGE_RENDER_FULL_PC34;
            out_plan->blit_box = s_full_damage_box;
            out_plan->bitmap_width = 96;
            out_plan->bitmap_height = 45;
            out_plan->bitmap_byte_width = 48;
        } else if (damage > DM1_V1_ACTION_DAMAGE_SMALL_MAX_PC34) {
            out_plan->render_kind = DM1_V1_ACTION_DAMAGE_RENDER_MEDIUM_PC34;
            out_plan->blit_box = s_medium_damage_box;
            out_plan->derived_bitmap_index =
                DM1_V1_ACTION_DAMAGE_DERIVED_MEDIUM_PC34;
            out_plan->bitmap_width = 64;
            out_plan->bitmap_height = 37;
            out_plan->bitmap_byte_width = 32;
        } else {
            out_plan->render_kind = DM1_V1_ACTION_DAMAGE_RENDER_SMALL_PC34;
            out_plan->blit_box = s_small_damage_box;
            out_plan->derived_bitmap_index =
                DM1_V1_ACTION_DAMAGE_DERIVED_SMALL_PC34;
            out_plan->bitmap_width = 42;
            out_plan->bitmap_height = 37;
            out_plan->bitmap_byte_width = 24;
        }
        digit_count = dm1_v1_action_damage_write_decimal(damage, out_plan->text);
        out_plan->text_x = DM1_V1_ACTION_DAMAGE_TEXT_X_PC34 -
            digit_count * DM1_V1_ACTION_DAMAGE_TEXT_DIGIT_X_STEP_PC34;
        dm1_v1_action_damage_add_step(
            out_plan, DM1_V1_ACTION_DAMAGE_STEP_BLIT_DAMAGE_GRAPHIC_PC34);
        dm1_v1_action_damage_add_step(
            out_plan, DM1_V1_ACTION_DAMAGE_STEP_PRINT_TEXT_PC34);
    }

    dm1_v1_action_damage_add_step(
        out_plan, DM1_V1_ACTION_DAMAGE_STEP_DISABLE_SCREEN_UPDATE_PC34);
    return 1;
}

int F0385_MENUS_DrawActionDamage(
    int damage,
    DM1_V1_ActionDamageRenderConsumerPc34Compat consumer,
    void *context,
    DM1_V1_ActionDamageRenderPlanPc34Compat *out_plan)
{
    if (!DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(damage, out_plan)) {
        return 0;
    }
    if (!consumer) {
        return 1;
    }
    return DM1_V1_ActionDamageRenderPlan_ConsumePc34Compat(
        out_plan, consumer, context);
}

int DM1_V1_ActionDamageRenderPlan_ConsumePc34Compat(
    const DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
    DM1_V1_ActionDamageRenderConsumerPc34Compat consumer,
    void *context)
{
    int i;

    if (!plan || !plan->valid || !consumer || plan->step_count < 1 ||
        plan->step_count > DM1_V1_ACTION_DAMAGE_MAX_STEPS_PC34) {
        return 0;
    }
    for (i = 0; i < plan->step_count; ++i) {
        if (!consumer(context, plan, &plan->steps[i])) {
            return 0;
        }
    }
    return 1;
}
