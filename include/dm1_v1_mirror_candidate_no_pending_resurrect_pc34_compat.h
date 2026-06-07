#ifndef DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Dm1V1MirrorCandidateNoPendingResurrectStepIdPc34Compat {
    STEP_BEGIN_G0299_LIVE_G0305_ZERO = 0,
    STEP_RESURRECT_CLICK_DISPATCHED,
    STEP_GATE_NO_CANDIDATE_BLOCKS_F0282,
    STEP_F0282_NOT_INVOKED,
    STEP_ASSERT_G0299_G0305_UNCHANGED,
    STEP_ASSERT_CHAMPION_INVENTORY_PANEL_UNCHANGED
} Dm1V1MirrorCandidateNoPendingResurrectStepIdPc34Compat;

typedef struct Dm1V1MirrorCandidateNoPendingResurrectStepPc34Compat {
    Dm1V1MirrorCandidateNoPendingResurrectStepIdPc34Compat id;
    const char *name;
    const char *redmcsb_anchor;
    const char *assertion;
} Dm1V1MirrorCandidateNoPendingResurrectStepPc34Compat;

typedef struct Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat {
    int contract_only;
    int g0299_panel_live_at_start;
    int g0305_no_pending_at_start;
    int resurrect_action_invoked_with_no_pending;
    int g0299_unchanged_after_no_op;
    int g0305_unchanged_after_no_op;
    int f0282_not_invoked;
    int champion_state_unchanged;
    int c040_panel_state_unchanged;
    int inventory_state_unchanged;
    int resurrect_click_consumed;
    const char *redmcsb_f0280_anchor;
    const char *redmcsb_f0282_anchor;
    const char *redmcsb_command_2159_anchor;
    const char *redmcsb_command_2302_anchor;
    const char *non_op_note;
    const char *source_summary;
} Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat;

const Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat *
dm1_v1_mirror_candidate_no_pending_resurrect_contract_pc34_compat(void);

size_t dm1_v1_mirror_candidate_no_pending_resurrect_steps_pc34_compat(
    Dm1V1MirrorCandidateNoPendingResurrectStepPc34Compat *out,
    size_t cap);

#ifdef __cplusplus
}
#endif

#endif
