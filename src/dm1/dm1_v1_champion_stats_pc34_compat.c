#include "dm1_v1_champion_stats_pc34_compat.h"
#include <stdio.h>
#include <string.h>

static const char* stat_names[] = {
    "Health", "Stamina", "Mana", "Strength", "Dexterity",
    "Wisdom", "Vitality", "Antifire", "Antimagic", "Luck"
};

static const char* skill_names[] = {
    "Fighter", "Ninja", "Priest", "Wizard"
};

void m11_stats_init(M11_ChampionStatsState* s) {
    if (!s) return;
    memset(s, 0, sizeof(M11_ChampionStatsState));
    s->leader = 0;
}

int m11_stats_add_champion(M11_ChampionStatsState* s, const char* name) {
    if (!s || !name) return -1;
    if (s->count >= M11_MAX_CHAMPIONS) return -1;

    int idx = s->count;
    M11_ChampionStats* c = &s->champions[idx];

    strncpy(c->name, name, sizeof(c->name) - 1);
    c->name[sizeof(c->name) - 1] = '\0';

    // Defaults
    c->stats[DM1_STAT_HEALTH] = 100;
    c->maxStats[DM1_STAT_HEALTH] = 100;

    c->stats[DM1_STAT_STAMINA] = 100;
    c->maxStats[DM1_STAT_STAMINA] = 100;

    c->stats[DM1_STAT_MANA] = 50;
    c->maxStats[DM1_STAT_MANA] = 50;

    c->stats[DM1_STAT_STRENGTH] = 30;
    c->maxStats[DM1_STAT_STRENGTH] = 30;

    c->stats[DM1_STAT_DEXTERITY] = 30;
    c->maxStats[DM1_STAT_DEXTERITY] = 30;

    c->stats[DM1_STAT_WISDOM] = 30;
    c->maxStats[DM1_STAT_WISDOM] = 30;

    c->stats[DM1_STAT_VITALITY] = 30;
    c->maxStats[DM1_STAT_VITALITY] = 30;

    c->stats[DM1_STAT_ANTIFIRE] = 10;
    c->maxStats[DM1_STAT_ANTIFIRE] = 10;

    c->stats[DM1_STAT_ANTIMAGIC] = 10;
    c->maxStats[DM1_STAT_ANTIMAGIC] = 10;

    c->stats[DM1_STAT_LUCK] = 10;
    c->maxStats[DM1_STAT_LUCK] = 10;

    for (int i = 0; i < DM1_SKILL_COUNT; i++) {
        c->skills[i] = 0;
        c->skillXP[i] = 0;
    }

    c->load = 0;
    c->wounds = 0;
    c->feetIconIndex = 0;
    c->level = 1;
    c->food = 1000;
    c->water = 1000;
    c->alive = 1;
    c->poisoned = 0;
    c->poisonAmount = 0;

    s->count++;
    return idx;
}

int m11_stats_get(const M11_ChampionStatsState* s, int champ, int stat) {
    if (!s) return 0;
    if (champ < 0 || champ >= s->count) return 0;
    if (stat < 0 || stat >= DM1_STAT_COUNT) return 0;
    return s->champions[champ].stats[stat];
}

void m11_stats_set(M11_ChampionStatsState* s, int champ, int stat, int val) {
    if (!s) return;
    if (champ < 0 || champ >= s->count) return;
    if (stat < 0 || stat >= DM1_STAT_COUNT) return;

    M11_ChampionStats* c = &s->champions[champ];
    if (val < 0) val = 0;
    if (val > c->maxStats[stat]) val = c->maxStats[stat];
    c->stats[stat] = val;
}

void m11_stats_modify(M11_ChampionStatsState* s, int champ, int stat, int delta) {
    if (!s) return;
    if (champ < 0 || champ >= s->count) return;
    if (stat < 0 || stat >= DM1_STAT_COUNT) return;

    M11_ChampionStats* c = &s->champions[champ];
    c->stats[stat] += delta;

    // Clamp
    if (c->stats[stat] < 0) c->stats[stat] = 0;
    if (c->stats[stat] > c->maxStats[stat]) c->stats[stat] = c->maxStats[stat];

    // If health reaches 0, kill
    if (stat == DM1_STAT_HEALTH && c->stats[DM1_STAT_HEALTH] <= 0) {
        m11_stats_kill(s, champ);
    }
}

