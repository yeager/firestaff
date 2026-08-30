/* Authentic FM Towns DM2 champion-portrait route.
 *
 * The FM Towns GRAPHICS.DAT is a v4/IMG2 corpus.  This gate exercises the
 * production HUD plan against every real CHAMPIONS type 0..15; it does not
 * manufacture a portrait or borrow the DOS companion graphics file.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_fmtowns_disc.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_graphics_from_archive(uint8_t **out, size_t *out_size)
{
    const char *archive = getenv("FIRESTAFF_DM2_FMTOWNS_ARCHIVE");
    DM2_V1_FmtownsDiscReceipt receipt;
    uint8_t *image = NULL;
    size_t image_size = 0u;
    int result = 0;

    if (!out || !out_size) return -1;
    *out = NULL;
    *out_size = 0u;
    if (!archive || !archive[0] ||
        firestaff_zip_extract_by_suffix(archive, ".img", &image,
                                        &image_size) != 0 ||
        !image || dm2_v1_fmtowns_disc_probe(image, image_size, &receipt) != 0 ||
        !receipt.valid || !receipt.has_graphics_dat ||
        dm2_v1_fmtowns_disc_extract_alloc(image, image_size,
                                           &receipt.graphics_dat,
                                           out, out_size) != 0 ||
        !*out || !*out_size) {
        free(*out);
        *out = NULL;
        *out_size = 0u;
        result = -1;
    }
    free(image);
    return result;
}

int main(void)
{
    uint8_t *data = NULL;
    size_t data_size = 0u;
    DM2_V1_AssetLoader loader;
    int failures = 0;
    int hero;

    if (read_graphics_from_archive(&data, &data_size) != 0) {
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
