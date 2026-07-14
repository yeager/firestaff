#include "redmcsb_f0703_release_champion_icon_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    uint16_t champion_icon_index;
    unsigned int calls;
} RedmcsbF0703CapturePc34Compat;

static void capture_champion_icon(void *context, uint16_t champion_icon_index)
{
    RedmcsbF0703CapturePc34Compat *capture = context;

    capture->champion_icon_index = champion_icon_index;
    ++capture->calls;
}

int main(void)
{
    RedmcsbF0703CapturePc34Compat capture = { 0 };
    RedmcsbF0703StatePc34Compat state = {
        0U,
        3,
        capture_champion_icon,
        &capture
    };

    assert(!redmcsb_f0703_release_champion_icon_pc34_compat(&state));
    assert(capture.calls == 0U);

    state.champion_icon_ordinal = 2U;
    state.mouse_pointer_champion_index = 1;
    assert(redmcsb_f0703_release_champion_icon_pc34_compat(&state));
    assert(capture.calls == 1U);
    assert(capture.champion_icon_index == 1U);

    state.click_champion_icon = NULL;
    assert(!redmcsb_f0703_release_champion_icon_pc34_compat(&state));
    assert(!redmcsb_f0703_release_champion_icon_pc34_compat(NULL));
    assert(strstr(redmcsb_f0703_release_champion_icon_source_evidence_pc34(),
                  "IO.C:3298-3304") != NULL);
    return 0;
}
