#ifndef FIRESTAFF_CSB_V1_X68K_PROGRAM_H
#define FIRESTAFF_CSB_V1_X68K_PROGRAM_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_x68k_hdm.h"

/* Read-only Human68k .X header receipt for CSB's CHAOS_ST.X.  The entry code
 * begins after its 64-byte header. This validates program layout recovered
 * from the original HDM and does not emulate 68000 instructions or DOS/IOCS. */
typedef struct {
    CSB_V1_X68kHdmReceipt media;
    uint32_t text_bytes;
    uint32_t data_bytes;
    uint32_t bss_bytes;
    uint32_t relocation_bytes;
    uint32_t symbol_bytes;
    uint32_t file_bytes;
    uint32_t entry_offset;
    int human68k_x_format;
    int executable_emulation_permitted;
} CSB_V1_X68kProgramReceipt;

int csb_v1_x68k_chaos_st_receipt(const uint8_t *hdm, size_t hdm_size,
                                 CSB_V1_X68kProgramReceipt *out_receipt);

#endif
