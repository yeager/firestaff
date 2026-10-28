#include "input_remap_m11.h"
#include "main_loop_m11.h"

#include <stdio.h>

static int expect(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    M11_Input_SetDefaults();

    /* Nexus uses its captured D-pad turn mapping for A/D and keeps the shared
     * DM1 bindings for the remaining movement keys. */
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_F, 1) == M12_MENU_INPUT_ACTION,
                "Nexus F should activate the retail title menu"))
        return 1;
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_RETURN, 1) == M12_MENU_INPUT_ACTION,
                "Nexus Enter should activate the retail title menu"))
        return 1;
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_SPACE, 1) == M12_MENU_INPUT_ACTION,
                "Nexus Space should activate the retail title menu"))
        return 1;
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_UP, 1) == M12_MENU_INPUT_UP,
                "Nexus Up should select the previous retail title choice"))
        return 1;
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_W, 1) == M12_MENU_INPUT_UP,
                "Nexus W should select the previous retail title choice"))
        return 1;
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_DOWN, 1) == M12_MENU_INPUT_DOWN,
                "Nexus Down should select the next retail title choice"))
        return 1;
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_S, 1) == M12_MENU_INPUT_DOWN,
                "Nexus S should select the next retail title choice"))
        return 1;
    if (!expect(M11_NexusStartupMenuInputFromScancode(
                    SDL_SCANCODE_RETURN, 0) == M12_MENU_INPUT_NONE,
                "Nexus title Accept keys should not become gameplay actions"))
        return 1;
    if (!expect(M11_NexusMovementInputFromScancode(SDL_SCANCODE_W) ==
                    M12_MENU_INPUT_NONE,
                "Nexus movement mapper should defer W to the shared map"))
        return 1;
    if (!expect(M11_NexusMovementInputFromScancode(SDL_SCANCODE_A) ==
                    M12_MENU_INPUT_TURN_LEFT,
                "Nexus A should turn left as in the retail ControlPad capture"))
        return 1;
    if (!expect(M11_NexusMovementInputFromScancode(SDL_SCANCODE_D) ==
                    M12_MENU_INPUT_TURN_RIGHT,
                "Nexus D should turn right as in the retail ControlPad capture"))
        return 1;
    if (!expect(M11_NexusLeaderInputFromScancode(SDL_SCANCODE_TAB, 1) ==
                    M12_MENU_INPUT_NEXUS_CYCLE_CHAMPION_PREVIOUS,
                "Nexus Shift+Tab should select the previous party leader"))
        return 1;
    if (!expect(M11_NexusLeaderInputFromScancode(SDL_SCANCODE_TAB, 0) ==
                    M12_MENU_INPUT_NONE,
                "unshifted Nexus Tab should retain the shared next-leader action"))
        return 1;
    if (!expect(M11_NexusMovementInputFromScancode(SDL_SCANCODE_LEFT) ==
                    M12_MENU_INPUT_NONE,
                "Nexus arrows should defer to the shared strafe bindings"))
        return 1;

    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_W) ==
                    M11_ACTION_MOVE_FORWARD,
                "shared DM1 W binding should move forward")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_S) ==
                    M11_ACTION_MOVE_BACKWARD,
                "shared DM1 S binding should move backward")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_A) ==
                    M11_ACTION_STRAFE_LEFT,
                "shared DM1 A binding should strafe left")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_D) ==
                    M11_ACTION_STRAFE_RIGHT,
                "shared DM1 D binding should strafe right")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_LEFT) ==
                    M11_ACTION_STRAFE_LEFT,
                "shared DM1 left-arrow binding should strafe left")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_RIGHT) ==
                    M11_ACTION_STRAFE_RIGHT,
                "shared DM1 right-arrow binding should strafe right")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_Q) ==
                    M11_ACTION_TURN_LEFT,
                "shared DM1 Q binding should turn left")) return 1;
    if (!expect(M11_Input_ActionForScancode(SDL_SCANCODE_E) ==
                    M11_ACTION_TURN_RIGHT,
                "shared DM1 E binding should turn right")) return 1;

    if (!expect(M11_NexusPointerToggleInputFromScancode(SDL_SCANCODE_Z) ==
                    M12_MENU_INPUT_NEXUS_POINTER_TOGGLE,
                "Z should toggle the Nexus hand-pointer mode")) return 1;
    if (!expect(M11_NexusPointerToggleInputFromScancode(SDL_SCANCODE_W) ==
                    M12_MENU_INPUT_NONE,
                "movement keys should not toggle the hand pointer")) return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_UP) ==
                    M12_MENU_INPUT_UP,
                "pointer mode Up should move the hand pointer up")) return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_DOWN) ==
                    M12_MENU_INPUT_DOWN,
                "pointer mode Down should move the hand pointer down")) return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_LEFT) ==
                    M12_MENU_INPUT_LEFT,
                "pointer mode Left should move the hand pointer left")) return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_RIGHT) ==
                    M12_MENU_INPUT_RIGHT,
                "pointer mode Right should move the hand pointer right")) return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_W) ==
                    M12_MENU_INPUT_UP,
                "pointer mode W should move the hand pointer up"))
        return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_A) ==
                    M12_MENU_INPUT_LEFT,
                "pointer mode A should move the hand pointer left"))
        return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_S) ==
                    M12_MENU_INPUT_DOWN,
                "pointer mode S should move the hand pointer down"))
        return 1;
    if (!expect(M11_NexusPointerInputFromScancode(SDL_SCANCODE_D) ==
                    M12_MENU_INPUT_RIGHT,
                "pointer mode D should move the hand pointer right"))
        return 1;
    if (!expect(M11_NexusPointerInputFromDirections(1, 0, 1, 0) ==
                    M12_MENU_INPUT_NEXUS_POINTER_UP_LEFT,
                "held W+A should move the hand pointer diagonally up-left"))
        return 1;
    if (!expect(M11_NexusPointerInputFromDirections(1, 1, 1, 0) ==
                    M12_MENU_INPUT_LEFT,
                "opposite vertical inputs should cancel for pointer movement"))
        return 1;

    /* These bounds are the verified retail DM.BIN movement-pad rectangle,
     * (27,142)-(129,207), parsed by nexus_v1_hud_hit_rects_parse_dm_bin. */
    if (!expect(M11_NexusMovementPadInputAt(35, 150, 27, 142, 129, 207) ==
                    M12_MENU_INPUT_TURN_LEFT,
                "movement-pad upper-left cell should turn left")) return 1;
    if (!expect(M11_NexusMovementPadInputAt(78, 150, 27, 142, 129, 207) ==
                    M12_MENU_INPUT_UP,
                "movement-pad upper-middle cell should move forward")) return 1;
    if (!expect(M11_NexusMovementPadInputAt(120, 150, 27, 142, 129, 207) ==
                    M12_MENU_INPUT_TURN_RIGHT,
                "movement-pad upper-right cell should turn right")) return 1;
    if (!expect(M11_NexusMovementPadInputAt(35, 190, 27, 142, 129, 207) ==
                    M12_MENU_INPUT_STRAFE_LEFT,
                "movement-pad lower-left cell should strafe left")) return 1;
    if (!expect(M11_NexusMovementPadInputAt(78, 190, 27, 142, 129, 207) ==
                    M12_MENU_INPUT_DOWN,
                "movement-pad lower-middle cell should move backward")) return 1;
    if (!expect(M11_NexusMovementPadInputAt(120, 190, 27, 142, 129, 207) ==
                    M12_MENU_INPUT_STRAFE_RIGHT,
                "movement-pad lower-right cell should strafe right")) return 1;
    if (!expect(M11_NexusMovementPadInputAt(129, 190, 27, 142, 129, 207) ==
                    M12_MENU_INPUT_NONE,
                "movement-pad right edge should be outside the hit rectangle"))
        return 1;
    if (!expect(M11_NexusMovementPadInputAt(50, 50, 20, 20, 20, 40) ==
                    M12_MENU_INPUT_NONE,
                "degenerate movement-pad geometry should remain inert")) return 1;

    puts("ok: Nexus keyboard, pointer and six-cell mouse movement controls match their contract");
    return 0;
}
