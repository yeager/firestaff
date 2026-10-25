/* Shared M11 controller bridge regression.
 *
 * The SDL event loop resolves a configured gamepad button/axis to an M12
 * action, then this shared M11 mapper emits the same source input token that
 * game-specific input routes consume. Keep this device layer independent
 * from a live controller so every build can verify it.
 */

#include "main_loop_m11.h"
#include "gamepad_config_m12.h"
#include "input_remap_m11.h"

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
    M12_GamepadStatus status = {0};

    M11_Input_SetDefaults();
    M12_GamepadMap_SetDefaults(&map);
    CHECK(map.enabled == 1);
    CHECK(M11_GamepadEnabledForInputMode(0, 1) == 1);
    CHECK(M11_GamepadEnabledForInputMode(1, 1) == 0);
    CHECK(M11_GamepadEnabledForInputMode(2, 1) == 0);
    CHECK(M11_GamepadEnabledForInputMode(3, 1) == 1);
    CHECK(M11_GamepadEnabledForInputMode(99, 1) == 1);
    CHECK(M11_GamepadEnabledForInputMode(3, 0) == 0);

    /* Controller routing must initialize SDL's gamepad subsystem before
     * asking SDL for attached devices, and release only its own reference. */
#if SDL_VERSION_ATLEAST(3, 0, 0)
    CHECK(SDL_WasInit(SDL_INIT_GAMEPAD) == 0);
    M12_GamepadStatus_Update(&status);
    CHECK(status.subsystemInitialized == 1);
    CHECK((SDL_WasInit(SDL_INIT_GAMEPAD) & SDL_INIT_GAMEPAD) != 0);
    M12_GamepadStatus_Close(&status);
    CHECK(status.subsystemInitialized == 0);
    CHECK(SDL_WasInit(SDL_INIT_GAMEPAD) == 0);
