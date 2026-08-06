#include "dm1_v1_fmtowns_pic_library_loader.h"
#include "dm1_v1_fmtowns_font_asset.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Build a bounded fixture picture-library file with `asset_count`
 * assets whose sizes are the caller-supplied list. The last asset is
 * populated with a caller-owned pattern; other assets get filler
 * bytes derived from their index so byte-exact reads can be verified.
 *
 * Layout mirrors the byte-verified real DATA/GRAPHICS.DAT:
 *   +0                 u16 asset_count
 *   +2                 asset_count * u16 primary size table
 *   +2 + count*2       asset_count * u16 secondary (mirror) size table
 *   +2 + count*4       payload = concatenated asset spans */
static void write_fixture(const char *path, uint16_t asset_count,
                          const uint16_t *sizes,
                          const uint8_t *last_asset_bytes) {
    FILE *fp = fopen(path, "wb");
    assert(fp != NULL);
    uint8_t hdr[2];
    hdr[0] = (uint8_t)(asset_count & 0xff);
    hdr[1] = (uint8_t)((asset_count >> 8) & 0xff);
    fwrite(hdr, 1, 2, fp);
    /* Primary table. */
    for (uint16_t i = 0; i < asset_count; ++i) {
        uint8_t e[2] = { (uint8_t)(sizes[i] & 0xff),
                         (uint8_t)((sizes[i] >> 8) & 0xff) };
        fwrite(e, 1, 2, fp);
    }
    /* Secondary mirror. */
    for (uint16_t i = 0; i < asset_count; ++i) {
        uint8_t e[2] = { (uint8_t)(sizes[i] & 0xff),
                         (uint8_t)((sizes[i] >> 8) & 0xff) };
        fwrite(e, 1, 2, fp);
    }
    /* Payload: fill each asset with byte value (i+1) except the last
     * which gets the caller-supplied pattern. */
    for (uint16_t i = 0; i + 1 < asset_count; ++i) {
        uint8_t *buf = (uint8_t *)malloc(sizes[i]);
        assert(buf != NULL);
        memset(buf, (int)(i + 1), sizes[i]);
        fwrite(buf, 1, sizes[i], fp);
        free(buf);
    }
    fwrite(last_asset_bytes, 1, sizes[asset_count - 1], fp);
    fclose(fp);
}

static void test_load_and_release(void) {
    const char *dir = "/tmp/test_fmtowns_loader";
    (void)mkdir(dir, 0700);
    char path[512];
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", dir);
    uint16_t sizes[] = { 8, 16 };
    uint8_t pattern[16];
    for (int i = 0; i < 16; ++i) pattern[i] = (uint8_t)(0x40 + i);
    write_fixture(path, 2, sizes, pattern);

    dm1_v1_fmtowns_pic_library_handle_t h;
    dm1_v1_fmtowns_pic_library_load_status_t st =
        dm1_v1_fmtowns_pic_library_load_from_cache_pc34(dir, NULL, &h);
    assert(st == DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK);
    assert(h.bytes != NULL);
    assert(h.view.asset_count == 2);
    assert(h.size_bytes == 2 + 2 * 4 + 8 + 16);

    /* Read asset 0: should be all 0x01. */
    const uint8_t *b0 = NULL; uint16_t n0 = 0;
    dm1_v1_fmtowns_pic_library_status_t rs =
        dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&h.view, 0, &b0, &n0);
    assert(rs == DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(n0 == 8);
    for (size_t i = 0; i < 8; ++i) assert(b0[i] == 0x01);

    /* Read asset 1: should match pattern. */
    const uint8_t *b1 = NULL; uint16_t n1 = 0;
    rs = dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&h.view, 1, &b1, &n1);
    assert(rs == DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(n1 == 16);
    for (size_t i = 0; i < 16; ++i) assert(b1[i] == pattern[i]);

    dm1_v1_fmtowns_pic_library_release_pc34(&h);
    assert(h.bytes == NULL);
    assert(h.size_bytes == 0);

    unlink(path);
    rmdir(dir);
}

