#include "theron_v1_huc6280_disassembly.h"
#include "theron_v1_track02.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void verify_stage2_vce_palette(const char *path,
                                      Theron_Track02Variant variant,
                                      const char *md5_hex) {
    FILE *file = fopen(path, "rb");
    long file_size;
    uint8_t *bytes;
    Theron_Track02Stage2VcePalette palette;
    Theron_Track02SignalStatus status;

    assert(file != NULL);
    assert(fseek(file, 0L, SEEK_END) == 0);
    file_size = ftell(file);
    assert(file_size > 0L);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    bytes = (uint8_t *)malloc((size_t)file_size);
    assert(bytes != NULL);
    assert(fread(bytes, 1u, (size_t)file_size, file) == (size_t)file_size);
    assert(fclose(file) == 0);

    status = theron_v1_track02_read_stage2_vce_palette(
        bytes, (size_t)file_size, md5_hex, &palette);
    free(bytes);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(palette.variant == variant);
    assert(palette.dispatch_entry_verified);
    assert(palette.routine_verified);
    assert(palette.payload_verified);
    assert(palette.vce_palette_start_index == 0x00e0u);
    assert(palette.vce_palette_entry_count == 32u);
    assert(palette.payload_byte_count ==
           THERON_TRACK02_STAGE2_VCE_PALETTE_BYTES);
    assert(palette.payload_checksum == 0x591d332bu);
    assert(palette.payload_raw_offset == palette.routine_raw_offset + 18u);
    assert(palette.groups[0].valid && palette.groups[1].valid);
    assert(palette.groups[0].nonblack_entry_count == 16u);
    assert(palette.groups[1].nonblack_entry_count == 15u);
}

/* This static Track 02 fragment initializes a four-byte prefix at $287f,
 * then copies it through an overlapping TII into $2883-$2896. Its runtime
 * bank/LBA and gameplay meaning are not established by this raw-BIN check. */
static void verify_buffer_setup_candidates(const char *path,
                                           Theron_Track02Variant variant) {
    static const uint32_t jp_offsets[] = {
        0x0a7344u, 0x0f0b44u, 0x13a344u, 0x183b44u,
        0x1cd344u, 0x216b44u, 0x260344u
    };
    static const uint32_t us_offsets[] = {
        0x0a7c74u, 0x0f1474u, 0x13ac74u, 0x184474u,
        0x1cdc74u, 0x217474u, 0x260c74u
    };
    static const unsigned char fragment[] = {
        0xa9u, 0x97u, 0x8du, 0x1au, 0x27u, 0x62u, 0x8du,
        0x80u, 0x28u, 0x8du, 0x81u, 0x28u, 0x8du, 0x82u,
        0x28u, 0x3au, 0x8du, 0x7fu, 0x28u, 0x73u, 0x7fu,
        0x28u, 0x83u, 0x28u, 0x14u, 0x00u
    };
    const uint32_t *expected_offsets;
    const size_t expected_count = THERON_V1_HUC6280_CALLEE_CANDIDATE_COUNT;
    FILE *file = fopen(path, "rb");
    unsigned char *bytes;
    long file_size;
    size_t matches = 0u;

    assert(variant == THERON_TRACK02_VARIANT_US_BIN ||
           variant == THERON_TRACK02_VARIANT_JP_BIN);
    expected_offsets = variant == THERON_TRACK02_VARIANT_JP_BIN ?
        jp_offsets : us_offsets;
    assert(file != NULL);
    assert(fseek(file, 0L, SEEK_END) == 0);
    file_size = ftell(file);
    assert(file_size > 0L);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    bytes = (unsigned char *)malloc((size_t)file_size);
    assert(bytes != NULL);
    assert(fread(bytes, 1u, (size_t)file_size, file) == (size_t)file_size);
    assert(fclose(file) == 0);

    for (size_t offset = 0u;
         offset + sizeof(fragment) <= (size_t)file_size; ++offset) {
        if (memcmp(bytes + offset, fragment, sizeof(fragment)) == 0) {
            assert(matches < expected_count);
            assert((uint32_t)offset == expected_offsets[matches]);
            ++matches;
        }
    }
    free(bytes);
    assert(matches == expected_count);
}