#endif

    /* ReDMCSB GAMELOOP.C consumes held commands on its source input
     * boundary.  DM1, DM2 and CSB participate in this sampler; a controller
     * axis emits only one SDL motion event while it remains held. */
    CHECK(M11_InputSourceSupportsHeldMotion("dm1", 1) == 1);
    CHECK(M11_InputSourceSupportsHeldMotion("csb", 1) == 1);
    CHECK(M11_InputSourceSupportsHeldMotion("dm2", 1) == 1);
    CHECK(M11_InputSourceSupportsHeldMotion("theron", 1) == 1);
    CHECK(M11_InputSourceSupportsHeldMotion("csb", 0) == 0);
    CHECK(M11_InputSourceSupportsHeldMotion(NULL, 1) == 0);

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
    /* Theron's use-item command is distinct from generic Action, which is a
     * wait/tick in its native runtime. */
    CHECK(M11_GamepadActionToMenuInput(M12_ACTION_USE_ITEM, 1) ==
          M12_MENU_INPUT_USE_ITEM);
    CHECK(M11_GamepadActionToMenuInput(M12_ACTION_USE_ITEM, 0) ==
          M12_MENU_INPUT_USE_ITEM);
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

    /* Theron's retail movement panel has separate turn and side-step
     * commands. Arrows/keypad turn; A/D reach original commands $06/$04. */
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_W) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_S) ==
          M12_MENU_INPUT_DOWN);
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_A) ==
          M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_D) ==
          M12_MENU_INPUT_STRAFE_RIGHT);
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_KP_8) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_KP_2) ==
          M12_MENU_INPUT_DOWN);
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_KP_4) ==
          M12_MENU_INPUT_LEFT);
    CHECK(M11_TheronNavigationInputFromScancode(SDL_SCANCODE_KP_6) ==
          M12_MENU_INPUT_RIGHT);

    /* Nexus desktop keys fall through to the shared M11 bindings, while the
     * connected Saturn ControlPad keeps its own D-pad turn semantics. */
    CHECK(M11_NexusMovementInputFromScancode(SDL_SCANCODE_A) ==
          M12_MENU_INPUT_NONE);
    CHECK(M11_NexusMovementInputFromScancode(SDL_SCANCODE_D) ==
          M12_MENU_INPUT_NONE);
    CHECK(M11_NexusMovementInputFromScancode(SDL_SCANCODE_LEFT) ==
          M12_MENU_INPUT_NONE);
    CHECK(M11_NexusMovementInputFromScancode(SDL_SCANCODE_RIGHT) ==
          M12_MENU_INPUT_NONE);
    CHECK(M11_NexusMovementInputFromScancode(SDL_SCANCODE_UP) ==
          M12_MENU_INPUT_NONE);
    CHECK(M11_NexusPointerInputFromScancode(SDL_SCANCODE_UP) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_NexusPointerInputFromScancode(SDL_SCANCODE_DOWN) ==
          M12_MENU_INPUT_DOWN);
    CHECK(M11_NexusPointerInputFromScancode(SDL_SCANCODE_LEFT) ==
          M12_MENU_INPUT_LEFT);
    CHECK(M11_NexusPointerInputFromScancode(SDL_SCANCODE_RIGHT) ==
          M12_MENU_INPUT_RIGHT);
    CHECK(M11_NexusPointerInputFromScancode(SDL_SCANCODE_A) ==
          M12_MENU_INPUT_LEFT);
    CHECK(M11_NexusPointerToggleInputFromScancode(SDL_SCANCODE_Z) ==
          M12_MENU_INPUT_NEXUS_POINTER_TOGGLE);
    CHECK(M11_Input_ActionForScancode(SDL_SCANCODE_A) ==
          M11_ACTION_STRAFE_LEFT);
    CHECK(M11_Input_ActionForScancode(SDL_SCANCODE_D) ==
          M11_ACTION_STRAFE_RIGHT);
    CHECK(M11_Input_ActionForScancode(SDL_SCANCODE_LEFT) ==
          M11_ACTION_STRAFE_LEFT);
    CHECK(M11_Input_ActionForScancode(SDL_SCANCODE_RIGHT) ==
          M11_ACTION_STRAFE_RIGHT);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_W) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_S) ==
          M12_MENU_INPUT_DOWN);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_A) ==
          M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_D) ==
          M12_MENU_INPUT_STRAFE_RIGHT);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_UP) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_DOWN) ==
          M12_MENU_INPUT_DOWN);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_LEFT) ==
          M12_MENU_INPUT_LEFT);
    CHECK(M11_TheronNavigationInputFromKeycode(SDLK_RIGHT) ==
          M12_MENU_INPUT_RIGHT);
    /* In the loaded dungeon, Theron's key layout matches DM1 while still
     * producing the Theron movement tokens. */
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_LEFT) ==
          M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_RIGHT) ==
          M12_MENU_INPUT_STRAFE_RIGHT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_HOME) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_Q) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_END) ==
          M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_E) ==
          M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_KP_1) ==
          M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_KP_3) ==
          M12_MENU_INPUT_STRAFE_RIGHT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_KP_4) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_KP_6) ==
          M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_LEFT) ==
          M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_RIGHT) ==
          M12_MENU_INPUT_STRAFE_RIGHT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_Q) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_E) ==
          M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_HOME) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_END) ==
          M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_KP_4) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_KP_6) ==
          M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronMouseButtonToInput(SDL_BUTTON_LEFT) ==
          M12_MENU_INPUT_ACCEPT);
    /* Button II cancels; it is not a world-tick action. A physical middle
     * button is not mouse button 1/2 and must remain inert. */
    CHECK(M11_TheronMouseButtonToInput(SDL_BUTTON_RIGHT) ==
          M12_MENU_INPUT_BACK);
    CHECK(M11_TheronMouseButtonToInput(SDL_BUTTON_MIDDLE) ==
          M12_MENU_INPUT_NONE);
    /* Pointer motion is deliberately not a source command.  The live SDL
     * route only updates the host pointer position; clicks are the sole
     * Button I/II ingress, so moving across HUD/object regions cannot hop
     * between objects as a selection cursor. */
    CHECK(M11_TheronMouseButtonToInput(0) == M12_MENU_INPUT_NONE);
    CHECK(M11_TheronTouchButtonInput(0) == M12_MENU_INPUT_ACCEPT);
    CHECK(M11_TheronTouchButtonInput(1) == M12_MENU_INPUT_BACK);

    if (failures) return 1;
    puts("PASS: m11_gamepad_csb_input_bridge");
    return 0;
}