static void test_load_missing_file(void) {
    dm1_v1_fmtowns_pic_library_handle_t h;
    dm1_v1_fmtowns_pic_library_load_status_t st =
        dm1_v1_fmtowns_pic_library_load_from_cache_pc34(
            "/nonexistent/path/xyz", NULL, &h);
    assert(st == DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_OPEN);
    assert(h.bytes == NULL);
}

static void test_null_args(void) {
    dm1_v1_fmtowns_pic_library_handle_t h;
    assert(dm1_v1_fmtowns_pic_library_load_from_cache_pc34(NULL, NULL, &h)
           == DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_NULL);
    assert(dm1_v1_fmtowns_pic_library_load_from_cache_pc34("/tmp", NULL, NULL)
           == DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_NULL);
    dm1_v1_fmtowns_pic_library_release_pc34(NULL); /* must not crash */
}

static void test_font_size_mismatch(void) {
    /* Fixture with fewer than 558 assets returns _ERR_CONTAINER
     * (index out of range) via the underlying view API. */
    const char *dir = "/tmp/test_fmtowns_font_size";
    (void)mkdir(dir, 0700);
    char path[512];
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", dir);
    uint16_t sizes[] = { 4, 4 };
    uint8_t pattern[4] = { 0xa, 0xb, 0xc, 0xd };
    write_fixture(path, 2, sizes, pattern);

    dm1_v1_fmtowns_pic_library_handle_t h;
    assert(dm1_v1_fmtowns_pic_library_load_from_cache_pc34(dir, NULL, &h)
           == DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK);

    uint8_t font_buf[DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES];
    dm1_v1_fmtowns_pic_library_load_status_t st =
        dm1_v1_fmtowns_pic_library_load_menu_font_pc34(
            &h, font_buf, sizeof(font_buf));
    /* Asset 557 is out of range in this fixture. */
    assert(st == DM1_V1_FMTOWNS_PIC_LIB_LOAD_ERR_CONTAINER);

    dm1_v1_fmtowns_pic_library_release_pc34(&h);
    unlink(path);
    rmdir(dir);
}

static void test_load_menu_font_from_fixture(void) {
    /* Build a fixture with exactly 558 assets so index 557 exists and
     * carries a 768-byte payload identical to the byte-verified font
     * allocation size. */
    const char *dir = "/tmp/test_fmtowns_font_load";
    (void)mkdir(dir, 0700);
    char path[512];
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", dir);
    const uint16_t asset_count = 558;
    uint16_t *sizes = (uint16_t *)malloc(asset_count * sizeof(uint16_t));
    for (uint16_t i = 0; i + 1 < asset_count; ++i) sizes[i] = 1;
    sizes[asset_count - 1] =
        (uint16_t)DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES;
    uint8_t *font_pattern = (uint8_t *)malloc(
        DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES);
    for (size_t i = 0; i < DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES; ++i)
        font_pattern[i] = (uint8_t)((i * 7 + 3) & 0xff);
    write_fixture(path, asset_count, sizes, font_pattern);

    dm1_v1_fmtowns_pic_library_handle_t h;
    assert(dm1_v1_fmtowns_pic_library_load_from_cache_pc34(dir, NULL, &h)
           == DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK);

    uint8_t got[DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES];
    dm1_v1_fmtowns_pic_library_load_status_t st =
        dm1_v1_fmtowns_pic_library_load_menu_font_pc34(
            &h, got, sizeof(got));
    assert(st == DM1_V1_FMTOWNS_PIC_LIB_LOAD_OK);
    for (size_t i = 0; i < DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES; ++i)
        assert(got[i] == font_pattern[i]);

    dm1_v1_fmtowns_pic_library_release_pc34(&h);
    free(sizes);
    free(font_pattern);
    unlink(path);
    rmdir(dir);
}

int main(void) {
    test_load_and_release();
    test_load_missing_file();
    test_null_args();
    test_font_size_mismatch();
    test_load_menu_font_from_fixture();
    printf("All dm1_v1_fmtowns_pic_library_loader tests passed.\n");
    return 0;
}
