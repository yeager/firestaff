
#include "nexus_v1_encumbrance.h"

/* DM.BIN 0x02A7FA: movement tick computation.
 * Reads item weight at champion+0x110, multiplies by 40.
 * Compares against max_load/16. Random mask & 0x0F for variability.
 * Overload path subtracts 12 (MOV #-12 at 0x02A85C).
 * Closed-form not fully reconstructed; load-ratio approximation retained. */
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

/* DM.BIN 0x02A93A: tiered stamina cost.
 * Light (load*8 <= max*5, i.e. <62.5%): 2
 * Medium (load*8 > max*5): 3
 * Overloaded: (excess*4)/max + 4 */
int nexus_v1_encumbrance_stamina_cost(const Nexus_V1_Champion *champion) {
    if (!champion) return NEXUS_STAMINA_COST_LIGHT;
    if (champion->max_load <= 0) return NEXUS_STAMINA_COST_LIGHT;
    if (champion->load > champion->max_load) {
        int excess = champion->load - champion->max_load;
        return (excess * 4) / champion->max_load + 4;
    }
    if (champion->load * 8 > champion->max_load * 5)
        return NEXUS_STAMINA_COST_MEDIUM;
    return NEXUS_STAMINA_COST_LIGHT;
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
