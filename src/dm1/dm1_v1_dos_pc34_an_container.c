#include "dm1_v1_dos_pc34_an_container.h"
#include <string.h>

static uint16_t rd_u16_be(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

dm1_v1_dos_pc34_an_status_t
dm1_v1_dos_pc34_an_parse_header_pc34(
        const uint8_t *blob, size_t blob_size,
        dm1_v1_dos_pc34_an_header_t *out) {
    if (!blob || !out) return DM1_V1_DOS_PC34_AN_BAD_ARGS;
    if (blob_size < DM1_V1_DOS_PC34_AN_HEADER_BYTES)
        return DM1_V1_DOS_PC34_AN_TOO_SMALL;
    if (blob[0] != 'A' || blob[1] != 'N')
        return DM1_V1_DOS_PC34_AN_BAD_SIGNATURE;
    uint16_t w = rd_u16_be(blob + 6);
    uint16_t h = rd_u16_be(blob + 8);
    uint16_t p = rd_u16_be(blob + 10);
    uint16_t s = rd_u16_be(blob + 12);
    if (w != DM1_V1_DOS_PC34_AN_WIDTH ||
        h != DM1_V1_DOS_PC34_AN_HEIGHT ||
        p != DM1_V1_DOS_PC34_AN_PLANES)
        return DM1_V1_DOS_PC34_AN_BAD_GEOMETRY;
    out->width   = w;
    out->height  = h;
    out->planes  = p;
    out->subtype = s;
    return DM1_V1_DOS_PC34_AN_OK;
}

static int is_known_tag(uint8_t a, uint8_t b) {
    /* Byte-verified in DM1 DOS 3.4 TITLE and END files. */
    if (a == 'B' && b == 'R') return 1;
    if (a == 'P' && b == '8') return 1;
    if (a == 'P' && b == 'L') return 1;
    if (a == 'E' && b == 'N') return 1;
    if (a == 'T' && b == 'D') return 1;
    return 0;
}

int dm1_v1_dos_pc34_an_find_next_tag_pc34(
        const uint8_t *blob, size_t blob_size,
        uint32_t from_offset, uint32_t *offset_out,
        char tag_out[3]) {
    if (!blob || !offset_out || !tag_out) return 0;
    if (blob_size < 2) return 0;
    for (uint32_t i = from_offset; (uint64_t)i + 1u < blob_size; ++i) {
        if (is_known_tag(blob[i], blob[i + 1])) {
            *offset_out = i;
            tag_out[0] = (char)blob[i];
            tag_out[1] = (char)blob[i + 1];
            tag_out[2] = '\0';
            return 1;
        }
    }
    return 0;
}
