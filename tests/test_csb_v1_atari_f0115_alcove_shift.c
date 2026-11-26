#include "csb_v1_atari_f0115_alcove_shift.h"

#include <stdio.h>

int main(void)
{
    /* A missing coordinate does not consume a pile offset. */
    if (csb_v1_atari_f0115_alcove_shift_advance(2u, 0) != 2u ||
        csb_v1_atari_f0115_alcove_shift_advance(13u, 0) != 13u) {
        fputs("missing G0218 coordinate consumed a G0217 shift\n", stderr);
        return 1;
    }
    /* F0115 advances before blitting, even if that later blit is transparent
     * or clipped and therefore draws no visible pixels. */
    if (csb_v1_atari_f0115_alcove_shift_advance(2u, 5) != 3u ||
        csb_v1_atari_f0115_alcove_shift_advance(13u, 5) != 2u) {
        fputs("visible G0218 coordinate did not advance/wrap G0217\n", stderr);
        return 1;
    }
    return 0;
}
