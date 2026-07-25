#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_stage_names(void)
{
    const char *name;

    name = dm1_v1_startup_stage_name_pc34(DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34);
    (void)name;
    assert(name != NULL);
    assert(strlen(name) > 0);

    name = dm1_v1_startup_stage_name_pc34(DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34);
    assert(name != NULL);

    name = dm1_v1_startup_stage_name_pc34(DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34);
    assert(name != NULL);
}

static void test_stage_ordering(void)
{
    int result;

    result = dm1_v1_startup_stage_after_pc34(
        DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34,
        DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34);
    (void)result;
    assert(result == 1);

    result = dm1_v1_startup_stage_after_pc34(
        DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34,
        DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34);
    assert(result == 1);

    result = dm1_v1_startup_stage_after_pc34(
        DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34,
        DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34);
    assert(result == 0);

    result = dm1_v1_startup_stage_after_pc34(
        DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34,
        DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34);
    assert(result == 0);
}

static void test_launch_path_bypass(void)
{
    int bypass;

    bypass = dm1_v1_startup_launch_path_bypasses_intro_pc34(
        DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34);
    (void)bypass;
    assert(bypass == 1);

    bypass = dm1_v1_startup_launch_path_bypasses_intro_pc34(
        DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34);
    assert(bypass == 0);
}

static void test_source_order_valid(void)
{
    int valid = dm1_v1_startup_sequence_source_order_valid_pc34();
    (void)valid;
    assert(valid == 1);
}

static void test_source_evidence(void)
{
    const char *ev = dm1_v1_startup_sequence_source_evidence_pc34();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_title_timing_constants(void)
{
    unsigned int zoom = dm1_v1_startup_title_zoom_steps_pc34();
    (void)zoom;
    assert(zoom > 0);

    unsigned int anim = dm1_v1_startup_title_source_animation_steps_pc34();
    (void)anim;
    assert(anim > 0);

    unsigned int bank = dm1_v1_startup_title_frame_bank_equivalent_steps_pc34();
    (void)bank;
    assert(bank > 0);

    unsigned int hold = dm1_v1_startup_title_presents_hold_vblanks_pc34();
    (void)hold;
    assert(hold > 0);

    unsigned int tick = dm1_v1_startup_title_vblank_tick_ms_pc34();
    (void)tick;
    assert(tick > 0);

    unsigned int hold_ms = dm1_v1_startup_title_presents_hold_ms_pc34();
    (void)hold_ms;
    assert(hold_ms > 0);
}

static void test_launch_path_receipt_null_rejected(void)
{
    DM1_V1_StartupLaunchPathReceipt_PC34 receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_startup_launch_path_receipt_pc34(NULL, &receipt);
    (void)rc;
    assert(rc == 0);

    rc = dm1_v1_startup_launch_path_receipt_pc34(NULL, NULL);
    assert(rc == 0);
}

static void test_runtime_start_receipt_null_rejected(void)
{
    DM1_V1_StartupRuntimeStartReceipt_PC34 receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_startup_runtime_start_receipt_pc34(NULL, &receipt);
    (void)rc;
    assert(rc == 0);
}

static void test_dungeon_path_receipt_null_rejected(void)
{
    DM1_V1_StartupDungeonPathReceipt_PC34 receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_startup_dungeon_path_receipt_pc34(NULL, &receipt);
    (void)rc;
    assert(rc == 0);
}

static void test_graphics_bind_receipt_null_rejected(void)
{
    DM1_V1_StartupGraphicsBindReceipt_PC34 receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_startup_graphics_bind_receipt_pc34(NULL, &receipt);
    (void)rc;
    assert(rc == 0);
}

static void test_full_graphics_media_receipt_null_rejected(void)
{
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_startup_full_graphics_media_receipt_pc34(NULL, &receipt);
    (void)rc;
    assert(rc == 0);

    rc = dm1_v1_startup_full_graphics_media_receipt_pc34("dm1", NULL);
    assert(rc == 0);
}

static void test_full_graphics_media_receipt_for_dm1(void)
{
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 receipt;
    int rc;

    memset(&receipt, 0, sizeof(receipt));
    rc = dm1_v1_startup_full_graphics_media_receipt_pc34("dm1", &receipt);
    (void)rc;
    assert(rc == 1);
    assert(receipt.handled == 1);
    assert(receipt.play_swsh == 1);
    assert(receipt.play_title == 1);
    assert(receipt.play_entrance == 1);
    assert(receipt.swsh_vblank_ms > 0);
    assert(receipt.title_zoom_step_count > 0);
    assert(receipt.entrance_source_animation_steps > 0);
}

static void test_handoff_prelude_plan_null_rejected(void)
{
    DM1_V1_StartupHandoffPreludePlan_PC34 plan;
    int rc;

    memset(&plan, 0, sizeof(plan));
    rc = dm1_v1_startup_handoff_prelude_plan_pc34(NULL, &plan);
    (void)rc;
    assert(rc == 0);

    rc = dm1_v1_startup_handoff_prelude_plan_pc34("dm1", NULL);
    assert(rc == 0);
}

static void test_receipt_phase_null_rejected(void)
{
    int rc = dm1_v1_startup_receipt_phase_pc34(0, 0, NULL, 0);
    (void)rc;
    assert(rc == 0);
}

static void test_receipt_phase_returns_string(void)
{
    char phase[64];
    int rc;

    memset(phase, 0, sizeof(phase));
    rc = dm1_v1_startup_receipt_phase_pc34(0, 0, phase, (int)sizeof(phase));
    (void)rc;
    assert(rc == 1);
    assert(strlen(phase) > 0);
}

int main(void)
{
    test_stage_names();
    test_stage_ordering();
    test_launch_path_bypass();
    test_source_order_valid();
    test_source_evidence();
    test_title_timing_constants();
    test_launch_path_receipt_null_rejected();
    test_runtime_start_receipt_null_rejected();
    test_dungeon_path_receipt_null_rejected();
    test_graphics_bind_receipt_null_rejected();
    test_full_graphics_media_receipt_null_rejected();
    test_full_graphics_media_receipt_for_dm1();
    test_handoff_prelude_plan_null_rejected();
    test_receipt_phase_null_rejected();
    test_receipt_phase_returns_string();

    puts("ok: DM1 startup sequence (Q-DM1-08) 15 tests passed");
    return 0;
}
