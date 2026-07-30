/* CSBWin SaveGame.cpp original-save provenance and DB11 admission test. */
#include "csb_v1_runtime_pc34_compat.h"
#include "csbwin_resume_fixture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(expr, name) do { \
    if (expr) { ++passed; printf("PASS: %s\\n", name); } \
    else { ++failed; printf("FAIL: %s\\n", name); } \
} while (0)

static int append_invalid_expool_tail(const char *path)
{
    unsigned char tail[256] = { 0 };
    FILE *fp;

    /* First DB11 bucket points beyond this 64-word EXPOOL page. */
    tail[32u * 4u] = 64u;
    fp = fopen(path, "ab");
    if (!fp) return 0;
    if (fwrite(tail, 1u, sizeof(tail), fp) != sizeof(tail)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void test_staged_real_csbwin_save(void)
{
    const char *path = getenv("FIRESTAFF_CSBWIN_REAL_SAVE");
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_CSBWinSaveProvenance_PC34 provenance;

    if (!path || path[0] == '\0') {
        printf("SKIP: FIRESTAFF_CSBWIN_REAL_SAVE is not staged\n");
        return;
    }

    csb_v1_runtime_init(&runtime, NULL);
    CHECK(csb_v1_runtime_apply_csbwin_resume_file(&runtime, path, 0u) == 0,
          "staged CSBWin save completes the production resume handoff");
    CHECK(csb_v1_runtime_get_csbwin_save_provenance(&runtime, &provenance) == 0 &&
          provenance.valid && provenance.source_size > provenance.core_offset &&
          provenance.core_offset > 0u &&
          provenance.key_verdict == CSB_V1_CSBWIN_512_VERDICT_CSB &&
          strcmp(provenance.source_path, path) == 0,
          "staged CSBWin resume preserves the authenticated prefix/core provenance");
    CHECK(runtime.csbwin_extended_features_valid &&
          runtime.party_state_valid && runtime.csbwin_body_runtime_summary_valid,
          "staged CSBWin save publishes Extended Features and source body state");
    csb_v1_runtime_cleanup(&runtime);
}

int main(void)
{
    const char *good = "firestaff-csbwin-provenance-good.sav";
    const char *bad = "firestaff-csbwin-provenance-bad.sav";
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_CSBWinSaveProvenance_PC34 provenance;
    uint32_t preserved_time;
    int preserved_x;

    CHECK(firestaff_test_write_csbwin_resume_fixture(good, 0),
          "writes checksum-verified CSBWin save");
    csb_v1_runtime_init(&runtime, NULL);
    runtime.game_time = 77u;
    runtime.party_x = 3;
    CHECK(csb_v1_runtime_apply_csbwin_resume_file(&runtime, good, 0u) == 0,
          "accepts a complete CSBWin header/body save");
    CHECK(csb_v1_runtime_get_csbwin_save_provenance(&runtime, &provenance) == 0 &&
          provenance.valid && provenance.source_size > 512u &&
          provenance.source_fnv1a != 0u && provenance.core_fnv1a != 0u &&
          provenance.key_verdict == CSB_V1_CSBWIN_512_VERDICT_CSB &&
          strcmp(provenance.source_path, good) == 0,
          "records exact original-save provenance after commit");

    CHECK(firestaff_test_write_csbwin_resume_fixture(bad, 0),
          "writes second verified CSBWin save");
    CHECK(append_invalid_expool_tail(bad),
          "appends malformed DB11/EXPOOL tail");
    preserved_time = runtime.game_time;
    preserved_x = runtime.party_x;
    CHECK(csb_v1_runtime_apply_csbwin_resume_file(&runtime, bad, 0u) != 0,
          "rejects malformed DB11/EXPOOL before runtime commit");
    CHECK(runtime.game_time == preserved_time && runtime.party_x == preserved_x &&
          runtime.csbwin_save_provenance.valid &&
          strcmp(runtime.csbwin_save_provenance.source_path, good) == 0,
          "failed original-save import rolls back live runtime and provenance");

    test_staged_real_csbwin_save();

    remove(good);
    remove(bad);
    printf("%d passed, %d failed\\n", passed, failed);
    return failed != 0;
}
