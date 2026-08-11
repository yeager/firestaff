#include "csb_v1_x68k_autoexec.h"

#include <stdlib.h>
#include <string.h>

int csb_v1_x68k_autoexec_receipt(const uint8_t *hdm, size_t hdm_size,
                                  CSB_V1_X68kAutoexecReceipt *out_receipt)
{
    uint8_t *bytes = NULL;
    size_t byte_count = 0u;
    size_t at = 0u;
    CSB_V1_X68kAutoexecReceipt receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "AUTOEXEC.BAT",
                                            NULL, 0u, &byte_count,
                                            &receipt.media) ||
        !byte_count || !(bytes = (uint8_t *)malloc(byte_count)) ||
        !csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "AUTOEXEC.BAT",
                                            bytes, byte_count, &byte_count,
                                            NULL)) {
        free(bytes);
        return 0;
    }
    while (at < byte_count && bytes[at] != 0x1au) {
        size_t length = 0u;
        if (receipt.command_count >= CSB_V1_X68K_AUTOEXEC_MAX_COMMANDS) {
            free(bytes);
            return 0;
        }
        while (at < byte_count && bytes[at] != '\r' && bytes[at] != 0x1au) {
            if (bytes[at] < '!' || bytes[at] > '~' || length >= 31u) {
                free(bytes);
                return 0;
            }
            receipt.commands[receipt.command_count][length++] = (char)bytes[at++];
        }
        if (!length) {
            free(bytes);
            return 0;
        }
        receipt.commands[receipt.command_count++][length] = '\0';
        /* The verified final command is immediately DOS-EOF terminated;
         * preceding commands use CRLF.  Both forms are explicit media facts. */
        if (at < byte_count && bytes[at] == 0x1au) break;
        if (at + 1u >= byte_count || bytes[at] != '\r' ||
            bytes[at + 1u] != '\n') {
            free(bytes);
            return 0;
        }
        at += 2u;
    }
    if (at >= byte_count || bytes[at] != 0x1au) {
        free(bytes);
        return 0;
    }
    receipt.dos_eof_terminated = 1;
    receipt.host_execution_permitted = 0;
    free(bytes);
    *out_receipt = receipt;
    return 1;
}
