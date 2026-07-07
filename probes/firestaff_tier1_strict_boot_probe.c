/*
 * firestaff_tier1_strict_boot_probe.c
 * ===================================
 *
 * Tier 1 #5 strict-boot-probe per path (gap-list Section H).
 *
 * For each EXTRACTED + VERIFIED data path that --scan-data marks
 * READY, this probe runs the firestaff launcher with --game <id>
 * --data-dir <path> --boot-probe under SDL_VIDEODRIVER=dummy and
 * asserts per-game startup and runtime receipts.
 *
 * Pass: all present in-scope paths reach their startup/runtime receipts.
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_tier1_strict_boot_probe
 *
 * Source-lock: docs/FIRESTAFF_GAP_LIST.md Section H Tier 1 #5.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef FIRESTAFF_BIN
#define FIRESTAFF_BIN "./build/firestaff"
#endif
#ifndef DEFAULT_DATA_ROOT
#define DEFAULT_DATA_ROOT "/Users/bosse/.firestaff/data"
#endif

typedef struct {
    const char* game;
    const char* path_suffix;
    const char* expect_phase;
    const char* script;
    int boot_frames;
    const char* label;
    int expect_startup_active;
    int expect_level_loaded;
    int expect_runtime_tick_max;
    const char* expect_animation;
    int expect_animation_active;
    int expect_title_frame_min;
    int expect_title_frame_max;
    int expect_title_frame_boundary;
    int expect_title_ready;
} Tier1PathSpec;

#define EXPECT_DM1_RUNTIME   0, 1, -1, "dm1-title", 0, 53, 53, 53, 1
#define EXPECT_CSB_RUNTIME   0, 1, -1, "csb-runtime", 0, 53, 53, 53, 1
#define EXPECT_DM2_RUNTIME   0, 1, -1, "dm2-runtime", 0, 0, 0, 0, 1
#define EXPECT_NEXUS_RUNTIME 0, 1, -1, "nexus-runtime", 0, -1, -1, 54, 1
#define EXPECT_THERON_RUNTIME 0, 1, -1, "theron-runtime", 0, 0, 0, 0, 1

static const Tier1PathSpec kPaths[] = {
    { "dm1",   "dm1",
      "dm1-runtime", NULL, 2,
      "DM1 canonical", EXPECT_DM1_RUNTIME },
    { "dm1",   "dm1-multilingual",
      "dm1-runtime", NULL, 2,
      "DM1 multilingual explicit leaf", EXPECT_DM1_RUNTIME },
    { "dm1",   "dm1-extras/legacy-dos",
      "dm1-runtime", NULL, 2,
      "DM1 legacy-dos (M11 hash-fallback)", EXPECT_DM1_RUNTIME },
    { "dm1",   "dm1-extras/pc-3.4-en-3.5in",
      "dm1-runtime", NULL, 2,
      "DM1 PC 3.4 EN 3.5in disk layout", EXPECT_DM1_RUNTIME },
    { "dm1",   "dm1-extras/pc-3.4-multi-3.5in",
      "dm1-runtime", NULL, 2,
      "DM1 PC 3.4 multilingual 3.5in disk layout", EXPECT_DM1_RUNTIME },
    { "csb",   "csb",
      "csb-runtime", "enter", 240,
      "CSB canonical (M11 stderr-pipe)", EXPECT_CSB_RUNTIME },
    { "csb",   "csb-atari-st-2x",
      "csb-runtime", "enter", 240,
      "CSB Atari ST 2.x hard-disk explicit leaf", EXPECT_CSB_RUNTIME },
    { "csb",   "csb-extras/legacy-amiga-dms",
      "csb-runtime", "enter", 240,
      "CSB Amiga 3.3 Meynaf FR (M11 stderr-pipe)", EXPECT_CSB_RUNTIME },
    { "dm2",   "dm2",
      "dm2-runtime", "enter", 2,
      "DM2 canonical (M11 stderr-pipe)", EXPECT_DM2_RUNTIME },
    { "dm2",   "dm2-extras/dos-en",
      "dm2-runtime", "enter", 2,
      "DM2 DOS EN extras data/ layout (M11 stderr-pipe)", EXPECT_DM2_RUNTIME },
    { "dm2",   "dm2-extras/dos-fr",
      "dm2-runtime", "enter", 2,
      "DM2 DOS FR extras data/ layout (M11 stderr-pipe)", EXPECT_DM2_RUNTIME },
    { "dm2",   "dm2-extras/pc-fr",
      "dm2-runtime", "enter", 2,
      "DM2 PC FR extras DATA/ layout (M11 stderr-pipe)", EXPECT_DM2_RUNTIME },
    { "dm2",   "dm2-extras/pc-de",
      "dm2-runtime", "enter", 2,
      "DM2 PC DE extras DATA/ layout (M11 stderr-pipe)", EXPECT_DM2_RUNTIME },
    { "nexus", "nexus",
      "nexus-runtime", "wait120,enter,enter,action", 2,
      "Nexus canonical (Saturn ISO/CUE recursive scan)", EXPECT_NEXUS_RUNTIME },
    { "nexus", "nexus-extras/saturn-ja",
      "nexus-runtime", "wait120,enter,enter,action", 2,
      "Nexus Saturn JA extras (Track 1 BIN recursive scan)", EXPECT_NEXUS_RUNTIME },
    { "theron", "theron",
      "theron-runtime", "enter,enter,action", 2,
      "Theron JP canonical (Track 02.iso)", EXPECT_THERON_RUNTIME },
    { "theron", "theron-extras/japan",
      "theron-runtime", "enter,enter,action", 2,
      "Theron JP extras (Track 02.bin)", EXPECT_THERON_RUNTIME },
    { "theron", "theron-extras/usa",
      "theron-runtime", "enter,enter,action", 2,
      "Theron US extras (Track 02.bin, first-matched-version fallback)",
      EXPECT_THERON_RUNTIME },
    /* Sentinel. */
    { NULL, NULL, NULL, NULL, 0, NULL, -1, -1, -1, NULL, -1, -1, -1, -1, -1 }
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

