#include "redmcsb_f0789_allocate_layout_range_pc34_compat.h"

#include <stdint.h>

static unsigned long captured_byte_count;
static int captured_allocation_type;
static uint16_t captured_memory_request;
static unsigned char allocation[8];

static unsigned char *capture_allocate(unsigned long byte_count,
                                       int allocation_type,
                                       uint16_t memory_request)
{
    captured_byte_count = byte_count;
    captured_allocation_type = allocation_type;
    captured_memory_request = memory_request;
    return allocation;
}

int main(void)
{
    unsigned char *result = redmcsb_f0789_allocate_layout_range_pc34_compat(
        1234UL, capture_allocate);

    if (result != allocation) {
        return 1;
    }
    if (captured_byte_count != 1234UL) {
        return 2;
    }
    if (captured_allocation_type != REDMCSB_F0789_ALLOCATION_PERMANENT) {
        return 3;
    }
    if (captured_memory_request !=
        REDMCSB_F0789_MEMORY_REQUEST_LAYOUT_RANGE) {
        return 4;
    }
    return 0;
}
