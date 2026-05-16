
#ifndef FIRESTAFF_AUDIO_H
#define FIRESTAFF_AUDIO_H

/* Audio system — CD audio playback + sound effects.
 * Uses SDL_mixer when available, stubs when headless. */

int fs_audio_init(void);
void fs_audio_shutdown(void);
int fs_audio_play_track(const char *path);  /* WAV/OGG/MP3 */
void fs_audio_stop(void);
void fs_audio_set_volume(int percent);      /* 0-100 */
int fs_audio_is_playing(void);

/* Sound effects */
int fs_audio_play_sfx(const char *path);
void fs_audio_sfx_set_volume(int percent);

#endif

