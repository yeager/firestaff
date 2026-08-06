/* Production boundary for the old standalone creature fixture.
 * SKProject SKWINSPX/src/v4/skcrture.cpp:6380-6430, ALLOC_NEW_CREATURE,
 * needs the live DB4 allocator, current map, record chain and RNG. */
#include "dm2_v1_creature.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_CreatureLiveState before;
    DM2_V1_CreatureLiveState after;
    DM2_V1_CreatureLiveState authored;

    if (dm2_v1_creature_spawn(19, 7, 9, 3, 2, 31) != -1) {
        fputs("FAIL production accepted caller-authored creature state\n", stderr);
        return 1;
    }
    if (dm2_v1_creature_set_gdat_animation_state(0, 0x1234u, 0x5678u) != -1) {
        fputs("FAIL production accepted caller-authored animation state\n", stderr);
        return 1;
    }
    if (dm2_v1_creature_export_live_state(&before) != 0 ||
        before.tick_counter != 0) {
        fputs("FAIL production creature pool is not initially empty\n", stderr);
        return 1;
    }
    authored = before;
    authored.instances[0].alive = 1;
    authored.instances[0].is_visible = 1;
    authored.instances[0].direction = 2;
    authored.instances[0].ai_index = 0;
    if (dm2_v1_creature_restore_live_state(&authored) != -1) {
        fputs("FAIL production accepted a caller-authored creature pool\n", stderr);
        return 1;
    }
    if (dm2_v1_creature_export_live_state(&after) != 0 ||
        after.instances[0].alive != 0) {
        fputs("FAIL rejected creature pool changed production state\n", stderr);
        return 1;
    }
    dm2_v1_runtime_tick();
    if (dm2_v1_creature_export_live_state(&after) != 0 ||
        after.tick_counter != before.tick_counter) {
        fputs("FAIL runtime tick advanced the fixture-only creature pool\n", stderr);
        return 1;
    }
    puts("PASS DM2 production creature-spawn gate");
    return 0;
}
