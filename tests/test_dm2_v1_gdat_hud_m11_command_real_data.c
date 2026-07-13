/* Canonical PC G1 GRAPHICS.DAT proof for the complete M11 HUD command plan. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_hud_m11_command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
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
    const char *home = getenv("HOME");
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[1024];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatHudM11CommandPlan plan;
    int failures = 0;
    int expected_kind[DM2_V1_GDAT_HUD_M11_COMMAND_MAX] = {
        DM2_V1_GDAT_HUD_M11_COMMAND_TOP_BAR,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_STRIP,
        DM2_V1_GDAT_HUD_M11_COMMAND_GOLD_BOX,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL
    };

    if (root && root[0]) {
        snprintf(path, sizeof(path), "%s/graphics.dat", root);
    } else if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm2/data/graphics.dat",
                 home);
    } else {
        puts("SKIP: no DM2 data root");
        return 0;
    }
    if (!read_file(path, &graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 GRAPHICS.DAT");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&plan, 0, sizeof(plan));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_gdat_hud_m11_command_plan_build(&loader, &plan)) {
        fputs("FAIL: canonical GDAT HUD command plan was not admitted\n", stderr);
        dm2_v1_gdat_hud_m11_command_plan_free(&plan);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    if (!plan.valid || plan.command_count != DM2_V1_GDAT_HUD_M11_COMMAND_MAX ||
        plan.command_hash == 0u) {
        ++failures;
    }
    for (int i = 0; i < plan.command_count; ++i) {
        const DM2_V1_GdatHudM11Command *command = &plan.commands[i];
        if (command->kind != expected_kind[i] ||
            command->gdat_category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
            command->gdat_index < 2 || command->gdat_index > 6 ||
            command->width <= 0 || command->height <= 0 || !command->pixels ||
            command->format == DM2_IMG_FMT_UNKNOWN || command->raw_hash == 0u ||
            command->raw_byte_count == 0u || command->palette_hash == 0u ||
            command->destination.w <= 0 || command->destination.h <= 0) {
            ++failures;
        }
        printf("command=%d type=%d source=%d/%d/%d %dx%d dst=%d,%d %dx%d\n",
               i, command->kind, command->gdat_category, command->gdat_index,
               command->gdat_field, command->width, command->height,
               command->destination.x, command->destination.y,
               command->destination.w, command->destination.h);
    }
    dm2_v1_gdat_hud_m11_command_plan_free(&plan);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (failures != 0) {
        fprintf(stderr, "FAIL: %d invalid GDAT HUD command(s)\n", failures);
        return 1;
    }
    puts("PASS: real GRAPHICS.DAT yields a complete fail-closed M11 HUD command family");
    return 0;
}
