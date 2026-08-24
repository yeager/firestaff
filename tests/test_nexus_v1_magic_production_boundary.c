#include <stdio.h>
#include <string.h>

#include "nexus_v1_magic.h"

int main(void)
{
    Nexus_V1_Champion champion;
    Nexus_SpellLookup lookup;
    int result;

    memset(&champion, 0, sizeof(champion));
    champion.mana = 500;
    champion.wizard_level = 5;
    champion.alive = 1;
    champion.max_health = 1000;
    champion.health = 500;

    /* power=5, element=FUL(3), form=IR(2), class=WIZARD(1)
     * table index = (3*4+2)*2+1 = 29, g_spell_table[29] = 0x0002 (LIGHT) */
    lookup = nexus_v1_spell_lookup(5, NEXUS_ELEM_FUL, NEXUS_FORM_IR,
                                   NEXUS_SPELL_CLASS_WIZARD);
    if (lookup.valid || lookup.spell_type != NEXUS_SPELL_INVALID ||
        lookup.mana_cost != -1) {
        fprintf(stderr, "FAIL: uncaptured spell lookup escaped production gate\n");
        return 1;
    }
    if (nexus_v1_spell_mana_cost(5, NEXUS_ELEM_FUL) != -1) {
        fprintf(stderr, "FAIL: uncaptured mana formula escaped production gate\n");
        return 1;
    }
    result = nexus_v1_cast_spell(&champion, 5, NEXUS_ELEM_FUL, NEXUS_FORM_IR, 0);
    if (result != -1 || champion.mana != 500) {
        fprintf(stderr, "FAIL: uncaptured spell cast mutated production state\n");
        return 1;
    }
    puts("PASS: production Nexus spell boundary remains fail-closed");
    return 0;
}
