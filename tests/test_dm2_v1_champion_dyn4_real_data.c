/* Canonical PC-DOS G1 sends the 16 champion-mirror actuators to
 * DM2_MARK_DYN_LOAD(0x16ffffff).  This test reads only the authenticated
 * GRAPHICS.DAT and verifies the corresponding original DYN4 wildcard set. */

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
    DM2_V1_GdatDyn4SelectionReceipt receipt;

    if (!path || !(graphics = read_file(path, &graphics_size))) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader) ||
        !dm2_v1_gdat_dyn4_selection_receipt(&loader, 0x16ffffffu,
                                             &receipt) ||
        !receipt.valid || receipt.matched_entry_count != 277u ||
        receipt.raw_loadable_entry_count != 277u ||
        receipt.scalar_entry_count != 0u ||
        receipt.high_bit_data_index_count != 0u ||
        receipt.sound_entry_count != 21u ||
        receipt.rejected_raw_count != 0u ||
        receipt.payload_bytes != 221878u ||
        receipt.receipt_hash != 0xbcb603efu) {
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        fputs("FAIL: champion DYN4 selector did not resolve original GDAT\n",
              stderr);
        return 1;
    }
    printf("PASS: 16ffffff selects %u rows, %u raw (%u bytes), "
           "%u scalar, %u high-bit, %u sound, hash %08x\n",
           receipt.matched_entry_count, receipt.raw_loadable_entry_count,
           receipt.payload_bytes, receipt.scalar_entry_count,
           receipt.high_bit_data_index_count, receipt.sound_entry_count,
           receipt.receipt_hash);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    return 0;
}
