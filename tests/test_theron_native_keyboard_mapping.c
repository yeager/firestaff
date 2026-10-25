#include "main_loop_m11.h"

#include <stdio.h>

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_UP) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_LEFT) ==
          M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_Q) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_TAB) ==
          M12_MENU_INPUT_CYCLE_CHAMPION);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_G) ==
          M12_MENU_INPUT_PICKUP_ITEM);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_I) ==
          M12_MENU_INPUT_INVENTORY_TOGGLE);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_P) ==
          M12_MENU_INPUT_DROP_ITEM);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_U) ==
          M12_MENU_INPUT_USE_ITEM);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_SPACE) ==
          M12_MENU_INPUT_ACTION);
    CHECK(M11_TheronGameplayInputFromScancode(SDL_SCANCODE_RETURN) ==
          M12_MENU_INPUT_ACTION);

    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_UP) ==
          M12_MENU_INPUT_UP);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_LEFT) ==
          M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_Q) ==
          M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_TAB) ==
          M12_MENU_INPUT_CYCLE_CHAMPION);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_G) ==
          M12_MENU_INPUT_PICKUP_ITEM);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_I) ==
          M12_MENU_INPUT_INVENTORY_TOGGLE);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_P) ==
          M12_MENU_INPUT_DROP_ITEM);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_U) ==
          M12_MENU_INPUT_USE_ITEM);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_SPACE) ==
          M12_MENU_INPUT_ACTION);
    CHECK(M11_TheronGameplayInputFromKeycode(SDLK_RETURN) ==
          M12_MENU_INPUT_ACTION);

    CHECK(M11_TheronGameplayInputFromScancodeWithButtonII(
              SDL_SCANCODE_LEFT, 0) == M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronGameplayInputFromScancodeWithButtonII(
              SDL_SCANCODE_LEFT, 1) == M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromScancodeWithButtonII(
              SDL_SCANCODE_A, 1) == M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromScancodeWithButtonII(
              SDL_SCANCODE_RIGHT, 1) == M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronGameplayInputFromScancodeWithButtonII(
              SDL_SCANCODE_D, 1) == M12_MENU_INPUT_TURN_RIGHT);
    CHECK(M11_TheronGameplayInputFromScancodeWithButtonII(
              SDL_SCANCODE_UP, 1) == M12_MENU_INPUT_UP);

    CHECK(M11_TheronGameplayInputFromKeycodeWithButtonII(
              SDLK_LEFT, 0) == M12_MENU_INPUT_STRAFE_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycodeWithButtonII(
              SDLK_LEFT, 1) == M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycodeWithButtonII(
              SDLK_RIGHT, 1) == M12_MENU_INPUT_TURN_RIGHT);
#if SDL_VERSION_ATLEAST(3, 0, 0)
    CHECK(M11_TheronGameplayInputFromKeycodeWithButtonII(
              SDLK_A, 1) == M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycodeWithButtonII(
              SDLK_D, 1) == M12_MENU_INPUT_TURN_RIGHT);
#else
    CHECK(M11_TheronGameplayInputFromKeycodeWithButtonII(
              SDLK_a, 1) == M12_MENU_INPUT_TURN_LEFT);
    CHECK(M11_TheronGameplayInputFromKeycodeWithButtonII(
              SDLK_d, 1) == M12_MENU_INPUT_TURN_RIGHT);
#endif

    if (failures) return 1;
    puts("PASS: native Theron gameplay keyboard mapping");
    return 0;
}
