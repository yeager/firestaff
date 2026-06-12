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
 * Non-overlap: this gate complements existing mirror-candidate rotation,
 * inventory-click, party-direction, reselect, scroll-pickup, and resurrect
 * champion-switch gates by pressing rotation while C040 is specifically in
 * the resurrect-confirmation pending state.
 */

#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_ROTATION_DURING_RESURRECT_CONFIRMATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_ROTATION_DURING_RESURRECT_CONFIRMATION_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1MirrorCandidateRotationDuringResurrectConfirmationResultPc34Compat {
    int assertions;
    int failures;
    unsigned int deterministic_hash;
    const char *positive_rotation_outcome;
    const char *negative_browse_rotation_outcome;
    int positive_rotation_dispatches;
    int negative_browse_rotation_dispatches;
    int resurrect_target_preserved_checks;
    int resurrect_state_preserved_checks;
    int panel_redraw_preserved_checks;
    int queue_dispatch_checks;
    int c160_c161_c162_consumption_checks;
    int leader_empty_hand_checks;
    int c30_g0425_mutation_checks;
} Dm1V1MirrorCandidateRotationDuringResurrectConfirmationResultPc34Compat;

int run_dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_self_test(void);

const Dm1V1MirrorCandidateRotationDuringResurrectConfirmationResultPc34Compat *
dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_last_self_test_result_pc34(void);

const char *
dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
