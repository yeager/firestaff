/*
 * CSBWin SaveGame.cpp/EXPOOL core-export integrity regression.
 * A preserved tail marked truncated is incomplete source data and must never
 * be re-emitted as an interoperable core save.
 */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++g_failures; }
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    uint8_t bytes[128];
    size_t size = 91u;

    csb_v1_runtime_init(&profile, NULL);
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 0;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = 128u;
    profile.csbwin_appended_tail_preserved_size = 128u;
    profile.csbwin_appended_tail_truncated = 1;
    memset(profile.csbwin_appended_tail, 0x4d,
           profile.csbwin_appended_tail_preserved_size);
    memset(bytes, 0xa5, sizeof(bytes));

    check(csb_v1_runtime_export_csbwin_core_save_to_memory(
              &profile, bytes, sizeof(bytes), &size) == -1 && size == 0u &&
              bytes[0] == 0xa5u && bytes[sizeof(bytes) - 1u] == 0xa5u,
          "truncated EXPOOL tail rejects before any core-save bytes are emitted");

    csb_v1_runtime_cleanup(&profile);
    return g_failures == 0 ? 0 : 1;
}
