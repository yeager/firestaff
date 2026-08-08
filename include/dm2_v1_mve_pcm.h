#ifndef FIRESTAFF_DM2_V1_MVE_PCM_H
#define FIRESTAFF_DM2_V1_MVE_PCM_H

#include "dm2_v1_mve_stream.h"

#include <stddef.h>
#include <stdint.h>

/* The admitted PC-DOS INTRO/END streams initialise version-0 MVE audio as
 * 22,050 Hz, 8-bit, stereo PCM.  A frame is a six-byte source header followed
 * by caller-owned PCM bytes; this owner never writes an extracted sound file. */
typedef struct {
    int initialized;
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t bits;
    uint16_t next_source_sequence;
    int have_source_sequence;
    uint32_t decoded_frame_count;
} DM2_V1_MvePcm;

typedef struct {
    int valid;
    const uint8_t *samples;
    uint32_t sample_bytes;
    uint32_t sample_frames;
    uint16_t source_sequence;
    uint16_t source_stream_mask;
} DM2_V1_MvePcmFrame;

typedef struct {
    int valid;
    uint32_t data_offset;
    uint16_t data_size;
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t bits;
    uint8_t compressed;
    uint16_t source_sequence;
    uint16_t source_stream_mask;
} DM2_V1_MvePcmSourceFrame;

typedef struct {
    const uint8_t *bytes;
    size_t byte_count;
    size_t offset;
    size_t chunk_end;
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t bits;
    uint8_t compressed;
    uint32_t audio_frame_count;
    int initialized;
    int audio_initialized;
    int ended;
} DM2_V1_MveAudioIterator;

void dm2_v1_mve_pcm_init(DM2_V1_MvePcm *pcm);

/* Validates and exposes one original uncompressed PCM frame.  It accepts no
 * inferred format, generated silence or compressed substitute. */
int dm2_v1_mve_pcm_decode_presentation(
    DM2_V1_MvePcm *pcm, const DM2_V1_MvePresentation *presentation,
    const uint8_t *movie_bytes, size_t movie_byte_count,
    DM2_V1_MvePcmFrame *out);

/* Enumerates every original opcode-0x08 audio frame, including the source
 * prebuffer that precedes the first image presentation. */
int dm2_v1_mve_audio_iterator_init(DM2_V1_MveAudioIterator *iterator,
                                   const uint8_t *movie_bytes,
                                   size_t movie_byte_count);
int dm2_v1_mve_audio_iterator_next(DM2_V1_MveAudioIterator *iterator,
                                   DM2_V1_MvePcmSourceFrame *out);
int dm2_v1_mve_pcm_decode_source_frame(
    DM2_V1_MvePcm *pcm, const DM2_V1_MvePcmSourceFrame *source,
    const uint8_t *movie_bytes, size_t movie_byte_count,
    DM2_V1_MvePcmFrame *out);

#endif /* FIRESTAFF_DM2_V1_MVE_PCM_H */