/* The private JP PCE Fast trace matched 155 logged instruction-byte records
 * in $C1DF-$C2FF against this authentic Track 02 window. The same exact
 * 256-byte window occurs in six retail copies; this raw-media check preserves
 * those candidates without claiming which copy supplied the loaded bank or
 * assigning the routine a gameplay meaning. See
 * docs/source-lock/theron-disassembly/theron-jp-c1df-runtime-candidates-20261027.md. */
static void verify_jp_c1df_runtime_candidates(const char *path) {
    static const uint32_t offsets[] = {
        0x09723fu, 0x0e0a3fu, 0x12a23fu,
        0x173a3fu, 0x1bd23fu, 0x206a3fu
    };
    uint8_t window[256];
    FILE *file = fopen(path, "rb");
    uint8_t *bytes;
    long file_size;
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    size_t matches = 0u;

    assert(file != NULL);
    assert(fseek(file, 0L, SEEK_END) == 0);
    file_size = ftell(file);
    assert(file_size > 0L && (uint64_t)file_size <= SIZE_MAX);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    bytes = (uint8_t *)malloc((size_t)file_size);
    assert(bytes != NULL);
    assert(fread(bytes, 1u, (size_t)file_size, file) == (size_t)file_size);
    assert(fclose(file) == 0);

    assert((size_t)offsets[0] + sizeof(window) <= (size_t)file_size);
    memcpy(window, bytes + offsets[0], sizeof(window));
    for (size_t i = 0u; i < sizeof(window); ++i) {
        hash ^= window[i];
        hash *= UINT64_C(0x100000001b3);
    }
    assert(hash == UINT64_C(0x86752173c063b9f9));
    for (size_t i = 0u; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
        assert((size_t)offsets[i] + sizeof(window) <= (size_t)file_size);
        assert(memcmp(bytes + offsets[i], window, sizeof(window)) == 0);
    }
    for (size_t offset = 0u;
         offset + sizeof(window) <= (size_t)file_size; ++offset) {
        if (memcmp(bytes + offset, window, sizeof(window)) == 0) {
            assert(matches < sizeof(offsets) / sizeof(offsets[0]));
            assert((uint32_t)offset == offsets[matches]);
            ++matches;
        }
    }
    free(bytes);
    assert(matches == sizeof(offsets) / sizeof(offsets[0]));
}

/* Source: authenticated JP/US Track 02 caller bytes following the $A704
 * setup candidate; exact offsets and regional decode are recorded in
 * docs/source-lock/theron-disassembly/
 * theron-track02-buffer-setup-caller-20261027.md. These bytes are static
 * candidates, not a runtime bank/source-LBA or click-semantics receipt. */
