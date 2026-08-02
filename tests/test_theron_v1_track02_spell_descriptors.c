#include "theron_v1_track02_spell_descriptors.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* N (index 0): cost=0, secondary=10 */
    assert(theron_v1_track02_action_spell_cost(0) == 0);
    assert(theron_v1_track02_action_spell_secondary(0) == 10);

    /* PUNCH (index 6): cost=1 (cheapest action) */
    assert(theron_v1_track02_action_spell_cost(6) == 1);

    /* FIREBALL (index 20): cost=42 (most expensive spell) */
    assert(theron_v1_track02_action_spell_cost(20) == 42);
    assert(theron_v1_track02_action_spell_secondary(20) == 8);

    /* LIGHTNING (index 23): cost=38 */
    assert(theron_v1_track02_action_spell_cost(23) == 38);

    /* HEAL (index 35): cost=2 (cheap spell) */
    assert(theron_v1_track02_action_spell_cost(35) == 2);

    /* CONFUSE (index 22): secondary=96 (highest secondary value) */
    assert(theron_v1_track02_action_spell_secondary(22) == 96);

    /* THROW (index 40): cost=22 */
    assert(theron_v1_track02_action_spell_cost(40) == 22);
    assert(theron_v1_track02_action_spell_secondary(40) == 0);

    /* Out of bounds */
    assert(theron_v1_track02_action_spell_cost(41) == 0);
    assert(theron_v1_track02_action_spell_secondary(41) == 0);

    printf("PASS: theron_v1_track02_spell_descriptors\n");
    return 0;
}
