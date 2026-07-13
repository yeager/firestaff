/* Canonical PC G1 GRAPHICS.DAT proof for the complete M11 HUD command plan. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_save_load.h"

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
    char boot_root[1024];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_BootProfile boot;
    DM2_V1_GdatHudM11CommandPlan plan;
    DM2_V1_HudPartyState party;
    int failures = 0;
    uint8_t champion_mask[261];
    uint8_t encoded_champion[261];
    uint8_t decoded_champion[261];
    uint8_t source_champion[261];
    int encoded_champion_size;
    int expected_kind[DM2_V1_GDAT_HUD_M11_COMMAND_MAX] = {
        DM2_V1_GDAT_HUD_M11_COMMAND_TOP_BAR,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_STRIP,
        DM2_V1_GDAT_HUD_M11_COMMAND_GOLD_BOX,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL,
        DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT,
        DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT,
        DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT,
        DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT
    };

    if (root && root[0]) {
        snprintf(path, sizeof(path), "%s/graphics.dat", root);
        snprintf(boot_root, sizeof(boot_root), "%s/..", root);
    } else if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm2/data/graphics.dat",
                 home);
        snprintf(boot_root, sizeof(boot_root), "%s/.firestaff/data/dm2",
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
    dm2_v1_boot_profile_init(&boot);
    memset(&plan, 0, sizeof(plan));
    memset(&party, 0, sizeof(party));
    party.champion_count = DM2_V1_HUD_CHAMPION_SLOT_COUNT;
    for (int i = 0; i < party.champion_count; ++i) {
        party.champions[i].occupied = 1;
        party.champions[i].portrait_type_source_bound = 1;
        party.champions[i].portrait_index = (uint8_t)i;
    }
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_gdat_hud_m11_command_plan_build_for_party(&loader, &party,
                                                            &plan)) {
        fputs("FAIL: canonical GDAT HUD command plan was not admitted\n", stderr);
        dm2_v1_gdat_hud_m11_command_plan_free(&plan);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    if (dm2_v1_boot_scan_assets(&boot, boot_root) != 0 ||
        dm2_v1_boot_enter_game(&boot) != 0) {
        fputs("FAIL: canonical DM2 boot profile was not entered\n", stderr);
        dm2_v1_boot_cleanup(&boot);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    for (int i = 0; i < DM2_V1_HUD_CHAMPION_SLOT_COUNT; ++i) {
        char first_name[8];
        if (!dm2_v1_boot_champion_hero_type_source_ready(
                &boot, (uint8_t)i, first_name) || first_name[0] == '\0') {
            ++failures;
        }
    }
    memset(source_champion, 0, sizeof(source_champion));
    source_champion[255] = 3u;
    dm2_suppress_champion_mask(champion_mask);
    encoded_champion_size = dm2_suppress_encode(
        source_champion, champion_mask, sizeof(source_champion),
        encoded_champion, sizeof(encoded_champion));
    memset(decoded_champion, 0, sizeof(decoded_champion));
    if (encoded_champion_size <= 0 || champion_mask[255] != 0xffu ||
        dm2_suppress_decode(encoded_champion, (size_t)encoded_champion_size,
                            champion_mask, sizeof(decoded_champion),
                            decoded_champion, 0u) < 0 ||
        decoded_champion[255] != source_champion[255]) {
        ++failures;
    }
    if (!plan.valid || plan.command_count != DM2_V1_GDAT_HUD_M11_COMMAND_MAX ||
        plan.command_hash == 0u) {
        ++failures;
    }
    for (int i = 0; i < plan.command_count; ++i) {
        const DM2_V1_GdatHudM11Command *command = &plan.commands[i];
        if (command->kind != expected_kind[i] ||
            (i < 9 && (command->gdat_category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
                       command->gdat_index < 2 || command->gdat_index > 6)) ||
            (i >= 9 && (command->gdat_category != DM2_GDAT_CATEGORY_CHAMPIONS ||
                        command->gdat_index != i - 9 || command->gdat_field != 0)) ||
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
    dm2_v1_boot_cleanup(&boot);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (failures != 0) {
        fprintf(stderr, "FAIL: %d invalid GDAT HUD command(s)\n", failures);
        return 1;
    }
    puts("PASS: real GRAPHICS.DAT yields a complete fail-closed M11 HUD command family");
    return 0;
}
