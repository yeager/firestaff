#include "csb_v1_x68k_autoexec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out_bytes, size_t *out_size)
{
    FILE *file = NULL;
    long length;
    uint8_t *bytes = NULL;
    if (!path || !out_bytes || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        if (file) fclose(file);
        free(bytes);
        return 0;
    }
    fclose(file);
    *out_bytes = bytes;
    *out_size = (size_t)length;
    return 1;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_X68K_HDM");
    CSB_V1_X68kAutoexecReceipt receipt;
    uint8_t *hdm = NULL;
    size_t hdm_size = 0u;
    int ok;

    if (!path || !path[0]) {
        puts("test_csb_v1_x68k_autoexec: SKIP FIRESTAFF_CSB_X68K_HDM unset");
        return 0;
    }
    ok = read_file(path, &hdm, &hdm_size) &&
        csb_v1_x68k_autoexec_receipt(hdm, hdm_size, &receipt) &&
        receipt.command_count == 3u && strcmp(receipt.commands[0], "CK") == 0 &&
        strcmp(receipt.commands[1], "VIDSET") == 0 &&
        strcmp(receipt.commands[2], "CHAOS_STRIKES_BACK") == 0 &&
        receipt.dos_eof_terminated && !receipt.host_execution_permitted;
    free(hdm);
    if (!ok) {
        puts("test_csb_v1_x68k_autoexec: original media mismatch");
        return 1;
    }
    puts("test_csb_v1_x68k_autoexec: PASS");
    return 0;
}
