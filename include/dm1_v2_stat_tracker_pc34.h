#ifndef FIRESTAFF_DM1_V2_STAT_TRACKER_PC34_H
#define FIRESTAFF_DM1_V2_STAT_TRACKER_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    M11_V2_STAT_TOTAL_STEPS,
    M11_V2_STAT_TOTAL_KILLS,
    M11_V2_STAT_ITEMS_FOUND,
    M11_V2_STAT_ITEMS_USED,
    M11_V2_STAT_SPELLS_CAST,
    M11_V2_STAT_DOORS_OPENED,
    M11_V2_STAT_TRAPS_TRIGGERED,
    M11_V2_STAT_DAMAGE_DEALT,
    M11_V2_STAT_DAMAGE_TAKEN,
    M11_V2_STAT_PLAY_TIME_SECONDS,
    M11_V2_STAT_DEATHS,
    M11_V2_STAT_CHAMPIONS_RESURRECTED,
    M11_V2_STAT_COUNT
} M11_V2_StatType;

typedef struct {
    uint32_t total_steps;
    uint32_t total_kills;
    uint32_t items_found;
    uint32_t items_used;
    uint32_t spells_cast;
    uint32_t doors_opened;
    uint32_t traps_triggered;
    uint64_t damage_dealt;
    uint64_t damage_taken;
    uint32_t play_time_seconds;
    uint32_t deaths;
    uint32_t champions_resurrected;
} M11_V2_GameStats;

void v2_stats_init(void);
void v2_stats_increment(M11_V2_StatType stat, uint64_t amount);
uint64_t v2_stats_get(M11_V2_StatType stat);
bool v2_stats_save(const char* path);
bool v2_stats_load(const char* path);
void v2_stats_reset(void);

#ifdef __cplusplus
}
#endif

#endif
