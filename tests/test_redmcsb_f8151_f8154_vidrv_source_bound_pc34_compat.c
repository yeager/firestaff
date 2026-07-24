#include "redmcsb_f8151_f8154_vidrv_source_bound_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct StatusScript {
    const uint8_t *values;
    size_t count;
    size_t position;
} StatusScript;

static int failures;

static uint8_t read_status(void *context)
{
    StatusScript *script = (StatusScript *)context;
    const size_t index = script->position < script->count
        ? script->position : script->count - 1u;
    ++script->position;
    return script->values[index];
}

static void expect_true(const char *name, int value)
{
    if (!value) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

static void expect_byte(const char *name, uint8_t actual, uint8_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s got %u expected %u\n", name,
                (unsigned)actual, (unsigned)expected);
        ++failures;
    }
}

int main(void)
{
    static const uint8_t palette[48] = {
        0, 0, 0, 4, 4, 4, 8, 8, 8, 12, 12, 12, 16, 16, 16, 20, 20, 20,
        24, 24, 24, 28, 28, 28, 32, 32, 32, 36, 36, 36, 40, 40, 40,
        44, 44, 44, 48, 48, 48, 52, 52, 52, 56, 56, 56, 60, 60, 60};
    static const uint8_t source_pixels[] = {1, 0, 3, 4};
    static const uint8_t vblank[] = {8, 0, 0, 8};
    uint8_t target_pixels[8 * 6];
    RedmcsbF8151F8154VideoDriverPc34 driver;
    RedmcsbF8151F8154SourceSurfacePc34 source;
    StatusScript statuses = {vblank, sizeof(vblank), 0u};
    const int16_t fill_box[4] = {0, 1, 0, 1};

    memset(target_pixels, 15, sizeof(target_pixels));
    expect_true("bind real presentation target",
                redmcsb_f8151_f8154_vidrv_bind_pc34(
                    &driver, target_pixels, sizeof(target_pixels), 6, 8, 6,
                    0x2435a73du, palette, sizeof(palette), 1, 1, read_status,
                    &statuses, 8u));
    source.pixels = source_pixels;
    source.pixel_count = sizeof(source_pixels);
    source.width = 2;
    source.height = 2;
    source.frame_fingerprint = 0x9b4c51d1u;
    source.palette_fingerprint = driver.target.palette_fingerprint;
    source.source_frame_verified = 1;
    source.no_synthetic_fallback = 1;
    expect_true("F8151 source blit", redmcsb_f8151_vidrv_source_blit_pc34(
                    &driver, &source, 2, 2, 0));
    expect_byte("opaque source pixel", target_pixels[2u * 6u + 2u], 1);
    expect_byte("transparent source skips target", target_pixels[2u * 6u + 3u], 15);
    expect_byte("second source row", target_pixels[3u * 6u + 2u], 3);
    expect_true("F8152 fill uses F0135", redmcsb_f8152_vidrv_source_fill_box_pc34(
                    &driver, fill_box, 0x8005u));
    expect_byte("alternate fill first", target_pixels[0], 5);
    expect_byte("alternate fill skipped", target_pixels[1], 15);
    expect_true("F8154 invert source-bound target",
                redmcsb_f8154_vidrv_source_invert_box_pc34(&driver, 2, 2, 2, 3));
    expect_byte("inverted source pixel", target_pixels[2u * 6u + 2u], 5);
    expect_byte("inverted second source pixel", target_pixels[3u * 6u + 2u], 7);
    expect_true("F8153 vblank exact transition",
                redmcsb_f8153_vidrv_wait_vertical_blank_pc34(&driver));
    expect_true("F8153 consumed source order", statuses.position == 4u);
    source.no_synthetic_fallback = 0;
    expect_true("synthetic source rejected", !redmcsb_f8151_vidrv_source_blit_pc34(
                    &driver, &source, 0, 0, -1));
    expect_true("palette substitution rejected", !redmcsb_f0134_f0135_blit_indexed_pc34(
                    &driver.target, source_pixels, sizeof(source_pixels), 2, 2,
                    0xdeadbeefu, 0, 0, -1));
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8151-F8154 source-bound video driver");
    return 0;
}
