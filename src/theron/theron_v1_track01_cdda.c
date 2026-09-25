#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef FIRESTAFF_NO_SDL_AUDIO
#include <SDL3/SDL.h>
#define THERON_HAVE_SDL_AUDIO 1
#else
#define THERON_HAVE_SDL_AUDIO 0
#endif

#if defined(FIRESTAFF_HAVE_VORBISFILE)
#include <vorbis/vorbisfile.h>
#define THERON_HAVE_VORBISFILE 1
#else
#define THERON_HAVE_VORBISFILE 0
#endif

#if THERON_HAVE_VORBISFILE
static size_t theron_cdda_memory_read(void *destination, size_t item_size,
                                      size_t item_count, void *source) {
    Theron_Track01CddaStream *stream = (Theron_Track01CddaStream *)source;
    size_t remaining, bytes, items;
    if (!stream || !destination || item_size == 0u ||
        stream->memory_audio_offset > stream->memory_audio_size) return 0u;
    remaining = stream->memory_audio_size - stream->memory_audio_offset;
    items = item_count > remaining / item_size
        ? remaining / item_size : item_count;
    bytes = items * item_size;
    if (bytes != 0u) {
        memcpy(destination,
               stream->memory_audio_bytes + stream->memory_audio_offset,
               bytes);
        stream->memory_audio_offset += bytes;
    }
    return items;
}

static int theron_cdda_memory_seek(void *source, ogg_int64_t offset,
                                   int origin) {
    Theron_Track01CddaStream *stream = (Theron_Track01CddaStream *)source;
    size_t base, target;
    uint64_t distance;
    if (!stream || stream->memory_audio_offset > stream->memory_audio_size)
        return -1;
    if (origin == SEEK_SET) base = 0u;
    else if (origin == SEEK_CUR) base = stream->memory_audio_offset;
    else if (origin == SEEK_END) base = stream->memory_audio_size;
    else return -1;
    if (offset < 0) {
        distance = (uint64_t)(-(offset + 1)) + 1u;
        if (distance > base) return -1;
        target = base - (size_t)distance;
    } else {
        distance = (uint64_t)offset;
        if (distance > stream->memory_audio_size - base) return -1;
        target = base + (size_t)distance;
    }
    stream->memory_audio_offset = target;
    return 0;
}

static int theron_cdda_memory_close(void *source) {
    (void)source;
    return 0;
}

static long theron_cdda_memory_tell(void *source) {
    Theron_Track01CddaStream *stream = (Theron_Track01CddaStream *)source;
    if (!stream || stream->memory_audio_offset > (size_t)LONG_MAX) return -1L;
    return (long)stream->memory_audio_offset;
}
#endif

int theron_v1_track01_cdda_stream_start(
    const Theron_Track01CddaHandoff *handoff,
    Theron_Track01CddaStream *out_stream) {
#if THERON_HAVE_SDL_AUDIO
    FILE *audio_file;
    SDL_AudioSpec spec;
    SDL_AudioStream *sdl_stream;

    if (!handoff || !out_stream || handoff->status != THERON_TRACK01_CDDA_AVAILABLE ||
        !handoff->original_cdda || !handoff->playback_handoff_ready ||
        (!handoff->audio_is_vorbis &&
         (handoff->audio_sector_count == 0u ||
          handoff->audio_start_byte > handoff->audio_file_bytes ||
          handoff->audio_file_bytes - handoff->audio_start_byte !=
              handoff->audio_sector_count * THERON_TRACK01_CDDA_SECTOR_BYTES))) {
        return 0;
    }
    memset(out_stream, 0, sizeof(*out_stream));
    audio_file = fopen(handoff->audio_path, "rb");
    if (!audio_file || (!handoff->audio_is_vorbis &&
                        fseek(audio_file, (long)handoff->audio_start_byte, SEEK_SET) != 0)) {
        if (audio_file) fclose(audio_file);
        return 0;
    }
#if !THERON_HAVE_VORBISFILE
    if (handoff->audio_is_vorbis) {
        fclose(audio_file);
        return 0;
    }
#endif
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        fclose(audio_file);
        return 0;
    }
    spec.format = SDL_AUDIO_S16LE;
    spec.channels = THERON_TRACK01_CDDA_CHANNELS;
    spec.freq = THERON_TRACK01_CDDA_SAMPLE_RATE;
    sdl_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                           &spec, NULL, NULL);
    if (!sdl_stream) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        fclose(audio_file);
        return 0;
    }
