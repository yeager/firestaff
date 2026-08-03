
#include "nexus_v1_traps.h"
#include <string.h>

void nexus_v1_trap_manager_init(Nexus_V1_TrapManager *mgr)
{
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_trap_add(Nexus_V1_TrapManager *mgr, const Nexus_V1_Trap *trap)
{
    if (!mgr || !trap || mgr->count >= NEXUS_MAX_TRAPS)
        return 0;
    if (trap->kind == NEXUS_TRAP_NONE)
        return 0;
    mgr->traps[mgr->count++] = *trap;
    return 1;
}

const Nexus_V1_Trap *nexus_v1_trap_find(const Nexus_V1_TrapManager *mgr,
                                        int level, int x, int y)
{
    int i;
    if (!mgr) return NULL;
    for (i = 0; i < mgr->count; ++i) {
        const Nexus_V1_Trap *t = &mgr->traps[i];
        if (t->level == level && t->x == x && t->y == y)
            return t;
    }
    return NULL;
}

Nexus_V1_TrapResult nexus_v1_trap_trigger(Nexus_V1_TrapManager *mgr,
                                          Nexus_V1_Champion *champion,
                                          int level, int x, int y)
{
    Nexus_V1_TrapResult result;
    int i;

    memset(&result, 0, sizeof(result));
    if (!mgr) return result;

    for (i = 0; i < mgr->count; ++i) {
        Nexus_V1_Trap *t = &mgr->traps[i];
        if (t->level != level || t->x != x || t->y != y)
            continue;
        if (!t->armed || t->cooldown > 0)
            continue;

        result.triggered = 1;

        switch (t->kind) {
            case NEXUS_TRAP_PIT:
                result.damage_dealt = t->damage > 0 ? t->damage : NEXUS_PIT_DAMAGE;
                result.fall_level = t->target_level;
                result.fall_x = t->target_x;
                result.fall_y = t->target_y;
                if (champion) {
                    champion->health -= result.damage_dealt;
                    if (champion->health < 0) champion->health = 0;
                }
                t->armed = 0;
                break;

            case NEXUS_TRAP_POISON_DART:
                result.damage_dealt = t->damage > 0 ? t->damage : NEXUS_POISON_DART_DAMAGE;
                result.poisoned = 1;
                if (champion) {
                    champion->health -= result.damage_dealt;
                    if (champion->health < 0) champion->health = 0;
                }
                if (t->rearm_ticks > 0)
                    t->cooldown = t->rearm_ticks;
                else
                    t->armed = 0;
                break;

            case NEXUS_TRAP_PRESSURE_PLATE:
                result.damage_dealt = 0;
                if (t->rearm_ticks > 0)
                    t->cooldown = t->rearm_ticks;
                break;

            case NEXUS_TRAP_TELEPORTER:
                result.fall_level = t->target_level;
                result.fall_x = t->target_x;
                result.fall_y = t->target_y;
                break;

            default:
                result.triggered = 0;
                break;
        }
        break;
    }
    return result;
}

void nexus_v1_trap_tick(Nexus_V1_TrapManager *mgr)
{
    int i;
    if (!mgr) return;
    for (i = 0; i < mgr->count; ++i) {
        if (mgr->traps[i].cooldown > 0)
            --mgr->traps[i].cooldown;
    }
}
