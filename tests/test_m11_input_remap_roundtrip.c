#include "input_remap_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int m11_test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-m11-remap-XXXXXX";
    const char* configPath;

    if (!mkdtemp(tmpTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    if (!expect(m11_test_setenv("HOME", tmpTemplate) == 0, "set HOME to test directory")) return 1;

    M11_Input_SetDefaults();
    M11_Input_SetBinding(M11_ACTION_ATTACK, 0, SDL_SCANCODE_J);
    M11_Input_SetBinding(M11_ACTION_ATTACK, 1, SDL_SCANCODE_K);

    configPath = M11_Input_GetConfigPath();
    if (!expect(strstr(configPath, tmpTemplate) != NULL, "config path should use temporary HOME")) return 1;
    if (!expect(M11_Input_Save() == 1, "save remap config")) return 1;
    if (!expect(access(configPath, R_OK) == 0, "saved keybinds.ini should exist")) return 1;

    M11_Input_SetDefaults();
    if (!expect(M11_Input_GetScancode(M11_ACTION_ATTACK) == SDL_SCANCODE_SPACE,
                "defaults should restore attack primary before reload")) return 1;
    if (!expect(M11_Input_Load() == 1, "load remap config")) return 1;

    if (!expect(M11_Input_GetScancode(M11_ACTION_ATTACK) == SDL_SCANCODE_J,
                "attack primary should round-trip")) return 1;
    if (!expect(M11_Input_GetSecondaryScancode(M11_ACTION_ATTACK) == SDL_SCANCODE_K,
                "attack secondary should round-trip")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_J) == M11_ACTION_ATTACK,
                "primary lookup should resolve remapped attack")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_K) == M11_ACTION_ATTACK,
                "secondary lookup should resolve remapped attack")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_SPACE) == M11_ACTION_COUNT,
                "old attack default should not remain bound after reload")) return 1;

    puts("ok: M11 input remap persists a minimal primary/secondary binding round-trip");
    return 0;
}
