#include "dm1_v1_champion_stats_pc34_compat.h"
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

void m11_stats_tick(M11_ChampionStatsState* s, int tickMs) {
    if (!s) return;

    for (int i = 0; i < s->count; i++) {
        M11_ChampionStats* c = &s->champions[i];
        if (!c->alive) continue;

        // Decrease food/water by tickMs/100
        int consumption = tickMs / 100;
        if (consumption < 1) consumption = 1; // Ensure at least some consumption if tickMs > 0

        c->food -= consumption;
        c->water -= consumption;

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

        // Regen stamina slowly (+1 per 500ms)
        if (tickMs >= 500) {
            int regen = tickMs / 500;
            if (c->stats[DM1_STAT_STAMINA] < c->maxStats[DM1_STAT_STAMINA]) {
                c->stats[DM1_STAT_STAMINA] += regen;
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