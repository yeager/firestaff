#ifndef FIRESTAFF_DM1_V2_INPUT_REMAP_PC34_H
#define FIRESTAFF_DM1_V2_INPUT_REMAP_PC34_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_V2_GA_MOVE_FORWARD,
    M11_V2_GA_MOVE_BACK,
    M11_V2_GA_STRAFE_LEFT,
    M11_V2_GA_STRAFE_RIGHT,
    M11_V2_GA_TURN_LEFT,
    M11_V2_GA_TURN_RIGHT,
    M11_V2_GA_ATTACK,
    M11_V2_GA_CAST,
    M11_V2_GA_USE,
    M11_V2_GA_PICKUP,
    M11_V2_GA_DROP,
    M11_V2_GA_MAP_TOGGLE,
    M11_V2_GA_INVENTORY,
    M11_V2_GA_JOURNAL,
    M11_V2_GA_QUICK_SAVE,
    M11_V2_GA_QUICK_LOAD,
    M11_V2_GA_REST,
    M11_V2_GA_COUNT
} M11_V2_GameAction;

typedef struct {
    M11_V2_GameAction action;
    int primary_key;
    int alt_key;
    int gamepad_button;
} M11_V2_KeyBinding;

void v2_input_init_defaults(void);
void v2_input_remap(M11_V2_GameAction action, int key);
int v2_input_get_action(int scancode);
bool v2_input_save(const char* path);
bool v2_input_load(const char* path);
void v2_input_reset_defaults(void);

#ifdef __cplusplus
}
#endif

#endif
