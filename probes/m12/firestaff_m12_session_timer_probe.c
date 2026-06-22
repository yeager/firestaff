#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "config_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(_WIN32)
#include <process.h>
static int portable_make_dir(const char* path) {
    return mkdir(path) == 0;
}
static char* portable_mkdtemp(char* templ) {
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) {
        return NULL;
    }
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld", ((long)_getpid() + i) % 1000000L);
        if (mkdir(templ) == 0) {
            return templ;
        }
    }
    return NULL;
}
static int portable_setenv(const char* name, const char* value, int overwrite) {
    (void)overwrite;
    return _putenv_s(name, value);
}
#else
static int portable_make_dir(const char* path) {
    return mkdir(path, 0777) == 0;
}
static char* portable_mkdtemp(char* templ) {
    return mkdtemp(templ);
}
static int portable_setenv(const char* name, const char* value, int overwrite) {
    return setenv(name, value, overwrite);
}
#endif

typedef struct {
    int total;
    int passed;
} ProbeTally;

static void probe_record(ProbeTally* tally,
                         const char* id,
                         int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

static int make_dir(const char* path) {
    return portable_make_dir(path);
}

static int file_contains(const char* path, const char* needle) {
    FILE* fp;
    char line[1024];
    if (!path || !needle || needle[0] == '\0') {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, needle) != NULL) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int main(void) {
    ProbeTally tally = {0, 0};
    M12_Config config;
    M12_Config imported;
    M12_StartupMenuState state;
    M12_StartupMenuState reloaded;
    char rootTemplate[] = "/tmp/firestaff-m12-session-timer-XXXXXX";
    char firestaffDir[1024];
    char dataDir[1024];
    char configPath[1024];
    char exportPath[1024];
    char* rootDir = portable_mkdtemp(rootTemplate);

    if (!rootDir) {
        perror("mkdtemp");
        return 2;
    }
    if (portable_setenv("HOME", rootDir, 1) != 0 ||
        portable_setenv("LANG", "C", 1) != 0) {
        perror("setenv");
        return 2;
    }

    snprintf(firestaffDir, sizeof(firestaffDir), "%s/.firestaff", rootDir);
    snprintf(dataDir, sizeof(dataDir), "%s/data", firestaffDir);
#if defined(__APPLE__)
    snprintf(configPath, sizeof(configPath), "%s/Library/Application Support/Firestaff/startup-menu.toml", rootDir);
#else
    snprintf(configPath, sizeof(configPath), "%s/.config/firestaff/startup-menu.toml", rootDir);
#endif
    snprintf(exportPath, sizeof(exportPath), "%s/session-timer-export.json", firestaffDir);

    if (!make_dir(firestaffDir) || !make_dir(dataDir)) {
        perror("mkdir");
        return 2;
    }

    M12_Config_SetDefaults(&config);
    probe_record(&tally,
                 "SESSION_TIMER_00",
                 config.sessionTimerIndex == M12_SessionTimer_IndexForMinutes(0) &&
                     M12_SessionTimer_MinutesForIndex(config.sessionTimerIndex) == 0,
                 "config defaults keep the session timer disabled");

    M12_StartupMenu_InitWithDataDir(&state, dataDir, NULL);
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = 30; /* Session Timer row in the launcher settings list. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);

    M12_StartupMenu_InitWithDataDir(&reloaded, dataDir, NULL);
    probe_record(&tally,
                 "SESSION_TIMER_01",
                 state.settings.sessionTimerIndex == M12_SessionTimer_IndexForMinutes(60) &&
                     reloaded.settings.sessionTimerIndex == M12_SessionTimer_IndexForMinutes(60) &&
                     M12_StartupMenu_SessionTimerLimitMinutes(&reloaded) == 60 &&
                     file_contains(configPath, "session_timer_index = 3"),
                 "settings row cycles to 60 minutes and persists through TOML reload");

    probe_record(&tally,
                 "SESSION_TIMER_02",
                 M12_StartupMenu_SessionTimerRemainingSeconds(&reloaded, -10) == 3600 &&
                     M12_StartupMenu_SessionTimerRemainingSeconds(&reloaded, 3590) == 10 &&
                     M12_StartupMenu_SessionTimerRemainingSeconds(&reloaded, 3600) == 0 &&
                     M12_StartupMenu_SessionTimerRemainingSeconds(NULL, 10) == -1,
                 "remaining-time helper clamps negative elapsed time and expires at zero");

    M12_Config_Load(&config, dataDir);
    config.sessionTimerIndex = M12_SessionTimer_IndexForMinutes(120);
    probe_record(&tally,
                 "SESSION_TIMER_03",
                 M12_Config_ExportJSON(&config, exportPath) &&
                     file_contains(exportPath, "\"session_timer_index\": 4"),
                 "JSON export includes the session timer preference");

    M12_Config_SetDefaults(&imported);
    probe_record(&tally,
                 "SESSION_TIMER_04",
                 M12_Config_ImportJSON(&imported, exportPath) &&
                     imported.sessionTimerIndex == M12_SessionTimer_IndexForMinutes(120),
                 "JSON import restores the session timer preference");

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
