#include "dm1_v1_action_spell_m11_blit_plan_pc34_compat.h"
#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_blit_max_constant(void)
{
    assert(DM1_V1_ACTION_SPELL_M11_BLIT_MAX_PC34 == 3);
}

static void test_presentation_kinds(void)
{
    assert(DM1_V1_ACTION_HUD_PRESENTATION_NONE_PC34 == 0);
    assert(DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 == 4);
    assert(DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 == 5);
    assert(DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34 == 6);
    assert(DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34 == 7);
    assert(DM1_V1_ACTION_HUD_PRESENTATION_SPELL_FAILURE_PC34 == 8);
}

static void test_plan_struct(void)
{
    DM1_V1_ActionSpellM11BlitPlanPc34 p;
    memset(&p, 0, sizeof(p));
    assert(p.accepted == 0);
    assert(p.blitCount == 0);
    assert(p.clearX == 0);
}

static void test_build_null(void)
{
    int ok = dm1_v1_action_spell_m11_blit_plan_build_pc34(4, 1, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_build_action_lock(void)
{
    DM1_V1_ActionSpellM11BlitPlanPc34 p;
    int ok = dm1_v1_action_spell_m11_blit_plan_build_pc34(
        DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34, 2, &p);
    (void)ok;
    assert(ok == 1);
    assert(p.accepted == 1);
    assert(p.blitCount == 1);
    assert(p.blits[0].graphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34);
}

static void test_build_action_invalid_rows(void)
{
    DM1_V1_ActionSpellM11BlitPlanPc34 p;
    int ok = dm1_v1_action_spell_m11_blit_plan_build_pc34(
        DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34, 0, &p);
    (void)ok;
    assert(ok == 0);
}

static void test_build_spell(void)
{
    DM1_V1_ActionSpellM11BlitPlanPc34 p;
    int ok = dm1_v1_action_spell_m11_blit_plan_build_pc34(
        DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34, 0, &p);
    (void)ok;
    assert(ok == 1);
    assert(p.accepted == 1);
    assert(p.blitCount == 3);
    assert(p.blits[0].graphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34);
    assert(p.blits[1].graphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34);
    assert(p.blits[2].graphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34);
}

static void test_build_invalid_kind(void)
{
    DM1_V1_ActionSpellM11BlitPlanPc34 p;
    int ok = dm1_v1_action_spell_m11_blit_plan_build_pc34(99, 1, &p);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_blit_max_constant();
    test_presentation_kinds();
    test_plan_struct();
    test_build_null();
    test_build_action_lock();
    test_build_action_invalid_rows();
    test_build_spell();
    test_build_invalid_kind();

    puts("ok: DM1 action spell M11 blit plan (Q-DM1-07) 8 tests passed");
    return 0;
}
