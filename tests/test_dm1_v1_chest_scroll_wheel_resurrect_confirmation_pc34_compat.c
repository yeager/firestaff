#include "firestaff/dm1/v1/chest/dm1_v1_chest_scroll_wheel_resurrect_confirmation_pc34_compat.h"

#include <stdio.h>

/*
 * ReDMCSB source-lock evidence for this contract-only test:
 * REVIVE.C F0282:744-806 is the main confirmation gate for C160/C161/C162;
 * F0280:124-132 and F0281 publish/set candidate state. CHEST.C F0333:30-67
 * and F0334:117-132 pin G0426/G0425 open/close. CHAMPION.C F0297:243-298,
 * CHAMPION.C F0298:270-298, CHAMPION.C F0300:511-584, CHAMPION.C
 * F0301:606-660, and CHAMPION.C F0302:662-713 pin the hand and C30+
 * exchange. COMMAND.C F0359:1985-1990, COMMAND.C F0378:1973-1983, and
 * COMMAND.C F0380:2045-2159 pin empty-hand C040 dispatch and queued pickup.
 * PANEL.C F0344/F0345 plus F0346/F0347:1619-1657 pin panel click/redraw.
 * UTAMSCR.C F0077/F0078:141-150 pins the pointer update bracket. DEFS.H
 * 338-340, 810-817, 873/876, 1878, 2088, 2200, 3001-3008, 3906-3913,
 * 4205-4207, 5694, and 5876-5881 pin commands, slots, panels, and globals.
 *
 * Non-overlap siblings: C040+C545 pickup/drop panel-live tests,
 * mirror_candidate_scroll_pickup_non_leader_panel_live, and
 * mirror_candidate_resurrect_reselect_with_inventory_pickup.
 */

int main(void)
{
    int rc;
    const Dm1V1ChestScrollWheelResurrectConfirmationResultPc34 *result;
    const char *status;

    rc = run_dm1_v1_chest_scroll_wheel_resurrect_confirmation_self_test();
    result =
        dm1_v1_chest_scroll_wheel_resurrect_confirmation_last_self_test_result_pc34();
    status = rc == 0 ? "PASS" : "FAIL";
    printf("%s test_dm1_v1_chest_scroll_wheel_resurrect_confirmation_pc34_compat "
           "assertions=%d failures=%d positive_rejections=%d "
           "negative_browse=%d negative_cancelled=%d leader_unchanged=%d "
           "c30_unchanged=%d g0426_unchanged=%d candidate_unchanged=%d "
           "hash=0x%08x\n",
           status,
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->positive_rejections : 0,
           result ? result->negative_browse_allowed : 0,
           result ? result->negative_cancelled_allowed : 0,
           result ? result->leader_hand_unchanged_checks : 0,
           result ? result->c30_chain_unchanged_checks : 0,
           result ? result->g0426_chain_unchanged_checks : 0,
           result ? result->candidate_command_unchanged_checks : 0,
           result ? result->deterministic_hash : 0u);
    return rc == 0 ? 0 : 1;
}
