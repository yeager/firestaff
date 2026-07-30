/* CSB controller bridge regression.
 *
 * The SDL event loop resolves a configured gamepad button/axis to an M12
 * action, then this shared M11 mapper emits the same source input token that
 * CSB's COMMAND.C keyboard and mouse routes consume.  Keep this device layer
 * independent from a live controller so every build can verify it.
 */

#include "main_loop_m11.h"
#include "gamepad_config_m12.h"

#include <stdio.h>

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static void expect_button(const M12_GamepadMap *map, SDL_GamepadButton button,
                          M12_MenuInput expected) {
    M12_InputAction action = M12_GamepadMap_ActionForButton(map, button);
    CHECK(action != M12_ACTION_COUNT);
    CHECK(M11_GamepadActionToMenuInput(action, 1) == expected);
}

int main(void) {
    M12_GamepadMap map;

    M12_GamepadMap_SetDefaults(&map);
    CHECK(map.enabled == 1);
    CHECK(M11_GamepadEnabledForInputMode(0, 1) == 1);
    CHECK(M11_GamepadEnabledForInputMode(1, 1) == 0);
    CHECK(M11_GamepadEnabledForInputMode(2, 1) == 0);
    CHECK(M11_GamepadEnabledForInputMode(3, 1) == 1);
    CHECK(M11_GamepadEnabledForInputMode(99, 1) == 1);
    CHECK(M11_GamepadEnabledForInputMode(3, 0) == 0);

    /* Default CSB gameplay mapping: these are source input tokens, not a
     * parallel controller simulation.  M11_GameView_HandleInput forwards
     * them to the CSB GAMEBLOCK/COMMAND.C owner. */
    expect_button(&map, SDL_GAMEPAD_BUTTON_DPAD_UP, M12_MENU_INPUT_UP);
    expect_button(&map, SDL_GAMEPAD_BUTTON_DPAD_DOWN, M12_MENU_INPUT_DOWN);
    expect_button(&map, SDL_GAMEPAD_BUTTON_DPAD_LEFT, M12_MENU_INPUT_TURN_LEFT);
    expect_button(&map, SDL_GAMEPAD_BUTTON_DPAD_RIGHT, M12_MENU_INPUT_TURN_RIGHT);
    expect_button(&map, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, M12_MENU_INPUT_STRAFE_LEFT);
    expect_button(&map, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, M12_MENU_INPUT_STRAFE_RIGHT);
    expect_button(&map, SDL_GAMEPAD_BUTTON_SOUTH, M12_MENU_INPUT_ACCEPT);
    expect_button(&map, SDL_GAMEPAD_BUTTON_EAST, M12_MENU_INPUT_BACK);
    expect_button(&map, SDL_GAMEPAD_BUTTON_WEST, M12_MENU_INPUT_ACTION);
    expect_button(&map, SDL_GAMEPAD_BUTTON_NORTH, M12_MENU_INPUT_CYCLE_CHAMPION);
    expect_button(&map, SDL_GAMEPAD_BUTTON_BACK, M12_MENU_INPUT_REST_TOGGLE);
    expect_button(&map, SDL_GAMEPAD_BUTTON_START, M12_MENU_INPUT_INVENTORY_TOGGLE);

    CHECK(M11_GamepadActionToMenuInput(M12_ACTION_TURN_LEFT, 0) ==
          M12_MENU_INPUT_LEFT);
    CHECK(M11_GamepadActionToMenuInput(M12_ACTION_STRAFE_RIGHT, 0) ==
          M12_MENU_INPUT_RIGHT);
    CHECK(M11_GamepadAxisToMenuInput(SDL_GAMEPAD_AXIS_LEFTY,
                                     M12_AXIS_ROLE_MOVE, -16000, 1) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_GamepadAxisToMenuInput(SDL_GAMEPAD_AXIS_LEFTX,
                                     M12_AXIS_ROLE_MOVE, 16000, 1) ==
          M12_MENU_INPUT_STRAFE_RIGHT);
    CHECK(M11_GamepadAxisToMenuInput(SDL_GAMEPAD_AXIS_RIGHTX,
                                     M12_AXIS_ROLE_TURN, -16000, 1) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_GamepadAxisToMenuInput(SDL_GAMEPAD_AXIS_LEFTY,
                                     M12_AXIS_ROLE_MOVE, 15999, 1) ==
          M12_MENU_INPUT_NONE);

    if (failures) return 1;
    puts("PASS: m11_gamepad_csb_input_bridge");
    return 0;
}
