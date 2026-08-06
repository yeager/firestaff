#include "nexus_v1_champions.h"
#include <string.h>

/* Legacy compatibility data for isolated tests only.  This is deliberately
 * outside firestaff_nexus: production must use authenticated RLOWFIX/PLRD
 * records and never these inferred names or statistics. */
static const struct {
    const char *ascii;
    const char *jp;
    int cls;
    int hp;
    int sta;
    int mp;
    int str;
    int dex;
    int wis;
    int vit;
} g_nexus_roster[] = {
    {"Syra", "\xe3\x82\xb7\xe3\x83\xa9", NEXUS_CLASS_FIGHTER, 70, 55, 15, 55, 40, 25, 50},
    {"Leyla", "\xe3\x83\xac\xe3\x82\xa4\xe3\x83\xa9", NEXUS_CLASS_WIZARD, 40, 35, 65, 25, 35, 60, 30},
    {"Nabi", "\xe3\x83\x8a\xe3\x83\x93", NEXUS_CLASS_NINJA, 55, 60, 25, 40, 60, 30, 45},
    {"Gando", "\xe3\x82\xac\xe3\x83\xb3\xe3\x83\x89", NEXUS_CLASS_PRIEST, 50, 40, 55, 35, 30, 55, 40},
    {"Torham", "\xe3\x83\x88\xe3\x83\xab\xe3\x83\x8f\xe3\x83\xa0", NEXUS_CLASS_FIGHTER, 65, 50, 20, 50, 45, 28, 48},
    {"Elija", "\xe3\x82\xa8\xe3\x83\xaa\xe3\x82\xb8\xe3\x83\xa3", NEXUS_CLASS_WIZARD, 38, 30, 70, 22, 32, 65, 28},
    {"Wu Tse", "\xe3\x82\xa6\xe3\x83\xbc\xe3\x83\x84\xe3\x82\xa7", NEXUS_CLASS_NINJA, 52, 58, 30, 38, 55, 35, 42},
    {"Stamm", "\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\xa0", NEXUS_CLASS_FIGHTER, 75, 60, 10, 60, 35, 20, 55},
    {"Tiggy", "Tiggy", NEXUS_CLASS_WIZARD, 32, 35, 70, 24, 42, 66, 32},
    {"Chani", "Chani", NEXUS_CLASS_PRIEST, 42, 42, 60, 30, 38, 62, 42},
    {"Hissssa", "Hissssa", NEXUS_CLASS_NINJA, 60, 62, 20, 48, 64, 26, 54},
    {"Daroou", "Daroou", NEXUS_CLASS_FIGHTER, 78, 58, 12, 66, 32, 22, 58},
    {"Gothmog", "Gothmog", NEXUS_CLASS_WIZARD, 45, 40, 68, 34, 34, 68, 38},
    {"Mophus", "Mophus", NEXUS_CLASS_PRIEST, 48, 38, 64, 32, 30, 70, 44},
    {"Alex", "Alex", NEXUS_CLASS_FIGHTER, 62, 55, 24, 52, 48, 36, 50},
    {"Nabi II", "Nabi II", NEXUS_CLASS_NINJA, 56, 61, 28, 42, 62, 34, 46},
    {"Linflas", "Linflas", NEXUS_CLASS_NINJA, 50, 60, 34, 38, 66, 40, 44},
    {"Iaido", "Iaido", NEXUS_CLASS_FIGHTER, 68, 54, 18, 58, 50, 28, 52},
    {"Boris", "Boris", NEXUS_CLASS_WIZARD, 44, 38, 72, 28, 36, 72, 36},
    {"Sonja", "Sonja", NEXUS_CLASS_FIGHTER, 72, 57, 16, 62, 44, 24, 56},
    {"Wuuf", "Wuuf", NEXUS_CLASS_PRIEST, 46, 46, 58, 34, 42, 58, 48},
    {"Leif", "Leif", NEXUS_CLASS_FIGHTER, 64, 52, 26, 54, 46, 38, 50},
    {"Azizi", "Azizi", NEXUS_CLASS_NINJA, 58, 59, 30, 44, 60, 36, 48},
    {"Hawk", "Hawk", NEXUS_CLASS_FIGHTER, 76, 63, 14, 64, 40, 24, 60},
    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0}
};

void nexus_v1_champions_init(Nexus_V1_ChampionPool *pool) {
    int i;
    int j;

    if (!pool) return;
    memset(pool, 0, sizeof(*pool));
    for (i = 0; g_nexus_roster[i].ascii; ++i) {
        Nexus_V1_Champion *c = &pool->champions[i];
        strncpy(c->name_ascii, g_nexus_roster[i].ascii, 31U);
        strncpy(c->name_jp, g_nexus_roster[i].jp, 63U);
        c->primary_class = g_nexus_roster[i].cls;
        c->health = c->max_health = g_nexus_roster[i].hp;
        c->stamina = c->max_stamina = g_nexus_roster[i].sta;
        c->mana = c->max_mana = g_nexus_roster[i].mp;
        c->strength = g_nexus_roster[i].str;
        c->dexterity = g_nexus_roster[i].dex;
        c->wisdom = g_nexus_roster[i].wis;
        c->vitality = g_nexus_roster[i].vit;
        c->food = 1500;
        c->water = 1500;
        c->alive = 1;
        c->portrait_index = i;
        for (j = 0; j < 30; ++j) c->inventory[j] = -1;
        for (j = 0; j < NEXUS_SLOT_COUNT; ++j) c->slots[j] = -1;
        c->max_load = nexus_champion_get_maximum_load(c);
        ++pool->champion_count;
    }
    for (i = 0; i < NEXUS_MAX_PARTY; ++i) pool->party[i] = -1;
    pool->leader_index = 0;
}
