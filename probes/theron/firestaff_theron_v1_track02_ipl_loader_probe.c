/* Hash-gated PC Engine Track 02 IPL loader provenance probe.
 *
 * Covers only the original bootstrap: IPL record 0x3a3, loaded executable
 * span, and its System Card CD_READ local-RAM setup.  It does not decode or
 * render graphics.  Optional real-media checks use
 * FIRESTAFF_THERON_TRACK02_{JP,US}_BIN. */

#include "asset_status_m12.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_DATA_OFFSET 16u

static int g_failures;

static void check(int condition, const char *label) {
    if (!condition) {
        ++g_failures;
        printf("FAIL %s\n", label);
    }
}

static void put_user(uint8_t *data, size_t sector, size_t offset, uint8_t value) {
    data[sector * RAW_SECTOR_BYTES + USER_DATA_OFFSET + offset] = value;
}

static void put_bytes(uint8_t *data, size_t sector, size_t offset,
                      const uint8_t *bytes, size_t byte_count) {
    size_t i;
    for (i = 0u; i < byte_count; ++i) put_user(data, sector, offset + i, bytes[i]);
}

static uint8_t *make_fixture(size_t index01, size_t executable_sectors,
                             size_t *out_size) {
    static const uint8_t signature[] = "PC Engine CD-ROM SYSTEM";
    static const uint8_t read_setup[] = {
        0xa9u, 0x00u, 0x85u, 0xfau, 0xa9u, 0x30u, 0x85u, 0xfbu,
        0xa9u, 0x01u, 0x85u, 0xffu, 0x20u, 0x09u, 0xe0u
    };
    static const uint8_t exec_setup[] = {
        0x82u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfcu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfeu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfdu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xf8u,
        0xa9u, 0x00u, 0x85u, 0xfau, 0xa9u, 0x40u, 0x85u, 0xfbu,
        0xa9u, 0x01u, 0x85u, 0xffu, 0x20u, 0x0fu, 0xe0u
    };
    static const uint8_t exec_record[] = {0x00u, 0xe7u, 0x03u, 0x11u};
    static const uint8_t stage2_read_setup[] = {
        0xa9u, 0x01u, 0x85u, 0xf8u, 0xa9u, 0x01u, 0x85u, 0xffu,
        0xa9u, 0x00u, 0x85u, 0xfau, 0xa9u, 0x38u, 0x85u, 0xfbu,
        0x20u, 0x09u, 0xe0u
    };
    size_t executable_sector = index01 + THERON_TRACK02_IPL_RECORD;
    size_t stage2_sector = index01 + THERON_TRACK02_IPL_STAGE2_RECORD;
    uint8_t *data = (uint8_t *)calloc(stage2_sector +
                                      THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
                                      RAW_SECTOR_BYTES);
    if (!data) return NULL;
    put_user(data, index01 + 1u, 0u, 0x00u);
    put_user(data, index01 + 1u, 1u, 0x03u);
    put_user(data, index01 + 1u, 2u, 0xa3u);
    put_user(data, index01 + 1u, 3u, (uint8_t)executable_sectors);
    put_user(data, index01 + 1u, 4u, 0x00u);
    put_user(data, index01 + 1u, 5u, 0x40u);
    put_user(data, index01 + 1u, 6u, 0x00u);
    put_user(data, index01 + 1u, 7u, 0x40u);
    put_bytes(data, index01 + 1u, 32u, signature, sizeof(signature) - 1u);
    put_bytes(data, executable_sector, 0xc1u, read_setup, sizeof(read_setup));
    put_bytes(data, executable_sector, 0x80u, exec_setup, sizeof(exec_setup));
    put_bytes(data, executable_sector, 0xd5u, exec_record, sizeof(exec_record));
    put_bytes(data, stage2_sector, 0x80u, stage2_read_setup,
              sizeof(stage2_read_setup));
    *out_size = (stage2_sector + THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT) *
                RAW_SECTOR_BYTES;
    return data;
}