static void verify_buffer_setup_caller_candidates(
    const char *path, Theron_Track02Variant variant) {
    static const uint32_t jp_offsets[] = {
        0x0a7344u, 0x0f0b44u, 0x13a344u, 0x183b44u,
        0x1cd344u, 0x216b44u, 0x260344u
    };
    static const uint32_t us_offsets[] = {
        0x0a7c74u, 0x0f1474u, 0x13ac74u, 0x184474u,
        0x1cdc74u, 0x217474u, 0x260c74u
    };
    static const uint8_t jp_caller[] = {
        0x20u, 0xabu, 0x43u, 0x9cu, 0x1fu, 0x27u, 0x9cu, 0x1du,
        0x27u, 0xa9u, 0xdfu, 0x8du, 0x1eu, 0x27u, 0xa9u, 0x01u,
        0x20u, 0x05u, 0x45u, 0x20u, 0xdfu, 0xc1u
    };
    static const uint8_t us_caller[] = {
        0x20u, 0xaeu, 0x43u, 0x9cu, 0x1fu, 0x27u, 0x9cu, 0x1du,
        0x27u, 0xa9u, 0xdfu, 0x8du, 0x1eu, 0x27u, 0xa9u, 0x01u,
        0x20u, 0x0bu, 0x45u, 0x20u, 0xdfu, 0xc1u
    };
    const uint32_t *offsets;
    const uint8_t *expected;
    const size_t candidate_count = THERON_V1_HUC6280_CALLEE_CANDIDATE_COUNT;
    FILE *file = fopen(path, "rb");
    uint8_t caller[sizeof(jp_caller)];

    assert(variant == THERON_TRACK02_VARIANT_US_BIN ||
           variant == THERON_TRACK02_VARIANT_JP_BIN);
    offsets = variant == THERON_TRACK02_VARIANT_JP_BIN ?
        jp_offsets : us_offsets;
    expected = variant == THERON_TRACK02_VARIANT_JP_BIN ?
        jp_caller : us_caller;
    assert(sizeof(jp_caller) == sizeof(us_caller));
    assert(file != NULL);
    for (size_t i = 0u; i < candidate_count; ++i) {
        uint8_t candidate[sizeof(caller)];
        uint8_t expected_candidate[sizeof(caller)];
        memcpy(expected_candidate, expected, sizeof(expected_candidate));
        if (variant == THERON_TRACK02_VARIANT_JP_BIN && i == 6u) {
            /* The seventh JP copy calls $4508; the preceding six call $4505. */
            expected_candidate[17] = 0x08u;
        }
        assert(fseek(file, (long)(offsets[i] + 26u), SEEK_SET) == 0);
        assert(fread(candidate, 1u, sizeof(candidate), file) ==
               sizeof(candidate));
        assert(memcmp(candidate, expected_candidate, sizeof(candidate)) == 0);
    }
    assert(fclose(file) == 0);
}

/* Source: docs/source-lock/theron-disassembly/
 * theron-jp-runtime-input-poll-bank-alias-20261009.md:388-457, HuC6280
 * helper $D588. JP indexes at $D5A7; US indexes at $D5B5. This verifies
 * authentic raw-BIN tables only; it does not bind descriptors or commands. */
static void verify_pointer_classifier_threshold_candidates(
    const char *path, Theron_Track02Variant variant) {
    static const uint32_t jp_offsets[] = {
        0x09d1e7u, 0x0e69e7u, 0x1301e7u, 0x1799e7u,
        0x1c31e7u, 0x20c9e7u, 0x2561e7u
    };
    static const uint32_t us_offsets[] = {
        0x09db33u, 0x0e7333u, 0x130b33u, 0x17a333u,
        0x1c3b33u, 0x20d333u, 0x256b33u
    };
    static const unsigned char jp_table[] = {
        0x0cu, 0xffu, 0x0bu, 0xa2u, 0x8au, 0x86u, 0x89u, 0x71u,
        0x88u, 0x5eu, 0x87u, 0x4bu, 0x05u, 0x40u, 0x12u, 0xffu,
        0x11u, 0x88u, 0x10u, 0x76u, 0x0fu, 0x62u, 0x0eu, 0x4fu,
        0x0du, 0x3du
    };
    static const unsigned char us_table[] = {
        0x12u, 0xffu, 0x11u, 0x88u, 0x10u, 0x76u, 0x0fu, 0x62u,
        0x0eu, 0x4fu, 0x0du, 0x3du, 0xadu, 0xbbu, 0x2eu, 0x4au,
        0xadu, 0x3bu, 0x2eu, 0x6au, 0xc9u, 0x6fu, 0x90u, 0x28u,
        0xc9u, 0xd0u
    };
    const uint32_t *expected_offsets;
    const unsigned char *table;
    const size_t table_size = sizeof(jp_table);
    const size_t expected_count = THERON_V1_HUC6280_CALLEE_CANDIDATE_COUNT;
    FILE *file = fopen(path, "rb");
    unsigned char *bytes;
    long file_size;
    size_t matches = 0u;

    assert(variant == THERON_TRACK02_VARIANT_US_BIN ||
           variant == THERON_TRACK02_VARIANT_JP_BIN);
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        expected_offsets = jp_offsets;
        table = jp_table;
    } else {
        expected_offsets = us_offsets;
        table = us_table;
    }
    assert(sizeof(us_table) == table_size);
    assert(file != NULL);
    assert(fseek(file, 0L, SEEK_END) == 0);
    file_size = ftell(file);
    assert(file_size > 0L);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    bytes = (unsigned char *)malloc((size_t)file_size);
    assert(bytes != NULL);
    assert(fread(bytes, 1u, (size_t)file_size, file) == (size_t)file_size);
    assert(fclose(file) == 0);

    for (size_t offset = 0u;
         offset + table_size <= (size_t)file_size; ++offset) {
        if (memcmp(bytes + offset, table, table_size) == 0) {
            assert(matches < expected_count);
            assert((uint32_t)offset == expected_offsets[matches]);
            ++matches;
        }
    }
    free(bytes);
    assert(matches == expected_count);
}

