
#ifndef FIRESTAFF_DM2_V2_TECH_CRAFTING_H
#define FIRESTAFF_DM2_V2_TECH_CRAFTING_H
#include <stdint.h>

/* DM2 V2.2 Tech Crafting — combine tech + magic items.
 * V1: items are fixed.
 * V2.2: combine base items + components to create hybrid items.
 * Recipe: base_item + component → result (if tech+magic reqs met). */

#define DM2_V2_MAX_RECIPES 64

typedef struct {
    int base_item_type;
    int component_type;
    int result_type;
    int tech_req;
    int magic_req;
} DM2_V2_CraftRecipe;

int dm2_v2_craft_can_combine(int base_type, int component_type,
    int champion_tech, int champion_magic);
int dm2_v2_craft_get_result(int base_type, int component_type);
const char *dm2_v2_craft_source_evidence(void);
#endif

