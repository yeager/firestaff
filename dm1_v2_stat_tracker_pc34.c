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
