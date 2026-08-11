#include "csb_v1_x68k_program.h"

#include <stdlib.h>
#include <string.h>

static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

int csb_v1_x68k_chaos_st_receipt(const uint8_t *hdm, size_t hdm_size,
                                 CSB_V1_X68kProgramReceipt *out_receipt) {
    uint8_t *bytes = NULL;
    size_t size = 0u;
    uint64_t stored_bytes;
    CSB_V1_X68kProgramReceipt receipt;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "CHAOS_ST.X", NULL,
                                            0u, &size, &receipt.media) ||
        size < 64u || !(bytes = (uint8_t *)malloc(size)) ||
        !csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "CHAOS_ST.X", bytes,
                                            size, &size, NULL) ||
        bytes[0] != 'H' || bytes[1] != 'U') {
        free(bytes);
        return 0;
    }
    receipt.text_bytes = be32(bytes + 4u);
    receipt.data_bytes = be32(bytes + 8u);
    receipt.bss_bytes = be32(bytes + 12u);
    receipt.relocation_bytes = be32(bytes + 16u);
    receipt.symbol_bytes = be32(bytes + 20u);
    stored_bytes = 64u + (uint64_t)receipt.text_bytes + receipt.data_bytes +
        receipt.relocation_bytes + receipt.symbol_bytes;
    if (stored_bytes != size || receipt.text_bytes == 0u) {
        free(bytes);
        return 0;
    }
    receipt.file_bytes = (uint32_t)size;
    receipt.entry_offset = 64u;
    receipt.human68k_x_format = 1;
    receipt.executable_emulation_permitted = 0;
    free(bytes);
    *out_receipt = receipt;
    return 1;
}
