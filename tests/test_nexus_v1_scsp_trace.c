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
        !receipt.intra_trace_observation_order_proven ||
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
                    receipt.first_mailbox_raw_offset != 0U &&
                    receipt.first_handler_raw_offset != 0U &&
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
    {
        const char *path = getenv("FIRESTAFF_NEXUS_MAIN_SCSP_TRACE");
        if (path && *path) {
            FILE *file = fopen(path, "rb");
            long length;
            uint8_t *raw = NULL;
            Nexus_V1_MainScspTraceReceipt main_receipt;
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
                external_ok = nexus_v1_main_scsp_write_trace_parse(
                    raw, (size_t)length, &main_receipt);
            if (!external_ok || !main_receipt.valid ||
                !main_receipt.producer_command_observed ||
                main_receipt.mailbox_value_02_count == 0U ||
                main_receipt.mailbox_value_0200_count == 0U ||
                !main_receipt.blocks_real_sfx_playback ||
                main_receipt.first_producer_command_raw_offset == 0U) {
                free(raw);
                fprintf(stderr, "FAIL: external main SCSP trace receipt\n");
                return 1;
            }
            free(raw);
        }
    }
    {
        static const char main_trace[] =
            NEXUS_V1_MAIN_SCSP_WRITE_TRACE_MAGIC "\n"
            "addr=0x00100400 size=1 value=0x00000002 "
            "pc0=0x06001652 pc1=0x00000000\n"
            "addr=0x00100400 size=2 value=0x00000200 "
            "pc0=0x06014eb2 pc1=0x00000000\n";
        Nexus_V1_MainScspTraceReceipt main_receipt;
        if (!nexus_v1_main_scsp_write_trace_parse(
                (const uint8_t *)main_trace, sizeof(main_trace) - 1U,
                &main_receipt) ||
            !main_receipt.valid || !main_receipt.header_valid ||
            main_receipt.record_count != 2U ||
            main_receipt.mailbox_value_02_count != 1U ||
            main_receipt.mailbox_value_0200_count != 1U ||
            !main_receipt.producer_command_observed ||
            !main_receipt.blocks_real_sfx_playback) {
            fprintf(stderr, "FAIL: main SCSP trace receipt\n");
            return 1;
        }
    }
    {
        static const char session_trace[] =
            NEXUS_V1_SCSP_WRITE_TRACE_MAGIC "\n"
            "session=unit-capture\n"
            "addr=0x100400 size=1 value=0x00000002 pc=0x00003224\n";
        static const char session_main_trace[] =
            NEXUS_V1_MAIN_SCSP_WRITE_TRACE_MAGIC "\n"
            "session=unit-capture\n"
            "addr=0x00100400 size=1 value=0x00000002 "
            "pc0=0x06001652 pc1=0x00000000\n";
        Nexus_V1_MainScspTraceReceipt main_receipt;
        if (!nexus_v1_scsp_write_trace_parse(
                (const uint8_t *)session_trace, sizeof(session_trace) - 1U,
                &receipt) || !receipt.capture_session_present ||
            strcmp(receipt.capture_session, "unit-capture") != 0 ||
            !nexus_v1_main_scsp_write_trace_parse(
                (const uint8_t *)session_main_trace,
                sizeof(session_main_trace) - 1U, &main_receipt) ||
            !main_receipt.capture_session_present ||
            strcmp(main_receipt.capture_session, "unit-capture") != 0) {
            fprintf(stderr, "FAIL: same-session SCSP trace metadata\n");
            return 1;
        }
    }
    puts("test_nexus_v1_scsp_trace: PASS");
    return 0;
}
