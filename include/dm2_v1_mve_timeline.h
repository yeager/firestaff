#ifndef FIRESTAFF_DM2_V1_MVE_TIMELINE_H
#define FIRESTAFF_DM2_V1_MVE_TIMELINE_H

#include <stddef.h>
#include <stdint.h>

/* A source-order boundary, not a host playback schedule.  The MVE timer
 * owns display time.  Opcode-0x08 PCM packets are retained exactly where the
 * retail stream placed them: before the following opcode-0x07 display.
 * Nothing here invents a PCM timestamp from packet duration or host latency. */
typedef struct {
    int valid;
    uint32_t presentation_index;
    uint64_t presentation_time_us;
    uint32_t first_audio_source_sequence;
    uint32_t audio_packet_count;
    uint64_t audio_sample_bytes;
    uint64_t audio_sample_frames;
} DM2_V1_MveDisplayAudioBoundary;

typedef struct {
    int valid;
    uint32_t timer_rate_us;
    uint16_t timer_subdivision;
    uint32_t presentation_count;
    uint32_t audio_packet_count;
    uint32_t prebuffer_audio_packet_count;
    uint32_t terminal_silent_presentation_count;
    uint32_t unpresented_audio_packet_count;
    uint64_t audio_sample_bytes;
    uint64_t audio_sample_frames;
} DM2_V1_MveTimelineReceipt;

/* Builds an exact byte-order join between opcode-0x08 PCM packets and
 * opcode-0x07 displays from a verified PC-DOS MVE member.  The caller first
 * calls with NULL/0 to obtain presentation_count, then supplies exactly that
 * many boundary slots.  The function is memory-only and rejects malformed,
 * compressed, non-stereo-U8 or non-22,050 Hz source audio. */
int dm2_v1_mve_display_audio_timeline(
    const uint8_t *movie_bytes, size_t movie_byte_count,
    DM2_V1_MveDisplayAudioBoundary *boundaries, size_t boundary_capacity,
    DM2_V1_MveTimelineReceipt *out);

#endif /* FIRESTAFF_DM2_V1_MVE_TIMELINE_H */
