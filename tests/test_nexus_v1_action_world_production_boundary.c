#include "nexus_v1_action_timer.h"
#include "nexus_v1_doors.h"
#include "nexus_v1_traps.h"
#include "nexus_v1_projectiles.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_ActionTimers timers;
    Nexus_ActionTimers timers_before;
    Nexus_DoorManager doors;
    Nexus_DoorManager doors_before;
    Nexus_V1_TrapManager traps;
    Nexus_V1_TrapManager traps_before;
    Nexus_ProjectileManager projectiles;
    Nexus_ProjectileManager projectiles_before;
    Nexus_V1_Trap trap;

    memset(&timers, 0x1a, sizeof(timers));
    memset(&doors, 0x2b, sizeof(doors));
    memset(&traps, 0x3c, sizeof(traps));
    memset(&projectiles, 0x4d, sizeof(projectiles));
    memset(&trap, 0x5e, sizeof(trap));
    timers_before = timers;
    doors_before = doors;
    traps_before = traps;
    projectiles_before = projectiles;

    nexus_v1_action_timers_init(&timers);
    nexus_v1_action_start_cooldown(&timers, 0, 24, NULL);
    nexus_v1_action_timers_tick(&timers);
    nexus_v1_door_manager_init(&doors);
    nexus_v1_door_register(&doors, 1, 2, 0, 0, -1, 0, 0);
    nexus_v1_door_tick(&doors);
    nexus_v1_trap_manager_init(&traps);
    nexus_v1_trap_add(&traps, &trap);
    nexus_v1_trap_tick(&traps);
    nexus_v1_projectiles_init(&projectiles);
    nexus_v1_projectile_spawn(&projectiles, NEXUS_PROJ_FIREBALL,
                              1, 2, 0, 10, 2, 0);

    if (memcmp(&timers, &timers_before, sizeof(timers)) != 0 ||
        memcmp(&doors, &doors_before, sizeof(doors)) != 0 ||
        memcmp(&traps, &traps_before, sizeof(traps)) != 0 ||
        memcmp(&projectiles, &projectiles_before, sizeof(projectiles)) != 0 ||
        nexus_v1_action_is_ready(&timers, 0) != 0 ||
        nexus_v1_door_register(&doors, 1, 2, 0, 0, -1, 0, 0) != -1 ||
        nexus_v1_trap_find(&traps, 0, 1, 2) != NULL ||
        nexus_v1_projectile_count(&projectiles) != 0) {
        fprintf(stderr, "FAIL: production Nexus action/world route mutated or admitted inferred state\n");
        return 1;
    }

    puts("PASS: production Nexus action/world route is fail-closed");
    return 0;
}
