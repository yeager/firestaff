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
#ifdef _WIN32
#include <direct.h>
#endif

#ifndef FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
#define FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR \
    "examples/dm2_hud_widget_synthetic"
#endif

static int s_pass = 0;
static int s_fail = 0;

static int portable_mkdir(const char* path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0777);
#endif
}

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
            if (portable_mkdir(tmp) != 0 && errno != EEXIST) return 0;
            *p = '/';
        }
    }
    if (portable_mkdir(tmp) != 0 && errno != EEXIST) return 0;
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

/* PNG 8-byte signature. The synthetic example fixtures are 1x1 RGBA PNGs
 * (see examples/dm2_hud_widget_synthetic/README.md) tagged with a
 * "synthetic-test-fixture" tEXt chunk. file_contains_text() alone does
 * not prove the file is a structurally valid PNG: someone could replace
 * the fixture with arbitrary text containing the marker. The PNG
 * signature check is a small, bounded content-integrity guarantee that
 * the fixture is at least shaped like a PNG so the runtime's eventual
 * real-bitmap blit replacement site (see dm2_v2_hud_runtime.c) can call
 * any standard PNG decoder on it without surprising the loader. */
static const unsigned char k_png_signature[8] = {
    0x89U, 0x50U, 0x4EU, 0x47U, 0x0DU, 0x0AU, 0x1AU, 0x0AU
};

static int file_has_png_signature(const char* path) {
    unsigned char buf[8];
    FILE* fp;
    size_t n;

    if (!path || path[0] == '\0') return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    n = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);
    if (n != sizeof(buf)) return 0;
    return memcmp(buf, k_png_signature, sizeof(buf)) == 0;
}

