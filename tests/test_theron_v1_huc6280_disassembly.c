#include "theron_v1_huc6280_disassembly.h"
#include "theron_v1_track02.h"

#include <assert.h>
#include <stdint.h>
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

static void verify_bank1f_initializer_call(const char *path,
                                           uint32_t bank_window_file_offset) {
    FILE *file = fopen(path, "rb");
    unsigned char call[2];
    int target;
    assert(file != NULL);
    assert(fseek(file, (long)(bank_window_file_offset +
                              (0x23dcu - 0x2386u)), SEEK_SET) == 0);
    assert(fread(call, 1u, sizeof(call), file) == sizeof(call));
    assert(fclose(file) == 0);
    assert(call[0] == 0x44u); /* HuC6280 BSR relative */
    target = 0x23de + (int)(int8_t)call[1];
    assert(target == 0x23a4);
}

/* JP $C414 preconsumer calls $C95D at $C422 and $CC3E at $C42B; see
 * theron-jp-c3a0-record-consumer.asm lines 87 and 91. Verify that the
 * authentic caller operands agree with the separately locked target windows. */
static void verify_jp_spawn_caller_targets(const char *path,
                                          uint16_t c95d_target,
                                          uint16_t cc3e_target) {
    static const long file_offset = 0x9bb94L;
    unsigned char caller[27];
    FILE *file = fopen(path, "rb");

    assert(file != NULL);
    assert(fseek(file, file_offset, SEEK_SET) == 0);
    assert(fread(caller, 1u, sizeof(caller), file) == sizeof(caller));
    assert(fclose(file) == 0);
    assert(caller[0x0eu] == 0x20u);
    assert(caller[0x0fu] == (unsigned char)(c95d_target & 0xffu));
    assert(caller[0x10u] == (unsigned char)(c95d_target >> 8u));
    assert(caller[0x17u] == 0x20u);
    assert(caller[0x18u] == (unsigned char)(cc3e_target & 0xffu));
    assert(caller[0x19u] == (unsigned char)(cc3e_target >> 8u));
    assert(caller[0x1au] == 0x60u);
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
    if (receipt.status == THERON_V1_HUC6280_DISASSEMBLY_UNAVAILABLE) {
        printf("SKIP: authentic %s disassembly source is not installed\n", label);
        return;
    }
    assert(receipt.status == THERON_V1_HUC6280_DISASSEMBLY_READY);
    assert(receipt.source_file_identity_verified);
    assert(receipt.bank_window_verified);
    assert(receipt.bank1f_disassembly_window_verified);
    assert(receipt.bank1f_disassembly_window_address == 0x2386u);
    assert(receipt.bank1f_disassembly_window_bytes == 0x17cu);
    assert(receipt.bank1f_disassembly_window_file_offset ==
           (variant == THERON_TRACK02_VARIANT_US_BIN ? 0x2bd586u :
            variant == THERON_TRACK02_VARIANT_JP_BIN ? 0x2bcc56u :
            0x1f2386u));
    assert(receipt.bank1f_disassembly_window_fnv1a == 0xd5465b33u);
    verify_bank1f_initializer_call(
        path, receipt.bank1f_disassembly_window_file_offset);
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
        assert(receipt.stage2_dispatch_table_verified);
        assert(receipt.stage2_dispatch_table_address == 0x410du);
        assert(receipt.stage2_dispatch_table_bytes == 170u);
        assert(receipt.stage2_dispatch_table_entries == 85u);
        assert(receipt.stage2_dispatch_table_file_offset ==
               (variant == THERON_TRACK02_VARIANT_US_BIN ? 0x2bee9du :
                                                           0x2be56du));
        assert(receipt.stage2_dispatch_table_fnv1a == 0x7f6a7f04u);
    } else {
        assert(!receipt.stage2_dispatch_table_verified);
    }
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
    if (variant == THERON_TRACK02_VARIANT_US_BIN ||
        variant == THERON_TRACK02_VARIANT_JP_BIN ||
        variant == THERON_TRACK02_VARIANT_US_ISO ||
        variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        assert(receipt.vce_palette_caller_verified);
        assert(receipt.vce_palette_caller_address == 0x966eu);
        assert(receipt.vce_palette_caller_bytes == 23u);
        assert(receipt.vce_palette_caller_address + 20u == 0x9682u);
        assert(receipt.vce_palette_caller_address +
                   receipt.vce_palette_caller_bytes == 0x9685u);
        assert(receipt.vce_palette_caller_file_offset ==
               (variant == THERON_TRACK02_VARIANT_US_BIN ? 0x2c4fdeu :
                variant == THERON_TRACK02_VARIANT_JP_BIN ? 0x2c46aeu :
                                                           0x1f8e6eu));
        assert(receipt.vce_palette_caller_fnv1a == 0xb3b3ccbbU);
        if (variant == THERON_TRACK02_VARIANT_US_BIN ||
            variant == THERON_TRACK02_VARIANT_JP_BIN) {
            assert(receipt.vce_palette_consumer_address -
                       receipt.vce_palette_caller_address ==
                   receipt.vce_palette_consumer_file_offset -
                       receipt.vce_palette_caller_file_offset);
        }
    } else {
        assert(!receipt.vce_palette_caller_verified);
    }
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        assert(receipt.spawn_rng_helper_verified);
        assert(receipt.spawn_rng_helper_address == 0x4667u);
        assert(receipt.spawn_rng_helper_bytes == 25u);
        assert(receipt.spawn_rng_helper_file_offset == 0x9c4e7u);
        assert(receipt.spawn_rng_helper_fnv1a == 0xb9075b31u);
        assert(receipt.spawn_rng_preconsumer_verified);
        assert(receipt.spawn_rng_preconsumer_address == 0x4644u);
        assert(receipt.spawn_rng_preconsumer_bytes == 27u);
        assert(receipt.spawn_rng_preconsumer_file_offset == 0x9c4c4u);
        assert(receipt.spawn_rng_preconsumer_fnv1a == 0xa3c3f7ebu);
        assert(receipt.spawn_rng_c96b_verified);
        assert(receipt.spawn_rng_c96b_address == 0xc96bu);
        assert(receipt.spawn_rng_c96b_bytes == 255u);
        assert(receipt.spawn_rng_c96b_file_offset == 0xa47ebu);
        assert(receipt.spawn_rng_c96b_fnv1a == 0xe689c658u);
        assert(receipt.spawn_rng_cc4c_verified);
        assert(receipt.spawn_rng_cc4c_address == 0xcc4cu);
        assert(receipt.spawn_rng_cc4c_bytes == 200u);
        assert(receipt.spawn_rng_cc4c_file_offset == 0xa4accu);
        assert(receipt.spawn_rng_cc4c_fnv1a == 0x4ad0801eu);
        assert(receipt.spawn_runtime_c3a0_verified);
        assert(receipt.spawn_runtime_c3a0_address == 0xc3a0u);
        assert(receipt.spawn_runtime_c3a0_bytes == 150u);
        assert(receipt.spawn_runtime_c3a0_file_offset == 0x9c450u);
        assert(receipt.spawn_runtime_c3a0_fnv1a == 0x666ded61u);
        assert(!receipt.spawn_runtime_c3a0_jp_verified);
    } else if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        verify_jp_spawn_caller_targets(
            path, receipt.spawn_rng_c95d_jp_candidate_address,
            receipt.spawn_rng_cc3e_jp_candidate_address);
        assert(!receipt.spawn_rng_helper_verified);
        assert(!receipt.spawn_rng_preconsumer_verified);
        assert(!receipt.spawn_rng_c96b_verified);
        assert(!receipt.spawn_rng_cc4c_verified);
        assert(!receipt.spawn_runtime_c3a0_verified);
        assert(receipt.spawn_runtime_c3a0_jp_verified);
        assert(receipt.spawn_runtime_c3a0_jp_address == 0xc3a0u);
        assert(receipt.spawn_runtime_c3a0_jp_bytes == 150u);
        assert(receipt.spawn_runtime_c3a0_jp_file_offset == 0x9bb20u);
        assert(receipt.spawn_runtime_c3a0_jp_fnv1a == 0xe292e892u);
        assert(receipt.spawn_rng_preconsumer_jp_verified);
        assert(receipt.spawn_rng_preconsumer_jp_address == 0xc414u);
        assert(receipt.spawn_rng_preconsumer_jp_bytes == 27u);
        assert(receipt.spawn_rng_preconsumer_jp_file_offset == 0x9bb94u);
        assert(receipt.spawn_rng_preconsumer_jp_fnv1a == 0x3d11a727u);
        assert(receipt.spawn_rng_helper_jp_verified);
        assert(receipt.spawn_rng_helper_jp_address == 0x4661u);
        assert(receipt.spawn_rng_helper_jp_bytes == 25u);
        assert(receipt.spawn_rng_helper_jp_file_offset == 0x9bbb7u);
        assert(receipt.spawn_rng_helper_jp_fnv1a == 0x1a732d61u);
        assert(receipt.spawn_rng_c95d_jp_candidate_verified);
        assert(receipt.spawn_rng_c95d_jp_candidate_address == 0xc95du);
        assert(receipt.spawn_rng_c95d_jp_candidate_bytes == 255u);
        assert(receipt.spawn_rng_c95d_jp_candidate_file_offset == 0x0a3ebbu);
        assert(receipt.spawn_rng_c95d_jp_candidate_fnv1a == 0x063b99e9u);
        assert(receipt.spawn_rng_cc3e_jp_candidate_verified);
        assert(receipt.spawn_rng_cc3e_jp_candidate_address == 0xcc3eu);
        assert(receipt.spawn_rng_cc3e_jp_candidate_bytes == 200u);
        assert(receipt.spawn_rng_cc3e_jp_candidate_file_offset == 0x0a419cu);
        assert(receipt.spawn_rng_cc3e_jp_candidate_fnv1a == 0x13a65ea6u);
        printf("PASS: authentic JP static $C3A0 counterpart at $%x/%u/%08x\n",
               (unsigned)receipt.spawn_runtime_c3a0_jp_address,
               (unsigned)receipt.spawn_runtime_c3a0_jp_bytes,
               (unsigned)receipt.spawn_runtime_c3a0_jp_fnv1a);
    } else {
        assert(!receipt.spawn_rng_helper_verified);
        assert(!receipt.spawn_rng_preconsumer_verified);
        assert(!receipt.spawn_rng_c96b_verified);
        assert(!receipt.spawn_rng_cc4c_verified);
        assert(!receipt.spawn_runtime_c3a0_verified);
        assert(!receipt.spawn_runtime_c3a0_jp_verified);
        assert(!receipt.spawn_rng_preconsumer_jp_verified);
        assert(!receipt.spawn_rng_helper_jp_verified);
        assert(!receipt.spawn_rng_c95d_jp_candidate_verified);
        assert(!receipt.spawn_rng_cc3e_jp_candidate_verified);
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
    printf("PASS: authentic %s HuC6280 disassembly source md5=%s fragment=%08x",
           label, receipt.source_md5, (unsigned)receipt.fragment_fnv1a);
    if (receipt.stage2_dispatch_table_verified) {
        printf(" dispatch=$410d/%u/%08x",
               (unsigned)receipt.stage2_dispatch_table_entries,
               (unsigned)receipt.stage2_dispatch_table_fnv1a);
    }
    putchar('\n');
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
