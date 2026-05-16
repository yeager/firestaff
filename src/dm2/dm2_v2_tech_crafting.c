
#include "dm2_v2_tech_crafting.h"

static DM2_V2_CraftRecipe g_recipes[DM2_V2_MAX_RECIPES];
static int g_recipe_count = 0;

int dm2_v2_craft_can_combine(int base_type, int component_type,
    int champion_tech, int champion_magic)
{
    for (int i = 0; i < g_recipe_count; i++) {
        if (g_recipes[i].base_item_type == base_type &&
            g_recipes[i].component_type == component_type &&
            champion_tech >= g_recipes[i].tech_req &&
            champion_magic >= g_recipes[i].magic_req) {
            return 1;
        }
    }
    return 0;
}

int dm2_v2_craft_get_result(int base_type, int component_type) {
    for (int i = 0; i < g_recipe_count; i++) {
        if (g_recipes[i].base_item_type == base_type &&
            g_recipes[i].component_type == component_type) {
            return g_recipes[i].result_type;
        }
    }
    return -1;
}

const char *dm2_v2_craft_source_evidence(void) {
    return "DM2 V2.2: tech/magic crafting system\n"
           "Combine base items + components with tech+magic requirements\n";
}

