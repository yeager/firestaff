#include "redmcsb_f7059_dungeon_part_checksum_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        failures++; \
    } \
} while (0)

static void test_read_checksum(void)
{
    static const uint8_t bytes[] = {0xfe, 0x01, 0x80, 0x7f};
    uint16_t checksum = 0xfeff;

    redmcsb_f7059_read_dungeon_part_with_checksum_pc34(bytes,
                                                        (uint16_t)sizeof(bytes),
                                                        &checksum);
    CHECK(checksum == 0x00fd,
          "F7059 adds unsigned dungeon bytes with 16-bit wraparound");
}

static void test_write_checksum(void)
{
    static const uint8_t bytes[] = {0xff, 0xff, 0x02};
    uint16_t checksum = 0x7ffe;

    redmcsb_f7060_write_dungeon_part_with_checksum_pc34(bytes,
                                                         (uint16_t)sizeof(bytes),
                                                         &checksum);
    CHECK(checksum == 0x81fe,
          "F7060 uses the same source byte accumulation as F7059");
}

static void test_zero_length_part(void)
{
    uint16_t checksum = 0x4321;

    redmcsb_f7059_read_dungeon_part_with_checksum_pc34(NULL, 0, &checksum);
    CHECK(checksum == 0x4321, "F7059 leaves an empty PC34 dungeon part alone");
    redmcsb_f7060_write_dungeon_part_with_checksum_pc34(NULL, 0, &checksum);
    CHECK(checksum == 0x4321, "F7060 leaves an empty PC34 dungeon part alone");
}

int main(void)
{
    test_read_checksum();
    test_write_checksum();
    test_zero_length_part();
    CHECK(strcmp(redmcsb_f7059_dungeon_part_checksum_pc34_source_evidence(),
                 "ReDMCSB CEDTINC6.C F7059/F7060") == 0,
          "source evidence identifies the original routines");

    if (failures != 0) {
        return 1;
    }
    puts("PASSED: ReDMCSB F7059/F7060");
    return 0;
}
