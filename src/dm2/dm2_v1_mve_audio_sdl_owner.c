#include "dm2_v1_mve_audio_sdl_owner.h"

#include <limits.h>
#include <string.h>

#include <SDL3/SDL.h>

int dm2_v1_mve_audio_sdl_owner_open(DM2_V1_MveAudioSdlOwner *owner)
{
    SDL_AudioSpec spec;
    SDL_AudioStream *stream;

    if (!owner) return 0;
    memset(owner, 0, sizeof(*owner));
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) return 0;
    owner->owns_audio_subsystem = 1;
    memset(&spec, 0, sizeof(spec));
    spec.format = SDL_AUDIO_U8;
    spec.channels = DM2_V1_MVE_AUDIO_CHANNELS;
    spec.freq = (int)DM2_V1_MVE_AUDIO_SAMPLE_RATE;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                       &spec, NULL, NULL);
    if (!stream || !SDL_ResumeAudioStreamDevice(stream)) {
        if (stream) SDL_DestroyAudioStream(stream);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        memset(owner, 0, sizeof(*owner));
        return 0;
    }
    owner->sdl_stream = stream;
    owner->initialized = 1;
    return 1;
}

int dm2_v1_mve_audio_sdl_owner_queue(DM2_V1_MveAudioSdlOwner *owner,
                                     const DM2_V1_MvePcmFrame *frame)
{
    SDL_AudioStream *stream;

    if (!owner || !frame || !owner->initialized || !owner->sdl_stream ||
        !frame->valid || !frame->samples || frame->sample_bytes == 0u ||
        frame->sample_bytes > (uint32_t)INT_MAX ||
        (frame->sample_bytes & 1u) != 0u ||
        frame->sample_frames != frame->sample_bytes / DM2_V1_MVE_AUDIO_CHANNELS ||
        frame->source_stream_mask != 1u ||
        (owner->have_source_sequence &&
         frame->source_sequence != owner->next_source_sequence))
        return 0;

    stream = (SDL_AudioStream *)owner->sdl_stream;
    /* SDL receives the original unsigned stereo bytes unchanged.  Conversion
     * for a physical device, if its driver requires it, remains SDL's device
     * concern; Firestaff supplies no resampler or mixer on this path. */
    if (!SDL_PutAudioStreamData(stream, frame->samples, (int)frame->sample_bytes))
        return 0;

    owner->queued_source_bytes += frame->sample_bytes;
    owner->queued_sample_frames += frame->sample_frames;
    ++owner->queued_source_packets;
    owner->next_source_sequence = (uint16_t)(frame->source_sequence + 1u);
    owner->have_source_sequence = 1;
    return 1;
}

void dm2_v1_mve_audio_sdl_owner_close(DM2_V1_MveAudioSdlOwner *owner)
{
    if (!owner) return;
    if (owner->sdl_stream)
        SDL_DestroyAudioStream((SDL_AudioStream *)owner->sdl_stream);
    if (owner->owns_audio_subsystem)
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    memset(owner, 0, sizeof(*owner));
}
