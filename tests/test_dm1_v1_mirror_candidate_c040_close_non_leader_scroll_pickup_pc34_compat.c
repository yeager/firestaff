#include "dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    int ok;
    int assertions;
    int failures;

    printf("probe=dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat\n");
    printf("%s\n",
           dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_source_evidence_pc34_compat());

    ok =
        run_dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat_self_test();
    assertions =
        dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_assertions_pc34_compat();
    failures =
        dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_failures_pc34_compat();

    if (!ok || failures != 0) {
        printf("FAIL dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat assertions=%d failures=%d\n",
               assertions,
               failures);
        return 1;
    }

    printf("PASS dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat assertions=%d failures=0\n",
           assertions);
    return 0;
}
