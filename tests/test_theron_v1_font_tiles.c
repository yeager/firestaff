#include "theron_v1_track02.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return 1; \
    } \
} while (0)

static uint8_t *load_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    *out_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = malloc(*out_size);
    if (data) fread(data, 1, *out_size, f);
    fclose(f);
    return data;
}

static const char *us_md5 = "f23601102138f87c33025877767ebf76";

static int test_font_tile_extraction(void) {
    const char *path = getenv("THERON_TRACK02_US_BIN");
    if (!path) {
        printf("SKIP: THERON_TRACK02_US_BIN not set\n");
        return 0;
    }
    size_t size;
    uint8_t *data = load_file(path, &size);
    CHECK(data != NULL);

    Theron_Track02FontTileReceipt receipt;
    Theron_Track02SignalStatus status =
        theron_v1_track02_extract_font_tiles(data, size, us_md5, &receipt);

    CHECK(status == THERON_TRACK02_SIGNAL_OK);
    CHECK(receipt.valid);
    CHECK(receipt.tile_count == 96u);
    CHECK(receipt.nonblank_tile_count > 50u);
    CHECK(receipt.user_data_offset == 0x263200u);
    CHECK(receipt.checksum != 0u);

    printf("font: %zu tiles, %zu nonblank, checksum=0x%08x\n",
           receipt.tile_count, receipt.nonblank_tile_count, receipt.checksum);

    free(data);
    return 0;
}

static int test_font_space_tile_is_blank(void) {
    const char *path = getenv("THERON_TRACK02_US_BIN");
    if (!path) { printf("SKIP\n"); return 0; }
    size_t size;
    uint8_t *data = load_file(path, &size);
    CHECK(data != NULL);

    Theron_Track02FontTileReceipt receipt;
    theron_v1_track02_extract_font_tiles(data, size, us_md5, &receipt);

    int space_idx = ' ' - THERON_TRACK02_FONT_FIRST_CHAR;
    CHECK(space_idx >= 0 && space_idx < 96);
    int all_zero = 1;
    for (int i = 0; i < 64; i++) {
        if (receipt.pixels[space_idx][i] != 0) {
            all_zero = 0;
            break;
        }
    }
    CHECK(all_zero);
    printf("space tile (index %d) is blank: OK\n", space_idx);

    free(data);
    return 0;
}

static int test_font_letter_a_has_content(void) {
    const char *path = getenv("THERON_TRACK02_US_BIN");
    if (!path) { printf("SKIP\n"); return 0; }
    size_t size;
    uint8_t *data = load_file(path, &size);
    CHECK(data != NULL);

    Theron_Track02FontTileReceipt receipt;
    theron_v1_track02_extract_font_tiles(data, size, us_md5, &receipt);

    int a_idx = 'A' - THERON_TRACK02_FONT_FIRST_CHAR;
    CHECK(a_idx >= 0 && a_idx < 96);
    int nonzero = 0;
    for (int i = 0; i < 64; i++) {
        if (receipt.pixels[a_idx][i] != 0) nonzero++;
    }
    CHECK(nonzero > 8);
    printf("letter A (index %d) has %d nonzero pixels: OK\n", a_idx, nonzero);

    free(data);
    return 0;
}

static int test_font_char_mapping(void) {
    CHECK('A' - THERON_TRACK02_FONT_FIRST_CHAR == 0x31);
    CHECK('0' - THERON_TRACK02_FONT_FIRST_CHAR == 0x20);
    CHECK(' ' - THERON_TRACK02_FONT_FIRST_CHAR == 0x10);
    CHECK('Z' - THERON_TRACK02_FONT_FIRST_CHAR == 0x4A);
    printf("char mapping OK\n");
    return 0;
}

int main(int argc, char **argv) {
    const char *test_name = (argc > 1) ? argv[1] : "all";

    if (strcmp(test_name, "font_tile_extraction") == 0 ||
        strcmp(test_name, "all") == 0) {
        if (test_font_tile_extraction()) return 1;
    }
    if (strcmp(test_name, "font_space_tile_is_blank") == 0 ||
        strcmp(test_name, "all") == 0) {
        if (test_font_space_tile_is_blank()) return 1;
    }
    if (strcmp(test_name, "font_letter_a_has_content") == 0 ||
        strcmp(test_name, "all") == 0) {
        if (test_font_letter_a_has_content()) return 1;
    }
    if (strcmp(test_name, "font_char_mapping") == 0 ||
        strcmp(test_name, "all") == 0) {
        if (test_font_char_mapping()) return 1;
    }

    printf("PASS\n");
    return 0;
}
