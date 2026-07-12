#include "asset_status_m12.h"
#include "theron_v1_system_card_irq2_first_transfer.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;
static int g_skip;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static uint8_t *read_file_bytes(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static int inspect(const char *track02_path,
                   const char *track02_md5,
                   const uint8_t *system_card_rom,
                   size_t system_card_rom_size,
                   const char *system_card_md5,
                   Theron_V1SystemCardIrq2FirstTransfer *out_transfer) {
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    uint8_t *track02_bytes;
    size_t track02_size;
    char actual_md5[33];
    int ok;

    track02_bytes = read_file_bytes(track02_path, &track02_size);
    if (!track02_bytes) return 0;
    ok = m12_file_md5_hex(track02_path, actual_md5) &&
        strcmp(actual_md5, track02_md5) == 0 &&
        theron_v1_track02_inspect_stage2_dynamic_payload(
            track02_bytes, track02_size, track02_md5, &payload) ==
              THERON_TRACK02_SIGNAL_OK &&
        theron_v1_system_card_irq2_first_transfer_from_original_media(
            &payload, system_card_rom, system_card_rom_size, system_card_md5,
            out_transfer);
    free(track02_bytes);
    return ok;
}

int main(void) {
    const char *jp_path = getenv("FIRESTAFF_THERON_TRACK02_JP_BIN");
    const char *us_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *system_card_path = getenv("FIRESTAFF_THERON_SYSCARD3_PCE");
    Theron_V1SystemCardIrq2FirstTransfer jp;
    Theron_V1SystemCardIrq2FirstTransfer us;
    uint8_t *system_card_rom;
    size_t system_card_rom_size;
    char system_card_md5[33];

    if (!jp_path || !us_path || !system_card_path) {
        ++g_skip;
        printf("[SKIP] set JP/US Track02 and FIRESTAFF_THERON_SYSCARD3_PCE paths\n");
        return 0;
    }
    system_card_rom = read_file_bytes(system_card_path, &system_card_rom_size);
    check(system_card_rom && m12_file_md5_hex(system_card_path, system_card_md5),
          "System Card 3.0 ROM reads and hashes");
    check(system_card_rom && inspect(jp_path, THERON_TRACK02_MD5_JP_BIN,
                                     system_card_rom, system_card_rom_size,
                                     system_card_md5, &jp),
          "JP raw Track02 reaches first original IRQ2 transfer gate");
    check(system_card_rom && inspect(us_path, THERON_TRACK02_MD5_US_BIN,
                                     system_card_rom, system_card_rom_size,
                                     system_card_md5, &us),
          "US raw Track02 reaches first original IRQ2 transfer gate");
    check(jp.valid && us.valid && jp.stage3_track02_record == 0x0004dfu &&
              us.stage3_track02_record == 0x0004e0u &&
              jp.handler_address == 0x44beu &&
              jp.accepted_a_minimum == 0x0au && jp.accepted_a_maximum == 0x22u &&
              jp.out_of_range_transfer_address == 0x4492u &&
              jp.in_range_transfer_address == 0x4541u &&
              jp.selected_transfer_unobserved && jp.record_request_unproven,
          "first IRQ2 transfers are exact while selected branch stays unobserved");
    free(system_card_rom);
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
