
#include "nexus_v1_magic.h"
#include <stdio.h>

/* Mana cost: power_level^2 * element_multiplier */
static const int g_elem_cost_mult[] = {2, 3, 2, 4, 3, 3};

int nexus_v1_spell_mana_cost(int power, int elem) {
    if (power < 0 || power > 5) return 999;
    if (elem < 0 || elem > 5) return 999;
    return (power + 1) * (power + 1) * g_elem_cost_mult[elem];
}

int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int elem, int form, int align) {
    int cost, skill_req;
    (void)form; (void)align; /* TODO: full rune combination parsing */

    if (!caster || !caster->alive) return -1;

    cost = nexus_v1_spell_mana_cost(power, elem);
    if (caster->mana < cost) {
        printf("%s: not enough mana (%d < %d)\n", caster->name_ascii, caster->mana, cost);
        return -1;
    }

    /* Skill check: need priest or wizard level >= power */
    skill_req = power;
    if (caster->priest_level < skill_req && caster->wizard_level < skill_req) {
        printf("%s: skill too low for power level %d\n", caster->name_ascii, power);
        return -1;
    }

    caster->mana -= cost;
    printf("%s casts spell (power=%d, elem=%d, cost=%d)\n",
        caster->name_ascii, power, elem, cost);
    return cost;
}

