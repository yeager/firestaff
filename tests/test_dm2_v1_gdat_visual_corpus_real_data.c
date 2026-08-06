/* PC English GRAPHICS.DAT visual-corpus census.
 *
 * Greatstone's PC 1.0 English catalogue identifies 5,624 exported visual
 * items.  That number is an external presentation catalogue, not an ENT1
 * count: GRAPHICS.DAT also contains words, text, palettes and raw controls.
 * This test instead walks every real dtImage ENT1 row and decodes its exact
 * RAW index.  It must not select a neighbouring category/index/field entry,
 * create a cache, or substitute pixels when one source image is unsupported.
 *
 * Source: SKProject SKULLWIN/c_querydb.cpp::QUERY_GDAT_IMAGE_ENTRY_BUFF and
 * c_gfx_decode.cpp::EXTRACT_GDAT_IMAGE / IMG3::Getpf / DECODE_IMG9.
 */

#include "dm2_v1_asset_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Greatstone's PC 1.0 English GRAPHICS.DAT catalogue has 5,624 raw entries.
 * The following derived ENT1/image census is from the hash-verified original
 * corpus used by the DM2 boot profile.  Keep this test specific to that
 * corpus: merely being a decodable GDAT file must not look like complete PC
 * asset coverage. */
enum {
    DM2_PC10_EN_RAW_COUNT = 5624u,
    DM2_PC10_EN_ENT1_COUNT = 11854u,
    DM2_PC10_EN_IMAGE_ROW_COUNT = 5676u,
    DM2_PC10_EN_UNIQUE_IMAGE_RAW_COUNT = 4031u,
    DM2_PC10_EN_DECODED_PIXEL_COUNT = 18633937u,
    DM2_PC10_EN_VISUAL_CENSUS_HASH = 0xbf5050d3u
};

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

static int load_canonical_graphics(uint8_t **graphics, size_t *graphics_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    const char *bases[2];
    char default_base[1024];
    char path[1152];
    int count = 0;

    if (root && root[0]) bases[count++] = root;
    if (home && home[0]) {
        snprintf(default_base, sizeof(default_base),
                 "%s/.firestaff/data/dm2/data", home);
        bases[count++] = default_base;
    }
    for (int i = 0; i < count; ++i) {
        snprintf(path, sizeof(path), "%s/graphics.dat", bases[i]);
        if (read_file(path, graphics, graphics_size)) return 1;
    }
    return 0;
}

static uint32_t fnv1a_step(uint32_t hash, uint32_t value)
{
    for (int byte = 0; byte < 4; ++byte) {
        hash ^= (value >> (byte * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    uint8_t *seen_raw = NULL;
    uint32_t image_rows = 0u;
    uint32_t unique_raw_images = 0u;
    uint32_t decoded_pixels = 0u;
    uint32_t corpus_hash = 2166136261u;
    int census_matches;
    int failures = 0;

    if (!load_canonical_graphics(&graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 GRAPHICS.DAT");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader) || loader.raw_data_count == 0u) {
        fputs("FAIL: canonical GRAPHICS.DAT was not admitted\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    seen_raw = (uint8_t *)calloc(loader.raw_data_count, 1u);
    if (!seen_raw) {
        fputs("FAIL: no bounded raw-image census storage\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }

    for (uint16_t ordinal = 0u; ordinal < loader.entry_count; ++ordinal) {
        const DM2_V1_GdatEntry *entry = &loader.entries[ordinal];
        uint16_t raw_index;
        uint8_t *pixels;
        int width = 0;
        int height = 0;
        DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;

        if (entry->cls3 != DM2_GDAT_ENTRY_TYPE_IMAGE) continue;
        ++image_rows;
        raw_index = (uint16_t)(entry->data_index & 0x7fffu);
        if (raw_index >= loader.raw_data_count) {
            fprintf(stderr, "FAIL: image row %u has raw index %u outside %u\n",
                    (unsigned int)ordinal, (unsigned int)raw_index,
                    (unsigned int)loader.raw_data_count);
            ++failures;
            continue;
        }
        if (seen_raw[raw_index]) continue;
        seen_raw[raw_index] = 1u;
        ++unique_raw_images;
        pixels = dm2_v1_asset_load_raw_image(&loader, raw_index, &width,
                                             &height, &format);
        if (!pixels || width <= 0 || height <= 0 ||
            format == DM2_IMG_FMT_UNKNOWN ||
            (uint32_t)width > UINT32_MAX / (uint32_t)height ||
            decoded_pixels > UINT32_MAX - (uint32_t)width * (uint32_t)height) {
            fprintf(stderr,
                    "FAIL: image row %u raw=%u failed exact source decode "
                    "(%dx%d format=%d)\n",
                    (unsigned int)ordinal, (unsigned int)raw_index, width,
                    height, (int)format);
            dm2_v1_asset_free_pixels(pixels);
            ++failures;
            continue;
        }
        decoded_pixels += (uint32_t)width * (uint32_t)height;
        corpus_hash = fnv1a_step(corpus_hash, raw_index);
        corpus_hash = fnv1a_step(corpus_hash, (uint32_t)width);
        corpus_hash = fnv1a_step(corpus_hash, (uint32_t)height);
        corpus_hash = fnv1a_step(corpus_hash, (uint32_t)format);
        dm2_v1_asset_free_pixels(pixels);
    }

    printf("raw=%u ENT1=%u image-rows=%u unique-raw-images=%u decoded-pixels=%u "
           "census-hash=%08x\n", (unsigned int)loader.raw_data_count,
           (unsigned int)loader.entry_count,
           (unsigned int)image_rows, (unsigned int)unique_raw_images,
           (unsigned int)decoded_pixels, corpus_hash);
    census_matches = loader.raw_data_count == DM2_PC10_EN_RAW_COUNT &&
        loader.entry_count == DM2_PC10_EN_ENT1_COUNT &&
        image_rows == DM2_PC10_EN_IMAGE_ROW_COUNT &&
        unique_raw_images == DM2_PC10_EN_UNIQUE_IMAGE_RAW_COUNT &&
        decoded_pixels == DM2_PC10_EN_DECODED_PIXEL_COUNT &&
        corpus_hash == DM2_PC10_EN_VISUAL_CENSUS_HASH;
    free(seen_raw);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (failures != 0 || !census_matches) {
        fputs("FAIL: PC 1.0 English visual corpus drifted or contains an unhandled image\n",
              stderr);
        return 1;
    }
    puts("PASS: every unique PC 1.0 English GDAT image payload decodes directly");
    return 0;
}
