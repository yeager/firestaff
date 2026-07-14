#include "redmcsb_f0917_prim_memory_allocate_x30j_compat.h"

uintptr_t redmcsb_f0917_prim_memory_allocate_x30j_compat(
    redmcsb_f0917_malloc_x30j_compat allocate,
    redmcsb_f0917_print_os_error_x30j_compat print_os_error,
    void *context,
    int32_t byte_count)
{
    uintptr_t buffer = allocate(context, byte_count);

    /* ReDMCSB PRIM1.C:10-25, MEDIA577_X30J route. */
    if (((uint32_t)buffer & REDMCSB_F0917_INVALID_ADDRESS_MASK_X30J) != 0) {
        print_os_error(context, 1);
    }
    return buffer;
}

const char *redmcsb_f0917_prim_memory_allocate_source_evidence_x30j(void)
{
    return "ReDMCSB PRIM1.C:10-25 (Atari ST MEDIA577_X30J): "
           "F0917 calls MALLOC(size), reports F0928_PrintOSError(1) when "
           "the 32-bit result has high nibble set, and returns that raw "
           "result.";
}
