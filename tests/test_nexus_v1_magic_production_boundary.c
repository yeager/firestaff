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
    if (!lookup.valid || lookup.spell_type == NEXUS_SPELL_INVALID) {
        fprintf(stderr, "FAIL: spell lookup should resolve a valid spell\n");
        return 1;
    }
    if (lookup.spell_type != NEXUS_SPELL_EFFECT_LIGHT) {
        fprintf(stderr, "FAIL: expected spell_type LIGHT (0x0002), got 0x%04X\n",
                lookup.spell_type);
        return 1;
    }

    /* mana cost for power=5: 50*5+25 = 275 */
    if (nexus_v1_spell_mana_cost(5, NEXUS_ELEM_FUL) != 275) {
        fprintf(stderr, "FAIL: mana cost for power 5 should be 275, got %d\n",
                nexus_v1_spell_mana_cost(5, NEXUS_ELEM_FUL));
        return 1;
    }

    /* cast_spell should succeed and deduct mana */
    result = nexus_v1_cast_spell(&champion, 5, NEXUS_ELEM_FUL, NEXUS_FORM_IR, 0);
    if (result < 0) {
        fprintf(stderr, "FAIL: cast_spell should succeed, got %d\n", result);
        return 1;
    }
    if (result != NEXUS_SPELL_EFFECT_LIGHT) {
        fprintf(stderr, "FAIL: cast_spell should return LIGHT, got %d\n", result);
        return 1;
    }
    if (champion.mana != 500 - 275) {
        fprintf(stderr, "FAIL: mana should be 225 after cast, got %d\n",
                champion.mana);
        return 1;
    }

    puts("PASS: production Nexus spell gameplay verification");
    return 0;
}
