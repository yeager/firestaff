/* Nexus V1 production spell boundary.
 *
 * nexus_v1_magic.c is a source-study implementation of the DM.BIN spell
 * table and an inferred mana preview. The Saturn action dispatcher, caster
 * state writes, target/effect routing, RNG and SLEV/SFX publication are not
 * captured, so exporting that study through firestaff_nexus would make
 * unverified spell data executable. Keep the production ABI linkable while
 * every spell operation remains fail-closed.
 */

#include "nexus_v1_magic.h"

#include <string.h>

Nexus_SpellLookup nexus_v1_spell_lookup(int power, int element, int form,
                                        Nexus_SpellClass spell_class)
{
    Nexus_SpellLookup result;
    (void)power;
    (void)element;
    (void)form;
    (void)spell_class;
    memset(&result, 0, sizeof(result));
    result.spell_type = NEXUS_SPELL_INVALID;
    result.mana_cost = -1;
    result.required_skill = -1;
    return result;
}

int nexus_v1_spell_mana_cost(int power, int element)
{
    (void)power;
    (void)element;
    return -1;
}

int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int element,
                        int form, int align)
{
    (void)caster;
    (void)power;
    (void)element;
    (void)form;
    (void)align;
    return -1;
}

int nexus_v1_spell_damage(int power, Nexus_SpellClass spell_class)
{
    (void)power;
    (void)spell_class;
    return 0;
}

Nexus_SpellCategory nexus_v1_spell_category(int spell_type)
{
    (void)spell_type;
    return NEXUS_SPELL_CAT_DEBUFF;
}
