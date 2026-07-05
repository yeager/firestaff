#include "gamepad_config_m12.h"

#include <stdio.h>

static int g_failures;

static void check_int(const char* label, int got, int expected) {
    if (got != expected) {
        fprintf(stderr, "FAIL: %s got=%d expected=%d\n", label, got, expected);
        ++g_failures;
    }
}

int main(void) {
    M12_GamepadMap map;
    const M12_GamepadAxisConfig* cfg;

    M12_GamepadMap_SetDefaults(&map);

    check_int("enabled", map.enabled, 1);
    check_int("dpad up -> move_forward",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_DPAD_UP),
              M12_ACTION_MOVE_FORWARD);
    check_int("dpad down -> move_backward",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_DPAD_DOWN),
              M12_ACTION_MOVE_BACKWARD);
    check_int("dpad left -> turn_left",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_DPAD_LEFT),
              M12_ACTION_TURN_LEFT);
    check_int("dpad right -> turn_right",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_DPAD_RIGHT),
              M12_ACTION_TURN_RIGHT);
    check_int("left shoulder -> strafe_left",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER),
              M12_ACTION_STRAFE_LEFT);
    check_int("right shoulder -> strafe_right",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER),
              M12_ACTION_STRAFE_RIGHT);
    check_int("south -> accept",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_SOUTH),
              M12_ACTION_ACCEPT);
    check_int("east -> back",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_EAST),
              M12_ACTION_BACK);
    check_int("west -> action",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_WEST),
              M12_ACTION_ACTION);
    check_int("north -> cycle champion",
              M12_GamepadMap_ActionForButton(&map, SDL_GAMEPAD_BUTTON_NORTH),
              M12_ACTION_CYCLE_CHAMPION);

    cfg = M12_GamepadMap_GetAxisConfig(&map, SDL_GAMEPAD_AXIS_LEFTY);
    check_int("left y role move", cfg ? cfg->role : -1, M12_AXIS_ROLE_MOVE);
    check_int("left y deadzone filters small motion",
              M12_GamepadAxis_Process(cfg, 1000), 0);
    check_int("left y positive survives deadzone",
              M12_GamepadAxis_Process(cfg, 20000) > 0, 1);
    check_int("left y negative survives deadzone",
              M12_GamepadAxis_Process(cfg, -20000) < 0, 1);

    cfg = M12_GamepadMap_GetAxisConfig(&map, SDL_GAMEPAD_AXIS_RIGHTX);
    check_int("right x role turn", cfg ? cfg->role : -1, M12_AXIS_ROLE_TURN);

    if (g_failures == 0) {
        puts("gamepad_config_m12: ok");
    }
    return g_failures ? 1 : 0;
}