static const char* data_root(void) {
    static char home_root[1024];
    const char* env = getenv("FIRESTAFF_DATA");
    const char* home;
    if (env && env[0]) {
        return env;
    }
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(home_root, sizeof(home_root), "%s/.firestaff/data", home);
        return home_root;
    }
    return DEFAULT_DATA_ROOT;
}

static int build_path(const char *root,
                      const char *suffix,
                      char *out,
                      size_t out_size) {
    int rc;
    if (!root || !suffix || !out || out_size == 0U) {
        return 0;
    }
    rc = snprintf(out, out_size, "%s/%s", root, suffix);
    return rc > 0 && (size_t)rc < out_size;
}

static int output_has_int_field(const char *buf,
                                const char *name,
                                int value) {
    char needle[64];
    int rc;
    if (!buf || !name || value < 0) {
        return 1;
    }
    rc = snprintf(needle, sizeof(needle), "%s=%d", name, value);
    if (rc <= 0 || (size_t)rc >= sizeof(needle)) {
        return 0;
    }
    return strstr(buf, needle) != NULL;
}

static int output_has_runtime_title_contract(const Tier1PathSpec *spec,
                                             const char *buf) {
    char needle[128];
    int rc;
    if (!spec || !buf) {
        return 0;
    }
    if (spec->expect_animation && spec->expect_animation[0] != '\0') {
        rc = snprintf(needle,
                      sizeof(needle),
                      "startupAnimation=%s",
                      spec->expect_animation);
        if (rc <= 0 || (size_t)rc >= sizeof(needle) ||
            strstr(buf, needle) == NULL) {
            return 0;
        }
    }
    if (!output_has_int_field(buf,
                              "startupAnimationActive",
                              spec->expect_animation_active)) {
        return 0;
    }
    if (spec->expect_title_frame_min >= 0 &&
        spec->expect_title_frame_min == spec->expect_title_frame_max &&
        !output_has_int_field(buf,
                              "titleFrame",
                              spec->expect_title_frame_min)) {
        return 0;
    }
    if (!output_has_int_field(buf,
                              "titleFrameMax",
                              spec->expect_title_frame_boundary)) {
        return 0;
    }
    if (!output_has_int_field(buf, "titleReady", spec->expect_title_ready)) {
        return 0;
    }
    return 1;
}

static int boot_probe_receipt_passed(const Tier1PathSpec *spec,
                                     const char *buf,
                                     int wait_status) {
    if (!spec || !buf) {
        return 0;
    }
    return wait_status == 0 &&
           strstr(buf, "FIRESTAFF BOOT PROBE READY") != NULL &&
           strstr(buf, spec->expect_phase) != NULL &&
           output_has_int_field(buf,
                                "startupActive",
                                spec->expect_startup_active) &&
           output_has_int_field(buf,
                                "levelLoaded",
                                spec->expect_level_loaded) &&
           output_has_int_field(buf,
                                "runtimeTick",
                                spec->expect_runtime_tick_max) &&
           output_has_runtime_title_contract(spec, buf);
}

