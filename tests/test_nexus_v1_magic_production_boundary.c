#include <stdio.h>
#include <string.h>

#include "nexus_v1_magic.h"

int main(void)
{
    Nexus_V1_Champion champion;
    Nexus_SpellLookup lookup;
    int mana_before;

    memset(&champion, 0, sizeof(champion));
    champion.mana = 500;
    mana_before = champion.mana;
    lookup = nexus_v1_spell_lookup(5, NEXUS_ELEM_FUL, NEXUS_FORM_IR,
                                   NEXUS_SPELL_CLASS_WIZARD);
    if (lookup.valid || lookup.spell_type != NEXUS_SPELL_INVALID ||
        lookup.mana_cost != -1 || lookup.required_skill != -1 ||
        nexus_v1_spell_mana_cost(5, NEXUS_ELEM_FUL) != -1 ||
        nexus_v1_spell_damage(5, NEXUS_SPELL_CLASS_WIZARD) != 0 ||
        nexus_v1_cast_spell(&champion, 5, NEXUS_ELEM_FUL, NEXUS_FORM_IR, 0) != -1 ||
        champion.mana != mana_before) {
        fprintf(stderr, "FAIL: production Nexus spell route exposed inferred behavior\n");
        return 1;
    }
    puts("PASS: production Nexus spell route is fail-closed");
    return 0;
}
