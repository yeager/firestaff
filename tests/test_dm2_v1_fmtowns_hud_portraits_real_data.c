/* Authentic FM Towns DM2 champion-portrait route.
 *
 * The FM Towns GRAPHICS.DAT is a v4/IMG2 corpus.  This gate exercises the
 * production HUD plan against every real CHAMPIONS type 0..15; it does not
 * manufacture a portrait or borrow the DOS companion graphics file.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_hud_m11_command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;
    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0) {
        if (file) fclose(file);
        return 0;
    }
    size = ftell(file);
    if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1u, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = data;
    *out_size = (size_t)size;
    return 1;
}

static const char *graphics_path(void)
{
    const char *explicit_path = getenv("FIRESTAFF_DM2_FMTOWNS_GRAPHICS_DAT");
    static char fallback[1024];
    const char *root = getenv("FIRESTAFF_DM2_FMTOWNS_ROOT");
    if (explicit_path && explicit_path[0]) return explicit_path;
    if (!root || !root[0]) return NULL;
    snprintf(fallback, sizeof(fallback), "%s/DATA/GRAPHICS.DAT", root);
    return fallback;
}

int main(void)
{
    const char *path = graphics_path();
    uint8_t *data = NULL;
    size_t data_size = 0u;
    DM2_V1_AssetLoader loader;
    int failures = 0;
    int hero;

    if (!path || !read_file(path, &data, &data_size)) {
        puts("SKIP: authentic FM Towns DM2 GRAPHICS.DAT is required");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, data, data_size) != 0 ||
        loader.gdat_version != 4u) {
        fprintf(stderr, "FAIL: FM Towns v4 GRAPHICS.DAT was not admitted\n");
        free(data);
        return 1;
    }

    for (hero = 0; hero < 16; ++hero) {
        DM2_V1_HudPartyState party;
        DM2_V1_GdatHudM11CommandPlan plan;
        const DM2_V1_GdatHudM11Command *portrait;
        memset(&party, 0, sizeof(party));
        memset(&plan, 0, sizeof(plan));
        party.champion_count = 1;
        party.champions[0].occupied = 1;
        party.champions[0].portrait_type_source_bound = 1;
        party.champions[0].portrait_index = (uint8_t)hero;
        if (!dm2_v1_gdat_hud_m11_command_plan_build_for_party(
                &loader, &party, &plan) ||
            plan.command_count != DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT + 1) {
            fprintf(stderr, "FAIL: FM Towns portrait type %d did not bind\n", hero);
            ++failures;
            dm2_v1_gdat_hud_m11_command_plan_free(&plan);
            continue;
        }
        portrait = &plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT];
        if (portrait->kind != DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT ||
            portrait->gdat_category != DM2_GDAT_CATEGORY_CHAMPIONS ||
            portrait->gdat_index != hero || portrait->gdat_field != 0 ||
            portrait->format == DM2_IMG_FMT_UNKNOWN || portrait->width <= 0 ||
            portrait->height <= 0 || portrait->raw_hash == 0u ||
            portrait->decoded_hash == 0u || portrait->palette_hash == 0u) {
            fprintf(stderr, "FAIL: FM Towns portrait type %d receipt invalid\n", hero);
            ++failures;
        }
        dm2_v1_gdat_hud_m11_command_plan_free(&plan);
    }
    dm2_v1_asset_loader_free(&loader);
    free(data);
    if (failures) return 1;
    puts("PASS: authentic DM2 FM Towns CHAMPIONS 0..15 bind through the M11 HUD route");
    return 0;
}
