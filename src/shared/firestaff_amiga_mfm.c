#include "firestaff_amiga_mfm.h"

#include <limits.h>

#define AMIGA_MFM_SYNC 0x44894489u
#define AMIGA_MFM_SECTOR_BYTES_ON_TRACK 1088u
#define AMIGA_MFM_POST_SYNC_BYTES (AMIGA_MFM_SECTOR_BYTES_ON_TRACK - 8u)
#define AMIGA_MFM_DATA_LONGS 128u
#define AMIGA_MFM_HEADER_LONGS 5u

/* ADFlib's ADF format FAQ, sections 2.2-2.4, documents the $4489 sync,
 * sector field layout, odd/even planes and XOR checksums. Its version history
 * also records correction of the odd/even-plane labeling. */
static unsigned read_bit_wrapped(const uint8_t *bits, size_t bit_count,
                                 size_t position)
{
    position %= bit_count;
    return (unsigned)((bits[position >> 3] >> (7u - (position & 7u))) & 1u);
}

static uint32_t read_u32_be_wrapped(const uint8_t *bits, size_t bit_count,
                                    size_t position)
{
    uint32_t value = 0;
    unsigned i;
    for (i = 0; i < 32; ++i)
        value = (value << 1) | read_bit_wrapped(bits, bit_count, position + i);
    return value;
}

static uint32_t read_pair_u32(const uint8_t *bits, size_t bit_count,
                              size_t position)
{
    const uint32_t odd = read_u32_be_wrapped(bits, bit_count, position);
    const uint32_t even = read_u32_be_wrapped(bits, bit_count, position + 32u);
    return ((odd & 0x55555555u) << 1) | (even & 0x55555555u);
}

static uint8_t byte_from_payload(const uint32_t *words, unsigned index)
{
    const unsigned word_index = index >> 2;
    const unsigned byte_index = index & 3u;
    return (uint8_t)(words[word_index] >> (24u - byte_index * 8u));
}

int firestaff_amiga_mfm_scan_track(
    const uint8_t *bit_cells,
    size_t byte_count,
    size_t bit_count,
    unsigned expected_track,
    unsigned sectors_per_track,
    FirestaffAmigaMfmVisitor visitor,
    void *context)
{
    size_t start;
    int delivered = 0;

    if (bit_cells == NULL || visitor == NULL || byte_count == 0u ||
        byte_count > FIRESTAFF_AMIGA_MFM_MAX_TRACK_BYTES ||
        bit_count < 64u || bit_count > byte_count * 8u ||
        expected_track > 159u ||
        (sectors_per_track != 11u && sectors_per_track != 22u))
        return -1;

    for (start = 0; start < bit_count; ++start) {
        uint32_t fields[AMIGA_MFM_HEADER_LONGS];
        uint32_t header_xor = 0;
        uint32_t data_xor = 0;
        uint32_t data_words[AMIGA_MFM_DATA_LONGS];
        uint32_t raw_info;
        unsigned i;
        unsigned sector_no;
        unsigned sectors_to_gap;
        size_t body_start;
        FirestaffAmigaMfmSector sector;

        if (read_u32_be_wrapped(bit_cells, bit_count, start) != AMIGA_MFM_SYNC)
            continue;

        body_start = start + 64u;
        /* Each MFM sector occupies 1088 bytes from sync start through its
         * data end. A smaller capture cannot contain the full sector. */
        if (bit_count < (size_t)AMIGA_MFM_SECTOR_BYTES_ON_TRACK * 8u)
            break;

        for (i = 0; i < AMIGA_MFM_HEADER_LONGS; ++i) {
            fields[i] = read_pair_u32(bit_cells, bit_count,
                                      body_start + (size_t)i * 64u);
            header_xor ^= fields[i];
        }
        header_xor ^= read_pair_u32(bit_cells, bit_count,
                                    body_start + 5u * 64u);
        if (header_xor != 0u)
            continue;

        raw_info = fields[0];
        sector_no = (raw_info >> 8) & 0xffu;
        sectors_to_gap = raw_info & 0xffu;
        if ((raw_info >> 24) != 0xffu ||
            ((raw_info >> 16) & 0xffu) != expected_track ||
            sector_no >= sectors_per_track ||
            sectors_to_gap == 0u || sectors_to_gap > sectors_per_track)
            continue;

        data_xor = read_pair_u32(bit_cells, bit_count,
                                 body_start + 6u * 64u);
        for (i = 0; i < AMIGA_MFM_DATA_LONGS; ++i) {
            data_words[i] = read_pair_u32(bit_cells, bit_count,
                                          body_start + (size_t)(7u + i) * 64u);
            data_xor ^= data_words[i];
        }
        if (data_xor != 0u)
            continue;

        sector.track = (uint8_t)expected_track;
        sector.cylinder = (uint8_t)(expected_track >> 1);
        sector.head = (uint8_t)(expected_track & 1u);
        sector.sector = (uint8_t)sector_no;
        sector.sectors_to_gap = (uint8_t)sectors_to_gap;
        for (i = 0; i < FIRESTAFF_AMIGA_MFM_SECTOR_BYTES; ++i)
            sector.payload[i] = byte_from_payload(data_words, i);

        if (delivered == INT_MAX)
            return delivered;
        ++delivered;
        if (visitor(&sector, context) != 0)
            break;
    }

    return delivered;
}
