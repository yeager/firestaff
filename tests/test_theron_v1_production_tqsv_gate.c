#include "theron_v1_startup_save_resume.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    Theron_V1StartupSaveResume snapshot;
    Theron_V1StartupContinueAvailability availability;
    Theron_V1_World world;
    char receipt[128];

    memset(&snapshot, 0, sizeof(snapshot));
    if (!theron_v1_startup_save_resume_evaluate(
            "/path/that/does/not/exist", &snapshot) ||
        snapshot.tqsv_total_slots != 0 ||
        snapshot.tqsv_valid_slots != 0 ||
        snapshot.tqsv_active_slot != -1 ||
        snapshot.srm_total_slots != 0 ||
        snapshot.srm_first_decoded_slot != -1 ||
        snapshot.resume_claim == THERON_V1_STARTUP_RESUME_TQSV ||
        snapshot.resume_claim == THERON_V1_STARTUP_RESUME_DUAL) {
        fputs("FAIL: production advertised a TQSV Continue route\n", stderr);
        return 1;
    }
    if (theron_v1_startup_save_resume_apply_explicit_path(
            &snapshot, "/path/that/does/not/exist/slot0.srm", NULL)) {
        fputs("FAIL: production accepted a Firestaff SRM envelope path\n",
              stderr);
        return 1;
    }
    if (theron_v1_startup_save_resume_apply_explicit_path(
            &snapshot, "/path/that/does/not/exist/slot0.tqsv", NULL)) {
        fputs("FAIL: production accepted an explicit TQSV path\n", stderr);
        return 1;
    }

    theron_v1_world_init(&world);
    memset(receipt, 0, sizeof(receipt));
    if (theron_v1_startup_continue_tqsv_apply(
            &world, "/path/that/does/not/exist", 0,
            receipt, sizeof(receipt)) ||
        strstr(receipt, "not an authenticated original-save route") == NULL) {
        fputs("FAIL: production TQSV Continue gate is not fail-closed\n",
              stderr);
        return 1;
    }
    memset(receipt, 0, sizeof(receipt));
    if (theron_v1_startup_continue_srm_path_apply(
            &world, "/path/that/does/not/exist/slot0.srm",
            receipt, sizeof(receipt)) ||
        strstr(receipt, "not an authenticated original-save route") == NULL) {
        fputs("FAIL: production Firestaff SRM Continue gate is not fail-closed\n",
              stderr);
        return 1;
    }
    if (!theron_v1_startup_continue_availability_from_state(
            THERON_V1_STARTUP_RESUME_DUAL, 0, 0,
            THERON_V1_SRM_PROGRESS_IMPORT_OK, &availability) ||
        availability.has_tqsv_continue || availability.has_srm_continue ||
        availability.has_any_continue) {
        fputs("FAIL: production advertised a synthetic Continue route\n",
              stderr);
        return 1;
    }

    puts("PASS: production ignores Firestaff TQSV/SRM envelopes and keeps original-save semantics gated");
    return 0;
}
