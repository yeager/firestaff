
#ifndef NEXUS_V1_TRAPS_H
#define NEXUS_V1_TRAPS_H

#include "nexus_v1_champions.h"

#define NEXUS_MAX_TRAPS         128
#define NEXUS_PIT_DAMAGE          15
#define NEXUS_POISON_DART_DAMAGE  10
#define NEXUS_POISON_DURATION     60

typedef enum {
    NEXUS_TRAP_NONE = 0,
    NEXUS_TRAP_PIT,
    NEXUS_TRAP_PRESSURE_PLATE,
    NEXUS_TRAP_POISON_DART,
    NEXUS_TRAP_TELEPORTER
} Nexus_TrapKind;

typedef struct {
    Nexus_TrapKind kind;
    int level, x, y;
    int armed;
    int damage;
    int target_level, target_x, target_y;
    int rearm_ticks;
    int cooldown;
} Nexus_V1_Trap;

typedef struct {
    Nexus_V1_Trap traps[NEXUS_MAX_TRAPS];
    int count;
} Nexus_V1_TrapManager;

void nexus_v1_trap_manager_init(Nexus_V1_TrapManager *mgr);

int nexus_v1_trap_add(Nexus_V1_TrapManager *mgr, const Nexus_V1_Trap *trap);

const Nexus_V1_Trap *nexus_v1_trap_find(const Nexus_V1_TrapManager *mgr,
                                        int level, int x, int y);

typedef struct {
    int triggered;
    int damage_dealt;
    int fall_level, fall_x, fall_y;
    int poisoned;
} Nexus_V1_TrapResult;

Nexus_V1_TrapResult nexus_v1_trap_trigger(Nexus_V1_TrapManager *mgr,
                                          Nexus_V1_Champion *champion,
                                          int level, int x, int y);

void nexus_v1_trap_tick(Nexus_V1_TrapManager *mgr);

#endif