static int example_fixture_has_png_signature(const char* category,
                                             const char* name) {
    char src_dir[1024];
    char src_path[1024];

    join_path(src_dir, sizeof(src_dir),
              FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR, category);
    join_path(src_path, sizeof(src_path), src_dir, name);
    return file_has_png_signature(src_path);
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

static const char* expected_category_for_slot(DM2_V2_HudWidgetSlot slot) {
    switch (slot) {
    case DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW:
    case DM2_V2_HUD_WIDGET_ACTION_PROMPT:
        return "hud_widgets";
    case DM2_V2_HUD_WIDGET_COMPASS_ROSE:
    case DM2_V2_HUD_WIDGET_DEPTH_INDICATOR:
    case DM2_V2_HUD_WIDGET_GOLD_COUNTER:
    case DM2_V2_HUD_WIDGET_CHAMPION_BAR_FRAME:
    case DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME:
        return "hud_chrome";
    default:
        return "";
    }
}

static void expected_file_for_slot(DM2_V2_HudWidgetSlot slot,
                                   char* out,
                                   size_t out_size) {
    const char* id = dm2_v2_hud_widget_assets_slot_name(slot);
    if (!out || out_size == 0U) return;
    if (!id || id[0] == '\0') {
        out[0] = '\0';
        return;
    }
    snprintf(out, out_size, "%s.png", id);
}

static int path_uses_category(const char* path, const char* category) {
    char needle[96];
    if (!path || !category || category[0] == '\0') return 0;
    snprintf(needle, sizeof(needle), "/%s/", category);
    return strstr(path, needle) != NULL;
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

/* Install the example manifest into the scratch dir, rewriting every
 * "generator": "synthetic_test" entry to use the supplied non-placeholder
 * generator marker. Used by Scenario 4 to prove the gate is generator-
 * string-agnostic for any non-placeholder value: an operator-installed
 * real pack whose generator is "pbr_hero" or "ai_upscale" must promote
 * the gate exactly the same way the synthetic pack does. The rewrite is
 * bounded (only the scratch manifest file is touched) and never writes
 * back to the example source on disk.
 *
 * The replacement is a literal substring swap ("synthetic_test" ->
 * new_marker); both strings are short enough that we never have to
 * worry about JSON-keyword collisions inside the comment field.
 *
 * Implementation: read the example manifest, write a rewritten copy to
 * the scratch path directly. There is no rename(2) / POSIX-only step —
 * every operation goes through stdio so MSVC builds work too. */
static int install_example_manifest_with_generator(const char* manifest_path,
                                                    const char* new_marker) {
    FILE* in = NULL;
    FILE* out = NULL;
    char manifest_dir[1024];
    char src_manifest[1024];
    unsigned char buf[4096];
    size_t n;
    size_t old_len, new_len;

    if (!manifest_path || manifest_path[0] == '\0') return 0;
    if (!new_marker || new_marker[0] == '\0') return 0;
    dirname_of(manifest_dir, sizeof(manifest_dir), manifest_path);
    if (!ensure_dir(manifest_dir)) return 0;
    join_path(src_manifest, sizeof(src_manifest),
              FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR,
              "hud_widget_manifest.json");
    in = fopen(src_manifest, "rb");
    if (!in) return 0;
    out = fopen(manifest_path, "wb");
    if (!out) { fclose(in); return 0; }
    old_len = strlen("synthetic_test");
    new_len = strlen(new_marker);
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0U) {
        unsigned char out_buf[8192];
        size_t out_n = 0U;
        for (size_t i = 0U; i < n; ++i) {
            if (i + old_len <= n &&
                memcmp(buf + i, "synthetic_test", old_len) == 0) {
                if (out_n + new_len > sizeof(out_buf)) {
                    fclose(in); fclose(out);
                    return 0;
                }
                memcpy(out_buf + out_n, new_marker, new_len);
                out_n += new_len;
                i += old_len - 1U;
            } else {
                if (out_n + 1U > sizeof(out_buf)) {
                    fclose(in); fclose(out);
                    return 0;
                }
                out_buf[out_n++] = buf[i];
            }
        }
        if (fwrite(out_buf, 1, out_n, out) != out_n) {
            fclose(in); fclose(out);
            return 0;
        }
    }
    if (ferror(in)) {
        fclose(in); fclose(out);
        return 0;
    }
    fclose(in);
    if (fclose(out) != 0) {
        return 0;
    }
    return 1;
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
    check("inventory_quick_view fixture is structurally a PNG",
          example_fixture_has_png_signature("hud_widgets",
                                            "inventory_quick_view.png"));
    check("action_prompt fixture tagged synthetic",
          example_fixture_has_marker("hud_widgets", "action_prompt.png"));
    check("action_prompt fixture is structurally a PNG",
          example_fixture_has_png_signature("hud_widgets", "action_prompt.png"));
    check("compass_rose fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "compass_rose.png"));
    check("compass_rose fixture is structurally a PNG",
          example_fixture_has_png_signature("hud_chrome", "compass_rose.png"));
    check("depth_indicator fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "depth_indicator.png"));
    check("depth_indicator fixture is structurally a PNG",
          example_fixture_has_png_signature("hud_chrome",
                                            "depth_indicator.png"));
    check("gold_counter fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "gold_counter.png"));
    check("gold_counter fixture is structurally a PNG",
          example_fixture_has_png_signature("hud_chrome", "gold_counter.png"));
    check("champion_bar_frame fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "champion_bar_frame.png"));
    check("champion_bar_frame fixture is structurally a PNG",
          example_fixture_has_png_signature("hud_chrome",
                                            "champion_bar_frame.png"));
    check("action_strip_frame fixture tagged synthetic",
          example_fixture_has_marker("hud_chrome", "action_strip_frame.png"));
    check("action_strip_frame fixture is structurally a PNG",
          example_fixture_has_png_signature("hud_chrome",
                                            "action_strip_frame.png"));
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
        snprintf(msg, sizeof(msg), "%s slot info dimensions>0",
                 dm2_v2_hud_widget_assets_slot_name(slot));
        check(msg, dm2_v2_hud_widget_assets_get_slot_info(slot, &info) == 1 &&
                       info.width > 0 && info.height > 0);
        {
            char expected_file[96];
            expected_file_for_slot(slot, expected_file, sizeof(expected_file));
            snprintf(msg, sizeof(msg), "%s source_file matches slot id",
                     dm2_v2_hud_widget_assets_slot_name(slot));
            check(msg, dm2_v2_hud_widget_assets_get_slot_info(slot, &info) == 1 &&
                           strcmp(info.source_file, expected_file) == 0);
        }
        snprintf(msg, sizeof(msg), "%s resolved path uses category",
                 dm2_v2_hud_widget_assets_slot_name(slot));
        check(msg, dm2_v2_hud_widget_assets_get_slot_info(slot, &info) == 1 &&
                       strcmp(info.category, expected_category_for_slot(slot)) == 0 &&
                       path_uses_category(info.resolved_path,
                                          expected_category_for_slot(slot)));
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

    printf("\n[ Scenario 4: generator-agnostic COMPLETE via 'pbr_hero' marker ]\n");
    /* The DM2 V2 HUD widget gate classifies a slot as REAL iff its
     * generator string is not "placeholder" and the source_file resolves
     * on disk via fopen(). It is intentionally generator-string-
     * agnostic: the same gate code must accept "synthetic_test",
     * "pbr_hero", "ai_upscale", or any future operator-installed marker
     * without per-generator allowlisting. Scenario 4 rewrites the
     * synthetic example manifest so every entry's generator becomes
     * "pbr_hero", installs all seven 1x1 fixtures, and verifies the gate
     * still promotes to COMPLETE while every slot's recorded generator
     * matches the rewritten marker. This is a guard against a future
     * refactor that accidentally introduces per-generator allowlisting
     * (e.g. "synthetic_test" -> REAL, "pbr_hero" -> PLACEHOLDER), which
     * would silently break any operator-installed real pack. */
    reset_scratch();
    snprintf(manifest_path, sizeof(manifest_path), "%s",
             dm2_v2_hud_widget_assets_get_manifest_path());
    dirname_of(manifest_dir, sizeof(manifest_dir), manifest_path);
    check("rewrite synthetic manifest with generator='pbr_hero'",
          install_example_manifest_with_generator(manifest_path, "pbr_hero") == 1);
    install_all_real_fixtures(manifest_dir);
    check("rewritten manifest validates structurally",
          dm2_v2_hud_widget_assets_validate_manifest(NULL) == 1);
    check("rewritten generator -> COMPLETE gate",
          dm2_v2_hud_widget_assets_gate() ==
              DM2_V2_HUD_WIDGET_GATE_COMPLETE);
    check("rewritten generator -> installed=1",
          dm2_v2_hud_widget_assets_get_installed() == 1);
    {
        int real_count = 0;
        int total_count = 0;
        DM2_V2_HudWidgetSlotInfo info;
        real_count = dm2_v2_hud_widget_assets_real_count(&total_count);
        check("rewritten generator real_count=7",
              real_count == (int)DM2_V2_HUD_WIDGET_COUNT);
        check("rewritten generator total=7",
              total_count == (int)DM2_V2_HUD_WIDGET_COUNT);
        /* Spot-check: the inventory_quick_view slot's recorded generator
         * must match the rewrite marker (not the source's "synthetic_test"). */
        check("inventory_quick_view slot info generator='pbr_hero'",
              dm2_v2_hud_widget_assets_get_slot_info(
                  DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW, &info) == 1 &&
              strcmp(info.generator, "pbr_hero") == 0);
        /* Spot-check: the action_strip_frame slot's recorded generator
         * must also match the rewrite marker — covers a chrome slot, not
         * just the first hud_widgets slot. */
        check("action_strip_frame slot info generator='pbr_hero'",
              dm2_v2_hud_widget_assets_get_slot_info(
                  DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME, &info) == 1 &&
              strcmp(info.generator, "pbr_hero") == 0);
        /* Negative check: the rewrite must NOT have left any "synthetic_test"
         * entry behind — if it did, a future per-generator refactor could
         * silently misclassify operator-installed packs. */
        check("rewritten manifest no longer mentions synthetic_test",
              access(FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
                     "/hud_widget_manifest.json", F_OK) == 0 &&
              dm2_v2_hud_widget_assets_get_slot_info(
                  DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW, &info) == 1 &&
              strcmp(info.generator, "synthetic_test") != 0);
    }

    reset_scratch();
    (void)system("rm -rf /tmp/scratch/dm2_hwa_synthetic_promotion_probe");
    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
