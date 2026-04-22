/*
 * firestaff_m12_modern_menu_probe.c
 *
 * Bounded M12 slice probe: exercises the new modern, high-resolution,
 * true-color startup-menu renderer and proves it meets the explicit
 * upgrade criteria:
 *
 *   INV_MODERN_01   Native canvas is 1280x720 (HD, modern).
 *   INV_MODERN_02   Renderer produces a non-empty output across all
 *                   cardinal views.
 *   INV_MODERN_03   Output uses far more than the 16-entry VGA palette,
 *                   proving real true-color output (>= 500 distinct
 *                   24-bit RGB values for the main view).
 *   INV_MODERN_04   Different views produce materially different
 *                   outputs (MAIN, SETTINGS, GAME OPTIONS, MESSAGE).
 *   INV_MODERN_05   V1 vs V2 vs V3 mode changes the rendered output
 *                   (the mode badge is active).
 *   INV_MODERN_06   Version selection is represented visually: a
 *                   different selected version yields a different
 *                   rendered byte signature.
 *   INV_MODERN_07   Checksum status is represented visually: toggling
 *                   availability changes the rendered output.
 *   INV_MODERN_08   PPM screenshots are written to the verification
 *                   directory so changes can be audited manually.
 *
 * The probe writes its artifacts to
 *   verification-m12/modern-menu/ (PPM files per view)
 */

#include "asset_status_m12.h"
#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    int total;
    int passed;
} Tally;

static void record(Tally* t, const char* id, int ok, const char* msg) {
    t->total += 1;
    if (ok) {
        t->passed += 1;
        printf("PASS %s %s\n", id, msg);
    } else {
        printf("FAIL %s %s\n", id, msg);
    }
}

static void ensure_dir(const char* path) {
    mkdir(path, 0777);
}

static int write_ppm(const char* path, const unsigned char* rgba, int w, int h) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; ++i) {
        unsigned char rgb[3] = {rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]};
        if (fwrite(rgb, 1, 3, fp) != 3) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

static unsigned long checksum(const unsigned char* buf, size_t n) {
    /* Simple FNV-1a over the RGB bytes (skip alpha). */
    unsigned long h = 2166136261UL;
    for (size_t i = 0; i < n; i += 4) {
        h ^= buf[i + 0]; h *= 16777619UL;
        h ^= buf[i + 1]; h *= 16777619UL;
        h ^= buf[i + 2]; h *= 16777619UL;
    }
    return h;
}

static int count_nonblack(const unsigned char* rgba, int w, int h) {
    int n = 0;
    for (int i = 0; i < w * h; ++i) {
        unsigned char r = rgba[i * 4 + 0];
        unsigned char g = rgba[i * 4 + 1];
        unsigned char b = rgba[i * 4 + 2];
        if (r > 10 || g > 10 || b > 10) n++;
    }
    return n;
}

