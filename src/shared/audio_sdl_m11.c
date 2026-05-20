#include "audio_sdl_m11.h"
#include "graphics_dat_snd3_loader_v1.h"
#include "song_dat_loader_v1.h"
#include "sound_event_snd3_map_v1.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define M11_AUDIO_VOLUME_MIN 0
#define M11_AUDIO_VOLUME_MAX 128
#define M11_AUDIO_SOUND_PACK_PATH_CAPACITY 1024
#define M11_AUDIO_SOUND_PACK_MAX_SECONDS 10u
#define M11_AUDIO_SOUND_PACK_MAX_BYTES (64u * 1024u * 1024u)

/*
 * Guard: SDL3 headers are only included when we actually attempt real audio.
 * The build always links SDL3 via CMake, so this is safe.
 */
#ifdef FIRESTAFF_NO_SDL_AUDIO
#define M11_HAVE_SDL_AUDIO 0
#else
#include <SDL3/SDL.h>
#define M11_HAVE_SDL_AUDIO 1
#endif

/* ── helpers ─────────────────────────────────────────────────────── */

static int m11_clamp_volume(int value) {
    if (value < M11_AUDIO_VOLUME_MIN) return M11_AUDIO_VOLUME_MIN;
    if (value > M11_AUDIO_VOLUME_MAX) return M11_AUDIO_VOLUME_MAX;
    return value;
}

static void m11_sound_free(M11_SoundBuffer* buf) {
    if (!buf) return;
    free(buf->samples);
    buf->samples = NULL;
    buf->sampleCount = 0;
    buf->capacity = 0;
}

static int m11_sound_reserve(M11_SoundBuffer* buf, int count) {
    float* next;
    if (!buf || count < 0) return 0;
    if (count <= buf->capacity) return 1;
    next = (float*)realloc(buf->samples, (size_t)count * sizeof(float));
    if (!next) return 0;
    buf->samples = next;
    buf->capacity = count;
    return 1;
}

static void m11_sound_clear(M11_SoundBuffer* buf) {
    if (!buf) return;
    buf->sampleCount = 0;
}

/*
 * Generate a sine-wave tone with exponential decay.
 *   freq_hz   — base frequency
 *   dur_ms    — duration in milliseconds
 *   amplitude — peak amplitude (0.0 .. 1.0)
 *   decay     — exponential decay rate (higher = faster fade)
 */
static void m11_gen_tone(M11_SoundBuffer* buf,
                         float freq_hz,
                         int dur_ms,
                         float amplitude,
                         float decay) {
    int count;
    int i;
    if (!buf) return;
    count = (M11_AUDIO_SAMPLE_RATE * dur_ms) / 1000;
    if (count > M11_AUDIO_MAX_SAMPLES) count = M11_AUDIO_MAX_SAMPLES;
    if (!m11_sound_reserve(buf, count)) return;
    buf->sampleCount = count;
    for (i = 0; i < count; ++i) {
        float t = (float)i / (float)M11_AUDIO_SAMPLE_RATE;
        float env = amplitude * expf(-decay * t);
        buf->samples[i] = env * sinf(2.0f * (float)M_PI * freq_hz * t);
    }
}

/*
 * Generate a noise burst with exponential decay (for percussive sounds).
 */
static void m11_gen_noise(M11_SoundBuffer* buf,
                          int dur_ms,
                          float amplitude,
                          float decay) {
    int count;
    int i;
    unsigned int seed = 0xDEAD;
    if (!buf) return;
    count = (M11_AUDIO_SAMPLE_RATE * dur_ms) / 1000;
    if (count > M11_AUDIO_MAX_SAMPLES) count = M11_AUDIO_MAX_SAMPLES;
    if (!m11_sound_reserve(buf, count)) return;
    buf->sampleCount = count;
    for (i = 0; i < count; ++i) {
        float t = (float)i / (float)M11_AUDIO_SAMPLE_RATE;
        float env = amplitude * expf(-decay * t);
        /* Simple LCG pseudo-random */
        seed = seed * 1103515245u + 12345u;
        float noise = ((float)(int)(seed >> 16) / 32768.0f) - 1.0f;
        buf->samples[i] = env * noise;
    }
}

/*
 * Mix: add src into dst (additive blend, clamped later by SDL).
 * If dst is shorter, extends it.
 */
static void m11_mix_into(M11_SoundBuffer* dst, const M11_SoundBuffer* src) {
    int i;
    if (!dst || !src) return;
    if (src->sampleCount <= 0) return;
    if (!m11_sound_reserve(dst, src->sampleCount)) return;
    if (src->sampleCount > dst->sampleCount)
        dst->sampleCount = src->sampleCount;
    for (i = 0; i < src->sampleCount; ++i) {
        dst->samples[i] += src->samples[i];
    }
}

/*
 * Pre-generate all marker sounds.  These are short procedural effects
 * designed to be recognizable placeholders for original DM1 sounds.
 */
