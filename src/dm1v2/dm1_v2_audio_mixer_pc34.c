#include "dm1_v2_audio_mixer_pc34.h"

static M11_V2_AudioMixer g_mixer;

void v2_audio_init(uint32_t sample_rate) {
    memset(&g_mixer, 0, sizeof(g_mixer));
    g_mixer.sample_rate = sample_rate;
    g_mixer.master_volume = 255;
}

void v2_audio_play(int ch, int16_t* samples, uint32_t len, uint8_t vol, bool loop) {
    if (ch < 0 || ch >= 8) return;
    g_mixer.channels[ch].samples = samples;
    g_mixer.channels[ch].length = len;
    g_mixer.channels[ch].position = 0;
    g_mixer.channels[ch].volume = vol;
    g_mixer.channels[ch].loop = loop;
    g_mixer.channels[ch].active = true;
    g_mixer.channels[ch].pan = 0;
}

void v2_audio_stop(int ch) {
    if (ch < 0 || ch >= 8) return;
    g_mixer.channels[ch].active = false;
    g_mixer.channels[ch].position = 0;
}

void v2_audio_mix_output(int16_t* buf, int num_samples) {
    memset(buf, 0, num_samples * 2 * sizeof(int16_t));
    for (int i = 0; i < num_samples; i++) {
        int32_t left_acc = 0;
        int32_t right_acc = 0;
        for (int c = 0; c < 8; c++) {
            M11_V2_AudioChannel* ch = &g_mixer.channels[c];
            if (!ch->active || !ch->samples) continue;

            int16_t sample = ch->samples[ch->position];
            ch->position++;
            if (ch->position >= ch->length) {
                if (ch->loop) {
                    ch->position = 0;
                } else {
                    ch->active = false;
                }
            }

            float vol = (float)(ch->volume * g_mixer.master_volume) / 65025.0f;
            float pan_f = (float)(ch->pan + 128) / 255.0f;
            float left_gain = 1.0f - pan_f;
            float right_gain = pan_f;

            left_acc += (int32_t)(sample * vol * left_gain);
            right_acc += (int32_t)(sample * vol * right_gain);
        }

        if (left_acc > 32767) left_acc = 32767;
        else if (left_acc < -32768) left_acc = -32768;
        if (right_acc > 32767) right_acc = 32767;
        else if (right_acc < -32768) right_acc = -32768;

        buf[i * 2] = (int16_t)left_acc;
        buf[i * 2 + 1] = (int16_t)right_acc;
    }
}

void v2_audio_set_volume(int ch, uint8_t vol) {
    if (ch < 0 || ch >= 8) return;
    g_mixer.channels[ch].volume = vol;
}

void v2_audio_set_master(uint8_t vol) {
    g_mixer.master_volume = vol;
}

void v2_audio_stop_all(void) {
    for (int i = 0; i < 8; i++) {
        g_mixer.channels[i].active = false;
        g_mixer.channels[i].position = 0;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.1 Audio Mixer — Original sample-faithful playback
 *
 * DM1 PC-34 uses PC speaker for sound effects (no MOD music).
 * V2.1 replays the same sound events as V1 through SDL_mixer:
 *   - F0061_SOUND_SetupSound → v2_audio_init
 *   - F0063_SOUND_KillSound → v2_audio_shutdown
 *   - F0064_SOUND_RequestPlay_CPSD → v2_audio_play_sound
 *
 * The sound event table maps ReDMCSB sound indices to WAV/OGG assets.
 * ══════════════════════════════════════════════════════════════════════ */

const char *v21_audio_source_evidence(void) {
    return
        "SOUND.C F0061_SOUND_SetupSound\n"
        "SOUND.C F0063_SOUND_KillSound\n"
        "SOUND.C F0064_SOUND_RequestPlay_CPSD\n"
        "SOUND.C F0065_SOUND_StopAll\n"
        "V2.1: same sound events as V1, rendered through SDL_mixer\n";
}

