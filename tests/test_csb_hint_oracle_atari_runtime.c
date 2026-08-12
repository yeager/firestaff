#include "csb_hint_oracle_atari_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "check failed %s:%d: %s\n", __FILE__, __LINE__, #x); \
    return 0; } } while (0)

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *f; long length; size_t got; uint8_t *data;
    if (!path || !out || !out_size || !(f = fopen(path, "rb"))) return 0;
    if (fseek(f, 0, SEEK_END) != 0 || (length = ftell(f)) <= 0 ||
        fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }
    data = (uint8_t *)malloc((size_t)length);
    if (!data) { fclose(f); return 0; }
    got = fread(data, 1u, (size_t)length, f); fclose(f);
    if (got != (size_t)length) { free(data); return 0; }
    *out = data; *out_size = got; return 1;
}

static int test_not_ready_fails_closed(void)
{
    CSB_HintOracleAtariRuntime runtime;
    CSB_V1_AtariSaveInfo info;
    uint8_t frame[64000];
    csb_hint_oracle_atari_runtime_init(&runtime);
    memset(&info, 0, sizeof(info));
    CHECK(csb_hint_oracle_atari_runtime_select_save(&runtime, &info) ==
          CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY);
    CHECK(csb_hint_oracle_atari_runtime_render_page(&runtime, frame, sizeof(frame)) ==
          CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY);
    csb_hint_oracle_atari_runtime_free(&runtime);
    return 1;
}

static int test_real_atari_r1_triplet_if_staged(void)
{
    const char *root = getenv("FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR");
    char mini_path[1024];
    uint8_t *mini = NULL;
    size_t mini_size = 0u;
    uint8_t frame[64000];
    CSB_V1_AtariSaveInfo info;
    CSB_HintOracleAtariRuntime runtime;
    int rc;
    if (!root || !root[0]) {
        puts("csb_hint_oracle_atari_runtime: SKIP (FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR unset)");
        return 1;
    }
    if (snprintf(mini_path, sizeof(mini_path), "%s/MINI.DAT", root) <= 0 ||
        !read_file(mini_path, &mini, &mini_size)) {
        fprintf(stderr, "missing staged MINI.DAT: %s\n", mini_path);
        return 0;
    }
    CHECK(csb_v1_atari_save_decode_pc34_compat(mini, mini_size, &info) ==
          CSB_V1_ATARI_SAVE_OK);
    csb_hint_oracle_atari_runtime_init(&runtime);
    CHECK(csb_hint_oracle_atari_runtime_load_assets(&runtime, root, NULL, 4) == 0);
    CHECK(strcmp(runtime.htc_panel.cache.matched_md5,
                 CSB_HINT_ORACLE_ATARI_R1_HTC_MD5) == 0);
    CHECK(strcmp(runtime.graphics.source.matched_md5,
                 CSB_HINT_ORACLE_ATARI_R1_DAT_MD5) == 0);
    CHECK(csb_hint_oracle_atari_runtime_select_save(&runtime, &info) == 0);
    CHECK(runtime.session.state == CSB_HINT_ORACLE_SESSION_HINT_LIST);
    CHECK(runtime.session.selected_hint_count > 0u);
    CHECK(csb_hint_oracle_atari_runtime_open_hint_row(&runtime, 0u) == 0);
    rc = csb_hint_oracle_atari_runtime_render_page(&runtime, frame, sizeof(frame));
    CHECK(rc == 0);
    CHECK(memcmp(frame, runtime.graphics.pixels, sizeof(frame)) != 0);
    printf("csb_hint_oracle_atari_runtime: real receipt map=%d x=%d y=%d hints=%zu\n",
           (int)info.party_map_index, (int)info.party_x, (int)info.party_y,
           runtime.session.selected_hint_count);
    csb_hint_oracle_atari_runtime_free(&runtime);
    free(mini);
    return 1;
}

int main(void)
{
    int ok = test_not_ready_fails_closed() && test_real_atari_r1_triplet_if_staged();
    puts(ok ? "csb_hint_oracle_atari_runtime: PASS" : "csb_hint_oracle_atari_runtime: FAIL");
    return ok ? 0 : 1;
}