static void m11_generate_sounds(M11_AudioState* state) {
    M11_SoundBuffer tmp;

    /* FOOTSTEP: low thud — 80 Hz sine + noise, 120 ms, fast decay */
    m11_sound_clear(&state->sounds[M11_AUDIO_MARKER_FOOTSTEP]);
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_FOOTSTEP], 80.0f, 120, 0.5f, 12.0f);
    memset(&tmp, 0, sizeof(tmp));
    m11_gen_noise(&tmp, 60, 0.25f, 20.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_FOOTSTEP], &tmp);

    /* DOOR: creak — 200 Hz + 150 Hz beating, 200 ms */
    m11_sound_clear(&state->sounds[M11_AUDIO_MARKER_DOOR]);
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_DOOR], 200.0f, 200, 0.4f, 6.0f);
    m11_sound_clear(&tmp);
    m11_gen_tone(&tmp, 150.0f, 200, 0.3f, 6.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_DOOR], &tmp);

    /* COMBAT: sharp metallic hit — 400 Hz + noise, 80 ms, fast decay */
    m11_sound_clear(&state->sounds[M11_AUDIO_MARKER_COMBAT]);
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_COMBAT], 400.0f, 80, 0.6f, 18.0f);
    m11_sound_clear(&tmp);
    m11_gen_noise(&tmp, 40, 0.4f, 25.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_COMBAT], &tmp);

    /* CREATURE: low growl — 100 Hz + modulated noise, 150 ms */
    m11_sound_clear(&state->sounds[M11_AUDIO_MARKER_CREATURE]);
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_CREATURE], 100.0f, 150, 0.5f, 8.0f);
    m11_sound_clear(&tmp);
    m11_gen_noise(&tmp, 100, 0.3f, 10.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_CREATURE], &tmp);

    /* SPELL: shimmer — 800 Hz high sine, 250 ms, gentle decay */
    m11_sound_clear(&state->sounds[M11_AUDIO_MARKER_SPELL]);
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_SPELL], 800.0f, 250, 0.35f, 4.0f);
    m11_sound_clear(&tmp);
    m11_gen_tone(&tmp, 1200.0f, 200, 0.2f, 5.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_SPELL], &tmp);
    m11_sound_free(&tmp);
}

