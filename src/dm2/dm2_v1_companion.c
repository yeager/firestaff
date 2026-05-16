
#include "dm2_v1_companion.h"
#include <string.h>

void dm2_v1_companion_init(DM2_V1_CompanionState *state) {
    if (state) memset(state, 0, sizeof(*state));
}

int dm2_v1_companion_add(DM2_V1_CompanionState *state, const char *name,
    int health, int attack, int defense)
{
    DM2_V1_Companion *c;
    if (!state || state->companion_count >= DM2_V1_MAX_COMPANIONS) return -1;
    c = &state->companions[state->companion_count];
    strncpy(c->name, name ? name : "NPC", 15);
    c->health = c->max_health = health;
    c->attack = attack; c->defense = defense;
    c->loyalty = 50; c->ai_behavior = 0; c->alive = 1;
    return state->companion_count++;
}

void dm2_v1_companion_tick(DM2_V1_CompanionState *state) {
    /* AI tick: companions follow party or fight nearby enemies */
    (void)state;
}

const char *dm2_v1_companion_source_evidence(void) {
    return "SKULL.ASM: NPC companion AI, loyalty, trading\n"
           "DM2 feature: party companions with own inventory\n";
}

