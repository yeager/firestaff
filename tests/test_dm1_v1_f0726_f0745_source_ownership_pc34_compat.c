#include "dm1_v1_f0726_f0745_source_ownership_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    const DM1_V1_F0726F0745OwnershipPc34 *entry;
    volatile unsigned int sentinel = 0x5A17u;
    unsigned int functionId;

    for (functionId = 726U; functionId <= 745U; ++functionId) {
        entry = dm1_v1_f0726_f0745_source_ownership_pc34(functionId);
        assert(entry != 0);
        assert(entry->symbol[0] != '\0');
        assert(entry->source_anchor[0] != '\0');
        assert(entry->owner_or_rationale[0] != '\0');
    }
    assert(dm1_v1_f0726_f0745_source_ownership_pc34(725U) == 0);
    assert(dm1_v1_f0726_f0745_source_ownership_pc34(746U) == 0);

    entry = dm1_v1_f0726_f0745_source_ownership_pc34(738U);
    assert(entry->kind == DM1_V1_F0726_F0745_PC34_NOOP_PC34);
    entry = dm1_v1_f0726_f0745_source_ownership_pc34(739U);
    assert(entry->kind == DM1_V1_F0726_F0745_PC34_NOOP_PC34);
    dm1_v1_f0738_music_continue_pc34_noop();
    dm1_v1_f0739_music_stop_pc34_noop();
    assert(sentinel == 0x5A17u);

    entry = dm1_v1_f0726_f0745_source_ownership_pc34(744U);
    assert(entry->kind == DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34);
    assert(strstr(entry->owner_or_rationale, "I34M") != 0);
    assert(strstr(dm1_v1_f0726_f0745_source_ownership_evidence_pc34(),
                  "MUSIC.C:513-524") != 0);
    return 0;
}