void m11_stats_tick(M11_ChampionStatsState* s) {
    if (!s) return;

    for (int i = 0; i < s->count; i++) {
        M11_ChampionStats* c = &s->champions[i];
        if (!c->alive) continue;

        // Fixed per-tick food/water consumption (1 per tick, frame-rate independent)
        c->food -= 1;
        c->water -= 1;

        if (c->food < 0) c->food = 0;
        if (c->water < 0) c->water = 0;

        // If food <= 0, decrease health
        if (c->food <= 0) {
            m11_stats_modify(s, i, DM1_STAT_HEALTH, -1);
        }

        // If poisoned, decrease health by poisonAmount
        if (c->poisoned && c->poisonAmount > 0) {
            m11_stats_modify(s, i, DM1_STAT_HEALTH, -c->poisonAmount);
        }

        // Stamina regen: +1 every 3rd tick
        if (s->staminaTick++ % 3 == 0) {
            if (c->stats[DM1_STAT_STAMINA] < c->maxStats[DM1_STAT_STAMINA]) {
                c->stats[DM1_STAT_STAMINA] += 1;
                if (c->stats[DM1_STAT_STAMINA] > c->maxStats[DM1_STAT_STAMINA]) {
                    c->stats[DM1_STAT_STAMINA] = c->maxStats[DM1_STAT_STAMINA];
                }
            }
        }
    }
}

int m11_stats_is_alive(const M11_ChampionStatsState* s, int champ) {
    if (!s) return 0;
    if (champ < 0 || champ >= s->count) return 0;
    return s->champions[champ].alive;
}

void m11_stats_kill(M11_ChampionStatsState* s, int champ) {
    if (!s) return;
    if (champ < 0 || champ >= s->count) return;

    M11_ChampionStats* c = &s->champions[champ];
    c->alive = 0;
    c->stats[DM1_STAT_HEALTH] = 0;
}

void m11_stats_resurrect(M11_ChampionStatsState* s, int champ, int hp) {
    if (!s) return;
    if (champ < 0 || champ >= s->count) return;

    M11_ChampionStats* c = &s->champions[champ];
    c->alive = 1;
    if (hp > c->maxStats[DM1_STAT_HEALTH]) hp = c->maxStats[DM1_STAT_HEALTH];
    if (hp < 0) hp = 0;
    c->stats[DM1_STAT_HEALTH] = hp;
}

const char* m11_stat_name(int stat) {
    if (stat < 0 || stat >= DM1_STAT_COUNT) return "Unknown";
    return stat_names[stat];
}

const char* m11_skill_name(int skill) {
    if (skill < 0 || skill >= DM1_SKILL_COUNT) return "Unknown";
    return skill_names[skill];
}

int dm1_stats_stamina_adjusted_value_pc34(int currentStamina,
                                          int maximumStamina,
                                          int value) {
    int halfMaximumStamina = maximumStamina >> 1;

    /* ReDMCSB CHAMPION.C:1078-1104 / F0306. The assignment shift is
       intentional: the original uses the halved value for both terms. */
    if (halfMaximumStamina > 0 && currentStamina < halfMaximumStamina) {
        value >>= 1;
        return value + (int)(((long)value * (long)currentStamina) /
                             halfMaximumStamina);
    }
    return value;
}

int m11_stats_maximum_load_pc34(const M11_ChampionStats* champion) {
    int maximumLoad;

    if (!champion) return 0;

    /* ReDMCSB CHAMPION.C:1157-1177 / F0309. Units are the original
       tenths-of-kilogram load units used by the champion panel. */
    maximumLoad = (champion->stats[DM1_STAT_STRENGTH] << 3) + 100;
    maximumLoad = dm1_stats_stamina_adjusted_value_pc34(
        champion->stats[DM1_STAT_STAMINA],
        champion->maxStats[DM1_STAT_STAMINA],
        maximumLoad);

    if (champion->wounds) {
        maximumLoad -= maximumLoad >> ((champion->wounds & DM1_WOUND_LEGS) ? 2 : 3);
    }

    if (champion->feetIconIndex == DM1_ICON_ARMOUR_ELVEN_BOOTS) {
        maximumLoad += maximumLoad >> 4;
    }

    maximumLoad += 9;
    maximumLoad -= maximumLoad % 10;
    return maximumLoad;
}

