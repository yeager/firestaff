#include "dm1_v2_stat_tracker_pc34.h"

static const M11_V2_GameStats k_no_stats;

void v2_stats_init(void) {
}

void v2_stats_increment(M11_V2_StatType stat, uint64_t amount) {
    (void)stat;
    (void)amount;
}

uint64_t v2_stats_get(M11_V2_StatType stat) {
    (void)stat;
    return 0;
}

bool v2_stats_save(const char* path) {
    (void)path;
    return false;
}

bool v2_stats_load(const char* path) {
    (void)path;
    return false;
}

void v2_stats_reset(void) {
}

int v2_stats_serialize(unsigned char *buf, int bufsize) {
    (void)buf;
    (void)bufsize;
    /* No host-created totals may be persisted as DM1 state. */
    return -1;
}

int v2_stats_deserialize(const unsigned char *buf, int bufsize) {
    (void)buf;
    (void)bufsize;
    return -1;
}

const M11_V2_GameStats *v2_stats_get_all(void) {
    return &k_no_stats;
}
