#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_savegame_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int failures = 0;
static int assertions = 0;

#define CHECK(label, condition) do { \
    ++assertions; \
    if (!(condition)) { \
        ++failures; \
        fprintf(stderr, "FAIL %s\n", (label)); \
    } \
} while (0)

int main(void)
{
    const unsigned char plain[4] = { 0x34u, 0x12u, 0x78u, 0x56u };
    unsigned char encoded[6] = {0};
    unsigned char recovered[4] = {0};
    unsigned char malformed[5] = { 3u, 0u, 0u, 0u, 0u };
    uint16_t checksum = 0u;
    uint16_t read_checksum;
    uint16_t actual_checksum = 0u;
    size_t cursor = 0u;
    size_t recovered_size = 0u;

    CHECK("F0420 writes length plus body",
          dm1_v1_original_save_pc34_write_part_f0420(
              encoded, sizeof(encoded), plain, sizeof(plain), 0x2a5cu,
              &checksum) == 6);
    CHECK("F0420 stores little-endian source length",
          encoded[0] == 4u && encoded[1] == 0u);
    CHECK("F0420 does not leave plaintext body", memcmp(encoded + 2, plain,
                                                          sizeof(plain)) != 0);
    memcpy(recovered, encoded + 2, sizeof(recovered));
    read_checksum = F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        recovered, sizeof(recovered) / 2u, 0x2a5cu);
    CHECK("F0420 read contract restores source bytes",
          memcmp(recovered, plain, sizeof(plain)) == 0);
    CHECK("F0420 read/write checksum agrees", read_checksum == checksum);
    CHECK("F0418 validates stored obfuscated body without mutation",
          F0418_SAVEUTIL_GetChecksumPC34_Compat(
              encoded + 2u, sizeof(plain) / 2u, 0x2a5cu) == checksum);
    cursor = 0u;
    memset(recovered, 0, sizeof(recovered));
    CHECK("F0419 reads F0420 source part",
          dm1_v1_original_save_pc34_read_part_f0419(
              encoded, sizeof(encoded), &cursor, 0x2a5cu, checksum,
              recovered, sizeof(recovered), &recovered_size,
              &actual_checksum) == SAVEGAME_PC34_OK);
    CHECK("F0419 consumes complete valid part", cursor == sizeof(encoded));
    CHECK("F0419 returns decoded source bytes",
          recovered_size == sizeof(plain) &&
          memcmp(recovered, plain, sizeof(plain)) == 0);
    CHECK("F0419 reports source checksum", actual_checksum == checksum);
    cursor = 0u;
    memset(recovered, 0, sizeof(recovered));
    recovered_size = 0u;
    actual_checksum = 0u;
    CHECK("F0419 rejects checksum mismatch after decode",
          dm1_v1_original_save_pc34_read_part_f0419(
              encoded, sizeof(encoded), &cursor, 0x2a5cu,
              (uint16_t)(checksum ^ 1u), recovered, sizeof(recovered),
              &recovered_size, &actual_checksum) ==
              SAVEGAME_PC34_ERROR_BAD_CHECKSUM);
    CHECK("F0419 mismatch remains diagnostic-only",
          cursor == sizeof(encoded) && recovered_size == sizeof(plain) &&
          actual_checksum == checksum &&
          memcmp(recovered, plain, sizeof(plain)) == 0);
    cursor = 0u;
    recovered_size = 99u;
    CHECK("F0419 rejects odd source span",
          dm1_v1_original_save_pc34_read_part_f0419(
              malformed, sizeof(malformed), &cursor, 1u, 0u, recovered,
              sizeof(recovered), &recovered_size, NULL) ==
              SAVEGAME_PC34_ERROR_BAD_SIZE);
    CHECK("F0419 only consumes malformed length prefix",
          cursor == 2u && recovered_size == 0u);
    cursor = SIZE_MAX - 1u;
    recovered_size = 99u;
    CHECK("F0419 rejects overflowing cursor before prefix access",
          dm1_v1_original_save_pc34_read_part_f0419(
              encoded, sizeof(encoded), &cursor, 1u, 0u, recovered,
              sizeof(recovered), &recovered_size, NULL) ==
              SAVEGAME_PC34_ERROR_BAD_SIZE);
    CHECK("F0419 overflow rejection leaves cursor and output untouched",
          cursor == SIZE_MAX - 1u && recovered_size == 0u);
    malformed[0] = 4u;
    cursor = 0u;
    CHECK("F0419 rejects truncated source span",
          dm1_v1_original_save_pc34_read_part_f0419(
              malformed, sizeof(malformed), &cursor, 1u, 0u, recovered,
              sizeof(recovered), &recovered_size, NULL) ==
              SAVEGAME_PC34_ERROR_BAD_SIZE);
    CHECK("F0420 rejects odd byte counts",
          dm1_v1_original_save_pc34_write_part_f0420(
              encoded, sizeof(encoded), plain, 3u, 1u, &checksum) < 0);
    CHECK("F0420 rejects truncated destination",
          dm1_v1_original_save_pc34_write_part_f0420(
              encoded, 5u, plain, sizeof(plain), 1u, &checksum) < 0);

    printf("test_dm1_v1_f0420_save_part_pc34_compat: %d/%d assertions passed\n",
           assertions - failures, assertions);
    return failures == 0 ? 0 : 1;
}
