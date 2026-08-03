
#include "nexus_v1_encumbrance.h"

int nexus_v1_encumbrance_move_ticks(const Nexus_V1_Champion *champion) {
    int ratio, ticks;
    if (!champion) return NEXUS_BASE_MOVE_TICKS;
    if (champion->max_load <= 0) return NEXUS_BASE_MOVE_TICKS;

    ratio = (champion->load * 100) / champion->max_load;
    ticks = NEXUS_BASE_MOVE_TICKS + (ratio * (NEXUS_MAX_MOVE_TICKS - NEXUS_BASE_MOVE_TICKS)) / 150;

    if (ticks < NEXUS_BASE_MOVE_TICKS) ticks = NEXUS_BASE_MOVE_TICKS;
    if (ticks > NEXUS_MAX_MOVE_TICKS) ticks = NEXUS_MAX_MOVE_TICKS;
    return ticks;
}

int nexus_v1_encumbrance_stamina_cost(const Nexus_V1_Champion *champion) {
    if (!champion) return 1;
    if (champion->max_load <= 0) return 1;
    if (champion->load > champion->max_load)
        return NEXUS_OVERLOADED_STAMINA_DRAIN;
    if (champion->load * 100 / champion->max_load > 75)
        return 2;
    return 1;
}

int nexus_v1_encumbrance_overloaded(const Nexus_V1_Champion *champion) {
    if (!champion) return 0;
    return champion->load > champion->max_load;
}

int nexus_v1_encumbrance_ratio(const Nexus_V1_Champion *champion) {
    if (!champion || champion->max_load <= 0) return 0;
    return (champion->load * 100) / champion->max_load;
}

void nexus_v1_encumbrance_recalc_max_load(Nexus_V1_Champion *champion) {
    int base, wound_penalty;
    if (!champion) return;

    base = (champion->strength << 3) + 100;

    wound_penalty = 0;
    if (champion->wounds & 1) wound_penalty += base / 8;
    if (champion->wounds & 2) wound_penalty += base / 6;
    if (champion->wounds & 4) wound_penalty += base / 4;
    if (champion->wounds & 8) wound_penalty += base / 4;

    champion->max_load = base - wound_penalty;
    if (champion->max_load < 10) champion->max_load = 10;
}
