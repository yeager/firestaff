#include "firestaff_save.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#define unlink(path) _unlink(path)
#define rmdir(path) _rmdir(path)
static char* portable_mkdtemp(char* templ) {
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) return NULL;
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld", ((long)_getpid() + i) % 1000000L);
        if (_mkdir(templ) == 0) return templ;
    }
    return NULL;
}
#else
#include <unistd.h>
static char* portable_mkdtemp(char* templ) {
    return mkdtemp(templ);
}
#endif

static int expect(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
#ifdef _WIN32
    char tmpTemplate[] = ".\\firestaff-save-root-XXXXXX";
#else
    char tmpTemplate[] = "/tmp/firestaff-save-root-XXXXXX";
#endif
    char path[256];
    FS_GameState state;
    FS_GameState loaded;

    if (!portable_mkdtemp(tmpTemplate)) {
        perror("portable_mkdtemp");
        return 1;
    }

    memset(&state, 0, sizeof(state));
    state.config.game = FS_GAME_CSB;
    state.config.version = FS_VERSION_V1;
    state.config.save_dir = tmpTemplate;
    state.current_level = 7;
    state.party_x = 11;
    state.party_y = 13;
    state.party_direction = 2;

    if (!expect(fs_save_slot_path_in_dir(tmpTemplate, FS_GAME_CSB, 3,
                                         path, (int)sizeof(path)) != NULL,
                "save-root path resolver succeeds")) return 1;
    if (!expect(strstr(path, "csb_slot3.sav") != NULL,
                "save-root path names CSB slot")) return 1;
    if (!expect(strstr(path, tmpTemplate) == path,
                "save-root path starts with requested save dir")) return 1;
    if (!expect(fs_save_exists_in_dir(tmpTemplate, FS_GAME_CSB, 3) == 0,
                "empty save root has no slot")) return 1;
    if (!expect(fs_save_game(&state, 3) == 0,
                "save writes to requested save root")) return 1;
    if (!expect(fs_save_exists_in_dir(tmpTemplate, FS_GAME_CSB, 3) == 1,
                "save exists in requested save root")) return 1;

    memset(&loaded, 0, sizeof(loaded));
    loaded.config.game = FS_GAME_CSB;
    loaded.config.version = FS_VERSION_V1;
    loaded.config.save_dir = tmpTemplate;
    if (!expect(fs_load_game(&loaded, 3) == 0,
                "load reads from requested save root")) return 1;
    if (!expect(loaded.current_level == 7 &&
                loaded.party_x == 11 &&
                loaded.party_y == 13 &&
                loaded.party_direction == 2,
                "load preserves saved slot header state")) return 1;

    unlink(path);
    rmdir(tmpTemplate);
    puts("test_firestaff_save_root: PASS");
    return 0;
}
