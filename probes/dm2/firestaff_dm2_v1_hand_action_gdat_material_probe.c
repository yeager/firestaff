/* skproject/SKWINSPX/src/v4/skguidrw.cpp DRAW_HAND_ACTION_ICONS
 * (0x29EE:026C-036C): real GDAT backdrop material receipt. */
#include "dm2_v1_hand_action_gdat.h"

#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = malloc((size_t)size);
    if (!data || fread(data, 1u, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

int main(int argc, char **argv)
{
    static const struct {
        DM2_V1_HandActionInput input;
        uint8_t entry;
        uint8_t rectno;
    } cases[] = {
        { { 0, 0, 0, 0 }, 2u, 0x4au },
        { { 0, 1, 3, 1 }, 3u, 0x4cu },
        { { 1, 0, 1, 3 }, 4u, 0x48u },
        { { 1, 1, 2, 0 }, 5u, 0x48u }
    };
    DM2_V1_AssetLoader loader;
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;

    if (argc != 2 || !read_file(argv[1], &graphics, &graphics_size) ||
        dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fprintf(stderr, "FAIL: original DM2 GRAPHICS.DAT is required\n");
        free(graphics);
        return 1;
    }
    for (size_t i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM2_V1_HandActionGdatRoute route;
        DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
        int width = 0;
        int height = 0;
        uint8_t *pixels = NULL;

        if (!dm2_v1_hand_action_gdat_route(&cases[i].input, &route) ||
            route.category !=
                DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
            route.subcategory != 4u || route.entry != cases[i].entry ||
            route.rectno != cases[i].rectno ||
            !(pixels = dm2_v1_hand_action_gdat_load_image(
                  &loader, &cases[i].input, &route, &width, &height,
                  &format)) || width <= 0 || height <= 0 ||
            format == DM2_IMG_FMT_UNKNOWN) {
            fprintf(stderr, "FAIL: source hand-action material %zu\n", i);
            free(pixels);
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            return 1;
        }
        free(pixels);
    }
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    fprintf(stderr, "PASS: original DM2 hand-action GDAT materials\n");
    return 0;
}
