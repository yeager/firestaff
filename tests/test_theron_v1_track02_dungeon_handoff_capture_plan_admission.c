#include "theron_v1_track02_dungeon_handoff_capture_plan_admission.h"

#include <stdio.h>

int main(void)
{
    Theron_V1Track02DungeonCapturePlanAdmissionReceipt receipt;
    if (!theron_v1_track02_dungeon_capture_plan_admit(
            NULL, NULL, NULL, NULL, NULL, 0u, 0u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_UNAVAILABLE)
        return 1;
    if (!theron_v1_track02_dungeon_capture_plan_admit(
            "/tmp/firestaff-missing-theron-dungeon-capture-plan", NULL, NULL,
            NULL, NULL, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_UNAVAILABLE)
        return 2;
    puts("test_theron_v1_track02_dungeon_handoff_capture_plan_admission: PASS (no local plan)");
    return 0;
}
