#ifndef DM2_V1_MAC_QUICKTIME_H
#define DM2_V1_MAC_QUICKTIME_H

#include <stddef.h>
#include <stdint.h>

/* Read-only QuickTime atom and sample-table admission for the Macintosh
 * retail MooV views.  This is deliberately a container reader only: callers
 * receive exact spans into their supplied in-memory view, never extracted
 * files or a guessed frame layout. */
typedef struct {
    uint32_t codec_fourcc;
    uint16_t width;
    uint16_t height;
    uint32_t time_scale;
    uint32_t sample_count;
    uint32_t first_sample_duration;
    const uint8_t *sample_data;
    size_t sample_size;
} DM2_V1_MacQuickTimeSample;

typedef struct {
    uint32_t video_codec_fourcc;
    uint32_t audio_codec_fourcc;
    uint16_t width;
    uint16_t height;
    uint16_t audio_channels;
    uint16_t audio_sample_size_bits;
    uint32_t audio_sample_rate_hz;
    uint32_t video_sample_count;
    uint32_t audio_sample_count;
    uint32_t video_time_scale;
    uint32_t audio_time_scale;
    uint32_t video_first_sample_duration;
    uint32_t audio_first_sample_duration;
} DM2_V1_MacQuickTimeInfo;

/* Validates every atom/table range and maps both tracks to the original view.
 * `sample_index` is zero based.  Returns 1 only when the requested source
 * sample is fully within `movie_bytes`; 0 is a malformed/unsupported view. */
int dm2_v1_mac_quicktime_inspect(const uint8_t *movie_bytes, size_t movie_size,
                                 DM2_V1_MacQuickTimeInfo *out_info);
int dm2_v1_mac_quicktime_video_sample(const uint8_t *movie_bytes, size_t movie_size,
                                      uint32_t sample_index,
                                      DM2_V1_MacQuickTimeSample *out_sample);
int dm2_v1_mac_quicktime_audio_sample(const uint8_t *movie_bytes, size_t movie_size,
                                      uint32_t sample_index,
                                      DM2_V1_MacQuickTimeSample *out_sample);

#endif
