#include "dm1_v2_audio_mixer_pc34.h"
#include <stdio.h>
#include <string.h>
int main(void) { int16_t b[8], before[8], s[2]={1,2}; memset(b,0xA5,sizeof b); memcpy(before,b,sizeof b); v2_audio_init(44100); v2_audio_play(0,s,2,255,1); v2_audio_mix_output(b,4); v2_audio_stop_all(); if(memcmp(b,before,sizeof b)) return 1; if(!strstr(v21_audio_source_evidence(),"F0064")) return 1; puts("dm1_v2_audio_mixer_pc34: ok"); return 0; }
