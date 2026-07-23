#include "dm1_v1_l0201_l0250_local_owner_audit.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    uint16_t label;
    unsigned int fail_closed = 0u;

    for (label = 201u; label <= 250u; ++label) {
        const DM1V1L0201L0250LocalOwnerAudit *entry =
            dm1_v1_l0201_l0250_local_owner_find(label);
        if (!entry || entry->label != label || !entry->enclosing_routine ||
            !entry->redmcsb_anchor || !entry->firestaff_owner ||
            !entry->standalone_local_forbidden ||
            !entry->fallback_or_synthetic_state_forbidden) return 1;
        if (strstr(entry->firestaff_owner, "fail_closed:")) ++fail_closed;
    }
    if (fail_closed != 10u || dm1_v1_l0201_l0250_local_owner_find(200u) ||
        dm1_v1_l0201_l0250_local_owner_find(251u) ||
        !strstr(dm1_v1_l0201_l0250_local_owner_evidence(), "No independent ABI")) return 1;
    puts("PASS: DM1 L0201-L0250 local-owner audit");
    return 0;
}
