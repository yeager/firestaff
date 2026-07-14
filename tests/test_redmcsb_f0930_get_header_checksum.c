#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "redmcsb_f0930_get_header_checksum.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    uint8_t header[REDMCSB_F0930_HEADER_BYTE_COUNT];
    uint8_t one_segment[REDMCSB_F0930_SEGMENT_HEADER_BYTE_COUNT];
    uint8_t two_segments[REDMCSB_F0930_SEGMENT_HEADER_BYTE_COUNT * 2u];
    uint8_t multiplier_wrap_segments[
        REDMCSB_F0930_SEGMENT_HEADER_BYTE_COUNT * 22u];
    uint16_t index;

    memset(header, 0, sizeof(header));
    memset(one_segment, 0, sizeof(one_segment));
    header[0] = 0xffu;
    header[1] = 0xffu;
    header[2] = 0xffu;
    header[3] = 0xffu;
    for (index = 4u; index < REDMCSB_F0930_HEADER_BYTE_COUNT; ++index) {
        header[index] = 1u;
    }
    assert(redmcsb_f0930_get_header_checksum(header, 0u, one_segment) ==
           UINT16_C(184));

    for (index = 0u; index < REDMCSB_F0930_HEADER_BYTE_COUNT; ++index) {
        header[index] = (uint8_t)index;
    }
    for (index = 0u; index < REDMCSB_F0930_SEGMENT_HEADER_BYTE_COUNT;
         ++index) {
        one_segment[index] = (uint8_t)(index + 1u);
    }
    assert(redmcsb_f0930_get_header_checksum(header, 1u, one_segment) ==
           UINT16_C(3106));

    memset(two_segments, 0xff, sizeof(two_segments));
    assert(redmcsb_f0930_get_header_checksum(header, 2u, two_segments) ==
           UINT16_C(13420));

    memset(header, 0, sizeof(header));
    memset(multiplier_wrap_segments, 0, sizeof(multiplier_wrap_segments));
    multiplier_wrap_segments[256u] = 1u;
    assert(redmcsb_f0930_get_header_checksum(header, 22u,
                                              multiplier_wrap_segments) ==
           UINT16_C(1));

    assert(strstr(redmcsb_f0930_get_header_checksum_source_evidence(),
                  "PRIM1.C:649-670") != NULL);
    assert(strstr(redmcsb_f0930_get_header_checksum_source_evidence(),
                  "CEDT013.C:412-429") != NULL);
    return 0;
}
