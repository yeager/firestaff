/* Canonical CREATURES animation-table triad receipt.
 * Source: SKProject GET_CREATURE_ANIMATION_FRAME and skcrture.cpp V5 helpers. */

#include "dm2_v1_asset_loader.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>

static int load_canonical_graphics(uint8_t **out, size_t *out_size)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    char path[2048];

    if (!archive || !archive[0]) return 0;
    snprintf(path, sizeof(path), "%s::data/graphics.dat", archive);
    return asset_read_path_alloc(path, out, out_size) && *out && *out_size;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    unsigned int complete_count = 0u;
    uint64_t complete_mask_low = 0u;
    uint64_t complete_mask_high = 0u;

    if (!getenv("FIRESTAFF_DM2_DOS_ARCHIVE") ||
        !getenv("FIRESTAFF_DM2_DOS_ARCHIVE")[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 0;
    }
    if (!load_canonical_graphics(&graphics, &graphics_size)) {
        fputs("FAIL: selected canonical DM2 GRAPHICS.DAT is unreadable\n", stderr);
        return 1;
    }
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        free(graphics);
        fputs("FAIL: canonical GRAPHICS.DAT was not accepted\n", stderr);
        return 1;
    }
    for (int creature = 0; creature < 64; ++creature) {
        size_t attribution_size = 0u;
        size_t info_size = 0u;
        size_t frame_size = 0u;
        const uint8_t *attribution = dm2_v1_asset_load_typed_sized(
            &loader, DM2_GDAT_CATEGORY_CREATURES, creature,
            DM2_GDAT_ENTRY_TYPE_RAW8, DM2_GDAT_CREATURE_ANIM_ATTRIBUTION,
            &attribution_size);
        const uint8_t *info = dm2_v1_asset_load_typed_sized(
            &loader, DM2_GDAT_CATEGORY_CREATURES, creature,
            DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
            &info_size);
        const uint8_t *frames = dm2_v1_asset_load_typed_sized(
            &loader, DM2_GDAT_CATEGORY_CREATURES, creature,
            DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_FRAME_SEQUENCE,
            &frame_size);

        if ((!attribution || attribution_size == 0u) &&
            (!info || info_size == 0u) && (!frames || frame_size == 0u)) {
            continue;
        }
        if (!attribution || attribution_size == 0u || !info || info_size == 0u ||
            !frames || frame_size == 0u) {
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            fputs("FAIL: partial CREATURES animation table triad\n", stderr);
            return 1;
        }
        ++complete_count;
        if (creature < 32) complete_mask_low |= UINT64_C(1) << creature;
        else complete_mask_high |= UINT64_C(1) << (creature - 32);
    }
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (complete_count == 0u || (complete_mask_low | complete_mask_high) == 0u) {
        fputs("FAIL: canonical CREATURES has no complete animation owner\n", stderr);
        return 1;
    }
    printf("PASS: canonical CREATURES animation triads=%u mask=%08llx/%08llx\n",
           complete_count, (unsigned long long)complete_mask_low,
           (unsigned long long)complete_mask_high);
    return 0;
}
