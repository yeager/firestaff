/* skproject/SKWINSPX/src/v4/skguidrw.cpp DRAW_HAND_ACTION_ICONS
 * (0x29EE:026C): INTERFACE_GENERAL/4 entry and QUERY_EXPANDED_RECT route. */
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
    DM2_V1_HandActionGdatRoute route;

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (!dm2_v1_hand_action_gdat_route(&cases[i].input, &route) ||
            route.category != 1u || route.subcategory != 4u ||
            route.entry != cases[i].entry || route.rectno != cases[i].rectno) {
            fprintf(stderr, "FAIL: route %zu\n", i);
            return 1;
        }
    }
    if (dm2_v1_hand_action_gdat_route(
            &(DM2_V1_HandActionInput){ 2, 0, 0, 0 }, &route) ||
        route.category != 0u || route.subcategory != 0u ||
        dm2_v1_hand_action_gdat_route(
            &(DM2_V1_HandActionInput){ 0, -1, 0, 0 }, &route) ||
        dm2_v1_hand_action_gdat_route(
            &(DM2_V1_HandActionInput){ 0, 0, 4, 0 }, &route) ||
        dm2_v1_hand_action_gdat_route(
            &(DM2_V1_HandActionInput){ 0, 0, 0, 4 }, &route)) {
        fprintf(stderr, "FAIL: invalid hand-action input accepted\n");
        return 1;
    }
    if (argc == 2) {
        DM2_V1_AssetLoader loader;
        uint8_t *graphics = NULL;
        size_t graphics_size = 0u;

        if (!read_file(argv[1], &graphics, &graphics_size) ||
            dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
            fprintf(stderr, "FAIL: real GRAPHICS.DAT did not initialize\n");
            free(graphics);
            return 1;
        }
        for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
            int width = 0;
            int height = 0;
            DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
            uint8_t *pixels = dm2_v1_hand_action_gdat_load_image(
                &loader, &cases[i].input, &route, &width, &height, &format);

            if (!pixels || route.entry != cases[i].entry || width <= 0 ||
                height <= 0 || format == DM2_IMG_FMT_UNKNOWN) {
                fprintf(stderr, "FAIL: real hand-action GDAT image %zu\n", i);
                dm2_v1_asset_free_pixels(pixels);
                dm2_v1_asset_loader_free(&loader);
                free(graphics);
                return 1;
            }
            dm2_v1_asset_free_pixels(pixels);
        }
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        fprintf(stderr, "PASS: original hand-action GDAT images\n");
    }
    fprintf(stderr, "PASS: skproject hand-action GDAT route\n");
    return 0;
}
