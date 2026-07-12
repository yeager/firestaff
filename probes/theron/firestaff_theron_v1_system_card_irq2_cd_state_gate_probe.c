#include "theron_v1_system_card_irq2_cd_state_gate.h"

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

static Theron_Track02Stage2DynamicPayloadReceipt payload(
    Theron_Track02Variant variant,
    uint32_t record) {
    Theron_Track02Stage2DynamicPayloadReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.variant = variant;
    receipt.track02_record = record;
    receipt.header_word0 = 0x00ffu;
    return receipt;
}

int main(void) {
    const char *system_card_path = getenv("FIRESTAFF_THERON_SYSCARD3_PCE");
    Theron_V1SystemCardIrq2CdStateGate jp;
    Theron_V1SystemCardIrq2CdStateGate us;
    Theron_Track02Stage2DynamicPayloadReceipt jp_payload =
        payload(THERON_TRACK02_VARIANT_JP_BIN, 0x0004dfu);
    Theron_Track02Stage2DynamicPayloadReceipt us_payload =
        payload(THERON_TRACK02_VARIANT_US_BIN, 0x0004e0u);
    uint8_t *system_card_rom;
    size_t system_card_rom_size;

    if (!system_card_path) {
        ++g_skip;
        printf("[SKIP] set FIRESTAFF_THERON_SYSCARD3_PCE to System Card 3.0\n");
        return 0;
    }
    system_card_rom = read_file_bytes(system_card_path, &system_card_rom_size);
    check(system_card_rom != NULL,
          "System Card 3.0 container reads");
    check(system_card_rom &&
              theron_v1_system_card_irq2_cd_state_gate_from_original_media(
                  &jp_payload, system_card_rom, system_card_rom_size,
                  "ff1a674273fe3540ccef576376407d1d", &jp),
          "JP stage-three receipt reaches authentic CD-state gate");
    check(system_card_rom &&
              theron_v1_system_card_irq2_cd_state_gate_from_original_media(
                  &us_payload, system_card_rom, system_card_rom_size,
                  "ff1a674273fe3540ccef576376407d1d", &us),
          "US stage-three receipt reaches authentic CD-state gate");
    check(jp.valid && us.valid && jp.handler_address == 0xe736u &&
              jp.clear_path_address == 0xe742u &&
              jp.cd_status_low_address == 0x1802u &&
              jp.cd_status_high_address == 0x1803u &&
              jp.irq_state_zero_page_address == 0xf2u && jp.branch_bit == 2u &&
              jp.branch_when_clear_address == 0xe7b3u &&
              jp.hardware_state_merged_and_stored && jp.selected_branch_unobserved,
          "IRQ2 clear path updates CD state before an unselected branch");
    free(system_card_rom);
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
