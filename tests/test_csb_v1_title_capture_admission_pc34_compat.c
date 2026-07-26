#include "csb_v1_boot.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        ++failures; \
        printf("FAIL: %s\\n", message); \
    } \
} while (0)

static int make_title_plan(int frame, CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupRenderState_PC34 state;

    memset(&state, 0, sizeof(state));
    state.entrance_active = 1;
    state.title_active = 1;
    state.title_frame = frame;
    return csb_v1_startup_source_render_plan_from_state_pc34(&state, out_plan);
}

int main(void)
{
    CSB_V1_StartupRenderPlan_PC34 plan;

    CHECK(make_title_plan(79, &plan) &&
              plan.title_source_step == 21 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              csb_v1_boot_startup_title_capture_plan_admit_pc34(&plan, 79),
          "C001 step 21 remains an admitted CHAOS hold wave");

    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
    CHECK(!csb_v1_boot_startup_title_capture_plan_admit_pc34(&plan, 79),
          "admission rejects a false C426 capture carrying source step 21");

    CHECK(make_title_plan(80, &plan) &&
              plan.title_source_step == 21 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              csb_v1_boot_startup_title_capture_plan_admit_pc34(&plan, 80),
          "frame 80 step 21 remains within CHAOS hold stage");

    CHECK(make_title_plan(100, &plan) &&
              plan.title_source_step == 22 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
              csb_v1_boot_startup_title_capture_plan_admit_pc34(&plan, 100),
          "C426 accepts the verified original STRIKES BACK wave at frame 100");

    return failures ? 1 : 0;
}
