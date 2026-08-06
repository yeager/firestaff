#include "dm1_v1_legacy_graphics_dat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void put16(unsigned char *p, unsigned value, int be)
{
    if (be) { p[0] = (unsigned char)(value >> 8); p[1] = (unsigned char)value; }
    else { p[0] = (unsigned char)value; p[1] = (unsigned char)(value >> 8); }
}

static size_t make_fixture(unsigned char *data, size_t cap, int be)
{
    const size_t header = 2u + 575u * 4u;
    size_t offset = header;
    unsigned i;
    assert(cap >= header + 575u * 5u);
    memset(data, 0, cap);
    put16(data, 575u, be);
    for (i = 0u; i < 575u; ++i) {
        put16(data + 2u + i * 2u, 5u, be);
        put16(data + 2u + 575u * 2u + i * 2u, 5u, be);
    }
    for (i = 0u; i < 575u; ++i) {
        put16(data + offset, i == 42u ? 2u : 1u, be);
        put16(data + offset + 2u, 1u, be);
        data[offset + 4u] = (unsigned char)(i == 42u ? 0x1au : 0x01u);
        offset += 5u;
    }
    return offset;
}

static void check_one(int be)
{
    unsigned char data[2u + 575u * 9u];
    unsigned char pixels[4];
    uint16_t width, height;
    size_t size = make_fixture(data, sizeof(data), be);
    assert(dm1_v1_legacy_graphics_probe(data, size, be));
    assert(dm1_v1_legacy_graphics_query(data, size, be, 42u, &width, &height));
    assert(width == 2u && height == 1u);
    assert(dm1_v1_legacy_graphics_decode(data, size, be, 42u, pixels,
                                          sizeof(pixels), &width, &height));
    assert(width == 2u && height == 1u);
    assert(pixels[0] == 10u && pixels[1] == 10u);
}

int main(void)
{
    check_one(0);
    check_one(1);
    puts("PASS dm1_v1_legacy_graphics_dat");
    return 0;
}
