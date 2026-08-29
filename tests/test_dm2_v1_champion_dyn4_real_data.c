/* Canonical PC-DOS G1 sends the 16 champion-mirror actuators to
 * DM2_MARK_DYN_LOAD(0x16ffffff).  This test reads only the authenticated
 * GRAPHICS.DAT and verifies the corresponding original DYN4 wildcard set. */

#include "dm2_v1_asset_loader.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>

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
    DM2_V1_GdatDyn4SelectionReceipt receipt;

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
