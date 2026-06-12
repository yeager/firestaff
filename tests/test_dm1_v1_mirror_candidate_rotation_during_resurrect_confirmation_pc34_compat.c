#include "firestaff/dm1/v1/mirror_candidate/dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Contract-only DM1 V1 mirror-candidate party-rotation-during-resurrect-
 * confirmation gate.
 *
 * ReDMCSB source anchors:
 * - REVIVE.C F0282:744-806 consumes C160/C161/C162 in the C040
 *   resurrect/reincarnate confirmation flow; F0280:124-132 publishes the
 *   candidate and F0281 owns resurrect state set/clear.
 * - COMMAND.C F0359:1985-1990 routes M568/C040 panel clicks only when the
 *   leader hand is empty; F0361/F0380:1709-1806,2045-2162 queue and dispatch
 *   input; F0380:2124-2131 dispatches C001/C002 turns before the later
 *   !G0299 gates for C100/C111/C140/C145 at 2302-2368.
 * - CHAMPION.C F0297/F0300/F0301:243-268,489-585,587-625 cover hand and C30+
 *   mutation paths that must not run in this rotation-only gate.
 * - CLIKCHAM.C F0368:51-72 aligns a selected leader to G0308 and skips a live
 *   G0299 candidate redraw; DUNGEON.C:2608-2612 and DUNVIEW.C:3913-3928 carry
 *   the C127 portrait routing.
 * - DEFS.H:2200 C040; 3001-3008 M568/M569; 338-340 C160/C161/C162;
 *   810-817 C30..C37; 5876-5881 G0425/G0426.
 *
 * This gate pins option (c) for the rotation-during-resurrect-confirmation
 * interaction. The firestaff code path is
 * src/dm1/dm1_v1_input_command_queue_pc34_compat.c
 * DM1_V1_InputCommandQueue_ProcessOnePc34Compat, which mirrors COMMAND.C
 * F0380: C001/C002 turn dispatch is processed before candidate-owned
 * C100/C111/C140/C145-style gates. The behavior is that rotation proceeds,
 * C040 remains in resurrect-confirmation state, and the resurrect target stays
 * bound to the original champion.
 *
 * Non-overlap: this gate complements existing mirror-candidate rotation,
 * inventory-click, party-direction, reselect, scroll-pickup, and resurrect
 * champion-switch gates by pressing rotation while C040 is specifically in
 * the resurrect-confirmation pending state.
 */

static int contains(const char *haystack, const char *needle)
{
    return haystack && needle && strstr(haystack, needle) != 0;
}

int main(void)
{
    int rc;
    int print_fail;
    const char *evidence =
        dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_source_evidence_pc34();
    const Dm1V1MirrorCandidateRotationDuringResurrectConfirmationResultPc34Compat
        *result;

    print_fail =
        !contains(evidence, "REVIVE.C F0282:744-806") ||
        !contains(evidence, "COMMAND.C F0359:1985-1990") ||
        !contains(evidence, "F0361/F0380:1709-1806,2045-2162") ||
        !contains(evidence, "CHAMPION.C F0297/F0300/F0301:243-268,489-585,587-625") ||
        !contains(evidence, "DEFS.H:2200 C040") ||
        !contains(evidence, "option_c");

    rc = run_dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_self_test();
    result =
        dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_last_self_test_result_pc34();
    if (!result) {
        printf("FAIL test_dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_pc34_compat assertions=0 failures=1 positive_outcome=unknown negative_outcome=unknown target_preserved=0 state_preserved=0 redraw_preserved=0 hash=0x00000000\n");
        return 1;
    }
    if (print_fail) {
        printf("FAIL test_dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_pc34_compat assertions=%d failures=%d positive_outcome=%s negative_outcome=%s target_preserved=%d state_preserved=%d redraw_preserved=%d hash=0x%08X\n",
               result->assertions,
               result->failures + 1,
               result->positive_rotation_outcome,
               result->negative_browse_rotation_outcome,
               result->resurrect_target_preserved_checks,
               result->resurrect_state_preserved_checks,
               result->panel_redraw_preserved_checks,
               result->deterministic_hash);
        return 1;
    }
    if (rc != 0 || result->failures != 0 ||
        strcmp(result->positive_rotation_outcome, "c") != 0 ||
        strcmp(result->negative_browse_rotation_outcome,
               "allowed_rotation") != 0) {
        printf("FAIL test_dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_pc34_compat assertions=%d failures=%d positive_outcome=%s negative_outcome=%s target_preserved=%d state_preserved=%d redraw_preserved=%d hash=0x%08X\n",
               result->assertions,
               result->failures,
               result->positive_rotation_outcome,
               result->negative_browse_rotation_outcome,
               result->resurrect_target_preserved_checks,
               result->resurrect_state_preserved_checks,
               result->panel_redraw_preserved_checks,
               result->deterministic_hash);
        return 1;
    }

    printf("PASS test_dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_pc34_compat assertions=%d failures=0 positive_outcome=%s negative_outcome=%s target_preserved=%d state_preserved=%d redraw_preserved=%d hash=0x%08X\n",
           result->assertions,
           result->positive_rotation_outcome,
           result->negative_browse_rotation_outcome,
           result->resurrect_target_preserved_checks,
           result->resurrect_state_preserved_checks,
           result->panel_redraw_preserved_checks,
           result->deterministic_hash);
    return 0;
}
