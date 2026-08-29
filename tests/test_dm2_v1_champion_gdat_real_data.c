/* SKProject c_querydb.cpp::DM2_QUERY_GDAT_IMAGE_ENTRY_BUFF:105-108 falls
 * back from a missing CHAMPIONS/HeroType/IMG/0 record to the real
 * MISCELLANEOUS/254/IMG/254 image.  This probes that exact PC-DOS route. */

#include "dm2_v1_asset_loader.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_graphics_from_archive(size_t *out_size)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    uint8_t *bytes = NULL;
    *out_size = 0u;
    if (!archive || !archive[0] ||
        firestaff_zip_extract_by_suffix(archive, "data/graphics.dat",
                                        &bytes, out_size) != 0 ||
        !bytes || *out_size == 0u) {
        free(bytes);
        return NULL;
    }
    return bytes;
}

int main(void)
{
    unsigned char *graphics;
    size_t graphics_size;
    DM2_V1_AssetLoader loader;
    int direct_count = 0;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint8_t *pixels;
    int raw8_count = 0;

    if (!getenv("FIRESTAFF_DM2_DOS_ARCHIVE") ||
        !getenv("FIRESTAFF_DM2_DOS_ARCHIVE")[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 0;
    }
    if (!(graphics = read_graphics_from_archive(&graphics_size))) {
        fputs("FAIL: selected canonical DM2 GRAPHICS.DAT is unreadable\n", stderr);
        return 1;
    }
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader)) {
        free(graphics);
        fputs("FAIL: canonical GRAPHICS.DAT was not accepted\n", stderr);
        return 1;
    }
    for (uint16_t hero_type = 0; hero_type < 16u; ++hero_type) {
        size_t raw8_size = 0u;
        const uint8_t *raw8 = dm2_v1_asset_load_typed_sized(
            &loader, DM2_GDAT_CATEGORY_CHAMPIONS, hero_type,
            DM2_GDAT_ENTRY_TYPE_RAW8, 0, &raw8_size);
        if (raw8 && raw8_size > 0u) {
            DM2_V1_ChampionReviveDataReceipt revive;
            ++raw8_count;
            if (!dm2_v1_asset_champion_revive_data(&loader, hero_type,
                                                   &revive) ||
                !revive.valid || revive.raw8_byte_count != raw8_size) {
                dm2_v1_asset_loader_free(&loader);
                free(graphics);
                fputs("FAIL: original champion revive data was not bounded\n",
                      stderr);
                return 1;
            }
            if (revive.hit_points_base == 0u ||
                revive.stamina_base == 0u || revive.raw8_hash == 0u ||
                revive.name_hash == 0u || revive.name1[0] == '\0') {
                dm2_v1_asset_loader_free(&loader);
                free(graphics);
                fputs("FAIL: original champion revive values are incomplete\n",
                      stderr);
                return 1;
            }
        }
    }
    for (uint16_t i = 0; i < loader.entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader.entries[i];
        if (entry->cls1 != DM2_GDAT_CATEGORY_CHAMPIONS ||
            entry->cls2 != 0xffu) {
            continue;
        }
        ++direct_count;
    }
    pixels = dm2_v1_asset_load_image_field(
        &loader, DM2_GDAT_CATEGORY_MISCELLANEOUS, 0xfeu, 0xfeu,
        &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 || format == DM2_IMG_FMT_UNKNOWN) {
        dm2_v1_asset_free_pixels(pixels);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        fputs("FAIL: original champion-image fallback was not decodable\n", stderr);
        return 1;
    }
    dm2_v1_asset_free_pixels(pixels);
    if (direct_count != 0) {
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        fputs("FAIL: CHAMPIONS/255 unexpectedly bypassed the source fallback\n",
              stderr);
        return 1;
    }
    if (raw8_count != 16) {
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        fputs("FAIL: complete original champion RAW8 roster is unavailable\n",
              stderr);
        return 1;
    }
    {
        DM2_V1_ChampionReviveDataReceipt anders;
        if (!dm2_v1_asset_champion_revive_data(&loader, 2u, &anders) ||
            strcmp(anders.name1, "ANDERS") != 0 ||
            strcmp(anders.name2, "LIGHT WIELDER") != 0) {
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            fputs("FAIL: CHAMPIONS/2 original name split is unavailable\n",
                  stderr);
            return 1;
        }
    }
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    printf("PASS: CHAMPIONS/255 uses real fallback %u/%u/IMG/%u (%dx%d)\n",
           (unsigned int)DM2_GDAT_CATEGORY_MISCELLANEOUS, 0xfeu, 0xfeu,
           width, height);
    return 0;
}
