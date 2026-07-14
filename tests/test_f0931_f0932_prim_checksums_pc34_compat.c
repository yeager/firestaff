#include <stdint.h>
#include <stdio.h>

#include "f0931_f0932_prim_checksums_pc34_compat.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    static const int16_t words[] = { 1, -1, 0x1000, 0x7000 };
    static const uint8_t bytes[] = { 0xffu, 0x02u, 0x10u, 0xffu };

    CHECK(f0931_checksum_words_pc34_compat(words, sizeof(words)) == 0x8000u);
    CHECK(f0931_checksum_words_pc34_compat(words, 5u) == 0x1000u);
    CHECK(f0931_checksum_words_pc34_compat(NULL, sizeof(words)) == 0u);
    CHECK(f0932_checksum_bytes_pc34_compat(bytes, sizeof(bytes)) == 0x0210u);
    CHECK(f0932_checksum_bytes_pc34_compat(NULL, sizeof(bytes)) == 0u);
    return failures != 0;
}
