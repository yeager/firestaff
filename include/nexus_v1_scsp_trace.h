#ifndef NEXUS_V1_SCSP_TRACE_H
#define NEXUS_V1_SCSP_TRACE_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_audio_receipt.h"

#define NEXUS_V1_SCSP_WRITE_TRACE_MAGIC \
    "FIRESTAFF_NEXUS_SCSP_WRITE_TRACE_V1"
#define NEXUS_V1_MAIN_SCSP_WRITE_TRACE_MAGIC \
    "FIRESTAFF_NEXUS_MAIN_SCSP_WRITE_TRACE_V1"
#define NEXUS_V1_SCSP_CAPTURE_SESSION_MAX 64U

/* Bounded receipt for the supported Mednafen sound-CPU write trace. It
 * records exactly what the trace contains; it never assigns a gameplay event
 * to a mailbox byte or treats a register write as decoded SAL audio. */
typedef struct {
    int valid;
    int header_valid;
    int capture_session_present;
    char capture_session[NEXUS_V1_SCSP_CAPTURE_SESSION_MAX];
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
    size_t first_mailbox_raw_offset;
    size_t first_handler_raw_offset;
    size_t first_voice_raw_offset;
    int intra_trace_observation_order_proven;
    int event_selector_semantics_proven;
    int sal_codec_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
} Nexus_V1_ScspTraceReceipt;

/* The main SH-2 producer is a separate trace stream. Keep its mailbox facts
 * separate from the sound-CPU trace so a producer write is never confused
 * with SDDRVS ownership or a decoded SFX event. */
typedef struct {
    int valid;
    int header_valid;
    int capture_session_present;
    char capture_session[NEXUS_V1_SCSP_CAPTURE_SESSION_MAX];
    int parse_complete;
    size_t raw_trace_byte_count;
    uint64_t raw_trace_fnv1a64;
    uint32_t record_count;
    uint32_t mailbox_write_count;
    uint32_t mailbox_nonzero_count;
    uint32_t mailbox_value_02_count;
    uint32_t mailbox_value_0200_count;
    int producer_command_observed;
    size_t first_producer_command_raw_offset;
    int blocks_real_sfx_playback;
} Nexus_V1_MainScspTraceReceipt;

/* Join receipt for one authenticated producer → SDDRVS → SCSP corridor.
 * This is the narrowest runtime fact that the current capture proves: a
 * producer command reaches the authenticated 68k driver and the driver
 * writes the SCSP voice-register family. It does not identify a gameplay
 * event, MAP selector, SAL codec, sample rate, or host playback operation. */
typedef struct {
    int valid;
    int source_identities_bound;
    int capture_session_bound;
    int producer_command_observed;
    int sound_cpu_command_observed;
    int driver_command_handler_observed;
    int pcm_voice_register_route_observed;
    int runtime_corridor_proven;
    int event_selector_semantics_proven;
    int sal_codec_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
} Nexus_V1_ScspRuntimeJoinReceipt;

/* Parse one complete sound-CPU trace. All records must use the exact
 * addr/size/value/pc schema emitted by scripts/mednafen_1.32.1_nexus_slev_scsp_trace.patch. */
int nexus_v1_scsp_write_trace_parse(
    const uint8_t *raw_trace,
    size_t raw_trace_size,
    Nexus_V1_ScspTraceReceipt *out_receipt);

int nexus_v1_main_scsp_write_trace_parse(
    const uint8_t *raw_trace,
    size_t raw_trace_size,
    Nexus_V1_MainScspTraceReceipt *out_receipt);

/* Bind independently authenticated traces and the source-owned SDDRVS
 * disassembly. The four identity arguments are supplied by the caller's
 * hash-verified SLEV/SAL/MAP/SDDRVS admission route; a trace alone cannot
 * establish those identities. Returns non-zero only for a complete,
 * source-bound runtime corridor. Playback remains explicitly blocked. */
int nexus_v1_scsp_runtime_join(
    const Nexus_V1_ScspTraceReceipt *sound_cpu_trace,
    const Nexus_V1_MainScspTraceReceipt *main_cpu_trace,
    const Nexus_V1_SddrvsDisassemblyReceipt *driver_receipt,
    int slev_source_verified,
    int sal_source_verified,
    int map_source_verified,
    int driver_source_verified,
    Nexus_V1_ScspRuntimeJoinReceipt *out_receipt);

#endif
