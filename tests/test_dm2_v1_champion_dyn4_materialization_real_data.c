/* Real PC-DOS regression for the exact raw-block layout used by the final
 * DM2_LOAD_DYN4 copy pass.  No fixture bytes are accepted. */

#include "dm2_v1_asset_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t rd16le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

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
    DM2_V1_GdatDyn4MaterializedSelection selection;
    uint16_t i;
    uint32_t payload_bytes = 0u;

    if (!path || !(graphics = read_file(path, &graphics_size))) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    memset(&selection, 0, sizeof(selection));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader) ||
        !dm2_v1_gdat_dyn4_materialize_selection(&loader, 0x16ffffffu,
                                                 &selection) ||
        !selection.valid || selection.block_count == 0u ||
        !selection.bytes || !selection.raw_indices || !selection.block_offsets) {
        dm2_v1_gdat_dyn4_materialized_selection_free(&selection);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        fputs("FAIL: champion DYN4 source blocks were not materialized\n", stderr);
        return 1;
    }
    for (i = 0; i < selection.block_count; ++i) {
        uint32_t offset = selection.block_offsets[i];
        uint16_t length;
        uint16_t raw_index;
        size_t raw_size = 0u;
        const uint8_t *raw;

        if (offset > selection.byte_count - 4u) {
            fputs("FAIL: DYN4 block offset outside RAM image\n", stderr);
            return 1;
        }
        length = rd16le(selection.bytes + offset);
        raw_index = rd16le(selection.bytes + offset + 2u +
                           ((uint32_t)length + 1u & ~1u));
        raw = dm2_v1_load_gdat_raw_data(&loader, raw_index, &raw_size);
        if (raw_index != selection.raw_indices[i] || !raw ||
            raw_size != length ||
            memcmp(selection.bytes + offset + 2u, raw, raw_size) != 0) {
            fputs("FAIL: DYN4 block differs from GRAPHICS.DAT source bytes\n", stderr);
            return 1;
        }
        payload_bytes += length;
    }
    if (selection.block_count == 85u && selection.byte_count == 25074u &&
        payload_bytes == 24701u && selection.skipped_sound_entry_count == 21u &&
        selection.payload_hash == 0x82fa9459u &&
        selection.receipt_hash == 0x193ee5d4u) {
        printf("PASS: 16ffffff materialized %u non-sound raw blocks "
               "(%u bytes, payload %u; %u sound rows deferred, hash %08x/%08x)\n",
               selection.block_count, selection.byte_count, payload_bytes,
               selection.skipped_sound_entry_count, selection.payload_hash,
               selection.receipt_hash);
    } else {
        fputs("FAIL: DYN4 sound rows were not explicitly deferred\n", stderr);
        return 1;
    }
    dm2_v1_gdat_dyn4_materialized_selection_free(&selection);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    return 0;
}
