#ifndef NEXUS_V1_SCSP_TRACE_H
#define NEXUS_V1_SCSP_TRACE_H

#include <stddef.h>
#include <stdint.h>

#define NEXUS_V1_SCSP_WRITE_TRACE_MAGIC \
    "FIRESTAFF_NEXUS_SCSP_WRITE_TRACE_V1"

/* Bounded receipt for the supported Mednafen sound-CPU write trace. It
 * records exactly what the trace contains; it never assigns a gameplay event
 * to a mailbox byte or treats a register write as decoded SAL audio. */
typedef struct {
    int valid;
    int header_valid;
    int parse_complete;
    int sound_cpu_trace;
    size_t raw_trace_byte_count;
    uint64_t raw_trace_fnv1a64;
    uint32_t record_count;
    uint32_t mailbox_write_count;
    uint32_t mailbox_nonzero_count;
    uint32_t mailbox_value_02_count;
    uint32_t command_handler_pc_3224_count;
    uint32_t scsp_voice_register_write_count;
    int mailbox_command_observed;
    int driver_command_handler_observed;
    int event_selector_semantics_proven;
    int sal_codec_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
} Nexus_V1_ScspTraceReceipt;

/* Parse one complete sound-CPU trace. All records must use the exact
 * addr/size/value/pc schema emitted by scripts/mednafen_1.32.1_nexus_slev_scsp_trace.patch. */
int nexus_v1_scsp_write_trace_parse(
    const uint8_t *raw_trace,
    size_t raw_trace_size,
    Nexus_V1_ScspTraceReceipt *out_receipt);

#endif
