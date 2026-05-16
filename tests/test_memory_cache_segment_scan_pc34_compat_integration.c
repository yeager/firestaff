#include <stdio.h>

#include "memory_cache_segment_scan_pc34_compat.h"

static int test_scan_converts_signed_sizes_to_segments(void) {
    struct MemoryCacheRawBlockHeader_Compat headers[4] = {
        {20, 0, 0},
        {-30, 1, 0},
        {10, 0, 0},
        {-16, 2, 1}
    };
    struct MemoryCacheDefragSegment_Compat segments[4] = {0};
    unsigned int count = MEMORY_CACHE_ScanSegments_Compat(headers, 4, segments, 4);
    return count == 4 &&
           segments[0].isUsed == 0 && segments[0].blockSize == 20 &&
           segments[1].isUsed == 1 && segments[1].bitmapIndex == 1 && segments[1].isDerivedBitmap == 0 && segments[1].blockSize == 30 &&
           segments[2].isUsed == 0 && segments[2].blockSize == 10 &&
           segments[3].isUsed == 1 && segments[3].bitmapIndex == 2 && segments[3].isDerivedBitmap == 1 && segments[3].blockSize == 16;
}

static int test_scan_skips_zero_sized_headers(void) {
    struct MemoryCacheRawBlockHeader_Compat headers[3] = {
        {0, 0, 0},
        {-12, 0, 0},
        {8, 0, 0}
    };
    struct MemoryCacheDefragSegment_Compat segments[3] = {0};
    unsigned int count = MEMORY_CACHE_ScanSegments_Compat(headers, 3, segments, 3);
    return count == 2 && segments[0].isUsed == 1 && segments[0].blockSize == 12 && segments[1].isUsed == 0 && segments[1].blockSize == 8;
}

static int test_scan_respects_output_capacity(void) {
    struct MemoryCacheRawBlockHeader_Compat headers[3] = {
        {-12, 0, 0},
        {8, 0, 0},
        {-16, 1, 1}
    };
    struct MemoryCacheDefragSegment_Compat segments[2] = {0};
    unsigned int count = MEMORY_CACHE_ScanSegments_Compat(headers, 3, segments, 2);
    return count == 2 && segments[0].blockSize == 12 && segments[1].blockSize == 8;
}

int main(void) {
    if (!test_scan_converts_signed_sizes_to_segments()) {
        fprintf(stderr, "test_scan_converts_signed_sizes_to_segments failed\n");
        return 1;
    }
    if (!test_scan_skips_zero_sized_headers()) {
        fprintf(stderr, "test_scan_skips_zero_sized_headers failed\n");
        return 1;
    }
    if (!test_scan_respects_output_capacity()) {
        fprintf(stderr, "test_scan_respects_output_capacity failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
