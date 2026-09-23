#include "firestaff_nexus_mednafen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

static int fs_nexus_readable(const char* path) {
#if defined(_WIN32)
    return path && path[0] && _access(path, 4) == 0;
#else
    return path && path[0] && access(path, R_OK) == 0;
#endif
}

static int fs_nexus_executable(const char* path) {
#if defined(_WIN32)
    return path && path[0] && _access(path, 0) == 0;
#else
    return path && path[0] && access(path, X_OK) == 0;
#endif
}

static int fs_nexus_copy(char* out, size_t out_size, const char* value) {
    int written;
    if (!out || !out_size || !value) return 0;
    written = snprintf(out, out_size, "%s", value);
    return written >= 0 && (size_t)written < out_size;
}

static int fs_nexus_cue(const char* path) {
    size_t length = path ? strlen(path) : 0U;
    return length >= 4U && strcmp(path + length - 4U, ".cue") == 0;
}

static int fs_nexus_find_disc(const char* data_dir, char* out, size_t out_size) {
    static const char cue_name[] = "Dungeon Master Nexus (English).cue";
    const char* root = data_dir && data_dir[0] ? data_dir : getenv("FIRESTAFF_DATA");
    int written;
    if (!root || !root[0]) return 0;
    if (fs_nexus_cue(root) && fs_nexus_readable(root))
        return fs_nexus_copy(out, out_size, root);
    written = snprintf(out, out_size, "%s/%s", root, cue_name);
    if (written >= 0 && (size_t)written < out_size && fs_nexus_readable(out))
        return 1;
    written = snprintf(out, out_size, "%s/nexus/%s", root, cue_name);
    return written >= 0 && (size_t)written < out_size && fs_nexus_readable(out);
}

int Firestaff_NexusMednafen_Discover(const char* dataDir,
                                     const char* emulatorOverride,
                                     const char* discOverride,
                                     const char* biosOverride,
                                     Firestaff_NexusMednafenLaunch* out) {
    static const char* const brew_candidates[] = {
        "/opt/homebrew/bin/mednafen", "/usr/local/bin/mednafen"
    };
    const char* emulator = emulatorOverride && emulatorOverride[0]
        ? emulatorOverride : getenv("FIRESTAFF_NEXUS_MEDNAFEN");
    const char* disc = discOverride && discOverride[0]
        ? discOverride : getenv("FIRESTAFF_NEXUS_DISC");
    const char* bios = biosOverride && biosOverride[0]
        ? biosOverride : getenv("FIRESTAFF_NEXUS_BIOS");
    size_t i;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!emulator || !emulator[0]) {
        for (i = 0U; i < sizeof(brew_candidates) / sizeof(brew_candidates[0]); ++i) {
            if (fs_nexus_executable(brew_candidates[i])) {
                emulator = brew_candidates[i];
                break;
            }
        }
    }
    if (!fs_nexus_executable(emulator) || !fs_nexus_copy(out->emulator, sizeof(out->emulator), emulator))
        return 0;
    if (disc && disc[0]) {
        if (!fs_nexus_cue(disc) || !fs_nexus_readable(disc) ||
            !fs_nexus_copy(out->disc, sizeof(out->disc), disc)) return 0;
    } else if (!fs_nexus_find_disc(dataDir, out->disc, sizeof(out->disc))) {
        return 0;
    }
    if (bios && bios[0]) {
        if (!fs_nexus_readable(bios) || !fs_nexus_copy(out->bios, sizeof(out->bios), bios))
            return 0;
        out->hasBios = 1;
    }
    return 1;
}

int Firestaff_NexusMednafen_Launch(const Firestaff_NexusMednafenLaunch* launch) {
#if defined(_WIN32)
    (void)launch;
    return -1;
#else
    char* argv[5];
    int argc = 0;
    pid_t child;
    int status;
    if (!launch || !launch->emulator[0] || !launch->disc[0]) return -1;
    argv[argc++] = (char*)launch->emulator;
    if (launch->hasBios) {
        argv[argc++] = "-ss.bios_jp";
        argv[argc++] = (char*)launch->bios;
    }
    argv[argc++] = (char*)launch->disc;
    argv[argc] = NULL;
    child = fork();
    if (child < 0) return -1;
    if (child == 0) {
        execv(launch->emulator, argv);
        _exit(127);
    }
    if (waitpid(child, &status, 0) < 0) return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}
