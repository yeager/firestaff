#ifndef FIRESTAFF_CSB_V1_F1065_F1066_F1067_F1068_PLATFORM_HELPERS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1065_F1066_F1067_F1068_PLATFORM_HELPERS_PC34_COMPAT_H

#include "redmcsb_f1066_get_usable_chip_memory_byte_count.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void F1065_SetExecBase(void);

int32_t F1066_GetUsableChipMemoryByteCount(
    const redmcsb_f1066_chip_memory_io *io,
    void *context);

void F1067_InitAmigaStuff(void);

void F1068_FreeAmigaStuff(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1065_F1066_F1067_F1068_PLATFORM_HELPERS_PC34_COMPAT_H */
