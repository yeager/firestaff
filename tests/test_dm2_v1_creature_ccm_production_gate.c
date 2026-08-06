/* DM2 production CCM gate: never infer a command stream from a decodable
 * CREATURE_AI field.  Source: SKProject SKULLWIN/c_creature.cpp
 * DM2_PROCEED_CCM reads the live record-owned b_1a/operands. */
#include "dm2_v1_creature.h"
#include "dm2_v1_ccm.h"
#include "dm2_v1_asset_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_bytes(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

int main(void)
{
    static const uint8_t candidate_bytes[] = { DM2_CCM_OP_WALK_NOW };
    uint32_t offsets[1] = { 0u };
    uint32_t sizes[1] = { (uint32_t)sizeof(candidate_bytes) };
    DM2_V1_GdatEntry entry;
    DM2_V1_AssetLoader loader;
    int selected_field = 99;
    const char *real_path = getenv("FIRESTAFF_DM2_GRAPHICS_DAT");

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

    /* A decodable byte sequence in the real GRAPHICS.DAT is not the
     * source-owned b_1a/b_17 CCM record consumed by DM2_PROCEED_CCM.
     * Source: SKProject SKULLWIN/c_creature.cpp, DM2_PROCEED_CCM and
     * DM2_THINK_CREATURE. */
    if (real_path && real_path[0]) {
        uint8_t *real_bytes = NULL;
        size_t real_size = 0u;
        DM2_V1_AssetLoader real_loader;
        int real_field = 99;

        if (!read_bytes(real_path, &real_bytes, &real_size) ||
            dm2_v1_asset_loader_init(&real_loader, real_bytes, real_size) != 0 ||
            dm2_v1_creature_load_ccm_programs_from_gdat_auto(
                &real_loader, &real_field) != 0 || real_field != -1 ||
            dm2_v1_creature_loaded_ccm_program_count() != 0 ||
            dm2_v1_creature_loaded_ccm_program_field() != -1) {
            fprintf(stderr, "DM2 real GRAPHICS.DAT CCM owner gate was reopened\n");
            free(real_bytes);
            return 1;
        }
        dm2_v1_asset_loader_free(&real_loader);
        free(real_bytes);
    }

    printf("DM2 production CCM field-probe gate passed\n");
    return 0;
}
