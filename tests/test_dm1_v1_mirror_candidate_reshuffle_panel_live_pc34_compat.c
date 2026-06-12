#include "dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat.h"

#include <stdio.h>

/* ReDMCSB source-lock evidence for this contract test:
 * CHEST.C F0333:30-67 and F0334:117-132 pin G0426/G0425;
 * CHAMPION.C F0284:93-130, F0287:243-268, F0297:243-268,
 * F0298:270-298, F0300:511-584, F0301:606-660, and F0302:662-713 pin
 * party/candidate/hand/slot behavior; COMMAND.C F0378:1973-1983 and
 * F0380:2045-2159 pin queued panel identity; REVIVE.C F0280:124-132 and
 * F0282:744-806 pin G0299 open/close; PANEL.C F0344/F0345 and
 * F0346/F0347:1619-1657 pin C040 routing/redraw; UTAMSCR.C
 * F0077/F0078:141-150 and DEFS.H:338-340, 810-817, 1874-1878, 2085-2088,
 * 2088-2096, 2200, 3001-3008, 5694, 5876-5881 pin constants/globals.
 */

int main(void)
{
    const Dm1V1MirrorCandidateReshuffleContractPc34Compat *contract =
        DM1_V1_MirrorCandidateReshufflePanelLive_ContractPc34Compat();
    Dm1V1MirrorCandidateReshuffleSelfTestStatsPc34Compat stats;
    int ok;

    if (!contract || !contract->contractOnly) {
        printf("dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat: "
               "contract missing\n");
        return 1;
    }

    ok = run_dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat_self_test();
    stats =
        DM1_V1_MirrorCandidateReshufflePanelLive_LastSelfTestStatsPc34Compat();
    printf("dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat: "
           "assertions=%d failures=%d seed=%d\n",
           stats.assertions,
           stats.failures,
           contract->seed);
    return ok ? 0 : 1;
}
