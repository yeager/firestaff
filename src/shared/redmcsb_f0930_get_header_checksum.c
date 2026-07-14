#include "redmcsb_f0930_get_header_checksum.h"

uint16_t redmcsb_f0930_get_header_checksum(
    const uint8_t header[REDMCSB_F0930_HEADER_BYTE_COUNT],
    uint16_t segment_count,
    const uint8_t *segment_headers)
{
    uint16_t index;
    uint16_t checksum = 0u;
    uint16_t segment_byte_count;

    for (index = 4u; index < REDMCSB_F0930_HEADER_BYTE_COUNT; ++index) {
        checksum = (uint16_t)(checksum + (header[index] * index));
    }

    segment_byte_count = (uint16_t)(
        segment_count * REDMCSB_F0930_SEGMENT_HEADER_BYTE_COUNT);
    for (index = 0u; index < segment_byte_count; ++index) {
        checksum = (uint16_t)(
            checksum + (segment_headers[index] * ((index & 0x00ffu) + 1u)));
    }

    return checksum;
}

const char *redmcsb_f0930_get_header_checksum_source_evidence(void)
{
    return "ReDMCSB PRIM1.C:649-670; PRIM2B.C:757-778; SWITCH.C:209-228; "
           "CEDT013.C:412-429; FTL.H:5-26";
}
