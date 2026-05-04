#ifndef FIRESTAFF_DM1_V2_CREATURE_ANIMATION_PC34_H
#define FIRESTAFF_DM1_V2_CREATURE_ANIMATION_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    CANIM_IDLE,
    CANIM_WALK,
    CANIM_ATTACK,
    CANIM_HIT,
    CANIM_DEATH,
    CANIM_CAST
} M11_V2_CreatureAnim;

typedef struct {
    int sprite_idx;
    float duration;
} M11_V2_AnimFrame;

typedef struct {
    M11_V2_AnimFrame frames[16];
    int frame_count;
    bool loop;
} M11_V2_AnimSequence;

typedef struct {
    M11_V2_CreatureAnim current;
    int frame;
    float timer;
    bool playing;
    int creature_id;
} M11_V2_CreatureAnimState;

#define MAX_ACTIVE_ANIMATIONS 32

void v2_creature_anim_init(void);
void v2_creature_anim_define(M11_V2_CreatureAnim anim, const M11_V2_AnimFrame* frames, int count, bool loop);
void v2_creature_anim_play(int creature_id, M11_V2_CreatureAnim anim);
void v2_creature_anim_stop(int creature_id);
void v2_creature_anim_update(float dt);
int v2_creature_anim_get_sprite(int creature_id);
bool v2_creature_anim_is_playing(int creature_id);

#endif
