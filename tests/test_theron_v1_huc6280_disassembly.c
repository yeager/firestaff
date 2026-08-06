#include "theron_v1_huc6280_disassembly.h"
#include "theron_v1_track02.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const char *path_for(const char *env_name, const char *name,
                            char *fallback, size_t capacity) {
    const char *value = getenv(env_name);
    const char *home = getenv("HOME");
    if (value && value[0]) return value;
    if (!home || !home[0]) return NULL;
    if (snprintf(fallback, capacity, "%s/.firestaff/data/theron/%s",
                 home, name) < 0) return NULL;
    return fallback;
}

static void verify(const char *env_name, const char *name, int variant,
                   const char *label) {
    char fallback[512];
    Theron_V1Huc6280DisassemblyReceipt receipt;
    const char *path = path_for(env_name, name, fallback, sizeof(fallback));

    if (!path) {
        printf("SKIP: %s bank-$1f disassembly source unavailable\n", label);
        return;
    }
    assert(theron_v1_huc6280_disassembly_read_file(path, variant, &receipt));
    assert(receipt.status == THERON_V1_HUC6280_DISASSEMBLY_READY);
    assert(receipt.source_file_identity_verified);
    assert(receipt.bank_window_verified);
    assert(receipt.forward_byte_step_verified);
    assert(receipt.bank_switch_table_verified);
    assert(receipt.reverse_byte_read_verified);
    assert(receipt.level_decompressor_fragment_verified);
    assert(receipt.level_decompressor_caller_verified);
    assert(receipt.stage2_resource_handler_verified);
    assert(receipt.stage2_resource_bank_table_population_verified);
    assert(receipt.stage2_resource_destination_registers_verified);
    if (variant == THERON_TRACK02_VARIANT_US_BIN ||
        variant == THERON_TRACK02_VARIANT_JP_BIN) {
        assert(receipt.vce_palette_consumer_verified);
        assert(receipt.vce_palette_consumer_address == 0x96a5u);
        assert(receipt.vce_palette_consumer_bytes == 37u);
        assert(receipt.vce_palette_consumer_file_offset != 0u);
        assert(receipt.vce_palette_consumer_fnv1a == 0xff51fac4u);
    } else {
        assert(!receipt.vce_palette_consumer_verified);
    }
    assert(!receipt.semantic_publication_allowed);
    assert(receipt.fragment_address == 0x243eu);
    assert(receipt.fragment_bytes == 134u);
    assert(receipt.fragment_fnv1a != 0u);
    assert(receipt.level_decompressor_address == 0x23adu);
    assert(receipt.level_decompressor_bytes == 382u);
    assert(receipt.level_decompressor_fnv1a == 0x3056f96cu);
    assert(receipt.level_decompressor_caller_address == 0x2386u);
    assert(receipt.level_decompressor_caller_bytes == 30u);
    assert(receipt.level_decompressor_caller_fnv1a == 0x699e8da1u);
    assert(receipt.stage2_resource_handler_address == 0x4c3fu);
    assert(receipt.stage2_resource_handler_bytes == 162u);
    assert(receipt.stage2_resource_handler_fnv1a ==
           (variant == THERON_TRACK02_VARIANT_US_BIN ? 0x58cd4b73u :
            variant == THERON_TRACK02_VARIANT_JP_BIN ? 0x788df8e7u :
            0x46360d97u));
    printf("PASS: authentic %s bank-$1f HuC6280 fragment md5=%s fnv=%08x\n",
           label, receipt.source_md5, (unsigned)receipt.fragment_fnv1a);
}

int main(void) {
    Theron_V1Huc6280DisassemblyReceipt missing;
    assert(theron_v1_huc6280_disassembly_read_file(
               "/definitely/missing/theron-track02.iso",
               THERON_TRACK02_VARIANT_US_ISO, &missing));
    assert(missing.status == THERON_V1_HUC6280_DISASSEMBLY_UNAVAILABLE);
    verify("FIRESTAFF_THERON_US_ISO", "TQUS19.iso",
           THERON_TRACK02_VARIANT_US_ISO, "US");
    verify("FIRESTAFF_THERON_JP_ISO", "TQJP19.iso",
           THERON_TRACK02_VARIANT_JP_REV1_ISO, "JP");
    verify("FIRESTAFF_THERON_US_BIN", "TQUS02.bin",
           THERON_TRACK02_VARIANT_US_BIN, "US raw BIN");
    verify("FIRESTAFF_THERON_JP_BIN", "TQJP02.bin",
           THERON_TRACK02_VARIANT_JP_BIN, "JP raw BIN");
    puts("PASS: theron_v1_huc6280_disassembly");
    return 0;
}
