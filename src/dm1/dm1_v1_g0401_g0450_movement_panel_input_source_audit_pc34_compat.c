#include "dm1_v1_g0401_g0450_movement_panel_input_source_audit_pc34_compat.h"

#define ROW(number, boundary) { number##u, "MOVESENS.C/CHAMPION.C/PANEL.C/COMMAND.C global", boundary, 1, 1, 1 }

static const DM1_V1_G0401G0450SourceAuditPc34 k_audit[] = {
    ROW(401, "fail_closed: no verified move-result-cell owner"), ROW(402, "fail_closed: no verified rope-pit owner"),
    ROW(403, "fail_closed: no verified sensor-rotation owner"), ROW(404, "fail_closed: no verified sensor-map-X owner"),
    ROW(405, "fail_closed: no verified sensor-map-Y owner"), ROW(406, "fail_closed: no verified sensor-cell owner"),
    ROW(407, "fail_closed: no verified Party ABI owner"), ROW(408, "fail_closed: CPSE code-patch boundary"),
    ROW(409, "fail_closed: no verified champion-pending-damage owner"), ROW(410, "fail_closed: no verified champion-pending-wounds owner"),
    ROW(411, "fail_closed: no verified leader-index owner"), ROW(412, "fail_closed: no source symbol"),
    ROW(413, "fail_closed: no source symbol"), ROW(414, "fail_closed: no source symbol"),
    ROW(415, "fail_closed: no verified leader-hand owner"), ROW(416, "fail_closed: no verified mouse-delay owner"),
    ROW(417, "fail_closed: no verified base-skill-name owner"), ROW(418, "fail_closed: CPSE event-time boundary"),
    ROW(419, "fail_closed: no verified integer-buffer owner"), ROW(420, "fail_closed: no verified mouse-icon owner"),
    ROW(421, "fail_closed: no verified object-description-X owner"), ROW(422, "fail_closed: no verified object-description-Y owner"),
    ROW(423, "fail_closed: no verified inventory-champion owner"), ROW(424, "fail_closed: no verified panel-content owner"),
    ROW(425, "fail_closed: no verified chest-slot-table owner"), ROW(426, "fail_closed: no verified open-chest owner"),
    ROW(427, "fail_closed: CPSE code-patch boundary"), ROW(428, "fail_closed: no verified skill-name owner"),
    ROW(429, "fail_closed: no verified Graphic21 result owner"), ROW(430, "fail_closed: no verified direction-name owner"),
    ROW(431, "fail_closed: no verified statistic-name owner"), ROW(432, "fail_closed: no verified command-queue owner"),
    ROW(433, "fail_closed: editor command-queue boundary"), ROW(434, "fail_closed: no verified command-queue-index owner"),
    ROW(435, "fail_closed: no verified command-queue-lock owner"), ROW(436, "fail_closed: editor pending-click boundary"),
    ROW(437, "fail_closed: editor pending-click boundary"), ROW(438, "fail_closed: editor pending-click boundary"),
    ROW(439, "fail_closed: editor pending-click boundary"), ROW(440, "fail_closed: no verified mouse-search owner"),
    ROW(441, "fail_closed: no verified primary-mouse-input owner"), ROW(442, "fail_closed: no verified secondary-mouse-input owner"),
    ROW(443, "fail_closed: no verified primary-keyboard-input owner"), ROW(444, "fail_closed: no verified secondary-keyboard-input owner"),
    ROW(445, "fail_closed: no verified entrance-input-table owner"), ROW(446, "fail_closed: no verified restart-input-table owner"),
    ROW(447, "fail_closed: no verified interface-input-table owner"), ROW(448, "fail_closed: no verified movement-input-table owner"),
    ROW(449, "fail_closed: no verified inventory-input-table owner"), ROW(450, "fail_closed: no verified party-rest-input-table owner")
};

#undef ROW

const DM1_V1_G0401G0450SourceAuditPc34 *
dm1_v1_g0401_g0450_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0401G0450SourceAuditPc34 *
dm1_v1_g0401_g0450_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0401_g0450_source_audit_evidence_pc34(void)
{
    return "ReDMCSB MOVESENS.C:9-14, CHAMPION.C:27-68, CHAMDRAW.C:18/371, "
           "PANEL.C:5-16, COMMAND.C:6-37, CEDT001.C:6-11, and DEFS.H:5870-5913 "
           "are the authority for G0401-G0450. Existing Firestaff state is only a "
           "semantic candidate or event-local receipt, never a verified global ABI "
           "owner; every row fails closed. The audit does not render or synthesize behavior.";
}