static int make_startup_spec(const Tier1PathSpec *runtime_spec,
                             Tier1PathSpec *startup_spec,
                             char *label,
                             size_t label_size) {
    int rc;
    if (!runtime_spec || !startup_spec || !label || label_size == 0U) {
        return 0;
    }
    *startup_spec = *runtime_spec;
    startup_spec->script = NULL;
    startup_spec->boot_frames = 2;
    if (strcmp(runtime_spec->game, "dm1") == 0) {
        startup_spec->expect_phase = "dm1-runtime";
        startup_spec->expect_startup_active = 0;
        startup_spec->expect_level_loaded = 1;
        startup_spec->expect_runtime_tick_max = -1;
        startup_spec->expect_animation = "dm1-title";
        startup_spec->expect_animation_active = 0;
        startup_spec->expect_title_frame_min = 53;
        startup_spec->expect_title_frame_max = 53;
        startup_spec->expect_title_frame_boundary = 53;
        startup_spec->expect_title_ready = 1;
    } else if (strcmp(runtime_spec->game, "csb") == 0) {
        startup_spec->expect_phase = "csb-title-1";
        startup_spec->expect_startup_active = 1;
        startup_spec->expect_level_loaded = 1;
        startup_spec->expect_runtime_tick_max = 0;
        startup_spec->expect_animation = "csb-title";
        startup_spec->expect_animation_active = 1;
        startup_spec->expect_title_frame_min = 1;
        startup_spec->expect_title_frame_max = -1;
        startup_spec->expect_title_frame_boundary = 53;
        startup_spec->expect_title_ready = 0;
    } else if (strcmp(runtime_spec->game, "dm2") == 0) {
        startup_spec->expect_phase = "dm2-startup-menu";
        startup_spec->expect_startup_active = 1;
        startup_spec->expect_level_loaded = 1;
        startup_spec->expect_runtime_tick_max = 0;
        startup_spec->expect_animation = "dm2-startup-menu";
        startup_spec->expect_animation_active = 1;
        startup_spec->expect_title_frame_min = 0;
        startup_spec->expect_title_frame_max = 0;
        startup_spec->expect_title_frame_boundary = 0;
        startup_spec->expect_title_ready = 0;
    } else if (strcmp(runtime_spec->game, "nexus") == 0) {
        startup_spec->expect_phase = "nexus-title";
        startup_spec->expect_startup_active = 1;
        startup_spec->expect_level_loaded = 1;
        startup_spec->expect_runtime_tick_max = 0;
        startup_spec->expect_animation = "nexus-title";
        startup_spec->expect_animation_active = 1;
        startup_spec->expect_title_frame_min = 1;
        startup_spec->expect_title_frame_max = -1;
        startup_spec->expect_title_frame_boundary = 54;
        startup_spec->expect_title_ready = 0;
    } else if (strcmp(runtime_spec->game, "theron") == 0) {
        startup_spec->expect_phase = "theron-startup-0";
        startup_spec->expect_startup_active = 1;
        startup_spec->expect_level_loaded = 0;
        startup_spec->expect_runtime_tick_max = 0;
        startup_spec->expect_animation = "theron-title";
        startup_spec->expect_animation_active = 1;
        startup_spec->expect_title_frame_min = 0;
        startup_spec->expect_title_frame_max = 0;
        startup_spec->expect_title_frame_boundary = 0;
        startup_spec->expect_title_ready = 0;
    } else {
        return 0;
    }
    rc = snprintf(label,
                  label_size,
                  "%s startup",
                  runtime_spec->label ? runtime_spec->label : runtime_spec->game);
    if (rc <= 0 || (size_t)rc >= label_size) {
        return 0;
    }
    startup_spec->label = label;
    return 1;
}