#if THERON_HAVE_VORBISFILE
    if (handoff->audio_is_vorbis) {
        OggVorbis_File *vorbis = (OggVorbis_File *)calloc(1u, sizeof(*vorbis));
        vorbis_info *info;
        if (!vorbis || ov_open_callbacks(audio_file, vorbis, NULL, 0u,
                                         OV_CALLBACKS_DEFAULT) != 0) {
            free(vorbis);
            SDL_DestroyAudioStream(sdl_stream);
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            fclose(audio_file);
            return 0;
        }
        info = ov_info(vorbis, -1);
        if (!info || info->rate != THERON_TRACK01_CDDA_SAMPLE_RATE ||
            info->channels != THERON_TRACK01_CDDA_CHANNELS) {
            /* ov_clear() releases the decoder's internals (and the FILE* it
             * took ownership of), but not the OggVorbis_File allocation
             * itself -- matching the stop path, which does ov_clear then
             * free. Without this the whole struct leaked on every title
             * start whose Track 01 OGG was not 44100 Hz stereo. */
            ov_clear(vorbis);
            free(vorbis);
            SDL_DestroyAudioStream(sdl_stream);
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return 0;
        }
        out_stream->audio_file = vorbis;
    } else {
        out_stream->audio_file = audio_file;
    }
#else
    out_stream->audio_file = audio_file;
#endif
    SDL_ResumeAudioStreamDevice(sdl_stream);
    out_stream->sdl_stream = sdl_stream;
    out_stream->audio_start_byte = handoff->audio_start_byte;
    out_stream->audio_sector_count = handoff->audio_sector_count;
    out_stream->audio_is_vorbis = handoff->audio_is_vorbis;
    out_stream->output_started = 1;
    return 1;
#else
    (void)handoff;
    (void)out_stream;
    return 0;
#endif
}

int theron_v1_track01_cdda_stream_start_memory(
    const Theron_Track01CddaHandoff *handoff,
    const uint8_t *audio_bytes,
    size_t audio_size,
    Theron_Track01CddaStream *out_stream) {
#if THERON_HAVE_SDL_AUDIO && THERON_HAVE_VORBISFILE
    OggVorbis_File *vorbis;
    vorbis_info *info;
    ov_callbacks callbacks;
    SDL_AudioSpec spec;
    SDL_AudioStream *sdl_stream;

    if (!handoff || !out_stream || !audio_bytes || audio_size < 4u ||
        handoff->status != THERON_TRACK01_CDDA_AVAILABLE ||
        !handoff->original_cdda || !handoff->playback_handoff_ready ||
        !handoff->audio_is_vorbis || handoff->audio_file_bytes != audio_size ||
        memcmp(audio_bytes, "OggS", 4u) != 0) return 0;
    memset(out_stream, 0, sizeof(*out_stream));
    out_stream->memory_audio_bytes = audio_bytes;
    out_stream->memory_audio_size = audio_size;
    vorbis = (OggVorbis_File *)calloc(1u, sizeof(*vorbis));
    if (!vorbis) return 0;
    callbacks.read_func = theron_cdda_memory_read;
    callbacks.seek_func = theron_cdda_memory_seek;
    callbacks.close_func = theron_cdda_memory_close;
    callbacks.tell_func = theron_cdda_memory_tell;
    if (ov_open_callbacks(out_stream, vorbis, NULL, 0u, callbacks) != 0) {
        free(vorbis);
        memset(out_stream, 0, sizeof(*out_stream));
        return 0;
    }
    info = ov_info(vorbis, -1);
    if (!info || info->rate != THERON_TRACK01_CDDA_SAMPLE_RATE ||
        info->channels != THERON_TRACK01_CDDA_CHANNELS) {
        ov_clear(vorbis);
        free(vorbis);
        memset(out_stream, 0, sizeof(*out_stream));
        return 0;
    }
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        ov_clear(vorbis);
        free(vorbis);
        memset(out_stream, 0, sizeof(*out_stream));
        return 0;
    }
    spec.format = SDL_AUDIO_S16LE;
    spec.channels = THERON_TRACK01_CDDA_CHANNELS;
    spec.freq = THERON_TRACK01_CDDA_SAMPLE_RATE;
    sdl_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                           &spec, NULL, NULL);
    if (!sdl_stream) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        ov_clear(vorbis);
        free(vorbis);
        memset(out_stream, 0, sizeof(*out_stream));
        return 0;
    }
    out_stream->audio_file = vorbis;
    out_stream->sdl_stream = sdl_stream;
    out_stream->audio_is_vorbis = 1;
    out_stream->output_started = 1;
    SDL_ResumeAudioStreamDevice(sdl_stream);
    return 1;
