/*
 * firestaff_tier1_strict_boot_probe.c
 * ===================================
 *
 * Tier 1 #5 strict-boot-probe per path (gap-list Section H).
 *
 * For each EXTRACTED + VERIFIED data path that --scan-data marks
 * READY, this probe runs the firestaff launcher with --game <id>
 * --data-dir <path> --boot-probe under SDL_VIDEODRIVER=dummy and
 * asserts the per-game runtime receipt. Runtime phases are validated by
 * main_loop_m11.c as active=1, startupActive=0, and levelLoaded=1.
 *
 * Paths intentionally excluded as out-of-scope for Tier 1 #5:
 *   - Nexus canonical + Nexus saturn-ja — the M11 launcher
 *     cannot open `Merged.iso::DM.BIN` or `Track 1.bin::DM.BIN`
 *     without an extract step; that is a Tier 4 launcher gap
 *     (Nexus runtime coverage), not a path-discovery gap.
 *
 * Pass: all present in-scope paths reach their boot milestone.
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_tier1_strict_boot_probe
 *
 * Source-lock: docs/FIRESTAFF_GAP_LIST.md Section H Tier 1 #5.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef FIRESTAFF_BIN
#define FIRESTAFF_BIN "./build/firestaff"
#endif
#ifndef DEFAULT_DATA_ROOT
#define DEFAULT_DATA_ROOT "/Users/bosse/.firestaff/data"
#endif

typedef struct {
    const char* game;
    const char* path;
    const char* expect_phase;
    const char* script;
    int boot_frames;
    const char* label;
} Tier1PathSpec;

static const Tier1PathSpec kPaths[] = {
    { "dm1",   DEFAULT_DATA_ROOT "/dm1",
      "dm1-runtime", NULL, 2,
      "DM1 canonical" },
    { "dm1",   DEFAULT_DATA_ROOT "/dm1-extras/legacy-dos",
      "dm1-runtime", NULL, 2,
      "DM1 legacy-dos (M11 hash-fallback)" },
    { "csb",   DEFAULT_DATA_ROOT "/csb",
      "csb-runtime", "enter", 240,
      "CSB canonical (M11 stderr-pipe)" },
    { "csb",   DEFAULT_DATA_ROOT "/csb-extras/legacy-amiga-dms",
      "csb-runtime", "enter", 240,
      "CSB Amiga 3.3 Meynaf FR (M11 stderr-pipe)" },
    { "dm2",   DEFAULT_DATA_ROOT "/dm2",
      "dm2-runtime", "enter", 2,
      "DM2 canonical (M11 stderr-pipe)" },
    { "dm2",   DEFAULT_DATA_ROOT "/dm2-extras/dos-en",
      "dm2-runtime", "enter", 2,
      "DM2 DOS EN extras data/ layout (M11 stderr-pipe)" },
    { "dm2",   DEFAULT_DATA_ROOT "/dm2-extras/dos-fr",
      "dm2-runtime", "enter", 2,
      "DM2 DOS FR extras data/ layout (M11 stderr-pipe)" },
    { "dm2",   DEFAULT_DATA_ROOT "/dm2-extras/pc-fr",
      "dm2-runtime", "enter", 2,
      "DM2 PC FR extras DATA/ layout (M11 stderr-pipe)" },
    { "dm2",   DEFAULT_DATA_ROOT "/dm2-extras/pc-de",
      "dm2-runtime", "enter", 2,
      "DM2 PC DE extras DATA/ layout (M11 stderr-pipe)" },
    { "theron", DEFAULT_DATA_ROOT "/theron",
      "theron-runtime", "enter,enter,action", 2,
      "Theron JP canonical (Track 02.iso)" },
    { "theron", DEFAULT_DATA_ROOT "/theron-extras/japan",
      "theron-runtime", "enter,enter,action", 2,
      "Theron JP extras (Track 02.bin)" },
    { "theron", DEFAULT_DATA_ROOT "/theron-extras/usa",
      "theron-runtime", "enter,enter,action", 2,
      "Theron US extras (Track 02.bin, first-matched-version fallback)" },
    /* Sentinel. */
    { NULL, NULL, NULL, NULL, 0, NULL }
};

