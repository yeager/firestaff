
#include "nexus_v1_champions.h"
#include <string.h>
#include <stdio.h>

/* Nexus champion roster — Japanese names with ASCII romanization.
 * Stats follow DM1 balance (Nexus uses identical formulas). */
static const struct { const char *ascii; const char *jp; int cls; int hp; int sta; int mp; int str; int dex; int wis; int vit; } g_nexus_roster[] = {
    {"Syra",      "\xe3\x82\xb7\xe3\x83\xa9", NEXUS_CLASS_FIGHTER, 70, 55, 15, 55, 40, 25, 50},
    {"Leyla",     "\xe3\x83\xac\xe3\x82\xa4\xe3\x83\xa9", NEXUS_CLASS_WIZARD,  40, 35, 65, 25, 35, 60, 30},
    {"Nabi",      "\xe3\x83\x8a\xe3\x83\x93", NEXUS_CLASS_NINJA,   55, 60, 25, 40, 60, 30, 45},
    {"Gando",     "\xe3\x82\xac\xe3\x83\xb3\xe3\x83\x89", NEXUS_CLASS_PRIEST,  50, 40, 55, 35, 30, 55, 40},
    {"Torham",    "\xe3\x83\x88\xe3\x83\xab\xe3\x83\x8f\xe3\x83\xa0", NEXUS_CLASS_FIGHTER, 65, 50, 20, 50, 45, 28, 48},
    {"Elija",     "\xe3\x82\xa8\xe3\x83\xaa\xe3\x82\xb8\xe3\x83\xa3", NEXUS_CLASS_WIZARD,  38, 30, 70, 22, 32, 65, 28},
    {"Wu Tse",    "\xe3\x82\xa6\xe3\x83\xbc\xe3\x83\x84\xe3\x82\xa7", NEXUS_CLASS_NINJA,   52, 58, 30, 38, 55, 35, 42},
    {"Stamm",     "\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\xa0", NEXUS_CLASS_FIGHTER, 75, 60, 10, 60, 35, 20, 55},
    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0}
};

void nexus_v1_champions_init(Nexus_V1_ChampionPool *pool) {
    int i;
    if (!pool) return;
    memset(pool, 0, sizeof(*pool));
    for (i = 0; g_nexus_roster[i].ascii; i++) {
        Nexus_V1_Champion *c = &pool->champions[i];
        strncpy(c->name_ascii, g_nexus_roster[i].ascii, 31);
        strncpy(c->name_jp, g_nexus_roster[i].jp, 63);
        c->primary_class = g_nexus_roster[i].cls;
        c->health = c->max_health = g_nexus_roster[i].hp;
        c->stamina = c->max_stamina = g_nexus_roster[i].sta;
        c->mana = c->max_mana = g_nexus_roster[i].mp;
        c->strength = g_nexus_roster[i].str;
        c->dexterity = g_nexus_roster[i].dex;
        c->wisdom = g_nexus_roster[i].wis;
        c->vitality = g_nexus_roster[i].vit;
        c->anti_magic = 5;
        c->anti_fire = 5;
        c->food = 1500;
        c->water = 1500;
        c->alive = 1;
        c->portrait_index = i;
        pool->champion_count++;
    }
    /* Empty party */
    for (i = 0; i < NEXUS_MAX_PARTY; i++)
        pool->party[i] = -1;
    pool->leader_index = 0;
}

int nexus_v1_champion_recruit(Nexus_V1_ChampionPool *pool, int mirror_index) {
    int i;
    if (!pool || mirror_index < 0 || mirror_index >= pool->champion_count) return -1;
    if (!pool->champions[mirror_index].alive) return -1;
    if (pool->party_count >= NEXUS_MAX_PARTY) return -1;
    /* Check not already in party */
    for (i = 0; i < pool->party_count; i++)
        if (pool->party[i] == mirror_index) return -1;
    pool->party[pool->party_count++] = mirror_index;
    printf("Recruited %s (%s)\n", pool->champions[mirror_index].name_ascii,
        pool->champions[mirror_index].name_jp);
    return pool->party_count - 1;
}

int nexus_v1_champion_resurrect(Nexus_V1_ChampionPool *pool, int party_slot) {
    int idx;
    if (!pool || party_slot < 0 || party_slot >= pool->party_count) return -1;
    idx = pool->party[party_slot];
    if (idx < 0 || pool->champions[idx].alive) return -1;
    pool->champions[idx].alive = 1;
    pool->champions[idx].health = pool->champions[idx].max_health / 4;
    pool->champions[idx].stamina = pool->champions[idx].max_stamina / 4;
    return 0;
}

