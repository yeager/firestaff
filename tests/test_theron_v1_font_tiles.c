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
static const char *jp_md5 = "b7afb338ad31be1025b53f9aff12d73a";

static int resolve_track02_path(const char *env_name,
                                const char *leaf,
                                char *out,
                                size_t out_size) {
    const char *override = getenv(env_name);
    const char *home = getenv("HOME");

    if (override && override[0]) {
        return snprintf(out, out_size, "%s", override) < (int)out_size;
    }
    if (!home || !home[0]) return 0;
    return snprintf(out, out_size, "%s/.firestaff/data/theron/%s",
                    home, leaf) < (int)out_size;
}

static int verify_real_font(const char *label,
                            const char *env_name,
                            const char *leaf,
                            const char *md5,
                            size_t expected_user_data_offset) {
    char path[1024];
    size_t size = 0u;
    uint8_t *data;
    Theron_Track02FontTileReceipt receipt;
    Theron_Track02SignalStatus status;

    if (!resolve_track02_path(env_name, leaf, path, sizeof(path))) {
        printf("SKIP: %s path unavailable\n", label);
        return 0;
    }
    data = load_file(path, &size);
    if (!data) {
        printf("SKIP: %s media unavailable at %s\n", label, path);
        return 0;
    }
    status = theron_v1_track02_extract_font_tiles(
        data, size, md5, &receipt);
    CHECK(status == THERON_TRACK02_SIGNAL_OK);
    CHECK(receipt.valid);
    CHECK(receipt.tile_count == 96u);
    CHECK(receipt.nonblank_tile_count > 50u);
    CHECK(receipt.user_data_offset == expected_user_data_offset);
    CHECK(receipt.checksum != 0u);
    printf("%s font: %zu tiles, %zu nonblank, UD=0x%zx checksum=0x%08x\n",
           label, receipt.tile_count, receipt.nonblank_tile_count,
           receipt.user_data_offset, receipt.checksum);
    free(data);
    return 0;
}

static int test_font_tile_extraction(void) {
    CHECK(verify_real_font("US", "THERON_TRACK02_US_BIN", "TQUS02.bin",
                           us_md5, 0x263200u) == 0);
    CHECK(verify_real_font("JP", "THERON_TRACK02_JP_BIN", "TQJP02.bin",
                           jp_md5, 0x262A00u) == 0);
    return 0;
}

static int load_us_font_receipt(Theron_Track02FontTileReceipt *receipt,
                                uint8_t **out_data) {
    char path[1024];
    size_t size = 0u;
    uint8_t *data;
    if (!resolve_track02_path("THERON_TRACK02_US_BIN", "TQUS02.bin",
                              path, sizeof(path))) {
        printf("SKIP\n");
        return 0;
    }
    data = load_file(path, &size);
    if (!data) {
        printf("SKIP\n");
        return 0;
    }
    CHECK(theron_v1_track02_extract_font_tiles(
              data, size, us_md5, receipt) == THERON_TRACK02_SIGNAL_OK);
    *out_data = data;
    return 1;
}

static int test_font_space_tile_is_blank(void) {
    Theron_Track02FontTileReceipt receipt;
    uint8_t *data;
    if (!load_us_font_receipt(&receipt, &data)) return 0;

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
    Theron_Track02FontTileReceipt receipt;
    uint8_t *data;
    if (!load_us_font_receipt(&receipt, &data)) return 0;

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
