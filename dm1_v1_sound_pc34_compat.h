#ifndef FIRESTAFF_DM1_V1_SOUND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SOUND_PC34_COMPAT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Based on ReDMCSB SOUND.C sound playback system */
enum {
    DM1_SOUND_FOOTSTEP=0, DM1_SOUND_DOOR_OPEN, DM1_SOUND_DOOR_CLOSE,
    DM1_SOUND_ATTACK, DM1_SOUND_SPELL_CAST, DM1_SOUND_CREATURE_ATTACK,
    DM1_SOUND_ITEM_PICKUP, DM1_SOUND_ITEM_DROP, DM1_SOUND_PIT_FALL,
    DM1_SOUND_TELEPORTER, DM1_SOUND_BUTTON_PRESS, DM1_SOUND_LEVER_PULL,
    DM1_SOUND_SCREAM, DM1_SOUND_CHAMPION_DEATH, DM1_SOUND_COUNT
};
typedef struct { int soundId; int volume; int pan; int priority; int delay; int playing; } M11_SoundRequest;
#define M11_SOUND_QUEUE_MAX 16
typedef struct {
    M11_SoundRequest queue[M11_SOUND_QUEUE_MAX]; int queueCount;
    int masterVolume; int sfxVolume; int musicVolume; int muted;
} M11_SoundState;
void m11_sound_init(M11_SoundState* s);
void m11_sound_play(M11_SoundState* s, int soundId, int volume, int pan);
void m11_sound_play_at_depth(M11_SoundState* s, int soundId, int viewDepth);
void m11_sound_tick(M11_SoundState* s, int tickMs);
void m11_sound_stop_all(M11_SoundState* s);
void m11_sound_set_master_volume(M11_SoundState* s, int vol);
const char* m11_sound_name(int soundId);
#ifdef __cplusplus
}
#endif
#endif
