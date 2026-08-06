#ifndef FIRESTAFF_AUDIO_SDL_M11_H
#define FIRESTAFF_AUDIO_SDL_M11_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_AUDIO_BACKEND_NONE = 0,
    M11_AUDIO_BACKEND_SDL3 = 1
} M11_AudioBackend;

typedef enum {
    M11_AUDIO_MARKER_NONE = 0,
    M11_AUDIO_MARKER_FOOTSTEP,
    M11_AUDIO_MARKER_DOOR,
    M11_AUDIO_MARKER_COMBAT,
    M11_AUDIO_MARKER_CREATURE,
    M11_AUDIO_MARKER_SPELL,
    M11_AUDIO_MARKER_COUNT
} M11_AudioMarker;

#define M11_AUDIO_SAMPLE_RATE 22050
#define M11_AUDIO_SOURCE_SND3_SAMPLE_RATE 6000
#define M11_AUDIO_SOURCE_SND8_SAMPLE_RATE 11025
#define M11_AUDIO_MAX_SOUND_MS 300
#define M11_AUDIO_MAX_SAMPLES ((M11_AUDIO_SAMPLE_RATE * M11_AUDIO_MAX_SOUND_MS) / 1000)
#define M11_AUDIO_ORIGINAL_SOUND_COUNT 35
#define M11_AUDIO_SOURCE_MUSIC_GAME_WON 2

typedef struct {
    float* samples;
    int sampleCount;
    int capacity;
} M11_SoundBuffer;

typedef struct {
    int initialized;
    M11_AudioBackend backend;
    int masterVolume;
    int sfxVolume;
    int musicVolume;
    int uiVolume;
    int playedMarkerCount;
    int queuedSampleCount;
    M11_AudioMarker lastMarker;
    int lastSoundIndex;
    int lastMusicTrackId;
    int originalSnd3Available;
    int originalSnd3LoadedCount;
    int soundPackAvailable;
    int soundPackLoadedCount;
    int originalSongAvailable;
    char originalSongDatPath[512];
    int originalSongPartCount;
    int originalSongSequenceWordCount;
    int originalSongPlayablePartCount;
    int originalSongLoopTargetPart;
    int titleMusicQueuedCount;
    int titleMusicPlayRequestCount;
    int titleMusicEnabled;
    void* sdlStream;
    void* cddaStream;
    int   cddaPlaying;
    M11_SoundBuffer sounds[M11_AUDIO_MARKER_COUNT];
    M11_SoundBuffer originalSounds[M11_AUDIO_ORIGINAL_SOUND_COUNT];
    M11_SoundBuffer titleMusic;
    M11_SoundBuffer dm1SwshProgram;
    M11_SoundBuffer csbSwshPcm;
    M11_SoundBuffer csbAtariStPsg;
    M11_SoundBuffer csbPc34RuntimePcm;
    M11_SoundBuffer csbAmigaRuntimePcm;
    int dm1SwshProgramAccepted;
    int dm1SwshRegisterWriteCount;
    int dm1SwshWaitVblankCount;
    int dm1SwshQueuedCount;
    int csbSwshSourceAccepted;
    int csbSwshSourceByteCount;
    int csbSwshSourcePeriod;
    unsigned int csbSwshSourceHash;
    int csbSwshQueuedCount;
    int csbAtariStSoundAccepted;
    int csbAtariStSoundQueuedCount;
    int csbAtariStSoundPeriod;
    unsigned int csbAtariStSoundHash;
    int csbPc34RuntimeSoundAccepted;
    int csbPc34RuntimeSoundByteCount;
    int csbPc34RuntimeSoundTimerDivisor;
    int csbPc34RuntimeSoundSourceVolume;
    unsigned int csbPc34RuntimeSoundHash;
    int csbPc34RuntimeSoundQueuedCount;
    int csbAmigaRuntimeSoundAccepted;
    int csbAmigaRuntimeSoundByteCount;
    int csbAmigaRuntimeSoundPeriod;
    int csbAmigaRuntimeSoundSourceVolume;
    unsigned int csbAmigaRuntimeSoundHash;
    int csbAmigaRuntimeSoundQueuedCount;
} M11_AudioState;

int M11_Audio_Init(M11_AudioState* state);
/* Empty selects SDL's system default.  The name is matched against the live
 * SDL playback-device list at each game start, so disconnected devices fail
 * safely back to the system default instead of inventing an output. */
void M11_Audio_SetPreferredPlaybackDeviceName(const char* name);
void M11_Audio_Shutdown(M11_AudioState* state);
int M11_Audio_IsAvailable(const M11_AudioState* state);
int M11_Audio_SetVolumes(M11_AudioState* state,
                         int masterVolume,
                         int sfxVolume,
                         int musicVolume,
                         int uiVolume);
int M11_Audio_GetVolumes(const M11_AudioState* state,
                         int* outMaster,
                         int* outSfx,
                         int* outMusic,
                         int* outUi);
