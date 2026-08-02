
#ifndef NEXUS_V1_STATUS_H
#define NEXUS_V1_STATUS_H

/* Nexus V1 status effects — timed buffs/debuffs on champions.
 * Source: DM1 CHAMPION.C poison tick, COMMAND.C spell buffs,
 *         CREATURE.C poison-on-hit (CRET byte 13). */

#include <stdint.h>

#define NEXUS_STATUS_POISON       0
#define NEXUS_STATUS_SHIELD       1
#define NEXUS_STATUS_HASTE        2
#define NEXUS_STATUS_FIRE_SHIELD  3
#define NEXUS_STATUS_SEE_THROUGH  4
#define NEXUS_STATUS_CONFUSION    5
#define NEXUS_STATUS_COUNT        6

typedef struct {
    int active[NEXUS_STATUS_COUNT];
    int ticks[NEXUS_STATUS_COUNT];
    int strength[NEXUS_STATUS_COUNT];
} Nexus_StatusEffects;

void nexus_v1_status_init(Nexus_StatusEffects *se);

void nexus_v1_status_apply(Nexus_StatusEffects *se, int effect, int duration, int str);

void nexus_v1_status_remove(Nexus_StatusEffects *se, int effect);

int nexus_v1_status_is_active(const Nexus_StatusEffects *se, int effect);

int nexus_v1_status_strength(const Nexus_StatusEffects *se, int effect);

/* Tick all effects. Returns bitmask of effects that expired this tick. */
int nexus_v1_status_tick(Nexus_StatusEffects *se);

/* Apply poison damage. Returns damage dealt (strength / 4, min 1). */
int nexus_v1_status_poison_damage(const Nexus_StatusEffects *se);

/* Defense bonus from shield effect. */
int nexus_v1_status_defense_bonus(const Nexus_StatusEffects *se);

/* ═══════════════════════════════════════════════════════════════════
 * Experience and leveling
 * Source: DM1 CHAMPION.C F0309 experience table, level thresholds.
 * ═══════════════════════════════════════════════════════════════════ */

#define NEXUS_XP_CLASS_FIGHTER 0
#define NEXUS_XP_CLASS_NINJA   1
#define NEXUS_XP_CLASS_PRIEST  2
#define NEXUS_XP_CLASS_WIZARD  3
#define NEXUS_XP_CLASS_COUNT   4

#define NEXUS_MAX_CLASS_LEVEL  16

typedef struct {
    int xp[NEXUS_XP_CLASS_COUNT];
    int level[NEXUS_XP_CLASS_COUNT];
} Nexus_Experience;

void nexus_v1_xp_init(Nexus_Experience *xp);

void nexus_v1_xp_add(Nexus_Experience *xp, int xp_class, int amount);

int nexus_v1_xp_check_levelup(Nexus_Experience *xp, int xp_class);

int nexus_v1_xp_threshold(int level);

#endif
