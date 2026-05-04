#ifndef FIRESTAFF_DM1_V2_AUDIO_MIXER_PC34_H
#define FIRESTAFF_DM1_V2_AUDIO_MIXER_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int16_t* samples;
    uint32_t length;
    uint32_t position;
    uint8_t volume;
    bool loop;
    bool active;
    int8_t pan;
} M11_V2_AudioChannel;

typedef struct {
    M11_V2_AudioChannel channels[8];
    uint8_t master_volume;
    uint32_t sample_rate;
} M11_V2_AudioMixer;

void v2_audio_init(uint32_t sample_rate);
void v2_audio_play(int ch, int16_t* samples, uint32_t len, uint8_t vol, bool loop);
void v2_audio_stop(int ch);
void v2_audio_mix_output(int16_t* buf, int num_samples);
void v2_audio_set_volume(int ch, uint8_t vol);
void v2_audio_set_master(uint8_t vol);
void v2_audio_stop_all(void);

#ifdef __cplusplus
}
#endif

#endif
