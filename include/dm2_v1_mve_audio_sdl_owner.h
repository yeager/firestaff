#ifndef FIRESTAFF_DM2_V1_MVE_AUDIO_SDL_OWNER_H
#define FIRESTAFF_DM2_V1_MVE_AUDIO_SDL_OWNER_H

#include "dm2_v1_mve_pcm.h"

#include <stdint.h>

/* SDL3 sink for the verified PC-DOS MVE PCM transport.  The MVE decoder has
 * already established the only admitted format: unsigned 8-bit, stereo,
 * 22050 Hz.  This owner neither resamples nor mixes nor invents prebuffer
 * samples; it gives those original bytes directly to an SDL stream requested
 * in that exact format.  It is deliberately independent of the visual MVE
 * route so callers retain control of the original presentation clock. */
#define DM2_V1_MVE_AUDIO_SAMPLE_RATE 22050u
#define DM2_V1_MVE_AUDIO_CHANNELS 2u
#define DM2_V1_MVE_AUDIO_BITS 8u

typedef struct {
    void *sdl_stream;
    uint64_t queued_source_bytes;
    uint64_t queued_sample_frames;
    uint32_t queued_source_packets;
    uint16_t next_source_sequence;
    int have_source_sequence;
    int initialized;
    int owns_audio_subsystem;
    /* No host output device was available.  Source PCM is still admitted in
     * order so a missing device cannot suppress the retail video/menu; no
     * substitute samples, mixer or resampler are introduced. */
    int output_unavailable;
} DM2_V1_MveAudioSdlOwner;

/* Opens an SDL stream in the original MVE PCM format when the host exposes a
 * playback device.  On a headless/no-device host it still opens the source
 * owner in explicit silent-delivery mode: callers can preserve exact video
 * timing and validate every PCM packet without inventing audio or changing
 * its format. */
int dm2_v1_mve_audio_sdl_owner_open(DM2_V1_MveAudioSdlOwner *owner);

/* Delivers exactly one previously validated source packet.  The packet must
 * retain MVE's contiguous sequence and native U8/stereo/22050 shape.  With
 * no device, it is consumed only by the source-order receipt; this is
 * explicit silence, not generated or substituted audio. */
int dm2_v1_mve_audio_sdl_owner_queue(DM2_V1_MveAudioSdlOwner *owner,
                                     const DM2_V1_MvePcmFrame *frame);

/* Destroy only the stream/subsystem reference opened above. */
void dm2_v1_mve_audio_sdl_owner_close(DM2_V1_MveAudioSdlOwner *owner);

#endif /* FIRESTAFF_DM2_V1_MVE_AUDIO_SDL_OWNER_H */
