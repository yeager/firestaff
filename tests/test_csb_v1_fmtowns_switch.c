#include "csb_v1_fmtowns_switch.h"

#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char **argv)
{
    const char *path = argc == 2 ? argv[1] : getenv("FIRESTAFF_CSB_FMTOWNS_SWITCH");
    CSB_V1_FmtownsSwitchReceipt receipt;
    CSB_V1_FmtownsItemDecodeReceipt page;
    uint8_t pixels[CSB_FMTOWNS_SWITCH_PIXELS];
    uint8_t *bytes;
    size_t byte_count;
    size_t index;

    if (!path) {
        puts("SKIP: set FIRESTAFF_CSB_FMTOWNS_SWITCH to original SWITCHTW.EXP");
        return 77;
    }
    bytes = read_file(path, &byte_count);
    if (!bytes) {
        puts("SKIP: original SWITCHTW.EXP unavailable");
        return 77;
    }
    CHECK(csb_v1_fmtowns_switch_parse(bytes, byte_count, &receipt),
          "recognises the complete F31E/F31J resource sequence");
    CHECK(receipt.valid && receipt.japanese_page.width == 320u &&
          receipt.english_page.height == 200u, "keeps both original pages");
    CHECK(receipt.japanese_page_byte_count == 7314u &&
          receipt.english_page_byte_count == 6541u,
          "keeps SWITCHDA.C stream boundaries");
    CHECK(receipt.japanese_page_offset < receipt.english_page_offset,
          "preserves executable resource ordering");
    CHECK(receipt.palette_offset < receipt.japanese_page_offset &&
          receipt.palette_byte_count == 68u && receipt.palette[8].red6 == 0x3fu &&
          receipt.palette[8].green6 == 0x3fu && receipt.palette[8].blue6 == 0u,
          "binds C26_SWITCH from the original executable");
    for (index = 0u; index < CSB_FMTOWNS_SWITCH_BUTTON_COUNT; ++index) {
        CHECK(receipt.buttons[index].image.valid &&
              receipt.buttons[index].width != 0u && receipt.buttons[index].height != 0u,
              "decodes a source-owned button");
    }
    CHECK(csb_v1_fmtowns_switch_decode_page(bytes, byte_count, &receipt,
                                            CSB_FMTOWNS_SWITCH_ENGLISH,
                                            pixels, sizeof(pixels), &page),
          "decodes the original English page");
    CHECK(page.pixel_fnv1a != 0u && page.pixel_fnv1a != receipt.japanese_page.pixel_fnv1a,
          "English page remains distinct from Japanese page");
    CHECK(!csb_v1_fmtowns_switch_parse(bytes, 100u, &receipt),
          "rejects truncated executable");
    free(bytes);
    printf("csb_v1_fmtowns_switch: %d/%d assertions passed\n", passed, passed + failed);
    return failed ? 1 : 0;
}
