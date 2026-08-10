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

static int parse_main_record(const char *line,
                             unsigned long *address,
                             unsigned long *size,
                             unsigned long *value,
                             unsigned long *pc0,
                             unsigned long *pc1)
{
    char tail;
    return sscanf(line, "addr=%lx size=%lu value=%lx pc0=%lx pc1=%lx %c",
                  address, size, value, pc0, pc1, &tail) == 5;
}

static int parse_capture_session(const char *line, char *out_session)
{
    size_t length;

    if (strncmp(line, "session=", 8U) != 0 || !line[8]) return 0;
    length = strlen(line + 8U);
    if (length >= NEXUS_V1_SCSP_CAPTURE_SESSION_MAX) return 0;
    if (strchr(line + 8U, ' ') || strchr(line + 8U, '\t')) return 0;
    memcpy(out_session, line + 8U, length + 1U);
    return 1;
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
        size_t raw_offset;
        unsigned long address;
        unsigned long size;
        unsigned long value;
        unsigned long pc;
        line = next;
        raw_offset = (size_t)(line - text);
        next = strchr(line, '\n');
        if (next) *next++ = '\0';
        if (*line == '\0') continue;
        if (!receipt.capture_session_present &&
            parse_capture_session(line, receipt.capture_session)) {
            receipt.capture_session_present = 1;
            continue;
        }
        if (!parse_record(line, &address, &size, &value, &pc) ||
            address > UINT32_MAX || size > UINT32_MAX ||
            value > UINT32_MAX || pc > UINT32_MAX || size == 0U) {
            free(text);
            *out_receipt = receipt;
            return 0;
        }
        ++receipt.record_count;
        if (address == UINT32_C(0x100400)) {
            if (receipt.mailbox_write_count == 0U)
                receipt.first_mailbox_raw_offset = raw_offset;
            ++receipt.mailbox_write_count;
            if (value != 0U) ++receipt.mailbox_nonzero_count;
            if (value == 2U) ++receipt.mailbox_value_02_count;
            if (value == 2U) receipt.mailbox_command_observed = 1;
        }
        if (pc == UINT32_C(0x3224)) {
            if (!receipt.driver_command_handler_observed)
                receipt.first_handler_raw_offset = raw_offset;
            ++receipt.command_handler_pc_3224_count;
            receipt.driver_command_handler_observed = 1;
        }
        if (address >= UINT32_C(0x100700) &&
            address < UINT32_C(0x100800)) {
            if (receipt.scsp_voice_register_write_count == 0U)
                receipt.first_voice_raw_offset = raw_offset;
            ++receipt.scsp_voice_register_write_count;
        }
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
    receipt.intra_trace_observation_order_proven =
        receipt.mailbox_command_observed &&
        receipt.driver_command_handler_observed &&
        receipt.scsp_voice_register_write_count != 0U &&
        receipt.first_mailbox_raw_offset <= receipt.first_handler_raw_offset &&
        receipt.first_handler_raw_offset <= receipt.first_voice_raw_offset;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_main_scsp_write_trace_parse(
    const uint8_t *raw_trace,
    size_t raw_trace_size,
    Nexus_V1_MainScspTraceReceipt *out_receipt)
{
    Nexus_V1_MainScspTraceReceipt receipt;
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
    if (strcmp(line, NEXUS_V1_MAIN_SCSP_WRITE_TRACE_MAGIC) != 0) {
        free(text);
        *out_receipt = receipt;
        return 0;
    }
    receipt.header_valid = 1;

    while (next) {
        size_t raw_offset;
        unsigned long address;
        unsigned long size;
        unsigned long value;
        unsigned long pc0;
        unsigned long pc1;
        line = next;
        raw_offset = (size_t)(line - text);
        next = strchr(line, '\n');
        if (next) *next++ = '\0';
        if (*line == '\0') continue;
        if (!receipt.capture_session_present &&
            parse_capture_session(line, receipt.capture_session)) {
            receipt.capture_session_present = 1;
            continue;
        }
        if (!parse_main_record(line, &address, &size, &value, &pc0, &pc1) ||
            address > UINT32_MAX || size > UINT32_MAX ||
            value > UINT32_MAX || pc0 > UINT32_MAX || pc1 > UINT32_MAX ||
            size == 0U) {
            free(text);
            *out_receipt = receipt;
            return 0;
        }
        ++receipt.record_count;
        if (address == UINT32_C(0x100400)) {
            ++receipt.mailbox_write_count;
            if (value != 0U) ++receipt.mailbox_nonzero_count;
            if (value == 2U) ++receipt.mailbox_value_02_count;
            if (value == UINT32_C(0x200)) ++receipt.mailbox_value_0200_count;
            if (value == 2U || value == UINT32_C(0x200))
                receipt.producer_command_observed = 1;
            if (receipt.first_producer_command_raw_offset == 0U)
                receipt.first_producer_command_raw_offset = raw_offset;
        }
    }
    free(text);
    receipt.parse_complete = 1;
    receipt.valid = receipt.record_count != 0U;
    /* The producer trace does not identify a gameplay event, MAP row, SAL
     * codec or sound-driver voice. */
    receipt.blocks_real_sfx_playback = 1;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_scsp_runtime_join(
    const Nexus_V1_ScspTraceReceipt *sound_cpu_trace,
    const Nexus_V1_MainScspTraceReceipt *main_cpu_trace,
    const Nexus_V1_SddrvsDisassemblyReceipt *driver_receipt,
    int slev_source_verified,
    int sal_source_verified,
    int map_source_verified,
    int driver_source_verified,
    Nexus_V1_ScspRuntimeJoinReceipt *out_receipt)
{
    Nexus_V1_ScspRuntimeJoinReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.blocks_real_sfx_playback = 1;
    receipt.event_selector_semantics_proven = 0;
    receipt.sal_codec_proven = 0;
    receipt.playback_permitted = 0;
    if (!out_receipt) return 0;

    receipt.source_identities_bound =
        slev_source_verified && sal_source_verified &&
        map_source_verified && driver_source_verified;
    receipt.capture_session_bound =
        sound_cpu_trace && main_cpu_trace &&
        sound_cpu_trace->capture_session_present &&
        main_cpu_trace->capture_session_present &&
        strcmp(sound_cpu_trace->capture_session,
               main_cpu_trace->capture_session) == 0;
    receipt.producer_command_observed =
        main_cpu_trace && main_cpu_trace->valid &&
        main_cpu_trace->producer_command_observed;
    receipt.sound_cpu_command_observed =
        sound_cpu_trace && sound_cpu_trace->valid &&
        sound_cpu_trace->mailbox_command_observed;
    receipt.driver_command_handler_observed =
        sound_cpu_trace && sound_cpu_trace->valid &&
        sound_cpu_trace->driver_command_handler_observed;
    receipt.pcm_voice_register_route_observed =
        driver_receipt && driver_receipt->valid &&
        driver_receipt->pcm_voice_register_route_proven &&
        sound_cpu_trace && sound_cpu_trace->scsp_voice_register_write_count > 0U;
    receipt.runtime_corridor_proven =
        receipt.source_identities_bound &&
        receipt.capture_session_bound &&
        receipt.producer_command_observed &&
        receipt.sound_cpu_command_observed &&
        receipt.driver_command_handler_observed &&
        receipt.pcm_voice_register_route_observed &&
        sound_cpu_trace->intra_trace_observation_order_proven;
    receipt.valid = receipt.runtime_corridor_proven;
    *out_receipt = receipt;
    return receipt.valid;
}
