#include "firestaff_amiga_mfm.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define REV_BITS 9000u
#define REV_BYTES ((REV_BITS + 7u) / 8u)
#define SECTOR_BITS (1088u * 8u)

typedef struct Capture {
    unsigned count;
    FirestaffAmigaMfmSector last;
} Capture;

static void put_bit(uint8_t *bits, size_t bit_count, size_t position,
                    unsigned value)
{
    position %= bit_count;
    if (value != 0u)
        bits[position >> 3] |= (uint8_t)(1u << (7u - (position & 7u)));
}

static void put_u32(uint8_t *bits, size_t bit_count, size_t position,
                    uint32_t value)
{
    unsigned i;
    for (i = 0; i < 32u; ++i)
        put_bit(bits, bit_count, position + i,
                (value >> (31u - i)) & 1u);
}

static uint32_t encode_lane(uint32_t data, unsigned *previous_data)
{
    uint32_t encoded = 0;
    int bit;
    for (bit = 30; bit >= 0; bit -= 2) {
        const unsigned current = (data >> bit) & 1u;
        const unsigned clock = (*previous_data == 0u && current == 0u);
        encoded = (encoded << 1) | clock;
        encoded = (encoded << 1) | current;
        *previous_data = current;
    }
    return encoded;
}

static void encode_long(uint8_t *bits, size_t bit_count, size_t position,
                        uint32_t value, unsigned *odd_previous,
                        unsigned *even_previous)
{
    const uint32_t odd = (value >> 1) & 0x55555555u;
    const uint32_t even = value & 0x55555555u;
    put_u32(bits, bit_count, position, encode_lane(odd, odd_previous));
    put_u32(bits, bit_count, position + 32u,
            encode_lane(even, even_previous));
}

/* Synthetic bytes here exercise only standard Amiga MFM encoding/decoding
 * round-trip behavior. They are not substituted game media or parity data. */
static void make_sector(uint8_t *bits, size_t bit_count, size_t start,
                        unsigned track, unsigned sector_no,
                        uint8_t expected_payload[512])
{
    uint32_t words[135];
    uint32_t header_xor = 0;
    uint32_t data_xor = 0;
    unsigned odd_previous = 0;
    unsigned even_previous = 0;
    unsigned i;
    size_t pos;

    memset(bits, 0, REV_BYTES);
    words[0] = 0xff000000u | (track << 16) | (sector_no << 8) | 11u;
    for (i = 1; i < 5u; ++i)
        words[i] = 0u;
    for (i = 0; i < 5u; ++i)
        header_xor ^= words[i];
    words[5] = header_xor;
    words[6] = 0u;
    for (i = 0; i < 128u; ++i) {
        const uint32_t value = 0x10203040u ^ (i * 0x01010101u);
        words[7u + i] = value;
        data_xor ^= value;
        expected_payload[i * 4u] = (uint8_t)(value >> 24);
        expected_payload[i * 4u + 1u] = (uint8_t)(value >> 16);
        expected_payload[i * 4u + 2u] = (uint8_t)(value >> 8);
        expected_payload[i * 4u + 3u] = (uint8_t)value;
    }
    words[6] = data_xor;

    put_u32(bits, bit_count, start, 0x44894489u);
    pos = start + 64u;
    for (i = 0; i < 135u; ++i) {
        /* MFM odd/even lanes are independently clock encoded. Resetting the
         * lane history at each sector keeps this fixture self contained. */
        if (i == 0u) {
            odd_previous = 0u;
            even_previous = 0u;
        }
        encode_long(bits, bit_count, pos, words[i],
                    &odd_previous, &even_previous);
        pos += 64u;
    }
}

static int capture_sector(const FirestaffAmigaMfmSector *sector, void *context)
{
    Capture *capture = (Capture *)context;
    ++capture->count;
    capture->last = *sector;
    return 0;
}

int main(void)
{
    uint8_t raw[REV_BYTES];
    uint8_t expected[512];
    Capture capture;
    int count;

    memset(&capture, 0, sizeof(capture));
    make_sector(raw, REV_BITS, 8500u, 3u, 7u, expected);
    count = firestaff_amiga_mfm_scan_track(raw, sizeof(raw), REV_BITS, 3u,
                                           11u, capture_sector, &capture);
    assert(count == 1);
    assert(capture.count == 1u);
    assert(capture.last.track == 3u);
    assert(capture.last.cylinder == 1u);
    assert(capture.last.head == 1u);
    assert(capture.last.sector == 7u);
    assert(capture.last.sectors_to_gap == 11u);
    assert(memcmp(capture.last.payload, expected, sizeof(expected)) == 0);

    /* Checksum failure is rejected, even if the header itself is sound. */
    {
        const size_t corrupt_bit = (8500u + 64u + 7u * 64u + 1u) % REV_BITS;
        raw[corrupt_bit >> 3] ^= (uint8_t)(1u << (7u - (corrupt_bit & 7u)));
    }
    memset(&capture, 0, sizeof(capture));
    count = firestaff_amiga_mfm_scan_track(raw, sizeof(raw), REV_BITS, 3u,
                                           11u, capture_sector, &capture);
    assert(count == 0);

    assert(firestaff_amiga_mfm_scan_track(raw, sizeof(raw), REV_BITS, 160u,
                                          11u, capture_sector, &capture) == -1);
    puts("Amiga MFM scanner round-trip passed");
    return 0;
}
