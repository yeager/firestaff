#include "dm1_v1_fmtowns_snd_api.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_count(void) {
    assert(DM1_V1_FMTOWNS_SND_API_COUNT == 36);
}

static void test_categories(void) {
    unsigned core = 0, fm = 0, pcm = 0, vol = 0;
    for (unsigned i = 0; i < DM1_V1_FMTOWNS_SND_API_COUNT; ++i) {
        const char *c = dm1_v1_fmtowns_snd_api_entries[i].category;
        if (strcmp(c, "core") == 0) ++core;
        else if (strcmp(c, "fm") == 0) ++fm;
        else if (strcmp(c, "pcm") == 0) ++pcm;
        else if (strcmp(c, "vol") == 0) ++vol;
    }
    assert(core == 11);
    assert(fm == 9);
    assert(pcm == 11);
    assert(vol == 5);
    assert(core + fm + pcm + vol == DM1_V1_FMTOWNS_SND_API_COUNT);
}

static void test_known_vaddrs(void) {
    assert(dm1_v1_fmtowns_snd_api_vaddr_pc34("SND_INIT") == 0x212dbu);
    assert(dm1_v1_fmtowns_snd_api_vaddr_pc34("SND_KEY_ON") == 0x21367u);
    assert(dm1_v1_fmtowns_snd_api_vaddr_pc34("SND_FM_READ_STATUS") == 0x21454u);
    assert(dm1_v1_fmtowns_snd_api_vaddr_pc34("SND_PCM_PLAY") == 0x215d4u);
    assert(dm1_v1_fmtowns_snd_api_vaddr_pc34("SND_ELEVOL_ALL_MUTE") == 0x219c8u);
    assert(dm1_v1_fmtowns_snd_api_vaddr_pc34("NOT_A_FUNCTION") == 0);
    assert(dm1_v1_fmtowns_snd_api_vaddr_pc34(NULL) == 0);
}

static void test_lookup_returns_full_entry(void) {
    const dm1_v1_fmtowns_snd_api_entry_t *e =
        dm1_v1_fmtowns_snd_api_lookup_pc34("SND_PCM_PLAY");
    assert(e != NULL);
    assert(strcmp(e->name, "SND_PCM_PLAY") == 0);
    assert(strcmp(e->category, "pcm") == 0);
    assert(e->purpose != NULL);
    assert(dm1_v1_fmtowns_snd_api_lookup_pc34(NULL) == NULL);
}

int main(void) {
    test_count();
    test_categories();
    test_known_vaddrs();
    test_lookup_returns_full_entry();
    puts("All dm1_v1_fmtowns_snd_api tests passed.");
    return 0;
}