#else
    (void)handoff;
    (void)audio_bytes;
    (void)audio_size;
    (void)out_stream;
    return 0;
#endif
}

int theron_v1_track01_cdda_stream_pump(Theron_Track01CddaStream *stream) {
#if THERON_HAVE_SDL_AUDIO
#if THERON_HAVE_VORBISFILE
    if (stream && stream->audio_is_vorbis) {
        uint8_t pcm[THERON_TRACK01_CDDA_SECTOR_BYTES];
        int queued_bytes;
        if (!stream->output_started || !stream->sdl_stream || !stream->audio_file) return 0;
        queued_bytes = SDL_GetAudioStreamQueued((SDL_AudioStream *)stream->sdl_stream);
        while (queued_bytes < (int)(THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS *
                                    THERON_TRACK01_CDDA_SECTOR_BYTES)) {
            int bitstream = 0;
            long decoded = ov_read((OggVorbis_File *)stream->audio_file,
                                   (char *)pcm, (int)sizeof(pcm), 0, 2, 1,
                                   &bitstream);
            (void)bitstream;
            if (decoded == 0) {
                if (ov_pcm_seek((OggVorbis_File *)stream->audio_file, 0) != 0) {
                    return 0;
                }
                ++stream->loop_count;
                continue;
            }
            if (decoded < 0 ||
                !SDL_PutAudioStreamData((SDL_AudioStream *)stream->sdl_stream,
                                        pcm, (int)decoded)) {
                return 0;
            }
            queued_bytes += (int)decoded;
            ++stream->sectors_queued;
        }
        return 1;
    }
#endif
    uint8_t sector[THERON_TRACK01_CDDA_SECTOR_BYTES];
    size_t queued_bytes;
    size_t queued_sectors;

    if (!stream || !stream->output_started || !stream->audio_file ||
        !stream->sdl_stream || stream->audio_sector_count == 0u) {
        return 0;
    }
    queued_bytes = (size_t)SDL_GetAudioStreamQueued(
        (SDL_AudioStream *)stream->sdl_stream);
    queued_sectors = queued_bytes / THERON_TRACK01_CDDA_SECTOR_BYTES;
    while (queued_sectors < THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS) {
        if (stream->sectors_read == stream->audio_sector_count) {
            if (fseek((FILE *)stream->audio_file,
                      (long)stream->audio_start_byte,
                      SEEK_SET) != 0) {
                return 0;
            }
            stream->sectors_read = 0u;
            ++stream->loop_count;
        }
        if (fread(sector, 1u, sizeof(sector), (FILE *)stream->audio_file) != sizeof(sector) ||
            !SDL_PutAudioStreamData((SDL_AudioStream *)stream->sdl_stream,
                                    sector, (int)sizeof(sector))) {
            return 0;
        }
        ++stream->sectors_read;
        ++stream->sectors_queued;
        ++queued_sectors;
    }
    return 1;
#else
    (void)stream;
    return 0;
#endif
}

void theron_v1_track01_cdda_stream_stop(Theron_Track01CddaStream *stream) {
    if (!stream) return;
#if THERON_HAVE_SDL_AUDIO
    if (stream->sdl_stream) {
        SDL_DestroyAudioStream((SDL_AudioStream *)stream->sdl_stream);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
#if THERON_HAVE_VORBISFILE
    if (stream->audio_is_vorbis) {
        if (stream->audio_file) {
            ov_clear((OggVorbis_File *)stream->audio_file);
            free(stream->audio_file);
        }
    } else
#endif
    if (stream->audio_file) fclose((FILE *)stream->audio_file);
#endif
    memset(stream, 0, sizeof(*stream));
}

int theron_v1_track01_cdda_lifecycle_update(
    const Theron_Track01CddaHandoff *handoff,
    int title_active,
    Theron_Track01CddaStream *stream) {
    if (!stream) {
        return 0;
    }
    if (!title_active) {
        theron_v1_track01_cdda_stream_stop(stream);
        return 1;
    }
    if (!handoff || handoff->status != THERON_TRACK01_CDDA_AVAILABLE ||
        !handoff->original_cdda || !handoff->playback_handoff_ready) {
        theron_v1_track01_cdda_stream_stop(stream);
        return 0;
    }
    if (!stream->output_started &&
        !theron_v1_track01_cdda_stream_start(handoff, stream)) {
        return 0;
    }
    return theron_v1_track01_cdda_stream_pump(stream);
}
