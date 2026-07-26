#include "dm1_v1_startup_handoff_m11_bridge_pc34_compat.h"
#include "entrance_frontend_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

int main(void)
{
    DM1_V1_StartupHandoffM11BridgeStatePc34 state;
    DM1_V1_StartupHandoffM11BridgeReceiptPc34 receipt;

    CHECK(dm1_v1_startup_handoff_m11_bridge_init_pc34(&state, NULL));
    CHECK(!state.swooshBound);
    CHECK(!state.swooshPlayed);
    CHECK(!state.titlePlayed);
    CHECK(!state.entranceCompleted);

    CHECK(dm1_v1_startup_handoff_m11_bridge_execute_prelude_pc34(
        &state, "dm1"));
    CHECK(state.preludePlanReceived);
    CHECK(state.windowRaised);

    CHECK(dm1_v1_startup_handoff_m11_bridge_execute_post_launch_pc34(
        &state, "dm1"));
    CHECK(state.postLaunchPlanReceived);
    CHECK(state.titlePlayed);
    CHECK(state.entranceCompleted);
    CHECK(state.entranceCommand == ENTRANCE_COMPAT_COMMAND_PATH_ENTER);

    CHECK(dm1_v1_startup_handoff_m11_bridge_receipt_pc34(&state, &receipt));
    CHECK(receipt.valid);
    CHECK(receipt.preludeExecuted);
    CHECK(receipt.postLaunchExecuted);
    CHECK(receipt.titlePlayed);
    CHECK(receipt.entranceCompleted);
    CHECK(receipt.sourceEvidence != NULL);

    CHECK(!dm1_v1_startup_handoff_m11_bridge_init_pc34(NULL, NULL));
    CHECK(!dm1_v1_startup_handoff_m11_bridge_execute_prelude_pc34(NULL, "dm1"));
    CHECK(!dm1_v1_startup_handoff_m11_bridge_execute_post_launch_pc34(
        NULL, "dm1"));
    CHECK(!dm1_v1_startup_handoff_m11_bridge_receipt_pc34(NULL, &receipt));
    CHECK(!dm1_v1_startup_handoff_m11_bridge_receipt_pc34(&state, NULL));

    printf("%s\n", failures ? "failed" : "ok: startup handoff M11 bridge");
    return failures ? 1 : 0;
}