static void check_receipt(const Theron_Track02IplLoaderReceipt *receipt,
                          Theron_Track02Variant variant, size_t index01,
                          size_t sectors, const char *prefix) {
    check(receipt->valid && receipt->variant == variant, prefix);
    check(receipt->data_track_index01_raw_sector == index01 &&
              receipt->information_raw_sector == index01 + 1u &&
              receipt->executable_raw_sector == index01 + THERON_TRACK02_IPL_RECORD,
          "receipt sector span");
    check(receipt->record == THERON_TRACK02_IPL_RECORD &&
              receipt->executable_sector_count == sectors &&
              receipt->executable_user_data_bytes == sectors * 2048u &&
              receipt->load_address == THERON_TRACK02_IPL_LOAD_ADDRESS &&
              receipt->entry_address == THERON_TRACK02_IPL_LOAD_ADDRESS,
          "receipt IPL record/load/entry");
    check(receipt->cd_read_user_data_offset == 0xc1u &&
              receipt->cd_read_cpu_address == THERON_TRACK02_IPL_CD_READ_CPU_ADDRESS &&
              receipt->cd_read_system_card_address ==
                  THERON_TRACK02_IPL_CD_READ_SYSTEM_CARD_ADDRESS &&
              receipt->cd_read_destination == THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM &&
              receipt->cd_read_local_destination ==
                  THERON_TRACK02_IPL_CD_READ_LOCAL_DESTINATION &&
              receipt->cd_exec_cpu_address == THERON_TRACK02_IPL_CD_EXEC_CPU_ADDRESS &&
              receipt->cd_exec_system_card_address ==
                  THERON_TRACK02_IPL_CD_EXEC_SYSTEM_CARD_ADDRESS &&
              receipt->stage2_record == THERON_TRACK02_IPL_STAGE2_RECORD &&
              receipt->stage2_sector_count == THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT &&
              receipt->stage2_destination == THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM &&
              receipt->stage2_load_address == THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS &&
              receipt->stage2_entry_address == THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS &&
              receipt->stage2_raw_sector == index01 + THERON_TRACK02_IPL_STAGE2_RECORD &&
              receipt->stage2_user_data_bytes ==
                  THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT * 2048u &&
              receipt->stage2_user_data_hash != 0u &&
              receipt->stage2_cd_read_cpu_address ==
                  THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS &&
              receipt->stage2_cd_read_sector_count == 1u &&
              receipt->stage2_cd_read_destination == THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM &&
              receipt->stage2_cd_read_local_destination ==
                  THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION &&
              !receipt->stage2_cd_read_record_proven &&
              receipt->stage2_cd_read_dynamic_boundary_valid &&
              receipt->stage2_cd_read_live_record_register_mask ==
                  THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_MASK &&
              !receipt->vram_transfer_proven,
          "receipt stage-two dynamic boundary stays local and VRAM-unbound");
}

static void check_real_media(const char *path, const char *md5,
                             Theron_Track02Variant variant) {
    FILE *file;
    long length;
    uint8_t *data;
    char actual_md5[33];
    Theron_Track02IplLoaderReceipt receipt;

    if (!path) return;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        printf("SKIP real IPL media unreadable: %s\n", path);
        return;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        ++g_failures;
        printf("FAIL real IPL media read\n");
        return;
    }
    fclose(file);
    check(m12_file_md5_hex(path, actual_md5) && strcmp(actual_md5, md5) == 0,
          "real IPL media MD5");
    check(theron_v1_track02_find_ipl_loader(data, (size_t)length, md5, &receipt) ==
              THERON_TRACK02_SIGNAL_OK,
          "real IPL loader receipt");
    check(receipt.variant == variant && receipt.executable_user_data_hash != 0u,
          "real IPL loader identity/hash");
    free(data);
}

int main(void) {
    uint8_t *data;
    size_t data_size;
    Theron_Track02IplLoaderReceipt receipt;

    data = make_fixture(225u, 4u, &data_size);
    check(data != NULL, "US IPL fixture allocation");
    if (data) {
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_OK,
              "US IPL fixture accepted");
        check_receipt(&receipt, THERON_TRACK02_VARIANT_US_BIN, 225u, 4u,
                      "US IPL fixture identity");
        put_user(data, 226u, 3u, 3u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_US_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "wrong IPL sector count rejects");
        free(data);
    }
    data = make_fixture(224u, 3u, &data_size);
    check(data != NULL, "JP IPL fixture allocation");
    if (data) {
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_OK,
              "JP IPL fixture accepted");
        check_receipt(&receipt, THERON_TRACK02_VARIANT_JP_BIN, 224u, 3u,
                      "JP IPL fixture identity");
        put_user(data, 1155u, 0xcdu, 0x00u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing System Card CD_READ rejects");
        put_user(data, 1155u, 0xcdu, 0x20u);
        data[(1223u * RAW_SECTOR_BYTES) + USER_DATA_OFFSET + 0x80u] = 0u;
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "missing second-stage local CD_READ rejects");
        put_user(data, 1223u, 0x80u, 0xa9u);
        put_user(data, 1155u, 0xd5u, 0x01u);
        check(theron_v1_track02_find_ipl_loader(data, data_size,
                                                 THERON_TRACK02_MD5_JP_BIN,
                                                 &receipt) == THERON_TRACK02_SIGNAL_NOT_FOUND,
              "wrong CD_EXEC record table rejects");
        free(data);
    }
    check(theron_v1_track02_find_ipl_loader(NULL, 0u, THERON_TRACK02_MD5_US_BIN,
                                             &receipt) == THERON_TRACK02_SIGNAL_BAD_INPUT,
          "bad IPL input rejects");
    check(theron_v1_track02_find_ipl_loader((const uint8_t *)"x", 1u,
                                             THERON_TRACK02_MD5_US_ISO,
                                             &receipt) == THERON_TRACK02_SIGNAL_BAD_INPUT,
          "non-sector input rejects before variant");

    check_real_media(getenv("FIRESTAFF_THERON_TRACK02_JP_BIN"),
                     THERON_TRACK02_MD5_JP_BIN, THERON_TRACK02_VARIANT_JP_BIN);
    check_real_media(getenv("FIRESTAFF_THERON_TRACK02_US_BIN"),
                     THERON_TRACK02_MD5_US_BIN, THERON_TRACK02_VARIANT_US_BIN);
    printf("summary: fail=%d\n", g_failures);
    return g_failures ? 1 : 0;
}
