
#include "nexus_v1_status.h"
#include <string.h>

void nexus_v1_status_init(Nexus_StatusEffects *se) {
    if (!se) return;
    memset(se, 0, sizeof(*se));
}

void nexus_v1_status_apply(Nexus_StatusEffects *se, int effect, int duration, int str) {
    if (!se || effect < 0 || effect >= NEXUS_STATUS_COUNT) return;
    se->active[effect] = 1;
    se->ticks[effect] = duration;
    se->strength[effect] = str;
}

void nexus_v1_status_remove(Nexus_StatusEffects *se, int effect) {
    if (!se || effect < 0 || effect >= NEXUS_STATUS_COUNT) return;
    se->active[effect] = 0;
    se->ticks[effect] = 0;
    se->strength[effect] = 0;
}

int nexus_v1_status_is_active(const Nexus_StatusEffects *se, int effect) {
    if (!se || effect < 0 || effect >= NEXUS_STATUS_COUNT) return 0;
    return se->active[effect];
}

int nexus_v1_status_strength(const Nexus_StatusEffects *se, int effect) {
    if (!se || effect < 0 || effect >= NEXUS_STATUS_COUNT) return 0;
    return se->strength[effect];
}

int nexus_v1_status_tick(Nexus_StatusEffects *se) {
    int i, expired = 0;
    if (!se) return 0;
    for (i = 0; i < NEXUS_STATUS_COUNT; i++) {
        if (!se->active[i]) continue;
        se->ticks[i]--;
        if (se->ticks[i] <= 0) {
            se->active[i] = 0;
            se->ticks[i] = 0;
            se->strength[i] = 0;
            expired |= (1 << i);
        }
    }
    return expired;
}

int nexus_v1_status_poison_damage(const Nexus_StatusEffects *se) {
    int dmg;
    if (!se || !se->active[NEXUS_STATUS_POISON]) return 0;
    dmg = se->strength[NEXUS_STATUS_POISON] / 4;
    return dmg > 0 ? dmg : 1;
}

int nexus_v1_status_defense_bonus(const Nexus_StatusEffects *se) {
    if (!se || !se->active[NEXUS_STATUS_SHIELD]) return 0;
    return se->strength[NEXUS_STATUS_SHIELD];
}

/* DM1 experience thresholds per level (approximate from ReDMCSB).
 * Level 1 = 500 XP, doubles each level up to 16. */
static const int xp_thresholds[NEXUS_MAX_CLASS_LEVEL + 1] = {
    0, 500, 1000, 2000, 4000, 8000, 16000, 32000,
    64000, 128000, 256000, 512000, 1024000, 2048000, 4096000, 8192000,
    16384000
};

void nexus_v1_xp_init(Nexus_Experience *xp) {
    if (!xp) return;
    memset(xp, 0, sizeof(*xp));
}

void nexus_v1_xp_add(Nexus_Experience *xp, int xp_class, int amount) {
    if (!xp || xp_class < 0 || xp_class >= NEXUS_XP_CLASS_COUNT) return;
    if (amount <= 0) return;
    xp->xp[xp_class] += amount;
}

int nexus_v1_xp_check_levelup(Nexus_Experience *xp, int xp_class) {
    int leveled = 0;
    if (!xp || xp_class < 0 || xp_class >= NEXUS_XP_CLASS_COUNT) return 0;
    while (xp->level[xp_class] < NEXUS_MAX_CLASS_LEVEL &&
           xp->xp[xp_class] >= xp_thresholds[xp->level[xp_class] + 1]) {
        xp->level[xp_class]++;
        leveled++;
    }
    return leveled;
}

int nexus_v1_xp_threshold(int level) {
    if (level < 0 || level > NEXUS_MAX_CLASS_LEVEL) return 0;
    return xp_thresholds[level];
}
