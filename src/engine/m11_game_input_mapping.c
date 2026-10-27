#include "main_loop_m11.h"

#include <string.h>

enum {
    M11_SOURCE_FB_WIDTH = 320,
    M11_SOURCE_FB_HEIGHT = 200
};

M12_MenuInput M11_GamepadActionToMenuInput(M12_InputAction action,
                                           int gameplayActive) {
    switch (action) {
        case M12_ACTION_MOVE_FORWARD: return M12_MENU_INPUT_UP;
        case M12_ACTION_MOVE_BACKWARD: return M12_MENU_INPUT_DOWN;
        case M12_ACTION_TURN_LEFT:
            return gameplayActive ? M12_MENU_INPUT_TURN_LEFT : M12_MENU_INPUT_LEFT;
        case M12_ACTION_TURN_RIGHT:
            return gameplayActive ? M12_MENU_INPUT_TURN_RIGHT : M12_MENU_INPUT_RIGHT;
        case M12_ACTION_STRAFE_LEFT:
            return gameplayActive ? M12_MENU_INPUT_STRAFE_LEFT : M12_MENU_INPUT_LEFT;
        case M12_ACTION_STRAFE_RIGHT:
            return gameplayActive ? M12_MENU_INPUT_STRAFE_RIGHT : M12_MENU_INPUT_RIGHT;
        case M12_ACTION_ACCEPT: return M12_MENU_INPUT_ACCEPT;
        case M12_ACTION_BACK: return M12_MENU_INPUT_BACK;
        case M12_ACTION_ACTION: return M12_MENU_INPUT_ACTION;
        case M12_ACTION_CYCLE_CHAMPION: return M12_MENU_INPUT_CYCLE_CHAMPION;
        case M12_ACTION_REST_TOGGLE: return M12_MENU_INPUT_REST_TOGGLE;
        case M12_ACTION_USE_STAIRS: return M12_MENU_INPUT_USE_STAIRS;
        case M12_ACTION_PICKUP_ITEM: return M12_MENU_INPUT_PICKUP_ITEM;
        case M12_ACTION_DROP_ITEM: return M12_MENU_INPUT_DROP_ITEM;
        case M12_ACTION_SPELL_RUNE_1: return M12_MENU_INPUT_SPELL_RUNE_1;
        case M12_ACTION_SPELL_RUNE_2: return M12_MENU_INPUT_SPELL_RUNE_2;
        case M12_ACTION_SPELL_RUNE_3: return M12_MENU_INPUT_SPELL_RUNE_3;
        case M12_ACTION_SPELL_RUNE_4: return M12_MENU_INPUT_SPELL_RUNE_4;
        case M12_ACTION_SPELL_RUNE_5: return M12_MENU_INPUT_SPELL_RUNE_5;
        case M12_ACTION_SPELL_RUNE_6: return M12_MENU_INPUT_SPELL_RUNE_6;
        case M12_ACTION_SPELL_CAST: return M12_MENU_INPUT_SPELL_CAST;
        case M12_ACTION_SPELL_CLEAR: return M12_MENU_INPUT_SPELL_CLEAR;
        /* Preserve this dedicated command: Theron's native runtime routes
         * USE_ITEM to its front-object handler, while ACTION is wait/tick. */
        case M12_ACTION_USE_ITEM: return M12_MENU_INPUT_USE_ITEM;
        case M12_ACTION_MAP_TOGGLE: return M12_MENU_INPUT_MAP_TOGGLE;
        case M12_ACTION_INVENTORY_TOGGLE: return M12_MENU_INPUT_INVENTORY_TOGGLE;
        case M12_ACTION_QUICK_SAVE: return M12_MENU_INPUT_SAVE_GAME;
        default: return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_GamepadAxisToMenuInput(SDL_GamepadAxis axis,
                                         M12_AxisRole role,
                                         int processedValue,
                                         int gameplayActive) {
    if (processedValue > -16000 && processedValue < 16000) {
        return M12_MENU_INPUT_NONE;
    }
    if (role == M12_AXIS_ROLE_MOVE) {
        if (axis == SDL_GAMEPAD_AXIS_LEFTY || axis == SDL_GAMEPAD_AXIS_RIGHTY) {
            return processedValue < 0 ? M12_MENU_INPUT_UP : M12_MENU_INPUT_DOWN;
        }
        if (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX) {
            if (gameplayActive) {
                return processedValue < 0 ? M12_MENU_INPUT_STRAFE_LEFT
                                          : M12_MENU_INPUT_STRAFE_RIGHT;
            }
            return processedValue < 0 ? M12_MENU_INPUT_LEFT : M12_MENU_INPUT_RIGHT;
        }
    } else if (role == M12_AXIS_ROLE_TURN &&
               (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX)) {
        if (gameplayActive) {
            return processedValue < 0 ? M12_MENU_INPUT_TURN_LEFT
                                      : M12_MENU_INPUT_TURN_RIGHT;
        }
        return processedValue < 0 ? M12_MENU_INPUT_LEFT : M12_MENU_INPUT_RIGHT;
    }
    return M12_MENU_INPUT_NONE;
}

M12_MenuInput M11_TheronNavigationInputFromScancode(int scancode)
{
    /* Keep startup navigation on the standard menu tokens. Loaded-dungeon
     * keys use M11_TheronGameplayInputFromScancode() so arrows follow DM1's
     * familiar strafe layout without changing menu behavior. */
    switch ((SDL_Scancode)scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_KP_8:
            return M12_MENU_INPUT_UP;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_KP_2:
            return M12_MENU_INPUT_DOWN;
        case SDL_SCANCODE_A:
            return M12_MENU_INPUT_STRAFE_LEFT;
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_KP_4:
            return M12_MENU_INPUT_LEFT;
        case SDL_SCANCODE_D:
            return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_KP_6:
            return M12_MENU_INPUT_RIGHT;
        /* Theron's Quest USA retail manual pp. 4--5 assigns Button I to
         * selection and Button II to cancel.  Keep Z/X layout-stable across
         * keyboard layouts, as in the documented desktop controller map. */
        case SDL_SCANCODE_Z:
            return M12_MENU_INPUT_ACTION;
        case SDL_SCANCODE_X:
            return M12_MENU_INPUT_BACK;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_NexusStartupMenuInputFromScancode(int scancode,
                                                    int title_active)
{
    if (!title_active) {
        return M12_MENU_INPUT_NONE;
    }
    switch ((SDL_Scancode)scancode) {
        case SDL_SCANCODE_F:
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_KP_ENTER:
        case SDL_SCANCODE_SPACE:
            return M12_MENU_INPUT_ACTION;
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
            return M12_MENU_INPUT_UP;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
            return M12_MENU_INPUT_DOWN;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_NexusMovementInputFromScancode(int scancode)
{
    /* The authenticated Ymir ControlPad profile maps host A/D to Saturn
     * D-pad left/right, which turn in Nexus movement mode. Keep that
     * game-specific mapping here; DM1 continues to use the shared A/D
     * strafe bindings. Arrow keys still reach the shared map for strafing. */
    switch ((SDL_Scancode)scancode) {
        case SDL_SCANCODE_A: return M12_MENU_INPUT_TURN_LEFT;
        case SDL_SCANCODE_D: return M12_MENU_INPUT_TURN_RIGHT;
        default: return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_NexusLeaderInputFromScancode(int scancode, int shift_down)
{
    if (scancode == SDL_SCANCODE_TAB && shift_down) {
        return M12_MENU_INPUT_NEXUS_CYCLE_CHAMPION_PREVIOUS;
    }
    return M12_MENU_INPUT_NONE;
}

M12_MenuInput M11_NexusPointerInputFromScancode(int scancode)
{
    /* Saturn's D-pad moves the hand pointer in eight directions. Retail Ymir
     * --debug input captures used W/S/A/D for the D-pad, so preserve that
     * familiar desktop layout while pointer mode is active. Arrow keys remain
     * aliases for players who prefer cursor-style navigation. Diagonal
     * simultaneous-key handling is resolved at the held-input sampler. */
    switch ((SDL_Scancode)scancode) {
        case SDL_SCANCODE_UP: return M12_MENU_INPUT_UP;
        case SDL_SCANCODE_W: return M12_MENU_INPUT_UP;
        case SDL_SCANCODE_DOWN: return M12_MENU_INPUT_DOWN;
        case SDL_SCANCODE_S: return M12_MENU_INPUT_DOWN;
        case SDL_SCANCODE_LEFT: return M12_MENU_INPUT_LEFT;
        case SDL_SCANCODE_A: return M12_MENU_INPUT_LEFT;
        case SDL_SCANCODE_RIGHT: return M12_MENU_INPUT_RIGHT;
        case SDL_SCANCODE_D: return M12_MENU_INPUT_RIGHT;
        default: return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_NexusPointerInputFromDirections(int up, int down,
                                                   int left, int right)
{
    const int dy = (up != down) ? (up ? -1 : 1) : 0;
    const int dx = (left != right) ? (left ? -1 : 1) : 0;

    if (dy < 0) {
        return dx < 0 ? M12_MENU_INPUT_NEXUS_POINTER_UP_LEFT
             : dx > 0 ? M12_MENU_INPUT_NEXUS_POINTER_UP_RIGHT
                      : M12_MENU_INPUT_UP;
    }
    if (dy > 0) {
        return dx < 0 ? M12_MENU_INPUT_NEXUS_POINTER_DOWN_LEFT
             : dx > 0 ? M12_MENU_INPUT_NEXUS_POINTER_DOWN_RIGHT
                      : M12_MENU_INPUT_DOWN;
    }
    if (dx < 0) return M12_MENU_INPUT_LEFT;
    if (dx > 0) return M12_MENU_INPUT_RIGHT;
    return M12_MENU_INPUT_NONE;
}

M12_MenuInput M11_NexusPointerToggleInputFromScancode(int scancode)
{
    return (SDL_Scancode)scancode == SDL_SCANCODE_Z
        ? M12_MENU_INPUT_NEXUS_POINTER_TOGGLE : M12_MENU_INPUT_NONE;
}

M12_MenuInput M11_NexusMovementPadInputAt(
    int x, int y, int x1, int y1, int x2, int y2)
{
    int column;
    int row;
    int width;
    int height;

    if (x2 <= x1 || y2 <= y1 || x < x1 || x >= x2 || y < y1 || y >= y2) {
        return M12_MENU_INPUT_NONE;
    }

    width = x2 - x1;
    height = y2 - y1;
    column = ((x - x1) * 3) / width;
    row = ((y - y1) * 2) / height;
    if (row == 0) {
        return column == 0 ? M12_MENU_INPUT_TURN_LEFT
             : column == 1 ? M12_MENU_INPUT_UP
                           : M12_MENU_INPUT_TURN_RIGHT;
    }
    return column == 0 ? M12_MENU_INPUT_STRAFE_LEFT
         : column == 1 ? M12_MENU_INPUT_DOWN
                       : M12_MENU_INPUT_STRAFE_RIGHT;
}

M12_MenuInput M11_TheronNavigationInputFromKeycode(int keycode)
{
    /* SDL normally supplies both fields.  Keep a keycode fallback for
     * macOS/SDL event sources that leave scancode unset, and for the
     * keycode-driven boot-probe path.  This is a host ingress fallback only;
     * the normalized M12 token still enters the same Theron runtime facade.
     */
    switch ((SDL_Keycode)keycode) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        case SDLK_W: return M12_MENU_INPUT_UP;
        case SDLK_S: return M12_MENU_INPUT_DOWN;
        case SDLK_A: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_D: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_Z: return M12_MENU_INPUT_ACTION;
        case SDLK_X: return M12_MENU_INPUT_BACK;
#else
        case SDLK_w: return M12_MENU_INPUT_UP;
        case SDLK_s: return M12_MENU_INPUT_DOWN;
        case SDLK_a: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_d: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_z: return M12_MENU_INPUT_ACTION;
        case SDLK_x: return M12_MENU_INPUT_BACK;
#endif
        case SDLK_UP: return M12_MENU_INPUT_UP;
        case SDLK_DOWN: return M12_MENU_INPUT_DOWN;
        case SDLK_LEFT: return M12_MENU_INPUT_LEFT;
        case SDLK_RIGHT: return M12_MENU_INPUT_RIGHT;
        default: return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_TheronGameplayInputFromScancode(int scancode)
{
    /* Match DM1's familiar dungeon keyboard layout while preserving Theron's
     * own native command tokens. Startup arrows remain menu navigation and
     * continue to use M11_TheronNavigationInputFromScancode(). */
    switch ((SDL_Scancode)scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_KP_5:
        case SDL_SCANCODE_KP_8:
            return M12_MENU_INPUT_UP;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_KP_2:
            return M12_MENU_INPUT_DOWN;
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_KP_1:
            return M12_MENU_INPUT_STRAFE_LEFT;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_KP_3:
            return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDL_SCANCODE_HOME:
        case SDL_SCANCODE_Q:
        case SDL_SCANCODE_KP_4:
            return M12_MENU_INPUT_TURN_LEFT;
        case SDL_SCANCODE_END:
        case SDL_SCANCODE_E:
        case SDL_SCANCODE_KP_6:
            return M12_MENU_INPUT_TURN_RIGHT;
        case SDL_SCANCODE_TAB:
            return M12_MENU_INPUT_CYCLE_CHAMPION;
        case SDL_SCANCODE_G:
            return M12_MENU_INPUT_PICKUP_ITEM;
        case SDL_SCANCODE_P:
            return M12_MENU_INPUT_DROP_ITEM;
        case SDL_SCANCODE_I:
            return M12_MENU_INPUT_INVENTORY_TOGGLE;
        case SDL_SCANCODE_U:
            return M12_MENU_INPUT_USE_ITEM;
        /* The native desktop binding for PC Engine Button I.  Keep the
         * conventional Return/Space aliases below for the DM1-like layout. */
        case SDL_SCANCODE_Z:
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_SPACE:
            return M12_MENU_INPUT_ACTION;
        case SDL_SCANCODE_X:
            return M12_MENU_INPUT_BACK;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_TheronGameplayInputFromScancodeWithButtonII(
    int scancode, int buttonIIHeld)
{
    if (buttonIIHeld) {
        /* The USA retail manual, printed p. 5, says to hold Button II while
         * pressing a direction to turn instead of side-step. On desktop this
         * is the right mouse button plus the matching direction key. */
        switch ((SDL_Scancode)scancode) {
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_A:
                return M12_MENU_INPUT_TURN_LEFT;
            case SDL_SCANCODE_RIGHT:
            case SDL_SCANCODE_D:
                return M12_MENU_INPUT_TURN_RIGHT;
            default:
                break;
        }
    }
    return M11_TheronGameplayInputFromScancode(scancode);
}

M12_MenuInput M11_TheronHeldMotionInputFromScancode(int scancode,
                                                    int buttonIIHeld)
{
    M12_MenuInput input =
        M11_TheronGameplayInputFromScancodeWithButtonII(scancode,
                                                        buttonIIHeld);
    switch (input) {
        case M12_MENU_INPUT_UP:
        case M12_MENU_INPUT_DOWN:
        case M12_MENU_INPUT_TURN_LEFT:
        case M12_MENU_INPUT_TURN_RIGHT:
        case M12_MENU_INPUT_STRAFE_LEFT:
        case M12_MENU_INPUT_STRAFE_RIGHT:
            return input;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_TheronHeldMotionInputFromKeyboardState(
    const uint8_t* keys, int keyCount, int buttonIIHeld)
{
    static const SDL_Scancode preferred[] = {
        SDL_SCANCODE_UP, SDL_SCANCODE_W,
        SDL_SCANCODE_DOWN, SDL_SCANCODE_S,
        SDL_SCANCODE_LEFT, SDL_SCANCODE_A,
        SDL_SCANCODE_RIGHT, SDL_SCANCODE_D,
        SDL_SCANCODE_HOME, SDL_SCANCODE_Q,
        SDL_SCANCODE_END, SDL_SCANCODE_E,
        SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_2,
        SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_3,
        SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_6
    };
    int i;

    if (!keys || keyCount <= 0) return M12_MENU_INPUT_NONE;
    for (i = 0; i < (int)(sizeof(preferred) / sizeof(preferred[0])); ++i) {
        const int scancode = (int)preferred[i];
        if (scancode >= 0 && scancode < keyCount && keys[scancode]) {
            const M12_MenuInput input =
                M11_TheronHeldMotionInputFromScancode(scancode,
                                                      buttonIIHeld);
            if (input != M12_MENU_INPUT_NONE) return input;
        }
    }
    return M12_MENU_INPUT_NONE;
}

M12_MenuInput M11_TheronGameplayInputFromKeycode(int keycode)
{
    /* SDL keycode fallback for events that do not carry a useful scancode. */
    switch ((SDL_Keycode)keycode) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        case SDLK_W: return M12_MENU_INPUT_UP;
        case SDLK_S: return M12_MENU_INPUT_DOWN;
        case SDLK_A: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_D: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_Q: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_E: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_TAB: return M12_MENU_INPUT_CYCLE_CHAMPION;
        case SDLK_G: return M12_MENU_INPUT_PICKUP_ITEM;
        case SDLK_P: return M12_MENU_INPUT_DROP_ITEM;
        case SDLK_I: return M12_MENU_INPUT_INVENTORY_TOGGLE;
        case SDLK_U: return M12_MENU_INPUT_USE_ITEM;
        case SDLK_Z: return M12_MENU_INPUT_ACTION;
        case SDLK_X: return M12_MENU_INPUT_BACK;
#else
        case SDLK_w: return M12_MENU_INPUT_UP;
        case SDLK_s: return M12_MENU_INPUT_DOWN;
        case SDLK_a: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_d: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_q: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_e: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_TAB: return M12_MENU_INPUT_CYCLE_CHAMPION;
        case SDLK_g: return M12_MENU_INPUT_PICKUP_ITEM;
        case SDLK_p: return M12_MENU_INPUT_DROP_ITEM;
        case SDLK_i: return M12_MENU_INPUT_INVENTORY_TOGGLE;
        case SDLK_u: return M12_MENU_INPUT_USE_ITEM;
        case SDLK_z: return M12_MENU_INPUT_ACTION;
        case SDLK_x: return M12_MENU_INPUT_BACK;
#endif
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE: return M12_MENU_INPUT_ACTION;
        case SDLK_UP:
        case SDLK_KP_5:
        case SDLK_KP_8: return M12_MENU_INPUT_UP;
        case SDLK_DOWN:
        case SDLK_KP_2: return M12_MENU_INPUT_DOWN;
        case SDLK_LEFT:
        case SDLK_KP_1: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_RIGHT:
        case SDLK_KP_3: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_HOME:
        case SDLK_KP_4: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_END:
        case SDLK_KP_6: return M12_MENU_INPUT_TURN_RIGHT;
        default: return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_TheronGameplayInputFromKeycodeWithButtonII(
    int keycode, int buttonIIHeld)
{
    if (buttonIIHeld) {
        switch ((SDL_Keycode)keycode) {
            case SDLK_LEFT:
#if SDL_VERSION_ATLEAST(3, 0, 0)
            case SDLK_A:
#else
            case SDLK_a:
#endif
                return M12_MENU_INPUT_TURN_LEFT;
            case SDLK_RIGHT:
#if SDL_VERSION_ATLEAST(3, 0, 0)
            case SDLK_D:
#else
            case SDLK_d:
#endif
                return M12_MENU_INPUT_TURN_RIGHT;
            default:
                break;
        }
    }
    return M11_TheronGameplayInputFromKeycode(keycode);
}

M12_MenuInput M11_TheronMouseButtonToInput(int button)
{
    if (button == SDL_BUTTON_LEFT) return M12_MENU_INPUT_ACCEPT;
    /* Theron's Quest Instruction Manual, printed p. 5, assigns Button II to
     * cancel and to the held simplified-movement modifier; it is not a
     * world-tick command. See docs/source-lock/
     * theron-button-ii-manual-source-lock.md. Keep Button II on an ordinary
     * right mouse button; SDL_BUTTON_MIDDLE remains inert. */
    if (button == SDL_BUTTON_RIGHT) {
        return M12_MENU_INPUT_BACK;
    }
    return M12_MENU_INPUT_NONE;
}

M12_MenuInput M11_TheronTouchButtonInput(int longPress)
{
    return longPress ? M12_MENU_INPUT_BACK : M12_MENU_INPUT_ACCEPT;
}

int M11_GamepadEnabledForInputMode(int inputModeIndex, int configuredEnabled)
{
    if (!configuredEnabled) return 0;
    /* M12's persisted order is Auto, Keyboard+Mouse, Touch, Gamepad.
     * Keyboard/mouse and touch are exclusive device choices; Auto and
     * Gamepad retain the user-owned controller map. Unknown values resolve
     * to Auto so a corrupt preference cannot disable a working controller. */
    return inputModeIndex != 1 && inputModeIndex != 2;
}

int M11_InputSourceSupportsHeldMotion(const char* sourceId, int active)
{
    if (!active || !sourceId) return 0;
    /* ReDMCSB GAMELOOP.C:164-219 owns the same wait-for-input / F0380
     * command boundary for Dungeon Master and Chaos Strikes Back.  CSB was
     * accidentally left out of M11's held-device sampler, which made a held
     * stick or key depend on SDL axis motion or OS key-repeat after its first
     * command. */
    return strcmp(sourceId, "dm1") == 0 || strcmp(sourceId, "csb") == 0 ||
           strcmp(sourceId, "dm2") == 0 || strcmp(sourceId, "theron") == 0;
}

int M11_MapPresentedGamePointToSourceForPresentation(int presentationMode,
                                                     int presentationWidth,
                                                     int presentationHeight,
                                                     int* x,
                                                     int* y) {
    int targetW = M11_SOURCE_FB_WIDTH;
    int targetH = M11_SOURCE_FB_HEIGHT;
    if (!x || !y) {
        return 0;
    }
    if (presentationMode == M12_PRESENTATION_V20_FILTERED) {
        targetW = M11_SOURCE_FB_WIDTH * 2;
        targetH = M11_SOURCE_FB_HEIGHT * 2;
    } else if ((presentationMode == M12_PRESENTATION_V1_ORIGINAL ||
                presentationMode == M12_PRESENTATION_V21_UPSCALED ||
                presentationMode == M12_PRESENTATION_V22_MODERN) &&
               presentationWidth > 0 &&
               presentationHeight > 0) {
        targetW = presentationWidth;
        targetH = presentationHeight;
    } else {
        return 0;
    }
    /* A source-sized Original page is already in COMMAND.C coordinates.
     * Preserve its established pass-through contract so callers can avoid a
     * needless conversion.  A selected host-sized Original target, however,
     * must take the same inverse map as its presenter or HoC/HUD clicks are
     * delivered as host pixels instead of source pixels. */
    if (presentationMode == M12_PRESENTATION_V1_ORIGINAL &&
        targetW == M11_SOURCE_FB_WIDTH && targetH == M11_SOURCE_FB_HEIGHT) {
        return 0;
    }
    if (targetW > 0 && targetH > 0) {
        *x = (*x * M11_SOURCE_FB_WIDTH) / targetW;
        *y = (*y * M11_SOURCE_FB_HEIGHT) / targetH;
    }
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= M11_SOURCE_FB_WIDTH) *x = M11_SOURCE_FB_WIDTH - 1;
    if (*y >= M11_SOURCE_FB_HEIGHT) *y = M11_SOURCE_FB_HEIGHT - 1;
    return 1;
}

/* Inverse of M11_MapPresentedGamePointToSourceForPresentation().
 *
 * Touch overlay hit-tests, HUD button bounds, and mouse cursor
 * positions on the *presented* surface all need the source-to-screen
 * direction: given a framebuffer coordinate (0..319, 0..199) used by
 * the source-locked ReDMCSB COMMAND.C / COORD.C dispatch, where on
 * the active presented surface (640x400 for V20_FILTERED, or the
 * user-selected 320x200..3840x2160 for Original host targets,
 * V21_UPSCALED, or V22_MODERN)
 * does that hit-test appear? This helper is the canonical answer.
 *
 * A source-sized V1_ORIGINAL mode is a pass-through (320x200 == source),
 * the same way its sibling returns 0 / no-op in the inverse direction.
 * A selected V1 host target, V20/V21/V22 with non-zero presentation extents
 * succeed and divide
 * the source coordinate by M11_SOURCE_FB_WIDTH / M11_SOURCE_FB_HEIGHT
 * using the same target-extent resolution rules as
 * M11_MapPresentedGamePointToSourceForPresentation().
 *
 * Source lock (mirrors the existing inverse helper):
 *   - ReDMCSB COMMAND.C:1379-1449 F0358 mouse-row scan
 *   - ReDMCSB COMMAND.C:1641-1660 F0359 primary click dispatch
 *   - ReDMCSB COORD.C:1903-1920 inclusive source zone expansion
 *   - src/engine/main_loop_m11.c m11_game_presentation_target (line 145)
 *     (target extent resolution; same V1/V21/V22=selected and
 *      V20=640x400 routing used here)
 *
 * Returns 1 on a successful mapping (selected V1 host target or V20 / V21 /
 * V22 with non-zero extents), 0 on source-sized V1_ORIGINAL passthrough,
 * V21/V22 with zero or
 * negative extents, unknown modes, or NULL output pointers.
 */
int M11_MapSourcePointToPresentedForPresentation(int presentationMode,
                                                 int presentationWidth,
                                                 int presentationHeight,
                                                 int* x,
                                                 int* y) {
    int targetW = M11_SOURCE_FB_WIDTH;
    int targetH = M11_SOURCE_FB_HEIGHT;
    if (!x || !y) {
        return 0;
    }
    if (presentationMode == M12_PRESENTATION_V20_FILTERED) {
        targetW = M11_SOURCE_FB_WIDTH * 2;
        targetH = M11_SOURCE_FB_HEIGHT * 2;
    } else if ((presentationMode == M12_PRESENTATION_V1_ORIGINAL ||
                presentationMode == M12_PRESENTATION_V21_UPSCALED ||
                presentationMode == M12_PRESENTATION_V22_MODERN) &&
               presentationWidth > 0 &&
               presentationHeight > 0) {
        targetW = presentationWidth;
        targetH = presentationHeight;
    } else {
        return 0;
    }
    if (presentationMode == M12_PRESENTATION_V1_ORIGINAL &&
        targetW == M11_SOURCE_FB_WIDTH && targetH == M11_SOURCE_FB_HEIGHT) {
        return 0;
    }
    if (targetW > 0 && targetH > 0) {
        /* Clamp the source coordinate to the source framebuffer
         * range BEFORE the multiply so a caller-supplied value
         * outside [0..319] x [0..199] cannot overflow the
         * `src * targetW` step on 4K presentations
         * (e.g. 1000000 * 3840 = 3.84 * 10^9, beyond INT_MAX). The
         * inverse helper only multiplies by 320, which is safe for
         * INT_MAX inputs, but this direction can blow up. */
        if (*x < 0) *x = 0;
        if (*y < 0) *y = 0;
        if (*x >= M11_SOURCE_FB_WIDTH) *x = M11_SOURCE_FB_WIDTH - 1;
        if (*y >= M11_SOURCE_FB_HEIGHT) *y = M11_SOURCE_FB_HEIGHT - 1;
        /* Map to the centre of the target pixel-cell, not its leading edge.
         * A host presentation target can undergo one further FIT mapping
         * before SDL delivers a pointer.  Leading-edge coordinates may then
         * round into the preceding source cell (for example C187's x=43
         * became x=42 through a 1920x1200 Original target on a 1512x982
         * window).  The cell centre composes with the inverse mapper and
         * preserves ReDMCSB COMMAND.C hit ownership. */
        {
            int64_t xLo = ((int64_t)*x * targetW +
                           M11_SOURCE_FB_WIDTH - 1) / M11_SOURCE_FB_WIDTH;
            int64_t xHi = (((int64_t)(*x + 1) * targetW) - 1) /
                          M11_SOURCE_FB_WIDTH;
            int64_t yLo = ((int64_t)*y * targetH +
                           M11_SOURCE_FB_HEIGHT - 1) / M11_SOURCE_FB_HEIGHT;
            int64_t yHi = (((int64_t)(*y + 1) * targetH) - 1) /
                          M11_SOURCE_FB_HEIGHT;
            *x = (int)((xLo + xHi) / 2);
            *y = (int)((yLo + yHi) / 2);
        }
    }
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= targetW) *x = targetW - 1;
    if (*y >= targetH) *y = targetH - 1;
    return 1;
}
