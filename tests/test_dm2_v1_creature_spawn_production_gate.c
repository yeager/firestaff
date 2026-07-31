/* Production boundary for the old standalone creature fixture.
 * SKProject SKWINSPX/src/v4/skcrture.cpp:6380-6430, ALLOC_NEW_CREATURE,
 * needs the live DB4 allocator, current map, record chain and RNG. */
#include "dm2_v1_creature.h"

#include <stdio.h>

int main(void)
{
    if (dm2_v1_creature_spawn(19, 7, 9, 3, 2, 31) != -1) {
        fputs("FAIL production accepted caller-authored creature state\n", stderr);
        return 1;
    }
    puts("PASS DM2 production creature-spawn gate");
    return 0;
}
