#include "csb_hint_oracle_atari_runtime.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "check failed %s:%d: %s\n", __FILE__, __LINE__, #x); \
    return 0; } } while (0)

static int test_not_ready_fails_closed(void)
{
    CSB_HintOracleAtariRuntime runtime;
    CSB_V1_AtariSaveInfo info;
    uint8_t frame[64000];
    csb_hint_oracle_atari_runtime_init(&runtime);
    memset(&info, 0, sizeof(info));
    CHECK(csb_hint_oracle_atari_runtime_select_save(&runtime, &info) ==
          CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY);
    CHECK(csb_hint_oracle_atari_runtime_render_frame(&runtime, frame, sizeof(frame)) ==
          CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY);
    CHECK(csb_hint_oracle_atari_runtime_render_page(&runtime, frame, sizeof(frame)) ==
          CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY);
    csb_hint_oracle_atari_runtime_free(&runtime);
    return 1;
}

static int read_verified_mini_from_root(const char *root,
                                        uint8_t **out_bytes,
                                        size_t *out_size)
{
    char mini_path[ASSET_PATH_MAX];
    if (!root || !out_bytes || !out_size ||
        !asset_find_by_md5(root, "531ea104a2fbc2011ea73d11f274c57d",
                           mini_path, (int)sizeof(mini_path), 8)) {
        return 0;
    }
    /* `asset_read_path_alloc` handles both loose and nested STX/ADF/ZIP
     * paths in bounded memory.  Do not extract an original campaign save to
     * a host temporary path merely to decode an already-supported member. */
    return asset_read_path_alloc(mini_path, out_bytes, out_size);
}

static int test_real_atari_r1_triplet_if_staged(void)
{
    const char *root = getenv("FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR");
    uint8_t *mini = NULL;
    size_t mini_size = 0u;
    uint8_t frame[64000];
    CSB_V1_AtariSaveInfo info;
    CSB_HintOracleAtariRuntime runtime;
    int rc;
    size_t selected_count;
    if (!root || !root[0]) {
        puts("csb_hint_oracle_atari_runtime: SKIP (FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR unset)");
        return 1;
    }
    if (!read_verified_mini_from_root(root, &mini, &mini_size)) {
        fprintf(stderr, "missing hash-verified Atari R1 MINI.DAT below: %s\n", root);
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
    /* HINTMAIN.C state 1: original prompt plus LOAD/EXIT on HCSB.DAT. */
    CHECK(csb_hint_oracle_atari_runtime_render_frame(&runtime, frame, sizeof(frame)) == 0);
    CHECK(memcmp(frame, runtime.graphics.pixels, sizeof(frame)) != 0);
    CHECK(csb_hint_oracle_atari_runtime_handle_click(&runtime, 130, 180, &info) == 0);
    CHECK(runtime.session.state == CSB_HINT_ORACLE_SESSION_HINT_LIST);
    CHECK(runtime.session.selected_hint_count > 0u);
    selected_count = runtime.session.selected_hint_count;
    /* HINTHINT.C C06: authored source-order title rows plus DONE. */
    CHECK(csb_hint_oracle_atari_runtime_render_frame(&runtime, frame, sizeof(frame)) == 0);
    CHECK(memcmp(frame, runtime.graphics.pixels, sizeof(frame)) != 0);
    CHECK(csb_hint_oracle_atari_runtime_handle_click(&runtime, 50, 35, NULL) == 0);
    rc = csb_hint_oracle_atari_runtime_render_page(&runtime, frame, sizeof(frame));
    CHECK(rc == 0);
    CHECK(memcmp(frame, runtime.graphics.pixels, sizeof(frame)) != 0);
    /* HINTMAIN.C command 5: page -> list, then list -> LOAD prompt. */
    CHECK(csb_hint_oracle_atari_runtime_handle_click(&runtime, 230, 180, NULL) == 0);
    CHECK(runtime.session.state == CSB_HINT_ORACLE_SESSION_HINT_LIST);
    CHECK(csb_hint_oracle_atari_runtime_handle_click(&runtime, 230, 180, NULL) == 0);
    CHECK(runtime.session.state == CSB_HINT_ORACLE_SESSION_AWAIT_LOAD);
    /* HINTMAIN.C's no-clue state has no inferred data: it is solely the
     * original fixed text and OK control over the decoded HCSB surface. */
    runtime.session.state = CSB_HINT_ORACLE_SESSION_NO_CLUE;
    CHECK(csb_hint_oracle_atari_runtime_render_frame(&runtime, frame, sizeof(frame)) == 0);
    CHECK(memcmp(frame, runtime.graphics.pixels, sizeof(frame)) != 0);
    printf("csb_hint_oracle_atari_runtime: real receipt map=%d x=%d y=%d hints=%zu\n",
           (int)info.party_map_index, (int)info.party_x, (int)info.party_y,
           selected_count);
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
