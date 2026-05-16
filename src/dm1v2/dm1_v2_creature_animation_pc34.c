#include "dm1_v2_creature_animation_pc34.h"

static M11_V2_CreatureAnimState g_active_anims[MAX_ACTIVE_ANIMATIONS];
static M11_V2_AnimSequence g_anim_sequences[6];

void v2_creature_anim_init(void) {
    memset(g_active_anims, 0, sizeof(g_active_anims));
    memset(g_anim_sequences, 0, sizeof(g_anim_sequences));
}

void v2_creature_anim_define(M11_V2_CreatureAnim anim, const M11_V2_AnimFrame* frames, int count, bool loop) {
    if (anim < 0 || anim >= 6) return;
    M11_V2_AnimSequence* seq = &g_anim_sequences[anim];
    int copy_count = count;
    if (copy_count < 0) copy_count = 0;
    if (copy_count > 16) copy_count = 16;
    seq->frame_count = copy_count;
    seq->loop = loop;
    if (frames && copy_count > 0) {
        memcpy(seq->frames, frames, sizeof(M11_V2_AnimFrame) * copy_count);
    }
}

void v2_creature_anim_play(int creature_id, M11_V2_CreatureAnim anim) {
    M11_V2_CreatureAnimState* state = NULL;
    for (int i = 0; i < MAX_ACTIVE_ANIMATIONS; ++i) {
        if (g_active_anims[i].creature_id == creature_id) {
            state = &g_active_anims[i];
            break;
        }
        if (!g_active_anims[i].playing) {
            state = &g_active_anims[i];
            break;
        }
    }
    
    if (!state) return;

    state->creature_id = creature_id;
    state->current = anim;
    state->frame = 0;
    state->timer = 0.0f;
    state->playing = true;
}

void v2_creature_anim_stop(int creature_id) {
    for (int i = 0; i < MAX_ACTIVE_ANIMATIONS; ++i) {
        if (g_active_anims[i].creature_id == creature_id && g_active_anims[i].playing) {
            g_active_anims[i].playing = false;
            g_active_anims[i].frame = 0;
            g_active_anims[i].timer = 0.0f;
            return;
        }
    }
}

void v2_creature_anim_update(float dt) {
    for (int i = 0; i < MAX_ACTIVE_ANIMATIONS; ++i) {
        M11_V2_CreatureAnimState* state = &g_active_anims[i];
        if (!state->playing) continue;

        state->timer += dt;
        
        if (state->current < 0 || state->current >= 6) {
            state->playing = false;
            continue;
        }

        M11_V2_AnimSequence* seq = &g_anim_sequences[state->current];
        if (seq->frame_count == 0) {
            state->playing = false;
            continue;
        }

        if (state->timer >= seq->frames[state->frame].duration) {
            state->timer -= seq->frames[state->frame].duration;
            state->frame++;
            
            if (state->frame >= seq->frame_count) {
                if (seq->loop) {
                    state->frame = 0;
                } else {
                    state->playing = false;
                    state->frame = seq->frame_count - 1;
                }
            }
        }
    }
}

int v2_creature_anim_get_sprite(int creature_id) {
    for (int i = 0; i < MAX_ACTIVE_ANIMATIONS; ++i) {
        if (g_active_anims[i].creature_id == creature_id && g_active_anims[i].playing) {
            M11_V2_CreatureAnimState* state = &g_active_anims[i];
            if (state->current < 0 || state->current >= 6) return -1;
            M11_V2_AnimSequence* seq = &g_anim_sequences[state->current];
            if (state->frame < 0 || state->frame >= seq->frame_count) return -1;
            return seq->frames[state->frame].sprite_idx;
        }
    }
    return -1;
}

bool v2_creature_anim_is_playing(int creature_id) {
    for (int i = 0; i < MAX_ACTIVE_ANIMATIONS; ++i) {
        if (g_active_anims[i].creature_id == creature_id && g_active_anims[i].playing) {
            return true;
        }
    }
    return false;
}

/* V2 Creature Animation — sprite frame sequencing */

#define V2_ANIM_MAX_FRAMES 16

typedef struct {
    int frame_indices[V2_ANIM_MAX_FRAMES];
    int frame_count;
    float frame_duration; /* seconds per frame */
    int loop;
} V2_AnimSequence;

typedef struct {
    const V2_AnimSequence *sequence;
    float timer;
    int current_frame;
    int finished;
} V2_AnimState;

void v2_anim_start(V2_AnimState *state, const V2_AnimSequence *seq) {
    if (!state || !seq) return;
    state->sequence = seq;
    state->timer = 0;
    state->current_frame = 0;
    state->finished = 0;
}

int v2_anim_tick(V2_AnimState *state, float dt) {
    if (!state || !state->sequence || state->finished) return -1;
    state->timer += dt;
    if (state->timer >= state->sequence->frame_duration) {
        state->timer -= state->sequence->frame_duration;
        state->current_frame++;
        if (state->current_frame >= state->sequence->frame_count) {
            if (state->sequence->loop) {
                state->current_frame = 0;
            } else {
                state->current_frame = state->sequence->frame_count - 1;
                state->finished = 1;
            }
        }
    }
    return state->sequence->frame_indices[state->current_frame];
}

int v2_anim_current_frame(const V2_AnimState *state) {
    if (!state || !state->sequence) return 0;
    return state->sequence->frame_indices[state->current_frame];
}

