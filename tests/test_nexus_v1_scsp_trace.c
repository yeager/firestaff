#include "nexus_v1_scsp_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    static const char trace[] =
        NEXUS_V1_SCSP_WRITE_TRACE_MAGIC "\n"
        "addr=0x100400 size=1 value=0x00000002 pc=0x00003224\n"
        "addr=0x100700 size=2 value=0x00000000 pc=0x00003230\n"
        "addr=0x100710 size=2 value=0x00000001 pc=0x00003230\n";
    char malformed[sizeof(trace)];
    Nexus_V1_ScspTraceReceipt receipt;

    if (!nexus_v1_scsp_write_trace_parse(
            (const uint8_t *)trace, sizeof(trace) - 1U, &receipt) ||
        !receipt.valid || !receipt.header_valid || !receipt.parse_complete ||
        receipt.record_count != 3U || receipt.mailbox_write_count != 1U ||
        receipt.mailbox_nonzero_count != 1U ||
        receipt.mailbox_value_02_count != 1U ||
        receipt.command_handler_pc_3224_count != 1U ||
        receipt.scsp_voice_register_write_count != 2U ||
        !receipt.mailbox_command_observed ||
        !receipt.driver_command_handler_observed ||
        receipt.event_selector_semantics_proven || receipt.sal_codec_proven ||
        receipt.playback_permitted || !receipt.blocks_real_sfx_playback) {
        fprintf(stderr, "FAIL: SCSP trace receipt\n");
        return 1;
    }

    memcpy(malformed, trace, sizeof(trace));
    malformed[0] = 'X';
    if (nexus_v1_scsp_write_trace_parse(
            (const uint8_t *)malformed, sizeof(trace) - 1U, &receipt) != 0 ||
        receipt.valid) {
        fprintf(stderr, "FAIL: malformed SCSP trace admitted\n");
        return 1;
    }
    {
        const char *path = getenv("FIRESTAFF_NEXUS_SCSP_TRACE");
        if (path && *path) {
            FILE *file = fopen(path, "rb");
            long length;
            uint8_t *raw = NULL;
            int external_ok = file != NULL;
            if (external_ok) external_ok = fseek(file, 0L, SEEK_END) == 0;
            length = external_ok ? ftell(file) : -1L;
            if (external_ok) external_ok = length > 0L;
            if (external_ok) external_ok = fseek(file, 0L, SEEK_SET) == 0;
            if (external_ok) raw = (uint8_t *)malloc((size_t)length);
            if (external_ok) external_ok = raw != NULL;
            if (external_ok)
                external_ok = fread(raw, 1U, (size_t)length, file) ==
                    (size_t)length;
            if (file) fclose(file);
            if (external_ok)
                external_ok = nexus_v1_scsp_write_trace_parse(
                    raw, (size_t)length, &receipt);
            if (external_ok)
                external_ok = receipt.valid && receipt.mailbox_nonzero_count > 0U &&
                    receipt.driver_command_handler_observed &&
                    !receipt.event_selector_semantics_proven &&
                    !receipt.sal_codec_proven && !receipt.playback_permitted;
            if (!external_ok) {
                free(raw);
                fprintf(stderr, "FAIL: external SCSP trace receipt\n");
                return 1;
            }
            free(raw);
        }
    }
    puts("test_nexus_v1_scsp_trace: PASS");
    return 0;
}