int main(void) {
    Tally t = {0, 0};
    const int W = M12_ModernMenu_NativeWidth();
    const int H = M12_ModernMenu_NativeHeight();
    const size_t rgbaBytes = (size_t)W * (size_t)H * 4U;
    unsigned char* buf = (unsigned char*)malloc(rgbaBytes);
    unsigned char* bufAlt = (unsigned char*)malloc(rgbaBytes);
    if (!buf || !bufAlt) {
        fprintf(stderr, "allocation failed\n");
        free(buf); free(bufAlt);
        return 2;
    }

    const char* outDir = "verification-m12/modern-menu";
    ensure_dir("verification-m12");
    ensure_dir(outDir);

    /* ----- INV_MODERN_01: native canvas dimensions ----- */
    record(&t, "INV_MODERN_01", W == 1280 && H == 720,
           "native canvas is 1280x720");

    /* ----- Fresh default state ----- */
    M12_StartupMenuState state;
    M12_StartupMenu_Init(&state);

    /* ----- Render MAIN view ----- */
    memset(buf, 0, rgbaBytes);
    M12_ModernMenu_Render(&state, buf, W, H);
    int nonBlackMain = count_nonblack(buf, W, H);
    record(&t, "INV_MODERN_02A", nonBlackMain > W * H / 4,
           "main view renders substantial non-black content");
    write_ppm("verification-m12/modern-menu/01_main.ppm", buf, W, H);
    unsigned long sigMain = checksum(buf, rgbaBytes);

    /* ----- INV_MODERN_03: >> 16 distinct colours in main view ----- */
    int distinctMain = M12_ModernMenu_CountDistinctColors(buf, W, H, 2000);
    char msg03[160];
    snprintf(msg03, sizeof(msg03),
             "main view has %d distinct 24-bit colours (>= 500 required, VGA only has 16)",
             distinctMain);
    record(&t, "INV_MODERN_03", distinctMain >= 500, msg03);

    /* ----- Render SETTINGS view ----- */
    M12_StartupMenuState state_s = state;
    state_s.view = M12_MENU_VIEW_SETTINGS;
    state_s.settingsSelectedIndex = 1;
    state_s.settings.languageIndex = 1;
    state_s.settings.graphicsIndex = 1;
    state_s.settings.windowModeIndex = 1;
    memset(buf, 0, rgbaBytes);
    M12_ModernMenu_Render(&state_s, buf, W, H);
    record(&t, "INV_MODERN_02B", count_nonblack(buf, W, H) > W * H / 4,
           "settings view renders substantial non-black content");
    unsigned long sigSettings = checksum(buf, rgbaBytes);
    write_ppm("verification-m12/modern-menu/02_settings.ppm", buf, W, H);

    /* ----- Render GAME OPTIONS view ----- */
    M12_StartupMenuState state_go = state;
    state_go.view = M12_MENU_VIEW_GAME_OPTIONS;
    state_go.selectedIndex = 0;
    state_go.gameOptSelectedRow = 2;
    state_go.settings.graphicsIndex = 1; /* V2 to exercise mode badge */
    memset(buf, 0, rgbaBytes);
    M12_ModernMenu_Render(&state_go, buf, W, H);
    record(&t, "INV_MODERN_02C", count_nonblack(buf, W, H) > W * H / 4,
           "game options view renders substantial non-black content");
    unsigned long sigGameOpts = checksum(buf, rgbaBytes);
    write_ppm("verification-m12/modern-menu/03_game_options_v2.ppm", buf, W, H);

    /* ----- Render MESSAGE view ----- */
    M12_StartupMenuState state_msg = state;
    state_msg.view = M12_MENU_VIEW_MESSAGE;
    state_msg.messageLine1 = "READY TO LAUNCH";
    state_msg.messageLine2 = "DUNGEON MASTER";
    state_msg.messageLine3 = "ESC RETURNS TO MENU";
    memset(buf, 0, rgbaBytes);
    M12_ModernMenu_Render(&state_msg, buf, W, H);
    record(&t, "INV_MODERN_02D", count_nonblack(buf, W, H) > W * H / 4,
           "message view renders substantial non-black content");
    unsigned long sigMessage = checksum(buf, rgbaBytes);
    write_ppm("verification-m12/modern-menu/04_message.ppm", buf, W, H);

    /* ----- INV_MODERN_04: views differ ----- */
    int viewsDiffer = (sigMain != sigSettings) &&
                      (sigMain != sigGameOpts) &&
                      (sigMain != sigMessage) &&
                      (sigSettings != sigGameOpts) &&
                      (sigSettings != sigMessage) &&
                      (sigGameOpts != sigMessage);
    record(&t, "INV_MODERN_04", viewsDiffer,
           "each menu view produces a distinct rendered signature");

    /* ----- INV_MODERN_05: V1/V2/V3 mode badge changes output ----- */
    M12_StartupMenuState state_v1 = state; state_v1.settings.graphicsIndex = 0;
    M12_StartupMenuState state_v2 = state; state_v2.settings.graphicsIndex = 1;
    M12_StartupMenuState state_v3 = state; state_v3.settings.graphicsIndex = 2;
    memset(buf, 0, rgbaBytes);    M12_ModernMenu_Render(&state_v1, buf, W, H);    unsigned long sigV1 = checksum(buf, rgbaBytes);
    memset(bufAlt, 0, rgbaBytes); M12_ModernMenu_Render(&state_v2, bufAlt, W, H); unsigned long sigV2 = checksum(bufAlt, rgbaBytes);
    memset(buf, 0, rgbaBytes);    M12_ModernMenu_Render(&state_v3, buf, W, H);    unsigned long sigV3 = checksum(buf, rgbaBytes);
    record(&t, "INV_MODERN_05",
           sigV1 != sigV2 && sigV1 != sigV3 && sigV2 != sigV3,
           "presentation mode V1/V2/V3 alters rendered output (mode badge is live)");
    write_ppm("verification-m12/modern-menu/05_mode_v3.ppm", buf, W, H);

    /* ----- INV_MODERN_06: version selection changes output ----- */
    M12_StartupMenuState state_verA = state;
    M12_StartupMenuState state_verB = state;
    state_verA.gameOptions[0].versionIndex = 0;
    state_verB.gameOptions[0].versionIndex = 1;
    memset(buf, 0, rgbaBytes); M12_ModernMenu_Render(&state_verA, buf, W, H);
    unsigned long sigVerA = checksum(buf, rgbaBytes);
    memset(buf, 0, rgbaBytes); M12_ModernMenu_Render(&state_verB, buf, W, H);
    unsigned long sigVerB = checksum(buf, rgbaBytes);
    record(&t, "INV_MODERN_06", sigVerA != sigVerB,
           "version selection alters rendered output (version row is live)");

    /* ----- INV_MODERN_07: checksum availability changes output ----- */
    M12_StartupMenuState state_missing = state;
    state_missing.entries[0].available = 0;
    for (int i = 0; i < M12_ASSET_MAX_VERSIONS_PER_GAME; ++i) {
        state_missing.assetStatus.versions[0][i].matched = 0;
    }
    M12_StartupMenuState state_ok = state;
    state_ok.entries[0].available = 1;
    if (M12_ASSET_MAX_VERSIONS_PER_GAME > 0) {
        state_ok.assetStatus.versions[0][0].versionId = "dm1_3_6";
        state_ok.assetStatus.versions[0][0].shortLabel = "DM 3.6";
        state_ok.assetStatus.versions[0][0].matched = 1;
    }
    memset(buf, 0, rgbaBytes); M12_ModernMenu_Render(&state_missing, buf, W, H);
    unsigned long sigMissing = checksum(buf, rgbaBytes);
    memset(buf, 0, rgbaBytes); M12_ModernMenu_Render(&state_ok, buf, W, H);
    unsigned long sigOk = checksum(buf, rgbaBytes);
    record(&t, "INV_MODERN_07", sigMissing != sigOk,
           "checksum verified/missing status alters rendered output");
    write_ppm("verification-m12/modern-menu/06_checksum_ok.ppm", buf, W, H);

    /* ----- INV_MODERN_08: artifacts exist ----- */
    struct stat st;
    int allPpm =
        stat("verification-m12/modern-menu/01_main.ppm", &st) == 0 &&
        stat("verification-m12/modern-menu/02_settings.ppm", &st) == 0 &&
        stat("verification-m12/modern-menu/03_game_options_v2.ppm", &st) == 0 &&
        stat("verification-m12/modern-menu/04_message.ppm", &st) == 0 &&
        stat("verification-m12/modern-menu/05_mode_v3.ppm", &st) == 0 &&
        stat("verification-m12/modern-menu/06_checksum_ok.ppm", &st) == 0;
    record(&t, "INV_MODERN_08", allPpm,
           "PPM screenshots written to verification-m12/modern-menu/");

    printf("# summary: %d/%d invariants passed\n", t.passed, t.total);
    free(buf); free(bufAlt);
    return (t.total > 0 && t.passed == t.total) ? 0 : 1;
}
