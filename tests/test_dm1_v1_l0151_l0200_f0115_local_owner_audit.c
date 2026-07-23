#include "dm1_v1_l0151_l0200_f0115_local_owner_audit.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const DM1V1L0151L0200LocalOwnerAudit *entries;
    size_t count;
    size_t index;
    unsigned int creature_count = 0u;
    unsigned int projectile_effect_count = 0u;

    entries = dm1_v1_l0151_l0200_local_owner_audit(&count);
    if (!entries || count != 50u) return 1;
    for (index = 0u; index < count; ++index) {
        const DM1V1L0151L0200LocalOwnerAudit *found;
        const uint16_t expected_label = (uint16_t)(151u + index);
        const uint16_t expected_routine = expected_label == 200u ? 116u : 115u;

        if (entries[index].label != expected_label ||
            entries[index].enclosing_routine != expected_routine ||
            !entries[index].redmcsb_anchor || !entries[index].firestaff_owner ||
            !entries[index].standalone_local_forbidden ||
            !entries[index].fallback_or_synthetic_state_forbidden) return 1;
        found = dm1_v1_l0151_l0200_local_owner_find(expected_label, expected_routine);
        if (found != &entries[index]) return 1;
        if (entries[index].owner_kind == DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL) {
            ++creature_count;
        }
        if (entries[index].owner_kind == DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT) {
            ++projectile_effect_count;
        }
    }
    if (creature_count != 24u || projectile_effect_count != 25u ||
        !strstr(dm1_v1_l0151_l0200_local_owner_evidence(), "No independent ABI") ||
        dm1_v1_l0151_l0200_local_owner_find(151u, 116u) ||
        dm1_v1_l0151_l0200_local_owner_find(201u, 116u)) return 1;
    puts("PASS: DM1 L0151-L0200 F0115/F0116 local-owner audit");
    return 0;
}
