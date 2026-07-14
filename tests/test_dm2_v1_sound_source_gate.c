#include "dm2_v1_sound.h"

#include <assert.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    assert(dm2_v1_sound_query_entry(DM2_SOUND_CATEGORY_STANDARD, 0U, 0U,
                                    DM2_SOUND_STD_EXPLOSION) == -1);
    assert(dm2_v1_sound_play(DM2_SOUND_STD_EXPLOSION, 127) == -1);
    assert(dm2_v1_sound_play_positional(DM2_SOUND_STD_EXPLOSION,
                                        1, 2, 3, 4) == -1);
    assert(dm2_v1_sound_play(-1, 0) == -1);
    return 0;
}