int M11_Audio_EmitMarker(M11_AudioState* state, M11_AudioMarker marker);
M11_AudioMarker M11_Audio_FallbackMarkerForSoundIndex(int soundIndex);
int M11_Audio_EmitSoundIndex(M11_AudioState* state,
                             int soundIndex,
                             M11_AudioMarker fallbackMarker);
/* Source-owned DM1 SND3 events never substitute a generated marker. Missing
 * or unplayable source material is silent and returns 0. */
int M11_Audio_EmitSourceSoundIndex(M11_AudioState* state, int soundIndex);
/* SWSH.C V0901005 is emulated from its original PSG program.  This does not
 * permit a marker/SND3/procedural substitute when the source program fails. */
int M11_Audio_PlayDm1SwshDosoundProgram(M11_AudioState* state,
                                        const unsigned char* program,
                                        int programBytes,
                                        unsigned int vblankMs);
/* CSB PC34 SWSHSND.C F0908 owns a raw signed 8-bit DMA sample.  This
 * accepts only the authenticated 9078-byte source buffer and never falls
 * back to a marker, SND3 effect, or generated waveform. */
int M11_Audio_PlayCsbSwshPcm(M11_AudioState* state,
                             const unsigned char* source,
                             int sourceBytes,
                             int sourcePeriod,
                             unsigned int sourceHash);
/* ANIM.C opcode 12 calls Atari ST F0060 with a SND1 packed amplitude stream.
 * The renderer consumes only that source stream and the original Timer-A
 * period; there is no marker or generated replacement on failure. */
int M11_Audio_PlayCsbAtariStPsg(M11_AudioState* state,
                                const unsigned char* source,
                                int sourceBytes,
                                int sourcePeriod,
                                unsigned int sourceHash);
/* ReDMCSB PC3.4 IO.C F0060 passes signed 8-bit GRAPHICS.DAT samples to the
 * IBMIO driver. The original driver uses a PIT timer divisor, so this helper
 * preserves the source bytes and divisor during host resampling. It rejects
 * invalid source material and has no marker/SND3/procedural fallback. */
int M11_Audio_PlayCsbPc34RuntimePcm(M11_AudioState* state,
                                    const unsigned char* source,
                                    int sourceBytes,
                                    int timerDivisor,
                                    unsigned int sourceHash);
/* PC3.4 SOUND.C has already resolved sourceVolume in its 1..3 distance
 * domain. The host applies it before the user SFX gain without altering the
 * authenticated PCM bytes or PIT cadence. */
int M11_Audio_PlayCsbPc34RuntimePcmAtSourceVolume(
    M11_AudioState* state,
    const unsigned char* source,
    int sourceBytes,
    int timerDivisor,
    unsigned int sourceHash,
    int sourceVolume);
/* ReDMCSB SOUND.C F0709 uses Amiga audio.device period 72800 / SOUND_DATA
 * period. The host preserves signed source bytes and that cadence, with no
 * PC PIT, SND3, marker, or procedural substitute. */
int M11_Audio_PlayCsbAmigaRuntimePcmAtSourceVolume(
    M11_AudioState* state,
    const unsigned char* source,
    int sourceBytes,
    int sourcePeriod,
    unsigned int sourceHash,
    int sourceVolume);
int M11_Audio_RequestSourceMusicTrack(M11_AudioState* state, int musicTrackId);
int M11_Audio_SetTitleMusicEnabled(M11_AudioState* state, int enabled);
int M11_Audio_TitleMusicEnabled(const M11_AudioState* state);
int M11_Audio_PlayTitleMusic(M11_AudioState* state);
int M11_Audio_OriginalSnd3Available(const M11_AudioState* state);
int M11_Audio_OriginalSongAvailable(const M11_AudioState* state);
int M11_Audio_SoundPackAvailable(const M11_AudioState* state);
/* Rebind DM1's source SND3 bank to the exact GRAPHICS.DAT path admitted by
 * the startup asset receipt. This avoids using an unrelated default file
 * when the launcher selected a different verified installation. */
int M11_Audio_BindOriginalSnd3Path(M11_AudioState* state,
                                   const char* graphicsDatPath);

/* CDDA playback for FM Towns: push raw 16-bit signed LE stereo 44100Hz PCM
 * to the dedicated CDDA audio stream. Returns 1 on success, 0 if unavailable. */
int M11_Audio_PlayCdda(M11_AudioState* state,
                       const uint8_t *pcm_data, size_t pcm_size,
                       int loop);
int M11_Audio_StopCdda(M11_AudioState* state);

/* Nexus SAL SFX: mix any active Nexus voices into the SDL3 audio stream.
 * Call once per frame from the Nexus game loop.
 * nexus_audio is a Nexus_SoundEngine* (opaque here to avoid header deps). */
int M11_Audio_MixNexusSfx(M11_AudioState* state, void *nexus_audio);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_AUDIO_SDL_M11_H */
