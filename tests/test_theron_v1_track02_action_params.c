#include "theron_v1_track02_action_params.h"
#include <assert.h>
#include <string.h>

int main(void) {
    assert(theron_v1_track02_spell_param_count() == 21);

    const Theron_Track02SpellParam *fireball = theron_v1_track02_spell_param(0);
    assert(fireball != NULL);
    assert(fireball->field0 == 0x06);
    assert(fireball->field1 == 0x08);

    const Theron_Track02SpellParam *heal = theron_v1_track02_spell_param(15);
    assert(heal != NULL);
    assert(heal->field0 == 0x07);
    assert(heal->field1 == 0x0E);

    const Theron_Track02SpellParam *throw_p = theron_v1_track02_spell_param(20);
    assert(throw_p != NULL);
    assert(throw_p->field0 == 0x0A);
    assert(throw_p->field1 == 0x00);

    assert(theron_v1_track02_spell_param(21) == NULL);

    const uint8_t *raw = theron_v1_track02_action_param_raw();
    assert(raw != NULL);
    assert(raw[0] == 0x06);
    assert(raw[1] == 0x08);

    /* Combat action parameters */
    assert(theron_v1_track02_combat_action_param_count() == 20);

    const Theron_Track02CombatActionParam *n_action = theron_v1_track02_combat_action_param(0);
    assert(n_action != NULL);
    assert(n_action->field0 == 0x00);
    assert(n_action->field1 == 0x04);

    const Theron_Track02CombatActionParam *thrust = theron_v1_track02_combat_action_param(15);
    assert(thrust != NULL);
    assert(thrust->field0 == 0x11);
    assert(thrust->field1 == 0x13);

    assert(theron_v1_track02_combat_action_param(20) == NULL);

    /* PCE string pointer table */
    const uint16_t *ptrs = theron_v1_track02_action_ptr_table();
    assert(ptrs != NULL);
    assert(ptrs[0] == 0x6EA3);
    assert(ptrs[43] == 0x6FAF);
    /* HEAL appears twice */
    assert(ptrs[35] == ptrs[36]);
    assert(ptrs[35] == 0x6F8B);

    return 0;
}
