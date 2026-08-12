#ifndef DM2_V1_MAC_MOVIE_DECODER_H
#define DM2_V1_MAC_MOVIE_DECODER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    void *opaque;
    uint8_t pixels[320u * 200u];
    uint8_t palette_rgb6[256u][3u];
    uint32_t frame_index;
    uint64_t presentation_time_us;
    uint64_t frame_duration_us;
    int width;
    int height;
    int frame_ready;
    int ended;
    int rejected;
    const int16_t *audio_samples;
    int audio_sample_count;
    int audio_rate_hz;
} DM2_V1_MacMovieDecoder;

/* Decode an authentic in-memory QuickTime view. No temporary movie file is
 * created. The output is a source-sized indexed presentation page and a
 * per-frame RGB332 palette suitable for Firestaff's indexed renderer. */
int dm2_v1_mac_movie_decoder_open(DM2_V1_MacMovieDecoder *decoder,
                                  const uint8_t *movie_bytes,
                                  size_t movie_size);
int dm2_v1_mac_movie_decoder_next(DM2_V1_MacMovieDecoder *decoder);
int dm2_v1_mac_movie_decoder_take_audio(DM2_V1_MacMovieDecoder *decoder,
                                        const int16_t **samples,
                                        int *sample_count,
                                        int *rate_hz);
void dm2_v1_mac_movie_decoder_close(DM2_V1_MacMovieDecoder *decoder);

#endif
