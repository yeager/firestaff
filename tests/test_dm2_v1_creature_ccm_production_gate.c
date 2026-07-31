/* DM2 production CCM gate: never infer a command stream from a decodable
 * CREATURE_AI field.  Source: SKProject SKULLWIN/c_creature.cpp
 * DM2_PROCEED_CCM reads the live record-owned b_1a/operands. */
#include "dm2_v1_creature.h"
#include "dm2_v1_ccm.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    static const uint8_t candidate_bytes[] = { DM2_CCM_OP_WALK_NOW };
    uint32_t offsets[1] = { 0u };
    uint32_t sizes[1] = { (uint32_t)sizeof(candidate_bytes) };
    DM2_V1_GdatEntry entry;
    DM2_V1_AssetLoader loader;
    int selected_field = 99;

    memset(&entry, 0, sizeof(entry));
    entry.cls1 = DM2_GDAT_CATEGORY_CREATURE_AI;
    entry.cls2 = DM2_AI_CAVE_BAT;
    entry.cls4 = 1;
    entry.data_index = 0;
    memset(&loader, 0, sizeof(loader));
    loader.data = candidate_bytes;
    loader.data_size = sizeof(candidate_bytes);
    loader.loaded = 1;
    loader.raw_data_count = 1;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.entries = &entry;
    loader.entry_count = 1;

    dm2_v1_creature_reset_ccm_programs();
    if (dm2_v1_creature_load_ccm_programs_from_gdat_auto(
            &loader, &selected_field) != 0 ||
        selected_field != -1 ||
        dm2_v1_creature_loaded_ccm_program_count() != 0 ||
        dm2_v1_creature_loaded_ccm_program_field() != -1) {
        fprintf(stderr, "DM2 production CCM field probe was not closed\n");
        return 1;
    }
    printf("DM2 production CCM field-probe gate passed\n");
    return 0;
}
