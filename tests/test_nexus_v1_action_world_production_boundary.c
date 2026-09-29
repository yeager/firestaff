#include "nexus_v1_action_timer.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_doors.h"
#include "nexus_v1_squares.h"
#include "nexus_v1_traps.h"
#include "nexus_v1_projectiles.h"
#include "nexus_v1_movement.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_Engine engine;
    Nexus_MechanicsState mechanics;
    Nexus_ActionTimers timers;
    Nexus_DoorManager doors;
    Nexus_V1_TrapManager traps;
    Nexus_ProjectileManager projectiles;
    Nexus_V1_Trap trap;

    if (nexus_v1_action_semantics_proven() != 0) {
        fprintf(stderr,
                "FAIL: un-captured Saturn action semantics opened production dispatch\n");
        return 1;
    }

    /* A retail source identity alone is not an action receipt.  Exercise the
     * actual production tick boundary so queued input cannot move an
     * unplaced party or advance gameplay timers before Saturn ownership is
     * bound. */
    memset(&engine, 0, sizeof(engine));
    memset(&mechanics, 0, sizeof(mechanics));
    engine.source = NEXUS_SRC_ISO;
    nexus_mechanics_init(&mechanics, 7, 9, 0);
    mechanics.map_index = 1;
    if (nexus_mechanics_push_command(&mechanics, NEXUS_CMD_FORWARD) != 0 ||
        nexus_mechanics_tick(&mechanics, &engine) != 0 ||
        mechanics.party_x != 7 || mechanics.party_y != 9 ||
        mechanics.party_dir != 0 ||
        mechanics.total_ticks != 0 || mechanics.input_count != 1) {
        fprintf(stderr,
                "FAIL: retail-source Nexus mechanics tick mutated world state without a Saturn action receipt\n");
        return 1;
    }

    /* Production adapters deliberately preserve caller-owned state.  Give
     * their no-mutation contract a deterministic baseline on every ABI. */
    memset(&timers, 0, sizeof(timers));
    memset(&doors, 0, sizeof(doors));
    memset(&traps, 0, sizeof(traps));
    memset(&projectiles, 0, sizeof(projectiles));
    nexus_v1_action_timers_init(&timers);
    nexus_v1_action_start_cooldown(&timers, 0, 24, NULL);
    nexus_v1_action_timers_tick(&timers);
    if (timers.cooldown[0] != 0 || nexus_v1_action_remaining(&timers, 0) != 0) {
        fprintf(stderr, "FAIL: production action route mutated timer state\n");
        return 1;
    }

    nexus_v1_door_manager_init(&doors);
    if (nexus_v1_door_register(&doors, 1, 2, 0, 0, -1, 0, 0) != -1) {
        fprintf(stderr, "FAIL: production door route opened dispatch\n");
        return 1;
    }

    nexus_v1_trap_manager_init(&traps);
    memset(&trap, 0, sizeof(trap));
    trap.kind = NEXUS_TRAP_PRESSURE_PLATE;
    trap.armed = 1;
    if (nexus_v1_trap_add(&traps, &trap) != 0 ||
        nexus_v1_trap_find(&traps, 0, 0, 0) != NULL) {
        fprintf(stderr, "FAIL: production trap route mutated state\n");
        return 1;
    }

    nexus_v1_projectiles_init(&projectiles);
    if (nexus_v1_projectile_spawn(&projectiles, NEXUS_PROJ_FIREBALL,
                                  1, 2, 0, 10, 2, 0) != -1 ||
        nexus_v1_projectile_count(&projectiles) != 0) {
        fprintf(stderr, "FAIL: production projectile route mutated state\n");
        return 1;
    }

    puts("PASS: production Nexus action/world boundary remains fail-closed");
    return 0;
}