static int g_pass = 0;
static int g_fail = 0;
static int g_skipped = 0;

static int path_exists(const char* p) {
    struct stat st;
    return (p && stat(p, &st) == 0) ? 1 : 0;
}

static const char* firestaff_bin(void) {
    const char* env = getenv("FIRESTAFF_BIN");
    return (env && env[0]) ? env : FIRESTAFF_BIN;
}

static void run_path(const Tier1PathSpec* spec) {
    if (!spec->game) return;
    if (!path_exists(spec->path)) {
        printf("  SKIP: %s (%s missing — supply your own data)\n",
               spec->label, spec->path);
        ++g_skipped;
        return;
    }

    char cmd[1400];
    if (spec->script && spec->script[0] != '\0') {
        snprintf(cmd, sizeof(cmd),
                 "SDL_VIDEODRIVER=dummy timeout 35 %s --game %s --data-dir '%s' --boot-probe --boot-probe-frames %d --script '%s' --boot-probe-expect-phase %s --duration 0 2>&1",
                 firestaff_bin(), spec->game, spec->path,
                 spec->boot_frames, spec->script, spec->expect_phase);
    } else {
        snprintf(cmd, sizeof(cmd),
                 "SDL_VIDEODRIVER=dummy timeout 35 %s --game %s --data-dir '%s' --boot-probe --boot-probe-frames %d --boot-probe-expect-phase %s --duration 0 2>&1",
                 firestaff_bin(), spec->game, spec->path,
                 spec->boot_frames, spec->expect_phase);
    }

    FILE* f = popen(cmd, "r");
    if (!f) {
        printf("  FAIL: %s — popen failed\n", spec->label);
        ++g_fail;
        return;
    }

    char buf[8192];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    int rc = pclose(f);
    int wait_status = WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;

    if (wait_status == 0 &&
        strstr(buf, "FIRESTAFF BOOT PROBE READY") != NULL &&
        strstr(buf, spec->expect_phase) != NULL &&
        strstr(buf, "startupActive=0") != NULL &&
        strstr(buf, "levelLoaded=1") != NULL) {
        printf("  PASS: %s (exit=%d, phase=%s)\n",
               spec->label, wait_status, spec->expect_phase);
        ++g_pass;
        return;
    }

    /* Out-of-scope-but-noted exclusions: silent CSB exit or direct-launch
     * failed prints do not count as failure for Tier 1 #5. */
    if (strstr(buf, "direct launch failed") != NULL ||
        strstr(buf, "phase-a run failed") != NULL) {
        printf("  FAIL: %s — direct-launch refused (%s)\n",
               spec->label, spec->path);
        printf("    captured: %.200s%s\n", buf,
               strlen(buf) > 200 ? "..." : "");
        ++g_fail;
        return;
    }

    if (n == 0) {
        printf("  FAIL: %s — silent exit (CSB-style launcher issue)\n",
               spec->label);
        ++g_fail;
        return;
    }

    printf("  FAIL: %s — runtime boot receipt %s not proven (exit=%d)\n",
           spec->label, spec->expect_phase, wait_status);
    printf("    captured: %.200s%s\n", buf, strlen(buf) > 200 ? "..." : "");
    ++g_fail;
}

int main(void) {
    printf("=== Firestaff Tier 1 #5 strict boot-probe per path ===\n");
    printf("FIRESTAFF_BIN=%s\n", firestaff_bin());
    printf("DEFAULT_DATA_ROOT=%s\n\n", DEFAULT_DATA_ROOT);

    for (size_t i = 0; kPaths[i].game != NULL; ++i) {
        printf("[%s]\n", kPaths[i].label);
        run_path(&kPaths[i]);
    }

    printf("\n# summary: %d passed, %d failed, %d skipped\n",
           g_pass, g_fail, g_skipped);
    return g_fail > 0 ? 1 : 0;
}
