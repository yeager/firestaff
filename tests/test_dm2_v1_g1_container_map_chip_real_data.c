/* Canonical PC G1 DB9 Container -> CONTAINERS/F9 material receipt proof. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_weather_gdat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_raw_calls;

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    long size;
    uint8_t *bytes;

    *out = NULL;
    *out_size = 0u;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
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

static const char *resolve_dm2_data_root(int argc, char **argv,
                                         char *buf, size_t buf_size)
{
    const char *root;
    const char *home;
    if (argc >= 2) return argv[1];
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) return root;
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(buf, buf_size, "%s/.firestaff/data/dm2/data", home);
        return buf;
    }
    return NULL;
}

static int raw_read(void *user, int entry_type, int category, int index,
                    int field, const uint8_t **out_data, uint32_t *out_size)
{
    DM2_V1_AssetLoader *loader = user;
    size_t size = 0u;
    const uint8_t *data;
    ++g_raw_calls;
    if (entry_type != DM2_GDAT_ENTRY_TYPE_IMAGE || category != 0x14 ||
        index != 0 || field != 0xf9) return 0;
    data = dm2_v1_asset_load_sized(loader, category, index, field, &size);
    if (!data || size == 0u || size > UINT32_MAX) return 0;
    *out_data = data;
    *out_size = (uint32_t)size;
    return 1;
}

static int metadata_read(void *user, int category, int index, int field,
                         int *width, int *height, int *format)
{
    DM2_V1_GdatImageMetadata metadata;
    if (category != 0x14 || index != 0 || field != 0xf9 ||
        !dm2_v1_asset_load_image_metadata(user, category, index, field,
                                          &metadata)) return 0;
    *width = metadata.width;
    *height = metadata.height;
    *format = metadata.bits_per_pixel;
    return 1;
}

static int palette_read(void *user, int category, int index, int field,
                        uint8_t palette[16], uint32_t *hash)
{
    return category == 0x14 && index == 0 && field == 0xf9 &&
        dm2_v1_asset_load_image_local_palette(user, category, index, field,
                                              palette, hash);
}

int main(int argc, char **argv)
{
    char root_buf[1024];
    char graphics_path[2048];
    char dungeon_path[2048];
    const char *root = resolve_dm2_data_root(argc, argv, root_buf, sizeof(root_buf));
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1ContainerMapChipRuntimeReceipt receipt;
    int ok;

    if (!root) {
        puts("SKIP: no local canonical DM2 data");
        free(graphics);
        free(dungeon_bytes);
        return 0;
    }
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    if (!read_file(graphics_path, &graphics, &graphics_size) ||
        !read_file(dungeon_path, &dungeon_bytes, &dungeon_size)) {
        puts("SKIP: no local canonical DM2 data");
        free(graphics);
        free(dungeon_bytes);
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    ok = dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0 &&
         dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) == 0 &&
         !dm2_v1_dungeon_materialize_g1_container_map_chip_runtime(
             &dungeon, 9, raw_read, metadata_read, palette_read, &loader,
             &receipt) && !receipt.valid && receipt.material_count == 0 &&
         g_raw_calls == 1;
    dm2_v1_dungeon_free(&dungeon);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);
    if (!ok) {
        fputs("FAIL: canonical DB9 Container missing-material gate changed\n",
              stderr);
        return 1;
    }
    puts("PASS: canonical DB9 Container blocks without exact CONTAINERS/0/F9");
    return 0;
}
