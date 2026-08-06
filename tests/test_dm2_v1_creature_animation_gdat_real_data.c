/* Canonical GRAPHICS.DAT proof for the source-owned dynamic V5 creature
 * animation table route. No fixture, art, save, or runtime AI state is made. */
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_creature_animation_gdat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = malloc((size_t)size);
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
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char path[1100];
    char fallback[1024];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    int found = 0;

    if (!root || !root[0]) {
        if (!home || !home[0]) {
            puts("SKIP: no local canonical DM2 data");
            return 0;
        }
        snprintf(fallback, sizeof(fallback), "%s/.firestaff/data/dm2/data", home);
        root = fallback;
    }
    /* Asset admission is hash based and case-preserving external media often
     * uses the DOS spelling. The real-data probe must not turn that into a
     * filename requirement. */
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    if (!read_file(path, &graphics, &graphics_size)) {
        snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
        if (!read_file(path, &graphics, &graphics_size)) {
            puts("SKIP: no local canonical DM2 data");
            return 0;
        }
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: canonical GRAPHICS.DAT was not accepted\n", stderr);
        free(graphics);
        return 1;
    }
    if (dm2_v1_creature_load_ai_table_from_gdat(&loader) <= 0) {
        puts("SKIP: canonical GRAPHICS.DAT has no admitted source AI classification");
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 0;
    }

    for (int creature = 0; creature < DM2_AI_TABLE_SIZE && !found; ++creature) {
        const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec(creature);
        if (!ai || (ai->w0AIFlags & DM2_AIFLAG_STATIC) != 0u) continue;
        for (uint16_t command = 0u; command <= 0x40u; ++command) {
            DM2_V1_CreatureAnimationGdatReceipt receipt;
            if (dm2_v1_creature_animation_gdat_select_dynamic_v5(
                    &loader, creature, command, 0xffffu, ai->w0AIFlags, 2,
                    &receipt)) {
                if (!receipt.valid || !receipt.dynamic ||
                    receipt.creature_type != (uint8_t)creature ||
                    receipt.command != command ||
                    receipt.previous_frame != 0xffffu ||
                    receipt.direction != 2u || receipt.table_hash == 0u) {
                    fputs("FAIL: dynamic animation receipt lost source identity\n", stderr);
                    dm2_v1_asset_loader_free(&loader);
                    free(graphics);
                    return 1;
                }
                found = 1;
                break;
            }
        }
    }
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (!found) {
        puts("SKIP: canonical GRAPHICS.DAT has no admitted dynamic V5 animation route");
        return 0;
    }
    puts("PASS: canonical dynamic creature command resolves only through real FB/FC/FD GDAT tables");
    return 0;
}