static void verify(const char *env_name, const char *name,
                   Theron_Track02Variant variant,
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
            assert((uint32_t)(receipt.vce_palette_consumer_address -
                              receipt.vce_palette_caller_address) ==
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
    if (variant == THERON_TRACK02_VARIANT_US_BIN ||
        variant == THERON_TRACK02_VARIANT_JP_BIN) {
        static const uint32_t us_offsets[
            THERON_V1_HUC6280_CALLEE_CANDIDATE_COUNT] = {
            0x9587eu, 0xdf07eu, 0x12887eu, 0x17207eu,
            0x1bb87eu, 0x20507eu, 0x24e87eu
        };
        static const uint32_t jp_offsets[
            THERON_V1_HUC6280_CALLEE_CANDIDATE_COUNT] = {
            0x94f4bu, 0xde74bu, 0x127f4bu, 0x17174bu,
            0x1baf4bu, 0x20474bu, 0x24df4bu
        };
        const uint32_t *expected_offsets =
            variant == THERON_TRACK02_VARIANT_US_BIN ? us_offsets : jp_offsets;
        assert(receipt.initializer_callee_candidates_verified);
        assert(receipt.initializer_callee_logical_address ==
               (variant == THERON_TRACK02_VARIANT_US_BIN ? 0x43aeu : 0x43abu));
        assert(receipt.initializer_callee_bytes == 10u);
        assert(receipt.initializer_callee_candidate_count ==
               THERON_V1_HUC6280_CALLEE_CANDIDATE_COUNT);
        assert(receipt.initializer_callee_fnv1a == 0x7bacabfau);
        assert(receipt.initializer_adjacent_helpers_verified);
        assert(receipt.initializer_adjacent_helpers_bytes == 23u);
        assert(receipt.initializer_adjacent_helpers_fnv1a == 0x6bfdc224u);
        assert(memcmp(receipt.initializer_callee_candidate_file_offsets,
                      expected_offsets, sizeof(us_offsets)) == 0);
    } else {
        assert(!receipt.initializer_callee_candidates_verified);
        assert(!receipt.initializer_adjacent_helpers_verified);
    }
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
    if (variant == THERON_TRACK02_VARIANT_US_BIN ||
        variant == THERON_TRACK02_VARIANT_JP_BIN) {
        verify_buffer_setup_candidates(path, variant);
        verify_buffer_setup_caller_candidates(path, variant);
        verify_pointer_classifier_threshold_candidates(path, variant);
        verify_stage2_vce_palette(path, variant, receipt.source_md5);
        if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
            verify_jp_c1df_runtime_candidates(path);
        }
    }
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
