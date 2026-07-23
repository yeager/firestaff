#include "dm1_v1_l0301_l0350_local_owner_audit.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    uint16_t label;
    unsigned int fail_closed = 0u;

    for (label = 301u; label <= 350u; ++label) {
        const DM1V1L0301L0350LocalOwnerAudit *entry =
            dm1_v1_l0301_l0350_local_owner_find(label);
        if (!entry || entry->label != label || !entry->enclosing_routine ||
            !entry->redmcsb_anchor || !entry->firestaff_owner ||
            !entry->standalone_local_forbidden ||
            !entry->fallback_or_synthetic_state_forbidden) return 1;
        if (strstr(entry->firestaff_owner, "fail_closed:")) ++fail_closed;
    }
    if (fail_closed != 1u || dm1_v1_l0301_l0350_local_owner_find(300u) ||
        dm1_v1_l0301_l0350_local_owner_find(351u) ||
        !strstr(dm1_v1_l0301_l0350_local_owner_evidence(), "No independent ABI")) return 1;
    puts("PASS: DM1 L0301-L0350 local-owner audit");
    return 0;
}
