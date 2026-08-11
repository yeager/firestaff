#include "csb_hint_oracle_dat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    /* count=3; two matching size tables; segments 3, 2 and 1 bytes. */
    const uint8_t valid[] = {
        0, 3, 0, 3, 0, 2, 0, 1, 0, 3, 0, 2, 0, 1,
        'a', 'b', 'c', 'd', 'e', 'f'
    };
    uint8_t mismatch[sizeof(valid)];
    uint8_t trailing[sizeof(valid) + 1u];
    CSB_HintOracleDAT archive;
    const uint8_t *segment = NULL;
    size_t size = 0u;
    const uint8_t solid_img[] = {0, 2, 0, 1, 0x05, 0x06};
    const uint8_t literal_img[] = {0, 2, 0, 1, 0x90, 1, 0xab};
    uint8_t pixels[4];
    uint16_t width = 0u, height = 0u;

    check(csb_hint_oracle_dat_parse(valid, sizeof(valid), &archive) ==
              CSB_HINT_ORACLE_DAT_OK,
          "valid duplicated-table archive parses");
    check(archive.segment_count == 3u, "segment count is preserved");
    check(archive.payload_offset == 14u, "payload starts at 2 + count * 4");
    check(csb_hint_oracle_dat_get_segment(&archive, 1u, &segment, &size) ==
              CSB_HINT_ORACLE_DAT_OK && size == 2u &&
              memcmp(segment, "de", 2u) == 0,
          "middle segment has exact boundary");
    check(csb_hint_oracle_dat_get_segment(&archive, 3u, &segment, &size) ==
              CSB_HINT_ORACLE_DAT_ERR_ARGUMENT,
          "out-of-range segment is rejected");

    memcpy(mismatch, valid, sizeof(valid));
    mismatch[10] = 0;
    mismatch[11] = 4;
    check(csb_hint_oracle_dat_parse(mismatch, sizeof(mismatch), &archive) ==
              CSB_HINT_ORACLE_DAT_ERR_BAD_TABLE,
          "mismatched duplicate size table is rejected");
    memcpy(trailing, valid, sizeof(valid));
    trailing[sizeof(valid)] = 0;
    check(csb_hint_oracle_dat_parse(trailing, sizeof(trailing), &archive) ==
              CSB_HINT_ORACLE_DAT_ERR_BAD_SIZE,
          "trailing payload byte is rejected");
    check(csb_hint_oracle_dat_parse(valid, 13u, &archive) ==
              CSB_HINT_ORACLE_DAT_ERR_TRUNCATED,
          "truncated header is rejected");
    check(csb_hint_oracle_dat_img2_decode(solid_img, sizeof(solid_img),
              &width, &height, pixels, sizeof(pixels), NULL) &&
              width == 2u && height == 1u && pixels[0] == 5u &&
              pixels[1] == 6u,
          "short IMG2 commands decode indexed pixels");
    check(csb_hint_oracle_dat_img2_decode(literal_img, sizeof(literal_img),
              &width, &height, pixels, sizeof(pixels), NULL) &&
              pixels[0] == 10u && pixels[1] == 11u,
          "literal IMG2 nibble pair decodes");
    check(!csb_hint_oracle_dat_img2_decode(literal_img, 6u,
              &width, &height, pixels, sizeof(pixels), NULL),
          "truncated IMG2 literal is rejected");

    return failures ? 1 : 0;
}
