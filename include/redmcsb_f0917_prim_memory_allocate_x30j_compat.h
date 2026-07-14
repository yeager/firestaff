/* ReDMCSB PRIM1.C F0917_PRIM_15_Memory_Allocate, Atari ST X30J route. */
#ifndef FIRESTAFF_REDMCSB_F0917_PRIM_MEMORY_ALLOCATE_X30J_COMPAT_H
#define FIRESTAFF_REDMCSB_F0917_PRIM_MEMORY_ALLOCATE_X30J_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F0917_INVALID_ADDRESS_MASK_X30J UINT32_C(0xf0000000)

/* Host bridges for GEMDOS MALLOC and F0928_PrintOSError(1). */
typedef uintptr_t (*redmcsb_f0917_malloc_x30j_compat)(void *context,
                                                       int32_t byte_count);
typedef void (*redmcsb_f0917_print_os_error_x30j_compat)(void *context,
                                                          int error_number);

/*
 * Returns the raw MALLOC result. The X30J source reports, but does not
 * replace, a result whose 32-bit high nibble is nonzero.
 */
uintptr_t redmcsb_f0917_prim_memory_allocate_x30j_compat(
    redmcsb_f0917_malloc_x30j_compat allocate,
    redmcsb_f0917_print_os_error_x30j_compat print_os_error,
    void *context,
    int32_t byte_count);

const char *redmcsb_f0917_prim_memory_allocate_source_evidence_x30j(void);

#ifdef __cplusplus
}
#endif

#endif
