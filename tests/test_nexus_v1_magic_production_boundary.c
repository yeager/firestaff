#include <stdio.h>
#include <string.h>

#include "nexus_v1_magic.h"

int main(void)
{
    Nexus_V1_Champion champion;
    Nexus_SpellLookup lookup;

    memset(&champion, 0, sizeof(champion));
    champion.mana = 500;
    champion.wizard_level = 5;
    lookup = nexus_v1_spell_lookup(5, NEXUS_ELEM_FUL, NEXUS_FORM_IR,
                                   NEXUS_SPELL_CLASS_WIZARD);
    if (!lookup.valid) {
        fprintf(stderr, "FAIL: real spell lookup did not return valid\n");
        return 1;
    }
    if (nexus_v1_spell_mana_cost(5, NEXUS_ELEM_FUL) <= 0) {
        fprintf(stderr, "FAIL: real spell mana cost not positive\n");
        return 1;
    }
    puts("PASS: production Nexus spell route returns real values");
    return 0;
}
