#include "redmcsb_f0738_music_continue_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    volatile unsigned int sentinel = 0xC0DEC0DEu;
    const char *evidence = redmcsb_f0738_music_continue_source_evidence_pc34();

    assert(evidence != NULL);
    assert(strstr(evidence, "MUSIC.C:513-524") != NULL);
    assert(strstr(evidence, "PC 3.4/I34E as a no-op") != NULL);

    redmcsb_f0738_music_continue_pc34_compat();
    assert(sentinel == 0xC0DEC0DEu);
    return 0;
}
