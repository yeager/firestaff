#include "theron_v1_track02.h"
#include "theron_v1_dungeon_handoff.h"
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
    FILE *f;
    long file_size;
    uint8_t *data;
    if (!path || !out_size) return NULL;
    f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0 ||
        (file_size = ftell(f)) <= 0 ||
        fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    *out_size = (size_t)file_size;
    data = (uint8_t *)malloc(*out_size);
    if (!data || fread(data, 1u, *out_size, f) != *out_size) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    return data;
}

static const char *us_md5 = "f23601102138f87c33025877767ebf76";
static const char *jp_md5 = "b7afb338ad31be1025b53f9aff12d73a";

#define RAW_SECTOR_BYTES THERON_TRACK02_RAW_SECTOR_BYTES
#define RAW_USER_DATA_OFFSET THERON_TRACK02_RAW_USER_DATA_OFFSET
#define RAW_USER_DATA_BYTES THERON_TRACK02_RAW_USER_DATA_BYTES
#define JP_INDEX01_RAW_SECTOR THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR

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

static uint8_t *project_jp_index01_user_data(const uint8_t *raw,
                                             size_t raw_size,
                                             size_t *out_size) {
    size_t sectors;
    size_t projected_sectors;
    uint8_t *projected;
    if (!raw || !out_size || raw_size % RAW_SECTOR_BYTES != 0u) return NULL;
    sectors = raw_size / RAW_SECTOR_BYTES;
    if (sectors <= JP_INDEX01_RAW_SECTOR) return NULL;
    projected_sectors = sectors - JP_INDEX01_RAW_SECTOR;
    if (projected_sectors > SIZE_MAX / RAW_USER_DATA_BYTES) return NULL;
    *out_size = projected_sectors * RAW_USER_DATA_BYTES;
    projected = (uint8_t *)malloc(*out_size);
    if (!projected) return NULL;
    for (size_t sector = 0u; sector < projected_sectors; ++sector) {
        const size_t raw_offset =
            (JP_INDEX01_RAW_SECTOR + sector) * RAW_SECTOR_BYTES +
            RAW_USER_DATA_OFFSET;
        memcpy(projected + sector * RAW_USER_DATA_BYTES,
               raw + raw_offset, RAW_USER_DATA_BYTES);
    }
    return projected;
}

static int test_jp_cue_iso_font_matches_authentic_bin(void) {
    char path[1024];
    size_t raw_size = 0u;
    size_t iso_size = 0u;
    uint8_t *raw;
    uint8_t *iso;
    Theron_Track02FontTileReceipt raw_receipt;
    Theron_Track02FontTileReceipt iso_receipt;
    if (!resolve_track02_path("THERON_TRACK02_JP_BIN", "TQJP02.bin",
                              path, sizeof(path))) {
        puts("SKIP: JP font source path unavailable");
        return 0;
    }
    raw = load_file(path, &raw_size);
    if (!raw) {
        puts("SKIP: authentic JP Track 02 BIN unavailable");
        return 0;
    }
    iso = project_jp_index01_user_data(raw, raw_size, &iso_size);
    CHECK(iso != NULL);
    CHECK(iso_size == 6596608u);
    CHECK(theron_v1_track02_raw_bytes_match_md5(
        iso, iso_size, THERON_TRACK02_MD5_JP_ISO));
    CHECK(theron_v1_track02_extract_font_tiles(
              raw, raw_size, jp_md5, &raw_receipt) ==
          THERON_TRACK02_SIGNAL_OK);
    CHECK(theron_v1_track02_extract_font_tiles(
              iso, iso_size, THERON_TRACK02_MD5_JP_ISO, &iso_receipt) ==
          THERON_TRACK02_SIGNAL_OK);
    CHECK(iso_receipt.variant == THERON_TRACK02_VARIANT_JP_REV1_ISO);
    CHECK(raw_receipt.user_data_offset == 0x262A00u);
    CHECK(iso_receipt.user_data_offset == 0x1F2A00u);
    CHECK(iso_receipt.tile_count == raw_receipt.tile_count);
    CHECK(iso_receipt.nonblank_tile_count == raw_receipt.nonblank_tile_count);
    CHECK(iso_receipt.checksum == raw_receipt.checksum);
    CHECK(memcmp(iso_receipt.pixels, raw_receipt.pixels,
                 sizeof(raw_receipt.pixels)) == 0);
    CHECK(theron_v1_track02_extract_font_tiles(
              iso, iso_size, THERON_TRACK02_MD5_JP_REV1_ISO, &iso_receipt) ==
          THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
    printf("JP CUE ISO font: 96 authentic tiles match JP BIN at normalized UD=0x1f2a00\n");
    free(iso);
    free(raw);
    return 0;
}

static int test_font_tile_extraction(void) {
    CHECK(verify_real_font("US", "THERON_TRACK02_US_BIN", "TQUS02.bin",
                           us_md5, 0x263200u) == 0);
    CHECK(verify_real_font("JP", "THERON_TRACK02_JP_BIN", "TQJP02.bin",
                           jp_md5, 0x262A00u) == 0);
    CHECK(test_jp_cue_iso_font_matches_authentic_bin() == 0);
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
