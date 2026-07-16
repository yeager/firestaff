#include "dm2_v1_memory_mement_helpers.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void test_zero_memory(void)
{
    uint8_t bytes[8] = {0x11u, 0x22u, 0x33u, 0x44u,
                        0x55u, 0x66u, 0x77u, 0x88u};
    DM2_V1_ZeroMemoryReceipt receipt;

    expect_true(dm2_v1_ZERO_MEMORY(bytes, sizeof(bytes), &receipt) == 1,
                "ZERO_MEMORY accepts caller-owned memory");
    expect_true(receipt.valid &&
                    receipt.source_locked &&
                    receipt.requested_bytes == sizeof(bytes) &&
                    receipt.zeroed_bytes == sizeof(bytes) &&
                    receipt.before_hash != receipt.after_hash &&
                    receipt.after_hash != 0u &&
                    strcmp(receipt.symbol, "ZERO_MEMORY") == 0,
                "ZERO_MEMORY receipt records zeroed byte range");
    expect_true(memcmp(bytes, "\0\0\0\0\0\0\0\0", sizeof(bytes)) == 0,
                "ZERO_MEMORY clears every requested byte");

    expect_true(dm2_v1_ZERO_MEMORY(0, 4u, &receipt) == 0,
                "ZERO_MEMORY blocks null nonzero buffer");
    expect_true(receipt.blocked && !receipt.valid,
                "ZERO_MEMORY null-buffer path is fail-closed");

    expect_true(dm2_v1_ZERO_MEMORY(0, 0u, &receipt) == 1 &&
                    receipt.valid &&
                    receipt.zeroed_bytes == 0u,
                "ZERO_MEMORY accepts empty source no-op");
}

static void test_validate_mements_accepts_source_table(void)
{
    DM2_V1_MementDescriptor entries[] = {
        {0u, 2u, DM2_V1_VALIDATE_MEMENTS_NULL_REF, 0u, 16u, 1u},
        {1u, DM2_V1_VALIDATE_MEMENTS_NULL_REF, 7u, 16u, 8u, 1u},
        {2u, 3u, 8u, 32u, 4u, 0u},
        {3u, 9u, 9u, 40u, 12u, 1u}
    };
    DM2_V1_ValidateMementsReceipt receipt;

    expect_true(dm2_v1_ValidateMements(entries, 4u, 64u, &receipt) == 1,
                "ValidateMements accepts bounded non-overlapping entries");
    expect_true(receipt.valid &&
                    receipt.source_locked &&
                    receipt.entry_count == 4u &&
                    receipt.active_count == 3u &&
                    receipt.inactive_count == 1u &&
                    receipt.used_bytes == 36u &&
                    receipt.table_hash != 0u &&
                    strcmp(receipt.symbol, "ValidateMements") == 0,
                "ValidateMements receipt records source table census");
}

static void test_validate_mements_rejects_bad_tables(void)
{
    DM2_V1_MementDescriptor bad_id[] = {
        {DM2_V1_VALIDATE_MEMENTS_NULL_REF, 1u, 2u, 0u, 4u, 1u}
    };
    DM2_V1_MementDescriptor bad_span[] = {
        {0u, 1u, 2u, 8u, 0u, 1u}
    };
    DM2_V1_MementDescriptor overlap[] = {
        {0u, 1u, 2u, 0u, 8u, 1u},
        {1u, 2u, 3u, 4u, 8u, 1u}
    };
    DM2_V1_ValidateMementsReceipt receipt;

    expect_true(dm2_v1_ValidateMements(0, 1u, 64u, &receipt) == 0 &&
                    receipt.blocked_null_entries,
                "ValidateMements blocks missing entry table");
    expect_true(dm2_v1_ValidateMements(bad_id, 1u, 64u, &receipt) == 0 &&
                    receipt.blocked_bad_id &&
                    receipt.first_invalid_index == 0u,
                "ValidateMements blocks active null mement id");
    expect_true(dm2_v1_ValidateMements(bad_span, 1u, 64u, &receipt) == 0 &&
                    receipt.blocked_bad_span,
                "ValidateMements blocks empty spans");
    expect_true(dm2_v1_ValidateMements(overlap, 2u, 64u, &receipt) == 0 &&
                    receipt.blocked_overlap &&
                    receipt.first_invalid_index == 1u,
                "ValidateMements blocks overlapping active spans");
}

int main(void)
{
    test_zero_memory();
    test_validate_mements_accepts_source_table();
    test_validate_mements_rejects_bad_tables();
    expect_true(strstr(dm2_v1_memory_mement_helpers_source_evidence(),
                       "ZERO_MEMORY:2166") != 0,
                "source evidence includes ZERO_MEMORY");
    expect_true(strstr(dm2_v1_memory_mement_helpers_source_evidence(),
                       "ValidateMements:3908") != 0,
                "source evidence includes ValidateMements");
    if (failures) {
        return 1;
    }
    puts("DM2 memory/mement helpers: ok");
    return 0;
}
