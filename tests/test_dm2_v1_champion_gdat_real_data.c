/* SKProject c_querydb.cpp::DM2_QUERY_GDAT_IMAGE_ENTRY_BUFF:105-108 falls
 * back from a missing CHAMPIONS/HeroType/IMG/0 record to the real
 * MISCELLANEOUS/254/IMG/254 image.  This probes that exact PC-DOS route. */

#include "dm2_v1_asset_loader.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    long size;
    unsigned char *bytes;

    *out_size = 0u;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static const char *resolve_graphics_path(int argc, char **argv,
                                         char *buf, size_t buf_size)
{
    const char *root;
    if (argc >= 2) return argv[1];
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) {
        snprintf(buf, buf_size, "%s/graphics.dat", root);
        return buf;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    char path_buf[1024];
    const char *path = resolve_graphics_path(argc, argv, path_buf,
                                             sizeof(path_buf));
    unsigned char *graphics;
    size_t graphics_size;
    DM2_V1_AssetLoader loader;
    int direct_count = 0;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint8_t *pixels;

    if (!path || !(graphics = read_file(path, &graphics_size))) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader)) {
        free(graphics);
        fputs("FAIL: canonical GRAPHICS.DAT was not accepted\n", stderr);
        return 1;
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
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (direct_count != 0) {
        fputs("FAIL: CHAMPIONS/255 unexpectedly bypassed the source fallback\n",
              stderr);
        return 1;
    }
    printf("PASS: CHAMPIONS/255 uses real fallback %u/%u/IMG/%u (%dx%d)\n",
           (unsigned int)DM2_GDAT_CATEGORY_MISCELLANEOUS, 0xfeu, 0xfeu,
           width, height);
    return 0;
}
