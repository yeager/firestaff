#include "dm1_v2_stat_tracker_pc34.h"

static M11_V2_GameStats g_stats;

void v2_stats_init(void) {
    memset(&g_stats, 0, sizeof(g_stats));
}

void v2_stats_increment(M11_V2_StatType stat, uint64_t amount) {
    switch (stat) {
        case M11_V2_STAT_TOTAL_STEPS: g_stats.total_steps += (uint32_t)amount; break;
        case M11_V2_STAT_TOTAL_KILLS: g_stats.total_kills += (uint32_t)amount; break;
        case M11_V2_STAT_ITEMS_FOUND: g_stats.items_found += (uint32_t)amount; break;
        case M11_V2_STAT_ITEMS_USED: g_stats.items_used += (uint32_t)amount; break;
        case M11_V2_STAT_SPELLS_CAST: g_stats.spells_cast += (uint32_t)amount; break;
        case M11_V2_STAT_DOORS_OPENED: g_stats.doors_opened += (uint32_t)amount; break;
        case M11_V2_STAT_TRAPS_TRIGGERED: g_stats.traps_triggered += (uint32_t)amount; break;
        case M11_V2_STAT_DAMAGE_DEALT: g_stats.damage_dealt += amount; break;
        case M11_V2_STAT_DAMAGE_TAKEN: g_stats.damage_taken += amount; break;
        case M11_V2_STAT_PLAY_TIME_SECONDS: g_stats.play_time_seconds += (uint32_t)amount; break;
        case M11_V2_STAT_DEATHS: g_stats.deaths += (uint32_t)amount; break;
        case M11_V2_STAT_CHAMPIONS_RESURRECTED: g_stats.champions_resurrected += (uint32_t)amount; break;
        default: break;
    }
}

uint64_t v2_stats_get(M11_V2_StatType stat) {
    switch (stat) {
        case M11_V2_STAT_TOTAL_STEPS: return g_stats.total_steps;
        case M11_V2_STAT_TOTAL_KILLS: return g_stats.total_kills;
        case M11_V2_STAT_ITEMS_FOUND: return g_stats.items_found;
        case M11_V2_STAT_ITEMS_USED: return g_stats.items_used;
        case M11_V2_STAT_SPELLS_CAST: return g_stats.spells_cast;
        case M11_V2_STAT_DOORS_OPENED: return g_stats.doors_opened;
        case M11_V2_STAT_TRAPS_TRIGGERED: return g_stats.traps_triggered;
        case M11_V2_STAT_DAMAGE_DEALT: return g_stats.damage_dealt;
        case M11_V2_STAT_DAMAGE_TAKEN: return g_stats.damage_taken;
        case M11_V2_STAT_PLAY_TIME_SECONDS: return g_stats.play_time_seconds


void v2_stats_reset(void) {
    memset(&g_stats, 0, sizeof(g_stats));
}

int v2_stats_serialize(unsigned char *buf, int bufsize) {
    if (!buf || bufsize < (int)sizeof(g_stats)) return -1;
    memcpy(buf, &g_stats, sizeof(g_stats));
    return (int)sizeof(g_stats);
}

int v2_stats_deserialize(const unsigned char *buf, int bufsize) {
    if (!buf || bufsize < (int)sizeof(g_stats)) return -1;
    memcpy(&g_stats, buf, sizeof(g_stats));
    return 0;
}

const M11_V2_GameStats *v2_stats_get_all(void) {
    return &g_stats;
}

/* V2.2 Stat Tracker — JSON serialization for display/export */

int v22_stats_to_json(char *buf, int bufsize) {
    const M11_V2_GameStats *s = v2_stats_get_all();
    if (!s || !buf || bufsize < 256) return -1;
    return snprintf(buf, bufsize,
        "{\n"
        "  \"total_steps\": %u,\n"
        "  \"total_kills\": %u,\n"
        "  \"items_found\": %u,\n"
        "  \"items_used\": %u,\n"
        "  \"spells_cast\": %u,\n"
        "  \"doors_opened\": %u,\n"
        "  \"traps_triggered\": %u,\n"
        "  \"damage_dealt\": %llu,\n"
        "  \"damage_taken\": %llu,\n"
        "  \"play_time_seconds\": %u,\n"
        "  \"deaths\": %u,\n"
        "  \"champions_resurrected\": %u\n"
        "}",
        s->total_steps, s->total_kills, s->items_found,
        s->items_used, s->spells_cast, s->doors_opened,
        s->traps_triggered, (unsigned long long)s->damage_dealt,
        (unsigned long long)s->damage_taken,
        s->play_time_seconds, s->deaths, s->champions_resurrected);
}

void v22_stats_on_step(void) { v2_stats_increment(M11_V2_STAT_TOTAL_STEPS, 1); }
void v22_stats_on_kill(void) { v2_stats_increment(M11_V2_STAT_TOTAL_KILLS, 1); }
void v22_stats_on_spell(void) { v2_stats_increment(M11_V2_STAT_SPELLS_CAST, 1); }
void v22_stats_on_door(void) { v2_stats_increment(M11_V2_STAT_DOORS_OPENED, 1); }
void v22_stats_on_damage_dealt(int amount) { v2_stats_increment(M11_V2_STAT_DAMAGE_DEALT, amount); }
void v22_stats_on_damage_taken(int amount) { v2_stats_increment(M11_V2_STAT_DAMAGE_TAKEN, amount); }
void v22_stats_on_death(void) { v2_stats_increment(M11_V2_STAT_DEATHS, 1); }
void v22_stats_on_resurrect(void) { v2_stats_increment(M11_V2_STAT_CHAMPIONS_RESURRECTED, 1); }
void v22_stats_tick_playtime(void) { v2_stats_increment(M11_V2_STAT_PLAY_TIME_SECONDS, 1); }

