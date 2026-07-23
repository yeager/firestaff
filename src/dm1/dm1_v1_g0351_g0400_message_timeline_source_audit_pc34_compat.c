#include "dm1_v1_g0351_g0400_message_timeline_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "TEXT.C/TIMELINE.C/GROUP.C runtime global", owner, 1, 1, 1 }

static const DM1_V1_G0351G0400SourceAuditPc34 k_audit[] = {
    ROW(351, "fail_closed: no verified vblank-exception owner"), ROW(352, "fail_closed: no verified object-name-table owner"),
    ROW(353, "fail_closed: no verified DM1 string-buffer owner"), ROW(354, "fail_closed: no verified scroll-line-count owner"),
    ROW(355, "dm1_v1_text_message_pc34_compat"), ROW(356, "redmcsb_f0696_update_message_area_pc34_compat"),
    ROW(357, "fail_closed: no verified interface-font owner"), ROW(358, "dm1_v1_text_message_pc34_compat"),
    ROW(359, "dm1_v1_text_message_pc34_compat"), ROW(360, "dm1_v1_text_message_pc34_compat"),
    ROW(361, "fail_closed: no verified creature-attack-time owner"), ROW(362, "fail_closed: no verified party-movement-time owner"),
    ROW(363, "fail_closed: no verified secondary-direction owner"), ROW(364, "fail_closed: no verified creature-damage owner"),
    ROW(365, "fail_closed: no verified launcher-projectile owner"), ROW(366, "fail_closed: no verified poison-attack owner"),
    ROW(367, "fail_closed: no verified projectile-attack owner"), ROW(368, "fail_closed: CPSE code-patch boundary"),
    ROW(369, "dm1_v1_event_timer_pc34_compat"), ROW(370, "dm1_v1_event_timer_pc34_compat"),
    ROW(371, "dm1_v1_event_timer_pc34_compat"), ROW(372, "dm1_v1_event_timer_pc34_compat"),
    ROW(373, "dm1_v1_event_timer_pc34_compat"), ROW(374, "fail_closed: NOCOPYPROTECTION watchdog boundary"),
    ROW(375, "fail_closed: non-isomorphic active-group table"), ROW(376, "fail_closed: source platform active-group capacity"),
    ROW(377, "dm1_v1_g0377_active_group_count_pc34_compat"), ROW(378, "fail_closed: event-local group X has no verified ABI owner"),
    ROW(379, "fail_closed: event-local group Y has no verified ABI owner"), ROW(380, "fail_closed: no verified current-group Thing owner"),
    ROW(381, "fail_closed: event-local distance has no verified ABI owner"), ROW(382, "fail_closed: event-local direction has no verified ABI owner"),
    ROW(383, "fail_closed: no verified secondary-group-direction owner"), ROW(384, "fail_closed: no verified tested-direction owner"),
    ROW(385, "fail_closed: no verified flux-cage owner"), ROW(386, "fail_closed: no verified flux-cage-count owner"),
    ROW(387, "fail_closed: no verified group-blocked-cell owner"), ROW(388, "fail_closed: no verified group-blocked-Thing owner"),
    ROW(389, "fail_closed: no verified group-door-blocked owner"), ROW(390, "fail_closed: no verified group-party-blocked owner"),
    ROW(391, "fail_closed: no verified drop-cell-count owner"), ROW(392, "fail_closed: no verified drop-cell-table owner"),
    ROW(393, "fail_closed: CPSE code-patch boundary"), ROW(394, "fail_closed: no verified steal-slot-table owner"),
    ROW(395, "fail_closed: no verified half-creature-time owner"), ROW(396, "fail_closed: no verified half-creature-group owner"),
    ROW(397, "fail_closed: no verified move-result-X owner"), ROW(398, "fail_closed: no verified move-result-Y owner"),
    ROW(399, "fail_closed: no verified move-result-map owner"), ROW(400, "fail_closed: no verified move-result-direction owner")
};

#undef ROW

const DM1_V1_G0351G0400SourceAuditPc34 *
dm1_v1_g0351_g0400_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0351G0400SourceAuditPc34 *
dm1_v1_g0351_g0400_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0351_g0400_source_audit_evidence_pc34(void)
{
    return "ReDMCSB TEXT.C:7-67, SCRLTASK.C:17, PROJEXPL.C:5-11, TIMELINE.C:6-25, "
           "GROUP.C:11-30/1602, MOVESENS.C:5-8, and DEFS.H:5765-5840 are the "
           "authority for G0351-G0400. Only existing source-named PC34 owners are "
           "bound; unverified runtime and group ABI state fails closed. The audit "
           "does not render or synthesize behavior.";
}
