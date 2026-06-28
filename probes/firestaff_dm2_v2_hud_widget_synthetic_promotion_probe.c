/*
 * firestaff_dm2_v2_hud_widget_synthetic_promotion_probe.c
 *
 * Installs the checked-in synthetic DM2 V2 HUD widget example into a
 * scratch asset root and proves the non-placeholder manifest path can
 * promote the gate to PARTIAL and COMPLETE without shipping original or
 * generated PBR HUD art.
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - include/dm2_v2_hud_widget_assets.h (module under test)
 *   - examples/dm2_hud_widget_synthetic/README.md (synthetic fixture)
 */

#include "dm2_v2_hud_widget_assets.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
#define FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR \
    "examples/dm2_hud_widget_synthetic"
#endif

static int s_pass = 0;
static int s_fail = 0;

static void check(const char* name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_pass++;
    } else {
        printf("  FAIL: %s\n", name);
        s_fail++;
    }
}

static int ensure_dir(const char* path) {
    char tmp[1024];
    size_t len;

    if (!path || path[0] == '\0') return 0;
    len = strlen(path);
    if (len >= sizeof(tmp)) return 0;
    memcpy(tmp, path, len + 1U);

    for (char* p = tmp + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0777) != 0 && errno != EEXIST) return 0;
            *p = '/';
        }
    }
    if (mkdir(tmp, 0777) != 0 && errno != EEXIST) return 0;
    return 1;
}

static int copy_file(const char* src, const char* dst) {
    FILE* in = fopen(src, "rb");
    FILE* out = NULL;
    unsigned char buf[4096];
    size_t n;

    if (!in) return 0;
    out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0U) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(out);
            fclose(in);
            return 0;
        }
    }
    if (ferror(in)) {
        fclose(out);
        fclose(in);
        return 0;
    }
    fclose(out);
    fclose(in);
    return 1;
}

static int file_contains_text(const char* path, const char* needle) {
    FILE* fp;
    char buf[4096];
    size_t n;
    size_t needle_len;

    if (!path || !needle || needle[0] == '\0') return 0;
    needle_len = strlen(needle);
    fp = fopen(path, "rb");
    if (!fp) return 0;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0U) {
        if (n >= needle_len) {
            for (size_t i = 0; i <= n - needle_len; ++i) {
                if (memcmp(buf + i, needle, needle_len) == 0) {
                    fclose(fp);
                    return 1;
                }
            }
        }
    }
    fclose(fp);
    return 0;
}

static void dirname_of(char* out, size_t out_size, const char* path) {
    const char* slash;
    size_t len;

    if (!out || out_size == 0U) return;
    out[0] = '\0';
    if (!path) return;
    slash = strrchr(path, '/');
    if (!slash) {
        snprintf(out, out_size, ".");
        return;
    }
    len = (size_t)(slash - path);
    if (len >= out_size) len = out_size - 1U;
    memcpy(out, path, len);
    out[len] = '\0';
}

static void join_path(char* out, size_t out_size,
                      const char* a, const char* b) {
    if (!out || out_size == 0U) return;
    if (!a || a[0] == '\0') {
        snprintf(out, out_size, "%s", b ? b : "");
    } else if (!b || b[0] == '\0') {
        snprintf(out, out_size, "%s", a);
    } else if (a[strlen(a) - 1U] == '/') {
        snprintf(out, out_size, "%s%s", a, b);
    } else {
        snprintf(out, out_size, "%s/%s", a, b);
    }
}

static int install_example_file(const char* manifest_dir,
                                const char* category,
                                const char* name) {
    char src_dir[1024];
    char src_path[1024];
    char dst_dir[1024];
    char dst_path[1024];

    join_path(src_dir, sizeof(src_dir),
              FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR, category);
    join_path(src_path, sizeof(src_path), src_dir, name);
    join_path(dst_dir, sizeof(dst_dir), manifest_dir, category);
    if (!ensure_dir(dst_dir)) return 0;
    join_path(dst_path, sizeof(dst_path), dst_dir, name);
    return copy_file(src_path, dst_path);
}

