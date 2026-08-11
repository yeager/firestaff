#include "csb_v1_atari_switch_dat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, text) do { \
    if (condition) ++passed; else { ++failed; printf("FAIL: %s\\n", text); } \
} while (0)

static void put_be16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value >> 8u);
    bytes[1] = (uint8_t)value;
}

static void put_be32(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)(value >> 24u);
    bytes[1] = (uint8_t)(value >> 16u);
    bytes[2] = (uint8_t)(value >> 8u);
    bytes[3] = (uint8_t)value;
}

static void put_segment(uint8_t *segment, uint16_t type, uint16_t id,
                        uint32_t offset, uint32_t size)
{
    put_be16(segment, type);
    put_be16(segment + 2u, id);
    put_be32(segment + 4u, offset);
    put_be32(segment + 8u, size);
}

static void set_checksum(uint8_t *bytes, size_t segment_count)
{
    uint32_t sum = 0u;
    size_t index;
    for (index = 4u; index < 20u; ++index) sum += (uint32_t)bytes[index] * index;
    for (index = 0u; index < segment_count * 12u; ++index)
        sum += (uint32_t)bytes[20u + index] * ((index & 0xffu) + 1u);
    put_be16(bytes + 2u, (uint16_t)sum);
}

static size_t make_switch_dat(uint8_t *bytes, size_t capacity)
{
    const size_t header_size = 20u;
    const size_t segments_size = 36u;
    const size_t data_offset = header_size + segments_size;
    const size_t switch_size = 2u + 2u * 52u;
    const size_t background_offset = data_offset + switch_size;
    const size_t button_offset = background_offset + 4u;
    const size_t palette_offset = button_offset + 4u;
    size_t total = palette_offset + 32u;
    uint8_t *option;
    if (capacity < total) return 0u;
    memset(bytes, 0, total);
    put_be16(bytes, 0x6160u);
    put_be16(bytes + 4u, 0x5000u);
    put_be16(bytes + 18u, 3u);
    put_segment(bytes + 20u, 0x30u, 0u, (uint32_t)data_offset,
                (uint32_t)switch_size);
    put_segment(bytes + 32u, 0x0001u, 0u, (uint32_t)background_offset, 4u);
    put_segment(bytes + 44u, 0x0002u, 7u, (uint32_t)button_offset, 4u);
    put_be16(bytes + data_offset, 2u);
    option = bytes + data_offset + 2u;
    put_be16(option, 1u);
    put_be16(option + 4u, 0u);
    put_be16(option + 6u, 0u);
    memcpy(option + 10u, "CSB", 4u);
    option += 52u;
    put_be16(option, 2u);
    put_be16(option + 2u, 7u);
    put_be16(option + 4u, 42u);
    put_be16(option + 6u, 84u);
    put_be16(option + 8u, 15u);
    memcpy(option + 10u, "GAME", 5u);
    put_be16(bytes + background_offset, 320u);
    put_be16(bytes + background_offset + 2u, 200u);
    put_be16(bytes + button_offset, 64u);
    put_be16(bytes + button_offset + 2u, 32u);
    set_checksum(bytes, 3u);
    return total;
}

int main(void)
{
    uint8_t bytes[256];
    size_t byte_count = make_switch_dat(bytes, sizeof(bytes));
    CSB_V1_AtariSwitchDatReceipt receipt;

    CHECK(byte_count != 0u, "builds a bounded FTL fixture");
    CHECK(csb_v1_atari_switch_dat_parse(bytes, byte_count, &receipt),
          "accepts a checksummed Atari ST SWITCH.DAT layout");
    CHECK(receipt.valid && receipt.header_segment_count == 3u &&
          receipt.option_count == 2u, "retains source header and option counts");
    CHECK(receipt.options[0].enabled && receipt.options[0].pixel_width == 320u &&
          receipt.options[0].pixel_height == 200u,
          "keeps the source-owned background segment span");
    CHECK(receipt.options[1].enabled && receipt.options[1].x == 42 &&
          receipt.options[1].y == 84 && receipt.options[1].transparent_color == 15 &&
          receipt.options[1].pixel_width == 64u && receipt.options[1].pixel_height == 32u &&
          strcmp(receipt.options[1].ftl_file_name, "GAME") == 0,
          "keeps the original button metadata without drawing replacement UI");
    bytes[2] ^= 1u;
    CHECK(!csb_v1_atari_switch_dat_parse(bytes, byte_count, &receipt),
          "rejects a bad FTL header checksum");
    bytes[2] ^= 1u;
    put_be16(bytes + 20u + 24u, 3u);
    set_checksum(bytes, 3u);
    CHECK(!csb_v1_atari_switch_dat_parse(bytes, byte_count, &receipt),
          "rejects a referenced graphic segment that no longer matches");
    printf("csb_v1_atari_switch_dat: %d/%d assertions passed\n", passed, passed + failed);
    return failed ? 1 : 0;
}