int m11_stats_movement_ticks_pc34(const M11_ChampionStats* champion) {
    int load;
    int maximumLoad;
    int ticks;
    int woundTicks;

    if (!champion) return 0;

    /* ReDMCSB CHAMPION.C:1180-1215 / F0310, preserving BUG0_72:
       load == maximumLoad follows the overloaded branch. */
    load = champion->load;
    maximumLoad = m11_stats_maximum_load_pc34(champion);
    if (maximumLoad > load) {
        ticks = 2;
        if (((long)load << 3) > ((long)maximumLoad * 5)) {
            ticks++;
        }
        woundTicks = 1;
    } else if (maximumLoad > 0) {
        ticks = 4 + (((load - maximumLoad) << 2) / maximumLoad);
        woundTicks = 2;
    } else {
        ticks = 4;
        woundTicks = 2;
    }

    if (champion->wounds & DM1_WOUND_FEET) {
        ticks += woundTicks;
    }

    if (champion->feetIconIndex == DM1_ICON_ARMOUR_BOOT_OF_SPEED) {
        ticks--;
    }

    return ticks < 1 ? 1 : ticks;
}

int m11_stats_movement_stamina_cost_pc34(const M11_ChampionStats* champion) {
    int maximumLoad;

    if (!champion) return 0;

    /* ReDMCSB MOVESENS.C:590-598 applies this through F0325 when a
       champion uses a rope to climb down a pit. */
    maximumLoad = m11_stats_maximum_load_pc34(champion);
    if (maximumLoad <= 0) return 0;
    return ((champion->load * 25) / maximumLoad) + 1;
}

int m11_stats_load_color_pc34(const M11_ChampionStats* champion) {
    int maximumLoad;

    if (!champion) return DM1_LOAD_COLOR_LIGHTEST_GRAY;

    /* ReDMCSB CHAMDRAW.C:958-966 / F0292. The displayed load turns
       red only above maximum load; exactly-at-maximum remains yellow. */
    maximumLoad = m11_stats_maximum_load_pc34(champion);
    if (champion->load > maximumLoad) {
        return DM1_LOAD_COLOR_RED;
    }
    if (((long)champion->load << 3) > ((long)maximumLoad * 5)) {
        return DM1_LOAD_COLOR_YELLOW;
    }
    return DM1_LOAD_COLOR_LIGHTEST_GRAY;
}

int m11_stats_format_load_pc34(const M11_ChampionStats* champion,
                               char* out,
                               size_t outSize) {
    int currentWhole;
    int currentTenths;
    int maximumWhole;

    if (!champion || !out || outSize == 0) return 0;

    /* ReDMCSB CHAMDRAW.C:986-1006 / F0292 formats tenths-of-kilogram
       load as current integer, decimal digit, '/', rounded max, and KG. */
    currentWhole = champion->load / 10;
    currentTenths = champion->load - (currentWhole * 10);
    maximumWhole = (m11_stats_maximum_load_pc34(champion) + 5) / 10;
    snprintf(out, outSize, "%3d.%d/%3d KG",
             currentWhole, currentTenths, maximumWhole);
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — STATS.C remaining function citations
 *
 *   STATS.C:869 F8022_I
 *   STATS.C:57 F8024_G
 *   STATS.C:70 F8025_G
 *   STATS.C:85 F8026_G
 *   STATS.C:95 F8027_H
 *   STATS.C:123 F8028_G
 *   STATS.C:171 F8029_R
 *   STATS.C:200 F8030_R
 *   STATS.C:262 F8032_G
 *   STATS.C:299 F8033_I
 *   STATS.C:313 F8034_I
 *   STATS.C:323 F8035_G
 *   STATS.C:343 F8036_T
 *   STATS.C:380 F8037_I
 *   STATS.C:392 F8038_G
 *   STATS.C:410 F8039_I
 *   STATS.C:433 F8040_I
 *   STATS.C:447 F8041_I
 *   STATS.C:469 F8042_I
 *   STATS.C:522 F8043_G
 *   STATS.C:538 F8044_G
 * ══════════════════════════════════════════════════════════════════════ */
