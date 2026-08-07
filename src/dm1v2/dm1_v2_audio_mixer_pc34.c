#include "dm1_v2_audio_mixer_pc34.h"

/* PC34 SOUND.C owns decoded sound events.  This compatibility surface must
 * not create host sample channels, pan values or synthetic PCM output. */
void v2_audio_init(uint32_t sample_rate) { (void)sample_rate; }
void v2_audio_play(int ch, int16_t* samples, uint32_t len, uint8_t vol, bool loop) { (void)ch; (void)samples; (void)len; (void)vol; (void)loop; }
void v2_audio_stop(int ch) { (void)ch; }
void v2_audio_mix_output(int16_t* buf, int num_samples) { (void)buf; (void)num_samples; }
void v2_audio_set_volume(int ch, uint8_t vol) { (void)ch; (void)vol; }
void v2_audio_set_master(uint8_t vol) { (void)vol; }
void v2_audio_stop_all(void) {}
const char *v21_audio_source_evidence(void) { return "SOUND.C F0061/F0063/F0064/F0065 own PC34 audio events; no host PCM mixer."; }
