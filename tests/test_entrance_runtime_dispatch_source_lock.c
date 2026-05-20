#include "main_loop_m11.h"
#include "entrance_mouse_routes_pc34_compat.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

/* IMG3 globals are required by firestaff_m10 when this focused gate links
 * the full runtime libraries through main_loop_m11.c. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int expect_command(const char* label,
                          int x,
                          int y,
                          unsigned int mask,
                          int want) {
    int got = M11_Entrance_DispatchSourceLockedPointerCommand(x, y, mask);
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        return 0;
    }
    printf("%s=%d\n", label, got);
    return 1;
}

static int expect_key(const char* label, int key, int want) {
    int got = M11_Entrance_DispatchSourceLockedKeyCommand(key);
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        return 0;
    }
    printf("%s=%d\n", label, got);
    return 1;
}

int main(void) {
    const char* evidence = ENTRANCE_Compat_GetMouseRouteEvidence();
    int ok = 1;

    printf("probe=firestaff_entrance_runtime_dispatch_source_lock\n");
    printf("entranceMouseRouteEvidence=%s\n", evidence);

    ok &= strstr(evidence, "ENTRANCE.C:739-747") != NULL;
    ok &= strstr(evidence, "ENTRANCE.C:850-883") != NULL;
    ok &= strstr(evidence, "COMMAND.C:340-353") != NULL;
    ok &= strstr(evidence, "COMMAND.C:1379-1449") != NULL;
    ok &= strstr(evidence, "COMMAND.C:1641-1660") != NULL;
    ok &= strstr(evidence, "COORD.C:1903-1920") != NULL;
    ok &= strstr(evidence, "DEFS.H:375-384") != NULL;

    ok &= expect_command("enter", 244, 45,
                         ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                         M11_ENTRANCE_RUNTIME_COMMAND_ENTER_DUNGEON);
    ok &= expect_command("bonus", 244, 45,
                         ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT,
                         M11_ENTRANCE_RUNTIME_COMMAND_ENTER_BONUS_DUNGEON);
    ok &= expect_command("resume", 298, 93,
                         ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                         M11_ENTRANCE_RUNTIME_COMMAND_RESUME);
    ok &= expect_command("quit", 243, 110,
                         ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                         M11_ENTRANCE_RUNTIME_COMMAND_QUIT);
    ok &= expect_command("credits", 248, 186,
                         ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                         M11_ENTRANCE_RUNTIME_COMMAND_DRAW_CREDITS);
    ok &= expect_command("outside_miss", 299, 58,
                         ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                         M11_ENTRANCE_RUNTIME_COMMAND_NONE);
    ok &= expect_command("zero_mask_miss", 244, 45, 0u,
                         M11_ENTRANCE_RUNTIME_COMMAND_NONE);

    ok &= expect_key("return_not_activation", SDLK_RETURN,
                     M11_ENTRANCE_RUNTIME_COMMAND_NONE);
    ok &= expect_key("space_not_activation", SDLK_SPACE,
                     M11_ENTRANCE_RUNTIME_COMMAND_NONE);
    ok &= expect_key("keypad_enter_not_activation", SDLK_KP_ENTER,
                     M11_ENTRANCE_RUNTIME_COMMAND_NONE);
    ok &= expect_key("escape_quit", SDLK_ESCAPE,
                     M11_ENTRANCE_RUNTIME_COMMAND_QUIT);

    printf("entranceRuntimeDispatchInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
