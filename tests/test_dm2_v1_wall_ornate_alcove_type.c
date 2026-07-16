/*
 * test_dm2_v1_wall_ornate_alcove_type.c
 *
 * Focused real-GDAT coverage for the skproject
 * GET_WALL_ORNATE_ALCOVE_TYPE -> DRAW_WALL_ORNATE wall material chain.
 */

#include "dm2_v1_wall_ornament.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_graphics(uint8_t **out_data, size_t *out_size,
                         char *path, size_t path_size)
{
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    size_t i;

    for (i = 0u; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_AssetLoader loader;
    unsigned int alcove_count = 0u;
    unsigned int material_count = 0u;
    uint32_t alcove_hash = 2166136261u;
    uint32_t material_hash = 2166136261u;

    memset(&loader, 0, sizeof(loader));
    printf("=== DM2 V1 GET_WALL_ORNATE_ALCOVE_TYPE Test ===\n");
    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return 0;
    }

    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real DM2 GRAPHICS.DAT initializes wall ornament loader");
    if (!loader.loaded) {
        free(graphics);
        printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
        return failed == 0 ? 0 : 1;
    }

    for (unsigned int index = 0u; index <= 255u; ++index) {
        DM2_V1_WallOrnateAlcoveTypeReceipt alcove;
        int has_alcove;

        has_alcove = dm2_v1_GET_WALL_ORNATE_ALCOVE_TYPE(
            &loader, (uint8_t)index, &alcove);
        if (has_alcove) {
            ++alcove_count;
            CHECK(alcove.valid && alcove.wall_gfx_index == index &&
                      alcove.alcove_type <=
                          DM2_V1_WALL_ORNATE_ALCOVE_TYPE_LIMIT &&
                      alcove.source_hash != 0u,
                  "real WALL_GFX 0x0A alcove type is source bounded");
            alcove_hash ^= alcove.source_hash;
            alcove_hash *= 16777619u;
        }

        if (has_alcove) {
            for (unsigned int field = 0u; field <= 255u; ++field) {
                DM2_V1_WallOrnamentReceipt material;

                if (dm2_v1_wall_ornament_material_receipt(
                        &loader, (uint8_t)index, (uint8_t)field,
                        &material)) {
                    ++material_count;
                    CHECK(material.valid && material.wall_gfx_index == index &&
                              material.image_field == field &&
                              material.alcove_type == alcove.alcove_type &&
                              material.image_metadata.bits_per_pixel == 4u &&
                              material.local_palette_hash != 0u &&
                              material.decoded_pixel_count > 0u &&
                              material.decoded_pixels_hash != 0u &&
                              material.material_hash != 0u,
                          "real DRAW_WALL_ORNATE selected-field material "
                          "consumes image/local palette");
                    material_hash ^= material.material_hash;
                    material_hash *= 16777619u;
                }
            }
        }
    }

    CHECK(alcove_count > 0u && alcove_hash != 0u,
          "real GRAPHICS.DAT exposes WALL_GFX alcove-type records");
    CHECK(material_count > 0u && material_hash != 0u,
          "real GRAPHICS.DAT exposes at least one complete wall ornament material");
    CHECK(dm2_v1_GET_WALL_ORNATE_ALCOVE_TYPE(&loader, 0u, NULL) == 0,
          "GET_WALL_ORNATE_ALCOVE_TYPE rejects missing output");
    CHECK(dm2_v1_wall_ornament_source_evidence()[0] != '\0',
          "wall ornament receipt reports skproject source evidence");

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
