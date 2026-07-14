#include "redmcsb_f1066_get_usable_chip_memory_byte_count.h"

enum {
    REDMCSB_F1066_MINIMUM_LARGEST_BLOCK_BYTE_COUNT = 8192,
    REDMCSB_F1066_MINIMUM_AVAILABLE_BYTE_COUNT = 16384
};

#define REDMCSB_F1066_ALERT_ALLOCATION_FAILURE UINT32_C(0x80FF0017)

int32_t redmcsb_f1066_get_usable_chip_memory_byte_count(
    const redmcsb_f1066_chip_memory_io *io,
    void *context)
{
    void *largest_block_bottom;
    int32_t largest_block_byte_count;
    int32_t second_largest_block_byte_count;
    int32_t available_except_largest_block_byte_count;

    io->forbid(context);
    largest_block_byte_count =
        io->available_chip_memory_byte_count(context, 1);
    if (largest_block_byte_count <
            REDMCSB_F1066_MINIMUM_LARGEST_BLOCK_BYTE_COUNT ||
        io->available_chip_memory_byte_count(context, 0) <
            REDMCSB_F1066_MINIMUM_AVAILABLE_BYTE_COUNT) {
        largest_block_byte_count = 0;
    } else {
        largest_block_bottom =
            io->allocate_chip_memory(context, largest_block_byte_count);
        if (largest_block_bottom == 0) {
            io->alert_csb_system_error(context,
                                      REDMCSB_F1066_ALERT_ALLOCATION_FAILURE);
        }
        second_largest_block_byte_count =
            io->available_chip_memory_byte_count(context, 1);
        available_except_largest_block_byte_count =
            io->available_chip_memory_byte_count(context, 0);
        if (largest_block_bottom != 0) {
            io->free_chip_memory(context, largest_block_bottom,
                                 largest_block_byte_count);
        }
        if (second_largest_block_byte_count <
            REDMCSB_F1066_MINIMUM_LARGEST_BLOCK_BYTE_COUNT) {
            largest_block_byte_count -=
                REDMCSB_F1066_MINIMUM_LARGEST_BLOCK_BYTE_COUNT;
            available_except_largest_block_byte_count +=
                REDMCSB_F1066_MINIMUM_LARGEST_BLOCK_BYTE_COUNT;
        }
        if (available_except_largest_block_byte_count <
            REDMCSB_F1066_MINIMUM_AVAILABLE_BYTE_COUNT) {
            largest_block_byte_count -=
                REDMCSB_F1066_MINIMUM_AVAILABLE_BYTE_COUNT -
                available_except_largest_block_byte_count;
        }
        if (largest_block_byte_count < 0) {
            largest_block_byte_count = 0;
        }
    }
    io->permit(context);
    return largest_block_byte_count;
}

const char *redmcsb_f1066_get_usable_chip_memory_byte_count_source_evidence(
    void)
{
    return "ReDMCSB AMIGINIT.C:498-560, "
           "F1066_GetUsableChipMemoryByteCount: calls Forbid, uses "
           "AvailMem(MEMF_LARGEST | MEMF_CHIP) and AvailMem(MEMF_CHIP), "
           "temporarily AllocMem/FreeMem's the largest block, preserves an "
           "8K block and 16K total, then calls Permit.";
}
