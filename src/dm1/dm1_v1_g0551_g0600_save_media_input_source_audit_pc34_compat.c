#include "dm1_v1_g0551_g0600_save_media_input_source_audit_pc34_compat.h"

#define ROW(number, boundary) { number##u, "DEFS.H/SOUND.C/CHAMPION.C/IO.C runtime global", boundary, 1, 1, 1 }

static const DM1_V1_G0551G0600SourceAuditPc34 k_audit[] = {
    ROW(551, "fail_closed: no verified save-progress-text owner"), ROW(552, "fail_closed: no verified save-progress-text owner"),
    ROW(553, "fail_closed: no verified format-progress-text owner"), ROW(554, "fail_closed: no verified format-confirmation-text owner"),
    ROW(555, "fail_closed: no verified disk-error-text owner"), ROW(556, "fail_closed: no verified load-text owner"),
    ROW(557, "fail_closed: no verified save-play-text owner"), ROW(558, "fail_closed: no verified save-quit-text owner"),
    ROW(559, "fail_closed: platform floppy-format boundary"), ROW(560, "fail_closed: no verified dialog-text owner"),
    ROW(561, "fail_closed: no verified dialog-text owner"), ROW(562, "fail_closed: no verified entrance-door-bitmap owner"),
    ROW(563, "fail_closed: no verified entrance-screen-bitmap owner"), ROW(564, "fail_closed: no verified credits-bitmap owner"),
    ROW(565, "fail_closed: no verified door-rattle-sound owner"), ROW(566, "fail_closed: no verified switch-sound owner"),
    ROW(567, "fail_closed: platform disk-drive boundary"), ROW(568, "fail_closed: platform disk-type boundary"),
    ROW(569, "fail_closed: no verified save-file-name owner"), ROW(570, "fail_closed: no verified save-backup-name owner"),
    ROW(571, "fail_closed: platform disk-drive boundary"), ROW(572, "fail_closed: platform disk-drive boundary"),
    ROW(573, "fail_closed: platform floppy-index boundary"), ROW(574, "fail_closed: platform floppy-index boundary"),
    ROW(575, "fail_closed: editor save-file-name boundary"), ROW(576, "fail_closed: editor save-backup-name boundary"),
    ROW(577, "fail_closed: platform test-file boundary"), ROW(578, "fail_closed: no verified byte-box-coordinate owner"),
    ROW(579, "fail_closed: no verified sound-state owner"), ROW(580, "fail_closed: no verified PSG-register owner"),
    ROW(581, "fail_closed: no verified system-variable owner"), ROW(582, "fail_closed: no verified sound-state owner"),
    ROW(583, "fail_closed: no verified pending-sound owner"), ROW(584, "fail_closed: no verified pending-volume owner"),
    ROW(585, "fail_closed: CPSE sound-patch boundary"), ROW(586, "fail_closed: CPSD immediate-sound boundary"),
    ROW(587, "fail_closed: no verified mouse-hide owner"), ROW(588, "fail_closed: no verified mouse-buttons owner"),
    ROW(589, "fail_closed: no verified mouse-hotspot owner"), ROW(590, "fail_closed: no verified mouse-hotspot owner"),
    ROW(591, "fail_closed: no verified mouse-position owner"), ROW(592, "fail_closed: no verified mouse-build-request owner"),
    ROW(593, "fail_closed: no verified mouse-build-completion owner"), ROW(594, "fail_closed: no verified mouse-exception owner"),
    ROW(595, "fail_closed: no verified mouse-visible owner"), ROW(596, "fail_closed: no verified mouse-vblank owner"),
    ROW(597, "fail_closed: no verified mouse-ignore owner"), ROW(598, "fail_closed: no verified mouse-bitmap owner"),
    ROW(599, "fail_closed: no verified champion-mouse-icon owner"), ROW(600, "fail_closed: no verified object-mouse-icon owner")
};

#undef ROW

const DM1_V1_G0551G0600SourceAuditPc34 *
dm1_v1_g0551_g0600_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0551G0600SourceAuditPc34 *
dm1_v1_g0551_g0600_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0551_g0600_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:6100-6135, FLOPPY.C:45, CEDT005.C:217-218, CEDT029.C:8-13, "
           "FLOPPYST.C:59, BLIT.C:16, SOUND.C:12-48/1518, CHAMPION.C:6-15, and "
           "IO.C:36-50 are the authority for G0551-G0600. Existing Firestaff media, "
           "input, and platform state is not a verified PC34 global ABI owner; every "
           "row fails closed. The audit does not render or synthesize behavior.";
}
