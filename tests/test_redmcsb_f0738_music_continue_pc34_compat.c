#include "redmcsb_f0738_music_continue_pc34_compat.h"

#include <string.h>

int main(void)
{
    volatile unsigned int sentinel = 0xC0DEC0DEu;
    const char *evidence = redmcsb_f0738_music_continue_source_evidence_pc34();

    if (evidence == NULL || strstr(evidence, "MUSIC.C:513-524") == NULL ||
        strstr(evidence, "PC 3.4/I34E as a no-op") == NULL) {
        return 1;
    }

    redmcsb_f0738_music_continue_pc34_compat();
    return sentinel == 0xC0DEC0DEu ? 0 : 1;
}
