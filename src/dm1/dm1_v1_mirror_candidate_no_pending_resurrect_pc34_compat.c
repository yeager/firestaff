#include "dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat.h"

static const char s_f0280_anchor[] =
    "ReDMCSB REVIVE.C F0280:272-276 publishes G0299 and increments G0305 "
    "after a C127 mirror portrait candidate is appended to the party.";

static const char s_f0282_anchor[] =
    "ReDMCSB REVIVE.C F0282:744-806 reads G0305 - 1, handles C162 cancel "
    "at 744-758, and clears G0299 only inside the live candidate path at "
    "785-806.";

static const char s_command_2159_anchor[] =
    "ReDMCSB COMMAND.C:2159-2181 gates champion status and inventory "
    "commands on !G0299 while the mirror candidate panel owns input.";

static const char s_command_2302_anchor[] =
    "ReDMCSB COMMAND.C:2302-2311 gates spell and action commands on !G0299 "
    "while the mirror candidate panel owns input.";

static const char s_no_op_note[] =
    "contract_only=1 no-pending resurrect gate: REVIVE.C F0280:272-276 "
    "normally pairs G0299 with a G0305 append, but this regression starts "
    "with G0299 unchanged and G0305 unchanged at zero; REVIVE.C "
    "F0282:744-806 is not entered, so F0282 not invoked and the click is "
    "not consumed.";

static const char s_source_summary[] =
    "contract_only=1 source summary: REVIVE.C F0280:272-276 creates the "
    "pending candidate, REVIVE.C F0282:744-806 owns cancel/resurrect side "
    "effects, COMMAND.C:2159-2181 and COMMAND.C:2302-2311 keep sibling "
    "commands blocked while G0299 is live; with G0305 unchanged at zero, "
    "G0299 unchanged, G0305 unchanged, and F0282 not invoked, champion, "
    "C040 panel, and inventory state remain unchanged.";

static const Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat
    s_contract = {
        1,
        1,
        0,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        0,
        s_f0280_anchor,
        s_f0282_anchor,
        s_command_2159_anchor,
        s_command_2302_anchor,
        s_no_op_note,
        s_source_summary
    };

static const Dm1V1MirrorCandidateNoPendingResurrectStepPc34Compat s_steps[] = {
    {
        STEP_BEGIN_G0299_LIVE_G0305_ZERO,
        "STEP_BEGIN_G0299_LIVE_G0305_ZERO",
        "ReDMCSB REVIVE.C F0280:272-276",
        "Begin with G0299 live and G0305 zero, proving no pending candidate."
    },
    {
        STEP_RESURRECT_CLICK_DISPATCHED,
        "STEP_RESURRECT_CLICK_DISPATCHED",
        "ReDMCSB COMMAND.C F0359:1985-1989; REVIVE.C F0282:744-806",
        "Dispatch a C160/C162-style mirror panel action from the open C040 panel."
    },
    {
        STEP_GATE_NO_CANDIDATE_BLOCKS_F0282,
        "STEP_GATE_NO_CANDIDATE_BLOCKS_F0282",
        "ReDMCSB REVIVE.C F0282:744-806",
        "The no-pending guard blocks the F0282 path when G0305 is zero."
    },
    {
        STEP_F0282_NOT_INVOKED,
        "STEP_F0282_NOT_INVOKED",
        "ReDMCSB REVIVE.C F0282:744-806",
        "F0282 not invoked, so cancel/resurrect side effects do not run."
    },
    {
        STEP_ASSERT_G0299_G0305_UNCHANGED,
        "STEP_ASSERT_G0299_G0305_UNCHANGED",
        "ReDMCSB REVIVE.C F0280:272-276; REVIVE.C F0282:744-806",
        "Assert G0299 unchanged and G0305 unchanged after the no-op action."
    },
    {
        STEP_ASSERT_CHAMPION_INVENTORY_PANEL_UNCHANGED,
        "STEP_ASSERT_CHAMPION_INVENTORY_PANEL_UNCHANGED",
        "ReDMCSB COMMAND.C:2159-2181; COMMAND.C:2302-2311",
        "Assert champion state, inventory state, and C040 panel state unchanged."
    }
};

const Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat *
dm1_v1_mirror_candidate_no_pending_resurrect_contract_pc34_compat(void)
{
    return &s_contract;
}

size_t dm1_v1_mirror_candidate_no_pending_resurrect_steps_pc34_compat(
    Dm1V1MirrorCandidateNoPendingResurrectStepPc34Compat *out,
    size_t cap)
{
    size_t i;
    size_t count = sizeof(s_steps) / sizeof(s_steps[0]);

    if (out) {
        size_t n = cap < count ? cap : count;
        for (i = 0; i < n; ++i) {
            out[i] = s_steps[i];
        }
    }
    return count;
}
