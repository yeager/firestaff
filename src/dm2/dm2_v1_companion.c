
#include "dm2_v1_companion.h"
#include <string.h>

void dm2_v1_companion_init(DM2_V1_CompanionState *state) {
    if (state) memset(state, 0, sizeof(*state));
}

int dm2_v1_companion_add(DM2_V1_CompanionState *state, const char *name,
    int health, int attack, int defense)
{
    /* A caller-provided name/stats tuple is not a DM2 companion record.
     * SKProject's creature route owns these values through the live DB4,
     * CAII/CCM and dialogue/inventory chains; none is available at this
     * compatibility boundary.  Keep the state byte-identical on rejection.
     */
    (void)state;
    (void)name;
    (void)health;
    (void)attack;
    (void)defense;
    return -1;
}

void dm2_v1_companion_tick(DM2_V1_CompanionState *state) {
    /* AI tick: companions follow party or fight nearby enemies */
    (void)state;
}

const char *dm2_v1_companion_source_evidence(void) {
    return "DM2 companion source-ownership gate\n"
           "Requires live DB creature, CAII/CCM, inventory and dialogue records\n"
           "Caller-authored companion name/stat defaults are unavailable.\n";
}
