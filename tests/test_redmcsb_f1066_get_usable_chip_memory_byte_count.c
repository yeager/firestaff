#include "redmcsb_f1066_get_usable_chip_memory_byte_count.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f1066_capture {
    int32_t initial_largest_block_byte_count;
    int32_t initial_available_byte_count;
    int32_t allocated_largest_block_byte_count;
    int32_t allocated_available_byte_count;
    int allocation_fails;
    unsigned int forbid_count;
    unsigned int permit_count;
    unsigned int availability_count;
    unsigned int allocation_count;
    unsigned int free_count;
    unsigned int alert_count;
    int availability_largest_block[4];
    int32_t allocation_byte_count;
    int32_t free_byte_count;
    uint32_t alert_code;
    unsigned char allocation_token;
} redmcsb_f1066_capture;

static void capture_forbid(void *context)
{
    redmcsb_f1066_capture *capture = context;

    ++capture->forbid_count;
}

static int32_t capture_available_chip_memory_byte_count(void *context,
                                                         int largest_block)
{
    redmcsb_f1066_capture *capture = context;

    assert(capture->availability_count < 4u);
    capture->availability_largest_block[capture->availability_count] =
        largest_block;
    ++capture->availability_count;
    if (capture->availability_count <= 2u || capture->allocation_fails != 0) {
        return largest_block != 0 ?
            capture->initial_largest_block_byte_count :
            capture->initial_available_byte_count;
    }
    return largest_block != 0 ? capture->allocated_largest_block_byte_count :
        capture->allocated_available_byte_count;
}

static void *capture_allocate_chip_memory(void *context, int32_t byte_count)
{
    redmcsb_f1066_capture *capture = context;

    ++capture->allocation_count;
    capture->allocation_byte_count = byte_count;
    return capture->allocation_fails != 0 ? NULL : &capture->allocation_token;
}

static void capture_free_chip_memory(void *context, void *memory,
                                     int32_t byte_count)
{
    (void)memory;
    redmcsb_f1066_capture *capture = context;

    assert(memory == &capture->allocation_token);
    ++capture->free_count;
    capture->free_byte_count = byte_count;
}

static void capture_alert_csb_system_error(void *context, uint32_t error_code)
{
    redmcsb_f1066_capture *capture = context;

    ++capture->alert_count;
    capture->alert_code = error_code;
}

static void capture_permit(void *context)
{
    redmcsb_f1066_capture *capture = context;

    ++capture->permit_count;
}

static const redmcsb_f1066_chip_memory_io capture_io = {
    capture_forbid,
    capture_available_chip_memory_byte_count,
    capture_allocate_chip_memory,
    capture_free_chip_memory,
    capture_alert_csb_system_error,
    capture_permit
};
(void)capture_io;

static void assert_lock_pair(const redmcsb_f1066_capture *capture)
{
    (void)capture;
    assert(capture->forbid_count == 1u);
    assert(capture->permit_count == 1u);
}

int main(void)
{
    redmcsb_f1066_capture insufficient_largest = { 8191, 20000, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0,
                                                    { 0, 0, 0, 0 }, 0, 0, 0,
                                                    0 };
    redmcsb_f1066_capture insufficient_total = { 20000, 16383, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0,
                                                  { 0, 0, 0, 0 }, 0, 0, 0,
                                                  0 };
    redmcsb_f1066_capture reserve_largest = { 30000, 50000, 4000, 20000, 0,
                                               0, 0, 0, 0, 0, 0,
                                               { 0, 0, 0, 0 }, 0, 0, 0, 0 };
    redmcsb_f1066_capture reserve_total = { 30000, 40000, 9000, 12000, 0,
                                             0, 0, 0, 0, 0, 0,
                                             { 0, 0, 0, 0 }, 0, 0, 0, 0 };
    redmcsb_f1066_capture allocation_failure = { 20000, 30000, 0, 0, 1,
                                                   0, 0, 0, 0, 0, 0,
                                                   { 0, 0, 0, 0 }, 0, 0, 0,
                                                   0 };
    const char *evidence;
    (void)evidence;

    assert(redmcsb_f1066_get_usable_chip_memory_byte_count(
               &capture_io, &insufficient_largest) == 0);
    assert(insufficient_largest.availability_count == 1u);
    assert(insufficient_largest.allocation_count == 0u);
    assert_lock_pair(&insufficient_largest);

    assert(redmcsb_f1066_get_usable_chip_memory_byte_count(
               &capture_io, &insufficient_total) == 0);
    assert(insufficient_total.availability_count == 2u);
    assert(insufficient_total.allocation_count == 0u);
    assert_lock_pair(&insufficient_total);

    assert(redmcsb_f1066_get_usable_chip_memory_byte_count(
               &capture_io, &reserve_largest) == 21808);
    assert(reserve_largest.availability_count == 4u);
    assert(reserve_largest.availability_largest_block[0] == 1);
    assert(reserve_largest.availability_largest_block[1] == 0);
    assert(reserve_largest.availability_largest_block[2] == 1);
    assert(reserve_largest.availability_largest_block[3] == 0);
    assert(reserve_largest.allocation_count == 1u);
    assert(reserve_largest.allocation_byte_count == 30000);
    assert(reserve_largest.free_count == 1u);
    assert(reserve_largest.free_byte_count == 30000);
    assert(reserve_largest.alert_count == 0u);
    assert_lock_pair(&reserve_largest);

    assert(redmcsb_f1066_get_usable_chip_memory_byte_count(
               &capture_io, &reserve_total) == 25616);
    assert(reserve_total.free_count == 1u);
    assert_lock_pair(&reserve_total);

    assert(redmcsb_f1066_get_usable_chip_memory_byte_count(
               &capture_io, &allocation_failure) == 20000);
    assert(allocation_failure.availability_count == 4u);
    assert(allocation_failure.free_count == 0u);
    assert(allocation_failure.alert_count == 1u);
    assert(allocation_failure.alert_code == UINT32_C(0x80FF0017));
    assert_lock_pair(&allocation_failure);

    evidence = redmcsb_f1066_get_usable_chip_memory_byte_count_source_evidence();
    assert(strstr(evidence, "AMIGINIT.C:498-560") != NULL);
    assert(strstr(evidence, "F1066_GetUsableChipMemoryByteCount") != NULL);
    puts("ok: ReDMCSB F1066 usable chip-memory byte count");
    return 0;
}
