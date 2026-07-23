#include "dm1_v1_f0361_f0380_core_action_source_audit_pc34_compat.h"

#include <stddef.h>

static const char s_evidence[] =
    "ReDMCSB COMMAND.C F0361:1709-1954,F0378:1956-1994,F0379:1996-2027,"
    "F0380:2045-2479; CLIKMENU.C F0362:35-81 through F0371:519-573; "
    "CLIKVIEW.C F0372:5-76 through F0377:311-516; CLIKCHAM.C F0367:2-36,F0368:38-72.";

#define EXISTING(N, A, O) {N, DM1_V1_F0361_F0380_AUDIT_EXISTING_OWNER, 1, 1, A, O}
#define BOUNDARY(N, A) {N, DM1_V1_F0361_F0380_AUDIT_FAIL_CLOSED_BOUNDARY, 1, 1, A, "no proven DM1 runtime owner"}
static const DM1_V1_F0361F0380SourceAuditPc34Compat s_audit[] = {
    BOUNDARY(361u, "COMMAND.C F0361:1709-1954"),
    EXISTING(362u, "CLIKMENU.C F0362:35-81", "command_highlight_box_enable_pc34_compat"),
    EXISTING(363u, "CLIKMENU.C F0363:83-122", "dm1_v1_command_highlight_box_disable_pc34_compat"),
    BOUNDARY(364u, "CLIKMENU.C F0364:124-140"),
    BOUNDARY(365u, "CLIKMENU.C F0365:142-178"),
    BOUNDARY(366u, "CLIKMENU.C F0366:180-349"),
    BOUNDARY(367u, "CLIKCHAM.C F0367:2-36"),
    BOUNDARY(368u, "CLIKCHAM.C F0368:38-72"),
    EXISTING(369u, "CLIKMENU.C F0369:352-384", "dm1_v1_f0369_spell_zone_admission_pc34_compat"),
    EXISTING(370u, "CLIKMENU.C F0370:386-517", "dm1_v1_f0369_spell_zone_admission_pc34_compat"),
    BOUNDARY(371u, "CLIKMENU.C F0371:519-573"),
    EXISTING(372u, "CLIKVIEW.C F0372:5-76", "dm1_v1_viewport_click_pc34_compat"),
    EXISTING(373u, "CLIKVIEW.C F0373:78-129", "dm1_v1_viewport_click_pc34_compat"),
    EXISTING(374u, "CLIKVIEW.C F0374:131-189", "dm1_v1_viewport_click_pc34_compat"),
    EXISTING(375u, "CLIKVIEW.C F0375:191-288", "dm1_v1_viewport_click_pc34_compat"),
    EXISTING(376u, "CLIKVIEW.C F0376:290-309", "dm1_v1_input_poll_pc34_compat"),
    EXISTING(377u, "CLIKVIEW.C F0377:311-516", "dm1_v1_viewport_click_pc34_compat"),
    BOUNDARY(378u, "COMMAND.C F0378:1956-1994"),
    EXISTING(379u, "COMMAND.C F0379:1996-2027", "dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_pc34_compat"),
    BOUNDARY(380u, "COMMAND.C F0380:2045-2479")
};
#undef EXISTING
#undef BOUNDARY

const DM1_V1_F0361F0380SourceAuditPc34Compat *
dm1_v1_f0361_f0380_core_action_source_audit_pc34(uint16_t function_number)
{
    size_t index;
    for (index = 0u; index < sizeof(s_audit) / sizeof(s_audit[0]); ++index)
        if (s_audit[index].functionNumber == function_number) return &s_audit[index];
    return NULL;
}

const char *dm1_v1_f0361_f0380_core_action_source_evidence_pc34(void)
{
    return s_evidence;
}
