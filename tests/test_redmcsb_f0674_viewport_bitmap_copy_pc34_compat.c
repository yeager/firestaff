#include "redmcsb_f0674_viewport_bitmap_copy_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct bitmap_fixture {
    int lookup_count;
    int byte_count_count;
    int16_t graphic_index;
    uint8_t bitmap[5];
} bitmap_fixture;

static const uint8_t *get_bitmap(void *context, int16_t graphic_index)
{
    bitmap_fixture *fixture = (bitmap_fixture *)context;

    ++fixture->lookup_count;
    fixture->graphic_index = graphic_index;
    return graphic_index == 77 ? fixture->bitmap : NULL;
}

static size_t get_bitmap_byte_count(void *context, const uint8_t *bitmap)
{
    bitmap_fixture *fixture = (bitmap_fixture *)context;

    ++fixture->byte_count_count;
    return bitmap == fixture->bitmap ? sizeof(fixture->bitmap) : 0U;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    (void)fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    bitmap_fixture fixture;
    redmcsb_f0674_viewport_bitmap_runtime_pc34_compat runtime;
    uint8_t destination[8];
    size_t copied_byte_count = 99U;

    (void)memset(&fixture, 0, sizeof(fixture));
    fixture.bitmap[0] = 0x12;
    fixture.bitmap[1] = 0x34;
    fixture.bitmap[2] = 0x56;
    fixture.bitmap[3] = 0x78;
    fixture.bitmap[4] = 0x9A;
    runtime.get_bitmap = get_bitmap;
    runtime.get_bitmap_byte_count = get_bitmap_byte_count;
    runtime.context = &fixture;

    (void)memset(destination, 0xCC, sizeof(destination));
    if (!expect(redmcsb_f0674_copy_viewport_bitmap_pc34_compat(
                    77, destination, sizeof(destination), &runtime,
                    &copied_byte_count),
                "F0674 copies a cached graphic") ||
        !expect(fixture.lookup_count == 1 && fixture.byte_count_count == 1 &&
                    fixture.graphic_index == 77,
                "F0674 performs the source lookup then byte-count route") ||
        !expect(copied_byte_count == 5U &&
                    memcmp(destination, fixture.bitmap, 5U) == 0 &&
                    destination[5] == 0xCC,
                "F0674 copies exactly F0653's caller-owned byte count")) {
        return 1;
    }

    (void)memset(destination, 0xCC, sizeof(destination));
    copied_byte_count = 99U;
    if (!expect(!redmcsb_f0674_copy_viewport_bitmap_pc34_compat(
                     76, destination, sizeof(destination), &runtime,
                     &copied_byte_count),
                "missing caller-owned graphics do not synthesize a bitmap") ||
        !expect(copied_byte_count == 0U && destination[0] == 0xCC,
                "missing graphics leave the destination untouched")) {
        return 1;
    }

    (void)memset(destination, 0xCC, sizeof(destination));
    copied_byte_count = 99U;
    if (!expect(!redmcsb_f0674_copy_viewport_bitmap_pc34_compat(
                     77, destination, 4U, &runtime, &copied_byte_count),
                "bounded adapter rejects a destination smaller than F0653") ||
        !expect(copied_byte_count == 0U && destination[0] == 0xCC,
                "short caller-owned destination is not partially copied")) {
        return 1;
    }
    return 0;
}