static int example_fixture_has_marker(const char* category, const char* name) {
    char src_dir[1024];
    char src_path[1024];

    join_path(src_dir, sizeof(src_dir),
              FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR, category);
    join_path(src_path, sizeof(src_path), src_dir, name);
    return file_contains_text(src_path, "synthetic-test-fixture");
}

static int install_example_manifest(const char* manifest_path) {
    char manifest_dir[1024];
    char src_manifest[1024];

    dirname_of(manifest_dir, sizeof(manifest_dir), manifest_path);
    if (!ensure_dir(manifest_dir)) return 0;
    join_path(src_manifest, sizeof(src_manifest),
              FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR,
              "hud_widget_manifest.json");
    return copy_file(src_manifest, manifest_path);
}

static void reset_scratch(void) {
    (void)system("rm -rf /tmp/scratch/dm2_hwa_synthetic_promotion_probe");
    (void)system("mkdir -p /tmp/scratch/dm2_hwa_synthetic_promotion_probe/firestaff-data/dm2");
    dm2_v2_hud_widget_assets_set_manifest_path(
        "/tmp/scratch/dm2_hwa_synthetic_promotion_probe/firestaff-data/dm2");
}

static void install_one_real_fixture(const char* manifest_dir) {
    check("copy synthetic inventory_quick_view fixture",
          install_example_file(manifest_dir, "hud_widgets",
                               "inventory_quick_view.png"));
}

static void install_all_real_fixtures(const char* manifest_dir) {
    check("copy synthetic inventory_quick_view fixture",
          install_example_file(manifest_dir, "hud_widgets",
                               "inventory_quick_view.png"));
    check("copy synthetic action_prompt fixture",
          install_example_file(manifest_dir, "hud_widgets",
                               "action_prompt.png"));
    check("copy synthetic compass_rose fixture",
          install_example_file(manifest_dir, "hud_chrome",
                               "compass_rose.png"));
    check("copy synthetic depth_indicator fixture",
          install_example_file(manifest_dir, "hud_chrome",
                               "depth_indicator.png"));
    check("copy synthetic gold_counter fixture",
          install_example_file(manifest_dir, "hud_chrome",
                               "gold_counter.png"));
    check("copy synthetic champion_bar_frame fixture",
          install_example_file(manifest_dir, "hud_chrome",
                               "champion_bar_frame.png"));
    check("copy synthetic action_strip_frame fixture",
          install_example_file(manifest_dir, "hud_chrome",
                               "action_strip_frame.png"));
}

