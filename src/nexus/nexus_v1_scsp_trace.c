#include "nexus_v1_scsp_trace.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NEXUS_V1_SCSP_TRACE_MAX_BYTES (16U * 1024U * 1024U)

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int parse_record(const char *line,
                        unsigned long *address,
                        unsigned long *size,
                        unsigned long *value,
                        unsigned long *pc)
{
    char tail;
    return sscanf(line, "addr=%lx size=%lu value=%lx pc=%lx %c",
                  address, size, value, pc, &tail) == 4;
}

int nexus_v1_scsp_write_trace_parse(
    const uint8_t *raw_trace,
    size_t raw_trace_size,
    Nexus_V1_ScspTraceReceipt *out_receipt)
{
    Nexus_V1_ScspTraceReceipt receipt;
    char *text;
    char *line;
    char *next;

    memset(&receipt, 0, sizeof(receipt));
    receipt.blocks_real_sfx_playback = 1;
    if (!out_receipt) return 0;
    if (!raw_trace || raw_trace_size == 0U ||
        raw_trace_size > NEXUS_V1_SCSP_TRACE_MAX_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    text = (char *)malloc(raw_trace_size + 1U);
    if (!text) {
        *out_receipt = receipt;
        return 0;
    }
    memcpy(text, raw_trace, raw_trace_size);
    text[raw_trace_size] = '\0';
    receipt.raw_trace_byte_count = raw_trace_size;
    receipt.raw_trace_fnv1a64 = fnv1a64(raw_trace, raw_trace_size);

    line = text;
    next = strchr(line, '\n');
    if (next) *next++ = '\0';
    if (strcmp(line, NEXUS_V1_SCSP_WRITE_TRACE_MAGIC) != 0) {
        free(text);
        *out_receipt = receipt;
        return 0;
    }
    receipt.header_valid = 1;

    while (next) {
        unsigned long address;
        unsigned long size;
        unsigned long value;
        unsigned long pc;
        line = next;
        next = strchr(line, '\n');
        if (next) *next++ = '\0';
        if (*line == '\0') continue;
        if (!parse_record(line, &address, &size, &value, &pc) ||
            address > UINT32_MAX || size > UINT32_MAX ||
            value > UINT32_MAX || pc > UINT32_MAX || size == 0U) {
            free(text);
            *out_receipt = receipt;
            return 0;
        }
        ++receipt.record_count;
        if (address == UINT32_C(0x100400)) {
            ++receipt.mailbox_write_count;
            if (value != 0U) ++receipt.mailbox_nonzero_count;
            if (value == 2U) ++receipt.mailbox_value_02_count;
            if (value == 2U) receipt.mailbox_command_observed = 1;
        }
        if (pc == UINT32_C(0x3224)) {
            ++receipt.command_handler_pc_3224_count;
            receipt.driver_command_handler_observed = 1;
        }
        if (address >= UINT32_C(0x100700) &&
            address < UINT32_C(0x100800))
            ++receipt.scsp_voice_register_write_count;
    }
    free(text);
    receipt.parse_complete = 1;
    receipt.sound_cpu_trace = 1;
    receipt.valid = receipt.record_count != 0U;
    /* The raw trace does not identify a host event, MAP row, SAL codec or
     * playback device. These remain hard false even when driver facts exist. */
    receipt.event_selector_semantics_proven = 0;
    receipt.sal_codec_proven = 0;
    receipt.playback_permitted = 0;
    receipt.blocks_real_sfx_playback = 1;
    *out_receipt = receipt;
    return receipt.valid;
}