static int run_firestaff_boot_probe(const Tier1PathSpec *spec,
                                    const char *path,
                                    char *buf,
                                    size_t buf_size) {
    int pipefd[2];
    pid_t pid;
    char frames[32];
    char expect_startup_active[16];
    char expect_level_loaded[16];
    char expect_runtime_tick_max[16];
    char expect_animation_active[16];
    char expect_title_frame_min[16];
    char expect_title_frame_max[16];
    char expect_title_frame_boundary[16];
    char expect_title_ready[16];
    const char *argv[64];
    int argc = 0;
    size_t used = 0;
    int status;
    time_t deadline;
    int child_done = 0;

    if (!spec || !path || !buf || buf_size == 0U) {
        return -1;
    }
    buf[0] = '\0';
    snprintf(frames, sizeof(frames), "%d", spec->boot_frames);

    argv[argc++] = firestaff_bin();
    argv[argc++] = "--game";
    argv[argc++] = spec->game;
    argv[argc++] = "--data-dir";
    argv[argc++] = path;
    argv[argc++] = "--boot-probe";
    argv[argc++] = "--boot-probe-frames";
    argv[argc++] = frames;
    if (spec->script && spec->script[0] != '\0') {
        argv[argc++] = "--script";
        argv[argc++] = spec->script;
    }
    argv[argc++] = "--boot-probe-expect-phase";
    argv[argc++] = spec->expect_phase;
    if (spec->expect_startup_active >= 0) {
        snprintf(expect_startup_active,
                 sizeof(expect_startup_active),
                 "%d",
                 spec->expect_startup_active);
        argv[argc++] = "--boot-probe-expect-startup-active";
        argv[argc++] = expect_startup_active;
    }
    if (spec->expect_level_loaded >= 0) {
        snprintf(expect_level_loaded,
                 sizeof(expect_level_loaded),
                 "%d",
                 spec->expect_level_loaded);
        argv[argc++] = "--boot-probe-expect-level-loaded";
        argv[argc++] = expect_level_loaded;
    }
    if (spec->expect_runtime_tick_max >= 0) {
        snprintf(expect_runtime_tick_max,
                 sizeof(expect_runtime_tick_max),
                 "%d",
                 spec->expect_runtime_tick_max);
        argv[argc++] = "--boot-probe-expect-runtime-tick-max";
        argv[argc++] = expect_runtime_tick_max;
    }
    if (spec->expect_animation && spec->expect_animation[0] != '\0') {
        argv[argc++] = "--boot-probe-expect-startup-animation";
        argv[argc++] = spec->expect_animation;
    }
    if (spec->expect_animation_active >= 0) {
        snprintf(expect_animation_active,
                 sizeof(expect_animation_active),
                 "%d",
                 spec->expect_animation_active);
        argv[argc++] = "--boot-probe-expect-startup-animation-active";
        argv[argc++] = expect_animation_active;
    }
    if (spec->expect_title_frame_min >= 0) {
        snprintf(expect_title_frame_min,
                 sizeof(expect_title_frame_min),
                 "%d",
                 spec->expect_title_frame_min);
        argv[argc++] = "--boot-probe-expect-title-frame-min";
        argv[argc++] = expect_title_frame_min;
    }
    if (spec->expect_title_frame_max >= 0) {
        snprintf(expect_title_frame_max,
                 sizeof(expect_title_frame_max),
                 "%d",
                 spec->expect_title_frame_max);
        argv[argc++] = "--boot-probe-expect-title-frame-max";
        argv[argc++] = expect_title_frame_max;
    }
    if (spec->expect_title_frame_boundary >= 0) {
        snprintf(expect_title_frame_boundary,
                 sizeof(expect_title_frame_boundary),
                 "%d",
                 spec->expect_title_frame_boundary);
        argv[argc++] = "--boot-probe-expect-title-frame-boundary";
        argv[argc++] = expect_title_frame_boundary;
    }
    if (spec->expect_title_ready >= 0) {
        snprintf(expect_title_ready,
                 sizeof(expect_title_ready),
                 "%d",
                 spec->expect_title_ready);
        argv[argc++] = "--boot-probe-expect-title-ready";
        argv[argc++] = expect_title_ready;
    }
    argv[argc++] = "--duration";
    argv[argc++] = "0";
    argv[argc] = NULL;

    if (pipe(pipefd) != 0) {
        snprintf(buf, buf_size, "pipe failed");
        return -1;
    }
    pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        snprintf(buf, buf_size, "fork failed");
        return -1;
    }
    if (pid == 0) {
        (void)setenv("SDL_VIDEODRIVER", "dummy", 1);
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execvp(firestaff_bin(), (char * const *)argv);
        perror("execvp firestaff");
        _exit(127);
    }

    close(pipefd[1]);
    deadline = time(NULL) + 35;
    while (!child_done || used + 1U < buf_size) {
        fd_set readfds;
        struct timeval tv;
        int ready;
        time_t now = time(NULL);

        if (!child_done) {
            pid_t wait_rc = waitpid(pid, &status, WNOHANG);
            if (wait_rc == pid) {
                child_done = 1;
            } else if (wait_rc < 0 && errno != EINTR) {
                child_done = 1;
                status = -1;
            }
        }

        if (!child_done && now >= deadline) {
            kill(pid, SIGKILL);
            (void)waitpid(pid, &status, 0);
            child_done = 1;
            status = 124 << 8;
        }

        FD_ZERO(&readfds);
        FD_SET(pipefd[0], &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        ready = select(pipefd[0] + 1, &readfds, NULL, NULL, &tv);
        if (ready > 0 && FD_ISSET(pipefd[0], &readfds)) {
            ssize_t n = read(pipefd[0], buf + used, buf_size - used - 1U);
            if (n > 0) {
                used += (size_t)n;
                buf[used] = '\0';
                continue;
            }
            if (n == 0) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (ready < 0 && errno != EINTR) {
            break;
        }
        if (child_done && ready == 0) {
            break;
        }
    }
    buf[used] = '\0';
    close(pipefd[0]);
    if (!child_done && waitpid(pid, &status, 0) < 0) {
        return -1;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static void run_path(const Tier1PathSpec* spec, const char *root) {
    char path[1024];
    char buf[8192];
    char startup_label[1100];
    Tier1PathSpec startup_spec;
    int wait_status;

    if (!spec->game) return;
    if (!build_path(root, spec->path_suffix, path, sizeof(path))) {
        printf("  FAIL: %s — path too long (%s/%s)\n",
               spec->label, root ? root : "", spec->path_suffix);
        ++g_fail;
        return;
    }
    if (!path_exists(path)) {
        printf("  SKIP: %s (%s missing — supply your own data)\n",
               spec->label, path);
        ++g_skipped;
        return;
    }

    if (make_startup_spec(spec,
                          &startup_spec,
                          startup_label,
                          sizeof(startup_label))) {
        wait_status = run_firestaff_boot_probe(&startup_spec,
                                               path,
                                               buf,
                                               sizeof(buf));
        if (boot_probe_receipt_passed(&startup_spec, buf, wait_status)) {
            printf("  PASS: %s (exit=%d, phase=%s, animation=%s)\n",
                   startup_spec.label,
                   wait_status,
                   startup_spec.expect_phase,
                   startup_spec.expect_animation);
            ++g_pass;
        } else {
            printf("  FAIL: %s — startup boot receipt %s not proven (exit=%d)\n",
                   startup_spec.label,
                   startup_spec.expect_phase,
                   wait_status);
            printf("    captured: %.200s%s\n",
                   buf,
                   strlen(buf) > 200 ? "..." : "");
            ++g_fail;
            return;
        }
    }

    wait_status = run_firestaff_boot_probe(spec, path, buf, sizeof(buf));

    if (boot_probe_receipt_passed(spec, buf, wait_status)) {
        printf("  PASS: %s (exit=%d, phase=%s, animation=%s)\n",
               spec->label,
               wait_status,
               spec->expect_phase,
               spec->expect_animation ? spec->expect_animation : "(none)");
        ++g_pass;
        return;
    }

    /* Out-of-scope-but-noted exclusions: silent CSB exit or direct-launch
     * failed prints do not count as failure for Tier 1 #5. */
    if (strstr(buf, "direct launch failed") != NULL ||
        strstr(buf, "phase-a run failed") != NULL) {
        printf("  FAIL: %s — direct-launch refused (%s)\n",
               spec->label, path);
        printf("    captured: %.200s%s\n", buf,
               strlen(buf) > 200 ? "..." : "");
        ++g_fail;
        return;
    }

    if (buf[0] == '\0') {
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
    const char *root = data_root();
    printf("=== Firestaff Tier 1 #5 strict boot-probe per path ===\n");
    printf("FIRESTAFF_BIN=%s\n", firestaff_bin());
    printf("DATA_ROOT=%s\n\n", root);

    for (size_t i = 0; kPaths[i].game != NULL; ++i) {
        printf("[%s]\n", kPaths[i].label);
        run_path(&kPaths[i], root);
    }

    printf("\n# summary: %d passed, %d failed, %d skipped\n",
           g_pass, g_fail, g_skipped);
    return g_fail > 0 ? 1 : 0;
}
