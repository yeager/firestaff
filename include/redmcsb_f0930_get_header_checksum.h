#ifndef FIRESTAFF_REDMCSB_F0930_GET_HEADER_CHECKSUM_H
#define FIRESTAFF_REDMCSB_F0930_GET_HEADER_CHECKSUM_H

#include <stdint.h>

/* ReDMCSB PRIM1.C F0930_GetHeaderChecksum. */
#define REDMCSB_F0930_HEADER_BYTE_COUNT 20u
#define REDMCSB_F0930_SEGMENT_HEADER_BYTE_COUNT 12u

/*
 * The original takes HEADER and SEGMENTHEADER pointers. This portable surface
 * receives their source-defined byte representations explicitly: the caller
 * supplies HEADER.SegmentCount and the contiguous SEGMENTHEADER bytes.
 */
uint16_t redmcsb_f0930_get_header_checksum(
    const uint8_t header[REDMCSB_F0930_HEADER_BYTE_COUNT],
    uint16_t segment_count,
    const uint8_t *segment_headers);

const char *redmcsb_f0930_get_header_checksum_source_evidence(void);

#endif /* FIRESTAFF_REDMCSB_F0930_GET_HEADER_CHECKSUM_H */
