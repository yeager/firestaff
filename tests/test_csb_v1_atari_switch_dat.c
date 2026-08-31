#include "csb_v1_atari_switch_dat.h"
#include "dm1_v1_atari_st_stx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, text) do { \
    if (condition) ++passed; else { ++failed; printf("FAIL: %s\\n", text); } \
} while (0)

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

static int file_exists(const char *path)
{
    FILE *file;
    if (!path || !(file = fopen(path, "rb"))) return 0;
    fclose(file);
    return 1;
}

/* SWITCH.DAT lives on the original CSB utility STX.  Keep the verification
 * in memory: a loose SWITCH.DAT is accepted for preservation analysis, while
 * an STX is opened and its source member is copied only into this process.
 * No test or runtime path materializes game media on disk. */
static int path_has_stx_suffix(const char *path)
{
    size_t length;
    if (!path) return 0;
    length = strlen(path);
    return length >= 4u && path[length - 4u] == '.' &&
           (path[length - 3u] == 's' || path[length - 3u] == 'S') &&
           (path[length - 2u] == 't' || path[length - 2u] == 'T') &&
           (path[length - 1u] == 'x' || path[length - 1u] == 'X');
}

static uint8_t *read_switch_dat(const char *path, size_t *out_size)
{
    uint8_t *image;
    size_t image_size;
    DM1_V1_AtariStx stx;
    uint8_t *switch_dat;
    size_t switch_size = 0u;

    if (!path_has_stx_suffix(path)) return read_file(path, out_size);
    image = read_file(path, &image_size);
    if (!image || !dm1_v1_atari_st_stx_open(image, image_size, &stx)) {
        free(image);
        return NULL;
    }
    switch_dat = (uint8_t *)malloc(7405u);
    if (!switch_dat || !dm1_v1_atari_st_stx_extract_file(&stx, "SWITCH.DAT",
                                                           switch_dat, 7405u,
                                                           &switch_size) ||
        switch_size != 7405u) {
        free(switch_dat);
        free(image);
        return NULL;
    }
    free(image);
    *out_size = switch_size;
    return switch_dat;
}

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

static void test_graphic_expansion(void)
{
    static const uint8_t literal[] = {
        0u, 5u, 0u, 1u, 0x90u, 4u, 0x12u, 0x34u, 0xa0u, 10u
    };
    static const uint8_t copied_rows[] = {
        0u, 17u, 0u, 2u, 0x81u, 15u, 0x82u, 15u,
        0xb0u, 15u, 0x83u, 14u
    };
    uint8_t pixels[64];
    CSB_V1_AtariSwitchGraphicReceipt receipt;

    CHECK(csb_v1_atari_switch_graphic_decode_indexed(
              literal, sizeof(literal), pixels, sizeof(pixels), &receipt) &&
              receipt.valid && receipt.visible_width == 5u &&
              receipt.height == 1u && receipt.row_stride == 16u &&
              receipt.pixel_count == 16u && receipt.source_bytes_consumed == sizeof(literal) &&
              pixels[0] == 0u && pixels[1] == 1u && pixels[2] == 2u &&
              pixels[3] == 3u && pixels[4] == 4u && pixels[15] == 0u,
          "expands F0466 literal colors, extended fills, and source-owned row padding");
    CHECK(csb_v1_atari_switch_graphic_decode_indexed(
              copied_rows, sizeof(copied_rows), pixels, sizeof(pixels), &receipt) &&
              receipt.row_stride == 32u && pixels[0] == 1u && pixels[15] == 1u &&
              pixels[16] == 2u && pixels[31] == 2u && pixels[32] == 1u &&
              pixels[47] == 1u && pixels[48] == 0u && pixels[49] == 3u &&
              pixels[63] == 3u,
          "expands F0466 previous-row commands across padded Atari rows");
    CHECK(!csb_v1_atari_switch_graphic_decode_indexed(
               literal, sizeof(literal) - 1u, pixels, sizeof(pixels), &receipt),
          "rejects a truncated F0466 literal stream");
    CHECK(!csb_v1_atari_switch_graphic_decode_indexed(
               copied_rows, sizeof(copied_rows), pixels, 63u, &receipt),
          "rejects a destination that cannot hold original padded rows");
}

int main(int argc, char **argv)
{
    uint8_t bytes[256];
    size_t byte_count = make_switch_dat(bytes, sizeof(bytes));
    CSB_V1_AtariSwitchDatReceipt receipt;
    const char *real_path = argc == 2 ? argv[1] : getenv("FIRESTAFF_CSB_ATARI_SWITCH");

    if (argc > 2) {
        fputs("usage: test_csb_v1_atari_switch_dat [original-SWITCH.DAT]\n", stderr);
        return 2;
    }

    if (real_path) {
        uint8_t *real_bytes;
        size_t real_size;
        if (!file_exists(real_path)) {
            puts("SKIP: original CSB utility media is not available");
            return 77;
        }
        real_bytes = read_switch_dat(real_path, &real_size);
        CHECK(real_bytes != NULL,
              "reads original SWITCH.DAT directly from the requested media");
        if (real_bytes) {
            CHECK(real_size == 7405u,
                  "keeps the documented Atari ST 2.x SWITCH.DAT byte size");
            CHECK(csb_v1_atari_switch_dat_parse(real_bytes, real_size, &receipt),
                  "parses the checksummed original Atari ST switch data");
            CHECK(receipt.valid && receipt.header_segment_count > 0u &&
                  receipt.option_count > 0u && receipt.options[0].enabled,
                  "retains an original source-owned switch background");
            CHECK(receipt.has_palette,
                  "binds the optional source-owned switch palette when present");
            {
                size_t index;
                size_t decoded = 0u;
                for (index = 0u; index < receipt.option_count; ++index) {
                    const CSB_V1_AtariSwitchOption *option = &receipt.options[index];
                    CSB_V1_AtariSwitchGraphicReceipt graphic_receipt;
                    uint8_t *pixels;
                    size_t stride;
                    size_t pixel_count;
                    if (!option->enabled) continue;
                    stride = ((size_t)option->pixel_width + 15u) & ~(size_t)15u;
                    pixel_count = stride * (size_t)option->pixel_height;
                    pixels = (uint8_t *)malloc(pixel_count);
                    CHECK(pixels != NULL && csb_v1_atari_switch_graphic_decode_indexed(
                              real_bytes + option->graphic_offset,
                              option->graphic_byte_count, pixels, pixel_count,
                              &graphic_receipt) && graphic_receipt.valid &&
                              graphic_receipt.visible_width == option->pixel_width &&
                              graphic_receipt.height == option->pixel_height &&
                              graphic_receipt.row_stride == stride &&
                              graphic_receipt.source_bytes_consumed <= option->graphic_byte_count,
                          "expands one original Atari ST switch graphic without replacement pixels");
                    free(pixels);
                    ++decoded;
                }
                CHECK(decoded > 0u,
                      "verifies every enabled original switch graphic span");
            }
            free(real_bytes);
        }
        printf("csb_v1_atari_switch_dat real: %d/%d assertions passed\n",
               passed, passed + failed);
        return failed ? 1 : 0;
    }

    CHECK(byte_count != 0u, "builds a bounded FTL fixture");
    test_graphic_expansion();
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
