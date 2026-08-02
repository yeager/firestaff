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

    return 0;
}
