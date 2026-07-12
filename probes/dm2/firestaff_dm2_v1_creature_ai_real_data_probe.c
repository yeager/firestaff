/* DM2-GDAT-FB-08 real-data CREATURE_AI binding probe.
 *
 * skproject/SKWINSPX/src/v4/skcrture.cpp:28-36 resolves
 * CREATURES[type] dtWordValue(0x05) to a CREATURE_AI AIDefinition row.
 * Exit 0 means PASS or SKIP; an available but invalid real file fails.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    unsigned char *data;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data) {
        fclose(file);
        return NULL;
    }
    if (fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return data;
}

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : getenv("FIRESTAFF_DM2_GRAPHICS_DAT");
    unsigned char *data;
    size_t size;
    DM2_V1_AssetLoader loader;
    int loaded;
    int type;
    int verified_type = -1;

    if (!path || !path[0]) {
        printf("SKIP: set FIRESTAFF_DM2_GRAPHICS_DAT or pass GRAPHICS.DAT\n");
        return 0;
    }
    data = read_file(path, &size);
    if (!data) {
        printf("SKIP: no real DM2 GRAPHICS.DAT at %s\n", path);
        return 0;
    }
    if (dm2_v1_asset_loader_init(&loader, data, size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader) ||
        !dm2_v1_asset_loader_validate_typed_graph(&loader)) {
        printf("FAIL: real GRAPHICS.DAT does not initialize as verified GDAT\n");
        free(data);
        return 1;
    }

    loaded = dm2_v1_creature_load_ai_table_from_gdat(&loader);
    if (loaded <= 0) {
        printf("FAIL: real GDAT exposes no CREATURES[type] -> CREATURE_AI bindings\n");
        dm2_v1_asset_loader_free(&loader);
        free(data);
        return 1;
    }
    for (type = 0; type < DM2_AI_TABLE_SIZE; ++type) {
        const DM2_AIDefinition *spec = dm2_v1_creature_ai_spec(type);
        if (spec && spec->BaseHP > 0) {
            verified_type = type;
            break;
        }
    }
    if (verified_type < 0 ||
        dm2_v1_creature_spawn(verified_type, 0, 0, 0, 0, 8) < 0) {
        printf("FAIL: no mapped real AI row provides spawnable verified HP\n");
        dm2_v1_asset_loader_free(&loader);
        free(data);
        return 1;
    }
    printf("PASS: %d mapped CREATURE_AI rows; type %d uses real BaseHP and attack flags\n",
           loaded, verified_type);
    dm2_v1_asset_loader_free(&loader);
    free(data);
    return 0;
}
