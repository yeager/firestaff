/* DM1 V1 Sound System — ReDMCSB SOUND.C. Q3.6+Opus. */
#include "dm1_v1_sound_pc34_compat.h"
#include <string.h>

void m11_sound_init(M11_SoundState* s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->masterVolume = 255; s->sfxVolume = 255; s->musicVolume = 128;
}

void m11_sound_play(M11_SoundState* s, int soundId, int volume, int pan) {
    if (!s || s->muted || s->queueCount >= M11_SOUND_QUEUE_MAX) return;
    M11_SoundRequest* r = &s->queue[s->queueCount++];
    r->soundId = soundId; r->volume = volume; r->pan = pan;
    r->priority = volume; r->delay = 0; r->playing = 1;
}

void m11_sound_play_at_depth(M11_SoundState* s, int soundId, int viewDepth) {
    /* ReDMCSB: sound volume attenuates with depth, ~64 per level */
    int vol = 255 - viewDepth * 64;
    if (vol < 0) vol = 0;
    m11_sound_play(s, soundId, vol, 0);
}

void m11_sound_tick(M11_SoundState* s, int tickMs) {
    if (!s) return;
    int i = 0;
    while (i < s->queueCount) {
        if (s->queue[i].playing) {
            s->queue[i].delay -= tickMs;
            if (s->queue[i].delay <= 0) s->queue[i].playing = 0;
        }
        if (!s->queue[i].playing) {
            for (int j = i; j < s->queueCount - 1; j++)
                s->queue[j] = s->queue[j + 1];
            s->queueCount--;
        } else { i++; }
    }
}

void m11_sound_stop_all(M11_SoundState* s) { if (s) s->queueCount = 0; }

void m11_sound_set_master_volume(M11_SoundState* s, int vol) {
    if (!s) return;
    if (vol < 0) vol = 0;
    if (vol > 255) vol = 255;
    s->masterVolume = vol;
}

const char* m11_sound_name(int soundId) {
    switch (soundId) {
        case DM1_SOUND_FOOTSTEP:       return "Footstep";
        case DM1_SOUND_DOOR_OPEN:      return "Door Open";
        case DM1_SOUND_DOOR_CLOSE:     return "Door Close";
        case DM1_SOUND_ATTACK:         return "Attack";
        case DM1_SOUND_SPELL_CAST:     return "Spell Cast";
        case DM1_SOUND_CREATURE_ATTACK:return "Creature Attack";
        case DM1_SOUND_ITEM_PICKUP:    return "Item Pickup";
        case DM1_SOUND_ITEM_DROP:      return "Item Drop";
        case DM1_SOUND_PIT_FALL:       return "Pit Fall";
        case DM1_SOUND_TELEPORTER:     return "Teleporter";
        case DM1_SOUND_BUTTON_PRESS:   return "Button Press";
        case DM1_SOUND_LEVER_PULL:     return "Lever Pull";
        case DM1_SOUND_SCREAM:         return "Scream";
        case DM1_SOUND_CHAMPION_DEATH: return "Champion Death";
        default:                       return "Unknown";
    }
}
