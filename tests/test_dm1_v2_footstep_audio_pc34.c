#include "dm1_v2_footstep_audio_pc34.h"

#include <stdint.h>
#include <stdio.h>

int main(void) {
    int16_t samples[4] = { 101, -202, 303, -404 };
    int length = 4;
    int ok = 1;

    v2_footstep_init();
    v2_footstep_set_surface(METAL);
    v2_footstep_set_echo(true);
    v2_footstep_trigger(true);
    ok &= v2_footstep_get_sample(samples, &length) == -1;
    ok &= length == 0;
    ok &= samples[0] == 101 && samples[1] == -202;
    ok &= samples[2] == 303 && samples[3] == -404;
    ok &= v2_footstep_get_sample(NULL, NULL) == -1;

    puts(ok ? "PASS dm1_v2_footstep_audio_pc34" :
              "FAIL dm1_v2_footstep_audio_pc34");
    return ok ? 0 : 1;
}
