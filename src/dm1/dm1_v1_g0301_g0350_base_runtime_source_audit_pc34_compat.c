#include "dm1_v1_g0301_g0350_base_runtime_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "BASE.C/DEFS.H runtime global", owner, 1, 1, 1 }

static const DM1_V1_G0301G0350SourceAuditPc34 k_audit[] = {
    ROW(301, "dm1_v1_game_state_pc34_compat"), ROW(302, "dm1_v1_endgame_system_pc34_compat"),
    ROW(303, "fail_closed: no verified party-dead owner"), ROW(304, "fail_closed: no verified palette-index owner"),
    ROW(305, "fail_closed: no verified champion-count owner"), ROW(306, "dm1_v1_movement_pc34_compat"),
    ROW(307, "dm1_v1_movement_pc34_compat"), ROW(308, "fail_closed: no verified party-direction owner"),
    ROW(309, "dm1_v1_current_map_f0173_f0174_pc34_compat"), ROW(310, "fail_closed: no verified movement-lock owner"),
    ROW(311, "dm1_v1_movement_timing_pc34_compat"), ROW(312, "fail_closed: no verified projectile-direction owner"),
    ROW(313, "fail_closed: no verified game-time owner"), ROW(314, "fail_closed: CPSDF sector-read boundary"),
    ROW(315, "fail_closed: CPSDF sector-read boundary"), ROW(316, "fail_closed: CPSDF sector-read boundary"),
    ROW(317, "fail_closed: no verified vblank-input owner"), ROW(318, "fail_closed: no verified vblank-input owner"),
    ROW(319, "dm1_v1_save_load"), ROW(320, "fail_closed: unreferenced source state"),
    ROW(321, "fail_closed: no verified input-wait owner"), ROW(322, "fail_closed: no verified palette-switch owner"),
    ROW(323, "fail_closed: no verified palette-request owner"), ROW(324, "fail_closed: no verified viewport-request owner"),
    ROW(325, "fail_closed: no verified mouse-pointer owner"), ROW(326, "fail_closed: no verified mouse-refresh owner"),
    ROW(327, "fail_closed: no verified deferred-map owner"), ROW(328, "fail_closed: CPSE time-bomb boundary"),
    ROW(329, "fail_closed: CPSD source state"), ROW(330, "fail_closed: no verified event-expiry owner"),
    ROW(331, "fail_closed: no verified eye-press owner"), ROW(332, "fail_closed: no verified eye-release owner"),
    ROW(333, "fail_closed: no verified mouth-press owner"), ROW(334, "fail_closed: no verified mouth-release owner"),
    ROW(335, "fail_closed: no verified dialog-choice owner"), ROW(336, "fail_closed: no verified highlight-coordinate owner"),
    ROW(337, "fail_closed: no verified highlight-coordinate owner"), ROW(338, "fail_closed: no verified highlight-coordinate owner"),
    ROW(339, "fail_closed: no verified highlight-coordinate owner"), ROW(340, "fail_closed: no verified highlight-inversion owner"),
    ROW(341, "dm1_v1_command_highlight_box_enable_pc34_compat"), ROW(342, "fail_closed: no verified dungeon-palette-request owner"),
    ROW(343, "fail_closed: no verified dialog-bitmap owner"), ROW(344, "fail_closed: CPSE code-patch boundary"),
    ROW(345, "fail_closed: no verified blank-buffer owner"), ROW(346, "fail_closed: no verified middle-palette owner"),
    ROW(347, "dm1_v1_palette_top_and_bottom_screen_pc34_compat"), ROW(348, "fail_closed: no verified screen-bitmap owner"),
    ROW(349, "fail_closed: no verified random-state owner"), ROW(350, "fail_closed: no verified palette-switch owner")
};

#undef ROW

const DM1_V1_G0301G0350SourceAuditPc34 *
dm1_v1_g0301_g0350_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0301G0350SourceAuditPc34 *
dm1_v1_g0301_g0350_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0301_g0350_source_audit_evidence_pc34(void)
{
    return "ReDMCSB BASE.C:43-181 and DEFS.H:5741-5754 are the authority for "
           "G0301-G0350. Only existing source-named PC34 owners are bound; all "
           "unverified runtime, palette, dialog, and platform state fails closed. "
           "The audit does not render or synthesize behavior.";
}
