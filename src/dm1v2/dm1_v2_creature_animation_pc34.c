#include "dm1_v2_creature_animation_pc34.h"

/* PC34 updates creature aspects through GROUP.C:F0179 and selects decoded
 * G0219/DUNVIEW.C bitmaps in F0115.  This V2 API has no source record for
 * caller-defined sprite indices or floating-point frame durations. */
void v2_creature_anim_init(void) {}
void v2_creature_anim_define(M11_V2_CreatureAnim anim,
                             const M11_V2_AnimFrame* frames,
                             int count,
                             bool loop) {
    (void)anim;
    (void)frames;
    (void)count;
    (void)loop;
}
void v2_creature_anim_play(int creature_id, M11_V2_CreatureAnim anim) {
    (void)creature_id;
    (void)anim;
}
void v2_creature_anim_stop(int creature_id) { (void)creature_id; }
void v2_creature_anim_update(float dt) { (void)dt; }
int v2_creature_anim_get_sprite(int creature_id) {
    (void)creature_id;
    return -1;
}
bool v2_creature_anim_is_playing(int creature_id) {
    (void)creature_id;
    return false;
}
