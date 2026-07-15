#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_savegame_pc34_compat.h"

#include <stdio.h>
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
    uint16_t checksum = 0u;
    uint16_t read_checksum;

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
