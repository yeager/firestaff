/*
 * theron_v1_champions.c — Theron's Quest V1 Phase 3: Champion State
 *
 * Champion structs for Theron's Quest: Theron + 3 companions.
 * Companion persistence: THQUEST.ASM T800.
 *
 * Source references:
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T800  — champion persistence + inventory reset per dungeon
 *   THQUEST.ASM T560  — dungeon loading (header + dungeon_seed)
 */

#include "theron_v1_champions.h"
#include <string.h>

size_t theron_v1_champion_block_size(void) {
    return sizeof(Theron_V1_Champion);
}

int theron_v1_champion_is_alive(const Theron_V1_Champion *c) {
    return c && c->alive;
}

int theron_v1_champion_skill_level(const Theron_V1_Champion *c) {
    if (!c) return 0;
    switch (c->primary_class) {
        case THERON_CLASS_FIGHTER: return c->fighter_level;
        case THERON_CLASS_NINJA:   return c->ninja_level;
        case THERON_CLASS_PRIEST:  return c->priest_level;
        case THERON_CLASS_WIZARD:  return c->wizard_level;
        default: return 0;
    }
}

void theron_v1_champion_reset_inventory(Theron_V1_Champion *c) {
    if (!c) return;
    memset(c->inventory, 0, sizeof(c->inventory));
    for (int i = 0; i < THERON_EQUIP_SLOT_COUNT; i++) c->slots[i] = -1;
    c->load = 0;
}

const char *theron_v1_champions_source_evidence(void) {
    return "THQUEST.ASM T520/T560/T800  "
           "+ tqr_v1_phase0_provenance_gate_H2339.md";
}
