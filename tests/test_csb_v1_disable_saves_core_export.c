/* CSBWin SaveGame.cpp:1972-1976 EDBT_DisableSaves export-policy regression. */

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
    uint8_t bytes[64];
    size_t size = 99u;
    char path[256];
    FILE *probe;

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_saves_disabled = 1;
    memset(bytes, 0xa5, sizeof(bytes));
    snprintf(path, sizeof(path), "/tmp/firestaff_csb_disable_saves_%p.sav",
             (void *)&profile);
    remove(path);

    check(csb_v1_runtime_export_csbwin_core_save_to_memory(
              &profile, bytes, sizeof(bytes), &size) == -1 && size == 0u &&
              bytes[0] == 0xa5u && bytes[sizeof(bytes) - 1u] == 0xa5u,
          "DisableSaves rejects core export without emitting replacement bytes");
    {
        int export_result =
            csb_v1_runtime_export_csbwin_core_save_to_path(&profile, path);
        probe = fopen(path, "rb");
        check(export_result == -1 &&
              probe == NULL,
              "DisableSaves rejects core export path before any save file exists");
    }
    if (probe) fclose(probe);
    remove(path);

    csb_v1_runtime_cleanup(&profile);
    return g_failures == 0 ? 0 : 1;
}