static int m11_file_exists(const char* path) {
    FILE* f;
    if (!path || !*path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static unsigned int m11_read_le16(const unsigned char* p) {
    return ((unsigned int)p[0]) | ((unsigned int)p[1] << 8);
}

static unsigned int m11_read_le32(const unsigned char* p) {
    return ((unsigned int)p[0]) |
           ((unsigned int)p[1] << 8) |
           ((unsigned int)p[2] << 16) |
           ((unsigned int)p[3] << 24);
}

static int m11_path_join(char* out, size_t outBytes, const char* dir, const char* name) {
    int n;
    size_t dirLen;
    if (!out || outBytes == 0 || !dir || !*dir || !name || !*name) return 0;
    dirLen = strlen(dir);
    n = snprintf(out, outBytes, "%s%s%s", dir, (dirLen > 0 && dir[dirLen - 1] == '/') ? "" : "/", name);
    return (n > 0 && (size_t)n < outBytes) ? 1 : 0;
}

static int m11_try_sound_pack_name(char* out,
                                   size_t outBytes,
                                   const char* dir,
                                   const char* name) {
    if (!m11_path_join(out, outBytes, dir, name)) return 0;
    return m11_file_exists(out);
}

static int m11_find_sound_pack_file(char* out,
                                    size_t outBytes,
                                    const char* dir,
                                    const V1_SoundEventSnd3MapEntry* entry) {
    char name[256];
    int n;
    if (!out || outBytes == 0 || !dir || !entry) return 0;

    n = snprintf(name, sizeof(name), "%02d.wav", entry->soundIndex);
    if (n > 0 && (size_t)n < sizeof(name) && m11_try_sound_pack_name(out, outBytes, dir, name)) return 1;
    n = snprintf(name, sizeof(name), "sound-%02d.wav", entry->soundIndex);
    if (n > 0 && (size_t)n < sizeof(name) && m11_try_sound_pack_name(out, outBytes, dir, name)) return 1;
    n = snprintf(name, sizeof(name), "sound_%02d.wav", entry->soundIndex);
    if (n > 0 && (size_t)n < sizeof(name) && m11_try_sound_pack_name(out, outBytes, dir, name)) return 1;
    n = snprintf(name, sizeof(name), "snd3-%03u.wav", entry->snd3ItemIndex);
    if (n > 0 && (size_t)n < sizeof(name) && m11_try_sound_pack_name(out, outBytes, dir, name)) return 1;
    n = snprintf(name, sizeof(name), "SND3_%03u.wav", entry->snd3ItemIndex);
    if (n > 0 && (size_t)n < sizeof(name) && m11_try_sound_pack_name(out, outBytes, dir, name)) return 1;
    n = snprintf(name, sizeof(name), "%s.wav", entry->macroName);
    if (n > 0 && (size_t)n < sizeof(name) && m11_try_sound_pack_name(out, outBytes, dir, name)) return 1;
    return 0;
}

static double m11_decode_wav_sample(const unsigned char* p,
                                    unsigned int audioFormat,
                                    unsigned int bitsPerSample) {
    if (audioFormat == 3 && bitsPerSample == 32) {
        float v;
        memcpy(&v, p, sizeof(v));
        if (v < -1.0f) v = -1.0f;
        if (v > 1.0f) v = 1.0f;
        return (double)v;
    }

    if (audioFormat != 1) return 0.0;
    switch (bitsPerSample) {
        case 8:
            return ((double)p[0] - 128.0) / 128.0;
        case 16: {
            int16_t v = (int16_t)m11_read_le16(p);
            return (double)v / 32768.0;
        }
        case 24: {
            int32_t v = (int32_t)((unsigned int)p[0] |
                                  ((unsigned int)p[1] << 8) |
                                  ((unsigned int)p[2] << 16));
            if (v & 0x00800000) v |= (int32_t)0xFF000000u;
            return (double)v / 8388608.0;
        }
        case 32: {
            int32_t v = (int32_t)m11_read_le32(p);
            return (double)v / 2147483648.0;
        }
        default:
            return 0.0;
    }
}

static double m11_decode_wav_frame(const unsigned char* data,
                                   unsigned int frameIndex,
                                   unsigned int channels,
                                   unsigned int bytesPerSample,
                                   unsigned int audioFormat,
                                   unsigned int bitsPerSample) {
    unsigned int ch;
    double mixed = 0.0;
    const unsigned char* frame = data + ((size_t)frameIndex * channels * bytesPerSample);
    for (ch = 0; ch < channels; ++ch) {
        mixed += m11_decode_wav_sample(frame + ((size_t)ch * bytesPerSample), audioFormat, bitsPerSample);
    }
    return mixed / (double)channels;
}

static int m11_load_wav_to_stream(M11_SoundBuffer* dst, const char* path) {
    FILE* f;
    long fileSize;
    unsigned char* bytes;
    unsigned int pos = 12;
    unsigned int fmtSeen = 0;
    unsigned int dataSeen = 0;
    unsigned int audioFormat = 0;
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    unsigned int bitsPerSample = 0;
    unsigned int blockAlign = 0;
    const unsigned char* data = NULL;
    unsigned int dataBytes = 0;
    unsigned int bytesPerSample;
    unsigned int frameCount;
    int outCount;
    int i;

    if (!dst || !path || !*path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    fileSize = ftell(f);
    if (fileSize < 44 || fileSize > (long)M11_AUDIO_SOUND_PACK_MAX_BYTES) { fclose(f); return 0; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }

    bytes = (unsigned char*)malloc((size_t)fileSize);
    if (!bytes) { fclose(f); return 0; }
    if (fread(bytes, 1, (size_t)fileSize, f) != (size_t)fileSize) {
        free(bytes);
        fclose(f);
        return 0;
    }
    fclose(f);

    if (memcmp(bytes, "RIFF", 4) != 0 || memcmp(bytes + 8, "WAVE", 4) != 0) {
        free(bytes);
        return 0;
    }

    while (pos + 8u <= (unsigned int)fileSize) {
        const unsigned char* chunk = bytes + pos;
        unsigned int chunkSize = m11_read_le32(chunk + 4);
        unsigned int payload = pos + 8u;
        if (chunkSize > (unsigned int)fileSize || payload > (unsigned int)fileSize - chunkSize) break;
        if (memcmp(chunk, "fmt ", 4) == 0 && chunkSize >= 16u) {
            audioFormat = m11_read_le16(bytes + payload);
            channels = m11_read_le16(bytes + payload + 2u);
            sampleRate = m11_read_le32(bytes + payload + 4u);
            blockAlign = m11_read_le16(bytes + payload + 12u);
            bitsPerSample = m11_read_le16(bytes + payload + 14u);
            fmtSeen = 1;
        } else if (memcmp(chunk, "data", 4) == 0) {
            data = bytes + payload;
            dataBytes = chunkSize;
            dataSeen = 1;
        }
        pos = payload + chunkSize + (chunkSize & 1u);
    }

    if (!fmtSeen || !dataSeen || !data ||
        (audioFormat != 1 && audioFormat != 3) ||
        (channels != 1u && channels != 2u) ||
        sampleRate < 8000u || sampleRate > 192000u ||
        (bitsPerSample != 8u && bitsPerSample != 16u && bitsPerSample != 24u && bitsPerSample != 32u)) {
        free(bytes);
        return 0;
    }
    if (audioFormat == 3 && bitsPerSample != 32u) {
        free(bytes);
        return 0;
    }
    bytesPerSample = bitsPerSample / 8u;
    if (blockAlign != channels * bytesPerSample || blockAlign == 0u) {
        free(bytes);
        return 0;
    }
    frameCount = dataBytes / blockAlign;
    if (frameCount == 0u || frameCount > sampleRate * M11_AUDIO_SOUND_PACK_MAX_SECONDS) {
        free(bytes);
        return 0;
    }

    outCount = (int)(((unsigned long long)frameCount * M11_AUDIO_SAMPLE_RATE + sampleRate - 1u) / sampleRate);
    if (outCount <= 0 || !m11_sound_reserve(dst, outCount)) {
        free(bytes);
        return 0;
    }
    for (i = 0; i < outCount; ++i) {
        double sourcePos = ((double)i * (double)sampleRate) / (double)M11_AUDIO_SAMPLE_RATE;
        unsigned int base = (unsigned int)sourcePos;
        double frac = sourcePos - (double)base;
        unsigned int next = base + 1u;
        double a;
        double b;
        if (base >= frameCount) base = frameCount - 1u;
        if (next >= frameCount) next = frameCount - 1u;
        a = m11_decode_wav_frame(data, base, channels, bytesPerSample, audioFormat, bitsPerSample);
        b = m11_decode_wav_frame(data, next, channels, bytesPerSample, audioFormat, bitsPerSample);
        dst->samples[i] = (float)(a + (b - a) * frac);
    }
    dst->sampleCount = outCount;
    free(bytes);
    return 1;
}

static int m11_sdl_audio_backend_enabled(void) {
    const char* value = getenv("FIRESTAFF_AUDIO_ENABLE_SDL");
    if (!value || value[0] == '\0') {
        return 0;
    }
    if (value[0] == '0' && value[1] == '\0') {
        return 0;
    }
    return 1;
}

static const char* m11_find_song_dat_path(char* homeBuf, size_t homeBufBytes) {
    const char* envPath = getenv("FIRESTAFF_SONG_DAT");
    const char* legacyEnvPath = getenv("SONG_DAT_PATH");
    const char* home;
    if (m11_file_exists(envPath)) return envPath;
    if (m11_file_exists(legacyEnvPath)) return legacyEnvPath;
    if (m11_file_exists("SONG.DAT")) return "SONG.DAT";
    home = getenv("HOME");
    if (home && homeBuf && homeBufBytes > 0) {
        int n = snprintf(homeBuf, homeBufBytes, "%s/.firestaff/data/SONG.DAT", home);
        if (n > 0 && (size_t)n < homeBufBytes && m11_file_exists(homeBuf)) return homeBuf;
        n = snprintf(homeBuf, homeBufBytes, "%s/.firestaff/data/dm1-multilingual/SONG.DAT", home);
        if (n > 0 && (size_t)n < homeBufBytes && m11_file_exists(homeBuf)) return homeBuf;
        n = snprintf(homeBuf, homeBufBytes, "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/SONG.DAT", home);
        if (n > 0 && (size_t)n < homeBufBytes && m11_file_exists(homeBuf)) return homeBuf;
        n = snprintf(homeBuf, homeBufBytes, "%s/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/SONG.DAT", home);
        if (n > 0 && (size_t)n < homeBufBytes && m11_file_exists(homeBuf)) return homeBuf;
    }
    if (m11_file_exists("/tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT")) {
        return "/tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT";
    }
    return NULL;
}

static const char* m11_find_graphics_dat_path(char* homeBuf, size_t homeBufBytes) {
    const char* envPath = getenv("FIRESTAFF_GRAPHICS_DAT");
    const char* home;
    if (m11_file_exists(envPath)) return envPath;
    if (m11_file_exists("GRAPHICS.DAT")) return "GRAPHICS.DAT";
    home = getenv("HOME");
    if (home && homeBuf && homeBufBytes > 0) {
        int n = snprintf(homeBuf, homeBufBytes, "%s/.firestaff/data/GRAPHICS.DAT", home);
        if (n > 0 && (size_t)n < homeBufBytes && m11_file_exists(homeBuf)) return homeBuf;
    }
    return NULL;
}

static int m11_resample_snd3_to_stream(M11_SoundBuffer* dst,
                                       const V1_GraphicsSnd3Buffer* src) {
    int outCount;
    int i;
    if (!dst || !src || !src->samples || src->decodedSampleCount == 0) return 0;
    outCount = (int)(((long long)src->decodedSampleCount * M11_AUDIO_SAMPLE_RATE +
                      (M11_AUDIO_SOURCE_SND3_SAMPLE_RATE - 1)) /
                     M11_AUDIO_SOURCE_SND3_SAMPLE_RATE);
    if (outCount <= 0) return 0;
    if (!m11_sound_reserve(dst, outCount)) return 0;
    for (i = 0; i < outCount; ++i) {
        double sourcePos = ((double)i * (double)M11_AUDIO_SOURCE_SND3_SAMPLE_RATE) /
                           (double)M11_AUDIO_SAMPLE_RATE;
        int base = (int)sourcePos;
        double frac = sourcePos - (double)base;
        int next = base + 1;
        double a;
        double b;
        if (base < 0) base = 0;
        if (base >= (int)src->decodedSampleCount) base = (int)src->decodedSampleCount - 1;
        if (next >= (int)src->decodedSampleCount) next = (int)src->decodedSampleCount - 1;
        a = ((double)src->samples[base] - 128.0) / 128.0;
        b = ((double)src->samples[next] - 128.0) / 128.0;
        dst->samples[i] = (float)(a + (b - a) * frac);
    }
    dst->sampleCount = outCount;
    return 1;
}

static int m11_resample_snd8_to_stream(M11_SoundBuffer* dst,
                                       const V1_SndBuffer* src) {
    int outCount;
    int i;
    if (!dst || !src || !src->samples || src->decodedSampleCount == 0) return 0;
    outCount = (int)(((long long)src->decodedSampleCount * M11_AUDIO_SAMPLE_RATE +
                      (M11_AUDIO_SOURCE_SND8_SAMPLE_RATE - 1)) /
                     M11_AUDIO_SOURCE_SND8_SAMPLE_RATE);
    if (outCount <= 0) return 0;
    if (!m11_sound_reserve(dst, outCount)) return 0;
    for (i = 0; i < outCount; ++i) {
        double sourcePos = ((double)i * (double)M11_AUDIO_SOURCE_SND8_SAMPLE_RATE) /
                           (double)M11_AUDIO_SAMPLE_RATE;
        int base = (int)sourcePos;
        double frac = sourcePos - (double)base;
        int next = base + 1;
        double a;
        double b;
        if (base < 0) base = 0;
        if (base >= (int)src->decodedSampleCount) base = (int)src->decodedSampleCount - 1;
        if (next >= (int)src->decodedSampleCount) next = (int)src->decodedSampleCount - 1;
        a = (double)src->samples[base] / 128.0;
        b = (double)src->samples[next] / 128.0;
        dst->samples[i] = (float)(a + (b - a) * frac);
    }
    dst->sampleCount = outCount;
    return 1;
}

static int m11_sound_append(M11_SoundBuffer* dst, const M11_SoundBuffer* src) {
    int oldCount;
    int i;
    if (!dst || !src || src->sampleCount <= 0) return 0;
    oldCount = dst->sampleCount;
    if (!m11_sound_reserve(dst, oldCount + src->sampleCount)) return 0;
    for (i = 0; i < src->sampleCount; ++i) {
        dst->samples[oldCount + i] = src->samples[i];
    }
    dst->sampleCount = oldCount + src->sampleCount;
    return 1;
}

static void m11_try_load_original_song(M11_AudioState* state) {
    char homePath[1024];
    const char* path;
    V1_SongManifest manifest;
    V1_SongSequence seq;
    V1_SndBuffer raw[V1_SONG_DAT_ITEM_COUNT];
    M11_SoundBuffer part[V1_SONG_DAT_ITEM_COUNT];
    char err[256];
    unsigned int i;
    int ok = 1;
    if (!state) return;
    if (getenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG")) return;
    path = m11_find_song_dat_path(homePath, sizeof(homePath));
    if (!path) return;
    if (!V1_Song_ParseManifest(path, &manifest, err, sizeof(err))) return;
    if (!V1_Song_DecodeSequence(path, &manifest, &seq, err, sizeof(err))) return;
    if (!seq.hasEndMarker || seq.wordCount == 0) return;

    memset(raw, 0, sizeof(raw));
    memset(part, 0, sizeof(part));
    for (i = V1_SONG_DAT_FIRST_SND8_INDEX; i <= V1_SONG_DAT_LAST_SND8_INDEX; ++i) {
        if (!V1_Song_DecodeSnd8(path, &manifest, i, &raw[i], err, sizeof(err)) ||
            raw[i].sampleRateHz != M11_AUDIO_SOURCE_SND8_SAMPLE_RATE ||
            !m11_resample_snd8_to_stream(&part[i], &raw[i])) {
            ok = 0;
            break;
        }
        state->originalSongPartCount += 1;
    }

    if (ok) {
        m11_sound_clear(&state->titleMusic);
        for (i = 0; i < seq.wordCount; ++i) {
            unsigned int word = seq.words[i];
            unsigned int itemIndex = word & 0x7FFFu;
            if (word & 0x8000u) {
                state->originalSongLoopTargetPart = (int)itemIndex;
                break;
            }
            if (itemIndex < V1_SONG_DAT_FIRST_SND8_INDEX ||
                itemIndex > V1_SONG_DAT_LAST_SND8_INDEX ||
                !m11_sound_append(&state->titleMusic, &part[itemIndex])) {
                ok = 0;
                break;
            }
            state->originalSongPlayablePartCount += 1;
        }
        state->originalSongSequenceWordCount = (int)seq.wordCount;
    }

    for (i = V1_SONG_DAT_FIRST_SND8_INDEX; i <= V1_SONG_DAT_LAST_SND8_INDEX; ++i) {
        V1_Song_FreeSndBuffer(&raw[i]);
        m11_sound_free(&part[i]);
    }

    if (ok && state->titleMusic.sampleCount > 0 &&
        state->originalSongPartCount == V1_SONG_DAT_MUSIC_PART_COUNT &&
        state->originalSongLoopTargetPart >= V1_SONG_DAT_FIRST_SND8_INDEX &&
        state->originalSongLoopTargetPart <= V1_SONG_DAT_LAST_SND8_INDEX) {
        state->originalSongAvailable = 1;
    } else {
        m11_sound_free(&state->titleMusic);
        state->originalSongAvailable = 0;
        state->originalSongPartCount = 0;
        state->originalSongSequenceWordCount = 0;
        state->originalSongPlayablePartCount = 0;
        state->originalSongLoopTargetPart = 0;
    }
}

static void m11_try_load_original_snd3(M11_AudioState* state) {
    char homePath[1024];
    const char* path;
    V1_GraphicsSnd3Manifest manifest;
    char err[256];
    unsigned int i;
    if (!state) return;
    if (getenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3")) return;
    path = m11_find_graphics_dat_path(homePath, sizeof(homePath));
    if (!path) return;
    if (!V1_GraphicsSnd3_ParseManifest(path, &manifest, err, sizeof(err))) return;
    for (i = 0; i < V1_DM_SOUND_EVENT_COUNT && i < M11_AUDIO_ORIGINAL_SOUND_COUNT; ++i) {
        const V1_SoundEventSnd3MapEntry* entry = V1_SoundEventSnd3_Find((int)i);
        V1_GraphicsSnd3Buffer raw;
        if (!entry) continue;
        memset(&raw, 0, sizeof(raw));
        if (V1_GraphicsSnd3_DecodeItem(path, &manifest, entry->snd3ItemIndex,
                                      &raw, err, sizeof(err))) {
            if (raw.sampleRateHz == M11_AUDIO_SOURCE_SND3_SAMPLE_RATE &&
                m11_resample_snd3_to_stream(&state->originalSounds[i], &raw)) {
                state->originalSnd3LoadedCount += 1;
            }
            V1_GraphicsSnd3_FreeBuffer(&raw);
        }
    }
    state->originalSnd3Available = (state->originalSnd3LoadedCount == M11_AUDIO_ORIGINAL_SOUND_COUNT) ? 1 : 0;
}

static void m11_try_load_sound_pack(M11_AudioState* state) {
    const char* dir;
    unsigned int i;
    if (!state) return;
    if (getenv("FIRESTAFF_AUDIO_DISABLE_SOUND_PACK")) return;
    dir = getenv("FIRESTAFF_SOUND_PACK_DIR");
    if (!dir || !*dir) return;

    if (!state->originalSnd3Available) {
        for (i = 0; i < M11_AUDIO_ORIGINAL_SOUND_COUNT; ++i) {
            m11_sound_free(&state->originalSounds[i]);
        }
        state->originalSnd3LoadedCount = 0;
    }

    for (i = 0; i < V1_DM_SOUND_EVENT_COUNT && i < M11_AUDIO_ORIGINAL_SOUND_COUNT; ++i) {
        const V1_SoundEventSnd3MapEntry* entry = V1_SoundEventSnd3_Find((int)i);
        char path[M11_AUDIO_SOUND_PACK_PATH_CAPACITY];
        M11_SoundBuffer replacement;
        if (!entry || !m11_find_sound_pack_file(path, sizeof(path), dir, entry)) continue;
        memset(&replacement, 0, sizeof(replacement));
        if (m11_load_wav_to_stream(&replacement, path)) {
            m11_sound_free(&state->originalSounds[i]);
            state->originalSounds[i] = replacement;
            state->soundPackLoadedCount += 1;
        } else {
            m11_sound_free(&replacement);
        }
    }
    state->soundPackAvailable = state->soundPackLoadedCount > 0 ? 1 : 0;
}

/* ── public API ──────────────────────────────────────────────────── */

int M11_Audio_Init(M11_AudioState* state) {
    if (!state) return 0;
    memset(state, 0, sizeof(*state));
    state->initialized = 1;
    state->masterVolume = M11_AUDIO_VOLUME_MAX;
    state->sfxVolume    = M11_AUDIO_VOLUME_MAX;
    state->musicVolume  = M11_AUDIO_VOLUME_MAX;
    state->uiVolume     = M11_AUDIO_VOLUME_MAX;
    state->titleMusicEnabled = 1;
    state->lastMarker   = M11_AUDIO_MARKER_NONE;
    state->lastSoundIndex = -1;
    state->sdlStream    = NULL;

    /* Pre-generate procedural sounds regardless of backend */
    m11_generate_sounds(state);
    /* Pass 54: opportunistically load SONG.DAT title music.  Source SND8
     * buffers are signed 8-bit mono at 11025 Hz; they are linearly resampled
     * once at init to the fixed 22050 Hz SDL float stream and concatenated
     * according to the SEQ2 words up to (not including) the bit-15 loop-back
     * marker.  The loop target is recorded; continuous loop scheduling is not
     * claimed without an original runtime capture. */
    m11_try_load_original_song(state);

    /* Pass 53: opportunistically replace event-index playback with decoded
     * GRAPHICS.DAT SND3 buffers.  Source samples are unsigned 8-bit mono at
     * 6000 Hz; they are linearly resampled once at init to the fixed SDL
     * stream rate of 22050 Hz and queued as float mono.  If the original
     * asset is missing or any mapping/decode fails, procedural marker
     * playback remains the runtime fallback. */
    m11_try_load_original_snd3(state);

    /* Sound-pack support is source-index locked to the same ReDMCSB DM PC 3.4
     * sound-event namespace as the GRAPHICS.DAT SND3 path. Packs are opt-in
     * via FIRESTAFF_SOUND_PACK_DIR and may override any subset with .wav files
     * named by event index, SND3 item, or ReDMCSB macro. */
    m11_try_load_sound_pack(state);

#if M11_HAVE_SDL_AUDIO
    {
        SDL_AudioSpec spec;
        SDL_AudioStream* stream;

        /* Preview safety: real SDL3/CoreAudio playback is opt-in for now.
         * The runtime still decodes original SONG.DAT/SND3 and records audio
         * markers, but opening the live macOS audio device has produced
         * delayed heap corruption in the interactive preview path.  Keep the
         * game stable by default; use FIRESTAFF_AUDIO_ENABLE_SDL=1 when
         * specifically testing the audio backend. */
        if (!m11_sdl_audio_backend_enabled()) {
            state->backend = M11_AUDIO_BACKEND_NONE;
            return 1;
        }

        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            /* Audio init failed — stay in fallback mode */
            state->backend = M11_AUDIO_BACKEND_NONE;
            return 1;
        }

        spec.format   = SDL_AUDIO_F32;
        spec.channels = 1;
        spec.freq     = M11_AUDIO_SAMPLE_RATE;

        stream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            &spec,
            NULL,  /* no callback — we push data */
            NULL
        );

        if (!stream) {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            state->backend = M11_AUDIO_BACKEND_NONE;
            return 1;
        }

        SDL_ResumeAudioStreamDevice(stream);
        state->sdlStream = stream;
        state->backend   = M11_AUDIO_BACKEND_SDL3;
    }
#else
    state->backend = M11_AUDIO_BACKEND_NONE;
#endif

    return 1;
}

void M11_Audio_Shutdown(M11_AudioState* state) {
    if (!state) return;

#if M11_HAVE_SDL_AUDIO
    if (state->sdlStream) {
        SDL_DestroyAudioStream((SDL_AudioStream*)state->sdlStream);
        state->sdlStream = NULL;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
#endif

    {
        int i;
        for (i = 0; i < M11_AUDIO_MARKER_COUNT; ++i) {
            m11_sound_free(&state->sounds[i]);
        }
        for (i = 0; i < M11_AUDIO_ORIGINAL_SOUND_COUNT; ++i) {
            m11_sound_free(&state->originalSounds[i]);
        }
        m11_sound_free(&state->titleMusic);
    }

    state->initialized = 0;
    state->backend = M11_AUDIO_BACKEND_NONE;
    state->originalSnd3Available = 0;
    state->originalSnd3LoadedCount = 0;
    state->soundPackAvailable = 0;
    state->soundPackLoadedCount = 0;
    state->originalSongAvailable = 0;
    state->originalSongPartCount = 0;
    state->originalSongSequenceWordCount = 0;
    state->originalSongPlayablePartCount = 0;
    state->originalSongLoopTargetPart = 0;
    state->titleMusicQueuedCount = 0;
    state->titleMusicEnabled = 0;
}

int M11_Audio_IsAvailable(const M11_AudioState* state) {
    if (!state || !state->initialized) return 0;
    return state->backend == M11_AUDIO_BACKEND_SDL3 ? 1 : 0;
}

int M11_Audio_SetVolumes(M11_AudioState* state,
                         int masterVolume,
                         int sfxVolume,
                         int musicVolume,
                         int uiVolume) {
    if (!state || !state->initialized) return 0;
    state->masterVolume = m11_clamp_volume(masterVolume);
    state->sfxVolume    = m11_clamp_volume(sfxVolume);
    state->musicVolume  = m11_clamp_volume(musicVolume);
    state->uiVolume     = m11_clamp_volume(uiVolume);

#if M11_HAVE_SDL_AUDIO
    if (state->sdlStream) {
        float gain = (float)state->masterVolume / (float)M11_AUDIO_VOLUME_MAX;
        float sfxGain = (float)state->sfxVolume / (float)M11_AUDIO_VOLUME_MAX;
        SDL_SetAudioStreamGain((SDL_AudioStream*)state->sdlStream,
                               gain * sfxGain);
    }
#endif

    return 1;
}

int M11_Audio_GetVolumes(const M11_AudioState* state,
                         int* outMaster,
                         int* outSfx,
                         int* outMusic,
                         int* outUi) {
    if (!state || !state->initialized ||
        !outMaster || !outSfx || !outMusic || !outUi) {
        return 0;
    }
    *outMaster = state->masterVolume;
    *outSfx    = state->sfxVolume;
    *outMusic  = state->musicVolume;
    *outUi     = state->uiVolume;
    return 1;
}

M11_AudioMarker M11_Audio_FallbackMarkerForSoundIndex(int soundIndex) {
    const V1_SoundEventSnd3MapEntry* entry = V1_SoundEventSnd3_Find(soundIndex);
    if (!entry) return M11_AUDIO_MARKER_NONE;

    /* Source-lock:
     * - ReDMCSB DEFS.H 100-127 defines the I34E 35-sound namespace.
     * - ReDMCSB TIMELINE.C 769/793/808 routes closing-door damage,
     *   creature-under-door thud, and door rattle through F0064.
     * - ReDMCSB GROUP.C 1807-1808 routes creature attack ordinals to
     *   G2003_aauc_CreatureSounds[][C0_ATTACK_SOUND]; GROUP.C 279-280
     *   routes creature movement via F0514_MOVE_GetSound.
     * - ReDMCSB PROJEXPL.C 587-600 routes projectile impact SFX.
     * The marker only selects the fallback procedural cue when original SND3
     * playback is unavailable; mapped source indices still prefer SND3 data. */
    if (soundIndex == 1 || soundIndex == 2 || soundIndex == 3) {
        return M11_AUDIO_MARKER_DOOR;
    }
    if ((soundIndex >= 19 && soundIndex <= 34) ||
        soundIndex == 17 || soundIndex == 18) {
        return M11_AUDIO_MARKER_CREATURE;
    }
    if (soundIndex == 5 || soundIndex == 6 || soundIndex == 14 ||
        soundIndex == 16) {
        return M11_AUDIO_MARKER_SPELL;
    }
    if (soundIndex == 8) {
        return M11_AUDIO_MARKER_CREATURE;
    }
    return M11_AUDIO_MARKER_COMBAT;
}

int M11_Audio_EmitMarker(M11_AudioState* state, M11_AudioMarker marker) {
    if (!state || !state->initialized) return 0;
    if (marker <= M11_AUDIO_MARKER_NONE || marker >= M11_AUDIO_MARKER_COUNT) return 0;

    state->lastMarker = marker;
    state->lastSoundIndex = -1;

    if (state->backend != M11_AUDIO_BACKEND_SDL3) {
        /* Fallback: record marker but don't play */
        return 0;
    }

#if M11_HAVE_SDL_AUDIO
    {
        const M11_SoundBuffer* snd = &state->sounds[marker];
        if (snd->sampleCount > 0 && state->sdlStream) {
            int byteLen = snd->sampleCount * (int)sizeof(float);
            SDL_PutAudioStreamData(
                (SDL_AudioStream*)state->sdlStream,
                snd->samples,
                byteLen
            );
            state->queuedSampleCount += snd->sampleCount;
        }
    }
#endif

    state->playedMarkerCount += 1;
    return 1;
}

int M11_Audio_EmitSoundIndex(M11_AudioState* state, int soundIndex, M11_AudioMarker fallbackMarker) {
    if (!state || !state->initialized) return 0;
    if (soundIndex < 0 || soundIndex >= M11_AUDIO_ORIGINAL_SOUND_COUNT) {
        return M11_Audio_EmitMarker(state, fallbackMarker);
    }

    state->lastSoundIndex = soundIndex;
    if (fallbackMarker > M11_AUDIO_MARKER_NONE && fallbackMarker < M11_AUDIO_MARKER_COUNT) {
        state->lastMarker = fallbackMarker;
    }

    if (state->backend == M11_AUDIO_BACKEND_SDL3 &&
        (state->originalSnd3Available || state->soundPackAvailable) &&
        state->originalSounds[soundIndex].sampleCount > 0) {
#if M11_HAVE_SDL_AUDIO
        const M11_SoundBuffer* snd = &state->originalSounds[soundIndex];
        if (state->sdlStream) {
            int byteLen = snd->sampleCount * (int)sizeof(float);
            SDL_PutAudioStreamData(
                (SDL_AudioStream*)state->sdlStream,
                snd->samples,
                byteLen
            );
            state->queuedSampleCount += snd->sampleCount;
            state->playedMarkerCount += 1;
            return 1;
        }
#endif
    }

    {
        int result = M11_Audio_EmitMarker(state, fallbackMarker);
        /* Preserve the source sound-event index even when playback has to fall
         * back to a marker/no-audio path.  Direct marker emissions still clear
         * lastSoundIndex in M11_Audio_EmitMarker(); this only applies to calls
         * that reached the event-index seam.  It gives headless probes a stable
         * ordering/identity breadcrumb without claiming original waveform or
         * cadence parity. */
        if (fallbackMarker > M11_AUDIO_MARKER_NONE && fallbackMarker < M11_AUDIO_MARKER_COUNT) {
            state->lastSoundIndex = soundIndex;
        }
        return result;
    }
}

int M11_Audio_EmitSourceSoundIndex(M11_AudioState* state, int soundIndex) {
    return M11_Audio_EmitSoundIndex(state,
                                    soundIndex,
                                    M11_Audio_FallbackMarkerForSoundIndex(soundIndex));
}

int M11_Audio_SetTitleMusicEnabled(M11_AudioState* state, int enabled) {
    if (!state || !state->initialized) return 0;
    state->titleMusicEnabled = enabled ? 1 : 0;
    return 1;
}

int M11_Audio_TitleMusicEnabled(const M11_AudioState* state) {
    return (state && state->titleMusicEnabled) ? 1 : 0;
}

int M11_Audio_PlayTitleMusic(M11_AudioState* state) {
    if (!state || !state->initialized) return 0;
    if (!state->titleMusicEnabled) return 0;
    if (!state->originalSongAvailable || state->titleMusic.sampleCount <= 0) return 0;

    if (state->backend != M11_AUDIO_BACKEND_SDL3) {
        return 0;
    }

#if M11_HAVE_SDL_AUDIO
    if (state->sdlStream) {
        int byteLen = state->titleMusic.sampleCount * (int)sizeof(float);
        SDL_PutAudioStreamData(
            (SDL_AudioStream*)state->sdlStream,
            state->titleMusic.samples,
            byteLen
        );
        state->queuedSampleCount += state->titleMusic.sampleCount;
        state->titleMusicQueuedCount += 1;
        return 1;
    }
#endif

    return 0;
}

int M11_Audio_OriginalSnd3Available(const M11_AudioState* state) {
    return (state && state->originalSnd3Available) ? 1 : 0;
}

int M11_Audio_OriginalSongAvailable(const M11_AudioState* state) {
    return (state && state->originalSongAvailable) ? 1 : 0;
}

int M11_Audio_SoundPackAvailable(const M11_AudioState* state) {
    return (state && state->soundPackAvailable) ? 1 : 0;
}
