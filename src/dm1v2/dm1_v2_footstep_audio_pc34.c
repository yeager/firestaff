#include "dm1_v2_footstep_audio_pc34.h"

void v2_footstep_init(void) {
    /* DM1 PC34 has no source-backed party-footstep request.  This retained
     * compatibility API intentionally has no audio state to initialise. */
}

void v2_footstep_set_surface(M11_V2_SurfaceType type) {
    (void)type;
}

void v2_footstep_trigger(bool left_right) {
    (void)left_right;
}

void v2_footstep_set_echo(bool enabled) {
    (void)enabled;
}

int v2_footstep_get_sample(int16_t* buf, int* len) {
    (void)buf;
    if (len) *len = 0;
    /* Do not synthesize noise, pitch variation or echo in lieu of source
     * media.  ReDMCSB evidence has no party-footstep sound request, while
     * actual DM1 SFX must come through the GRAPHICS.DAT SND3 event route. */
    return -1;
}
