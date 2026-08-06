#include "nexus_v1_action_timer.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_doors.h"
#include "nexus_v1_squares.h"
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
    Nexus_MechanicsState mechanics;
    Nexus_MechanicsState mechanics_before;
    Nexus_V1_Engine engine;
    int new_level = 99;

    memset(&timers, 0x1a, sizeof(timers));
    memset(&doors, 0x2b, sizeof(doors));
    memset(&traps, 0x3c, sizeof(traps));
    memset(&projectiles, 0x4d, sizeof(projectiles));
    memset(&trap, 0x5e, sizeof(trap));
    memset(&mechanics, 0x6f, sizeof(mechanics));
    memset(&engine, 0, sizeof(engine));
    timers_before = timers;
    doors_before = doors;
    traps_before = traps;
    projectiles_before = projectiles;
    mechanics_before = mechanics;
    engine.source = NEXUS_SRC_ISO;
    engine.automap.map_open = 0;

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
    nexus_doors_init();

    if (memcmp(&timers, &timers_before, sizeof(timers)) != 0 ||
        memcmp(&doors, &doors_before, sizeof(doors)) != 0 ||
        memcmp(&traps, &traps_before, sizeof(traps)) != 0 ||
        memcmp(&projectiles, &projectiles_before, sizeof(projectiles)) != 0 ||
        nexus_v1_action_is_ready(&timers, 0) != 0 ||
        nexus_v1_door_register(&doors, 1, 2, 0, 0, -1, 0, 0) != -1 ||
        nexus_v1_trap_find(&traps, 0, 1, 2) != NULL ||
        nexus_v1_projectile_count(&projectiles) != 0 ||
        nexus_mechanics_dispatch_event(&mechanics, &engine,
                                       NEXUS_UI_EVENT_MAP, 0) != -1 ||
        engine.automap.map_open != 0 ||
        nexus_mechanics_dispatch_event(&mechanics, &engine,
                                       NEXUS_UI_EVENT_INVENTORY, 3) != -1 ||
        memcmp(&mechanics, &mechanics_before, sizeof(mechanics)) != 0 ||
        nexus_v1_engine_level_change(&engine, &new_level) != -1 ||
        new_level != -1 ||
        nexus_doors_open(1, 2) != -1 ||
        nexus_doors_close(1, 2) != -1 ||
        nexus_doors_lock(1, 2, 66) != -1 ||
        nexus_doors_is_open(1, 2) != 0 ||
        nexus_doors_is_passable(1, 2) != 0 ||
        nexus_doors_can_open(1, 2, NULL) != 0 ||
        nexus_doors_tick_animation() != 0 ||
        nexus_doors_animation_step(1, 2) != 0) {
        fprintf(stderr, "FAIL: production Nexus action/world route mutated or admitted inferred state\n");
        return 1;
    }

    puts("PASS: production Nexus action/world route is fail-closed");
    return 0;
}
