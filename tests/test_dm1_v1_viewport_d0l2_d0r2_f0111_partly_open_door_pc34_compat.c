/*
 * DM1 V1 D0L2/D0R2 F0111 partly-open door source-lock contract test.
 *
 * This is contract-only evidence for the D0 corridor-side pair. It does
 * not load GRAPHICS.DAT / DUNGEON.DAT and does not claim original-DOS or
 * real-asset pixel parity.
 */
#include "firestaff/dm1/v1/viewport/d0l2_d0r2_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_self_test();
    const DM1_V1_D0L2D0R2F0111SelfTestResultPc34 *result =
        dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_last_self_test_result_pc34();

    printf("sourceEvidence=%s\n",
           dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_source_evidence_pc34());
    printf("%s test_dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat "
           "assertions=%d failures=%d ramp_ticks=%d total_door_bands=%d "
           "d0l2_first=%d d0r2_second=%d c10=%d row_guards=%d "
           "fakewall_reject=%d closed_edge=%d open_edge=%d door_bash=%d "
           "hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->ramp_ticks_checked : 0,
           result ? result->total_door_bands : 0,
           result ? result->d0l2_first_dispatch_ok : 0,
           result ? result->d0r2_second_dispatch_ok : 0,
           result ? result->c10_transparency_ok : 0,
           result ? result->row_guards_ok : 0,
           result ? result->fakewall_rejection_ok : 0,
           result ? result->closed_edge_ok : 0,
           result ? result->open_edge_ok : 0,
           result ? result->door_bash_chain_ok : 0,
           result ? result->deterministic_hash : 0u);

    return rc == 0 && result && result->failures == 0 ? 0 : 1;
}