int main(void) {
    char manifest_path[1024];
    char manifest_dir[1024];
    int total = 0;
    int real = 0;

    printf("=== DM2 V2 HUD Widget Synthetic Promotion probe ===\n");

    reset_scratch();
    snprintf(manifest_path, sizeof(manifest_path), "%s",
             dm2_v2_hud_widget_assets_get_manifest_path());
    dirname_of(manifest_dir, sizeof(manifest_dir), manifest_path);

    check("synthetic example manifest exists",
          access(FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
                 "/hud_widget_manifest.json", F_OK) == 0);
    check("inventory_quick_view fixture tagged synthetic",
          example_fixture_has_marker("hud_widgets", "inventory_quick_view.png"));
    check("action_prompt fixture tagged synthetic",
          example_fixture_has_marker("hud_widgets", "action_prompt.png"));
    check("compass_rose fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "compass_rose.png"));
    check("depth_indicator fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "depth_indicator.png"));
    check("gold_counter fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "gold_counter.png"));
    check("champion_bar_frame fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "champion_bar_frame.png"));
    check("action_strip_frame fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "action_strip_frame.png"));
    check("no manifest -> NO_MANIFEST gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST);
    check("no manifest -> installed=0",
          dm2_v2_hud_widget_assets_get_installed() == 0);

    printf("\n[ Scenario 1: PARTIAL via one synthetic non-placeholder slot ]\n");
    check("copy synthetic manifest", install_example_manifest(manifest_path));
    install_one_real_fixture(manifest_dir);
    check("synthetic manifest validates structurally",
          dm2_v2_hud_widget_assets_validate_manifest(NULL) == 1);
    check("one copied fixture -> PARTIAL gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_PARTIAL);
    check("PARTIAL -> installed=1",
          dm2_v2_hud_widget_assets_get_installed() == 1);
    check("inventory_quick_view -> REAL",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) ==
              DM2_V2_HUD_WIDGET_CLASS_REAL);
    check("action_prompt missing fixture -> PARTIAL",
          dm2_v2_hud_widget_assets_classify_slot(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT) ==
              DM2_V2_HUD_WIDGET_CLASS_PARTIAL);
    check("REAL slot disables procedural placeholder",
          dm2_v2_hud_widget_assets_uses_placeholder(
              DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW) == 0);
    check("missing fixture keeps procedural placeholder",
          dm2_v2_hud_widget_assets_uses_placeholder(
              DM2_V2_HUD_WIDGET_ACTION_PROMPT) == 1);
    real = dm2_v2_hud_widget_assets_real_count(&total);
    check("PARTIAL synthetic real_count=1", real == 1);
    check("PARTIAL synthetic total=7", total == 7);

    printf("\n[ Scenario 2: COMPLETE via all synthetic fixtures ]\n");
    reset_scratch();
    snprintf(manifest_path, sizeof(manifest_path), "%s",
             dm2_v2_hud_widget_assets_get_manifest_path());
    dirname_of(manifest_dir, sizeof(manifest_dir), manifest_path);
    check("copy synthetic manifest", install_example_manifest(manifest_path));
    install_all_real_fixtures(manifest_dir);
    check("all copied fixtures -> COMPLETE gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_COMPLETE);
    check("COMPLETE -> installed=1",
          dm2_v2_hud_widget_assets_get_installed() == 1);
    real = dm2_v2_hud_widget_assets_real_count(&total);
    check("COMPLETE synthetic real_count=7",
          real == (int)DM2_V2_HUD_WIDGET_COUNT);
    check("COMPLETE synthetic total=7",
          total == (int)DM2_V2_HUD_WIDGET_COUNT);

    for (int i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetSlot slot = (DM2_V2_HudWidgetSlot)i;
        DM2_V2_HudWidgetSlotInfo info;
        char msg[128];
        snprintf(msg, sizeof(msg), "%s -> REAL",
                 dm2_v2_hud_widget_assets_slot_name(slot));
        check(msg, dm2_v2_hud_widget_assets_classify_slot(slot) ==
                       DM2_V2_HUD_WIDGET_CLASS_REAL);
        snprintf(msg, sizeof(msg), "%s slot info file_exists=1",
                 dm2_v2_hud_widget_assets_slot_name(slot));
        check(msg, dm2_v2_hud_widget_assets_get_slot_info(slot, &info) == 1 &&
                       info.file_exists == 1 &&
                       strcmp(info.generator, "synthetic_test") == 0 &&
                       info.resolved_path[0] != '\0');
    }

    printf("\n[ Scenario 3: citations and honest boundary ]\n");
    {
        const char* ev = dm2_v2_hud_widget_assets_source_evidence();
        check("source evidence mentions SKULL.ASM T560",
              ev && strstr(ev, "SKULL.ASM T560") != NULL);
        check("source evidence mentions ReDMCSB PANEL.C",
              ev && strstr(ev, "ReDMCSB PANEL.C") != NULL);
        check("source evidence mentions synthetic example path",
              ev && strstr(ev, "examples/dm2_hud_widget_synthetic") != NULL);
        check("source evidence keeps no finished-art claim",
              ev && strstr(ev, "does NOT") != NULL &&
                    strstr(ev, "finished PBR widget art") != NULL);
    }

    reset_scratch();
    (void)system("rm -rf /tmp/scratch/dm2_hwa_synthetic_promotion_probe");
    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
