#ifndef FIRESTAFF_DM2_V1_MAC_INPUT_H
#define FIRESTAFF_DM2_V1_MAC_INPUT_H

#include <stdint.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"

/* Macintosh key values which are not ASCII characters. */
enum {
    DM2_V1_MAC_KEY_RETURN = 0x1000,
    DM2_V1_MAC_KEY_ENTER,
    DM2_V1_MAC_KEY_ESCAPE,
    DM2_V1_MAC_KEY_UP,
    DM2_V1_MAC_KEY_DOWN,
    DM2_V1_MAC_KEY_LEFT,
    DM2_V1_MAC_KEY_RIGHT,
    DM2_V1_MAC_KEY_HOME,
    DM2_V1_MAC_KEY_END,
    DM2_V1_MAC_KEY_PAGE_UP,
    DM2_V1_MAC_KEY_PAGE_DOWN,
    DM2_V1_MAC_KEY_HELP,
    DM2_V1_MAC_KEY_DELETE,
    DM2_V1_MAC_KEY_NUMPAD_1,
    DM2_V1_MAC_KEY_NUMPAD_2,
    DM2_V1_MAC_KEY_NUMPAD_3,
    DM2_V1_MAC_KEY_NUMPAD_4,
    DM2_V1_MAC_KEY_NUMPAD_5,
    DM2_V1_MAC_KEY_NUMPAD_6,
    DM2_V1_MAC_KEY_NUMPAD_7,
    DM2_V1_MAC_KEY_NUMPAD_8,
    DM2_V1_MAC_KEY_NUMPAD_9,
    DM2_V1_MAC_KEY_F1,
    DM2_V1_MAC_KEY_F2,
    DM2_V1_MAC_KEY_F3,
    DM2_V1_MAC_KEY_F4,
    DM2_V1_MAC_KEY_F13,
    DM2_V1_MAC_KEY_F14,
    DM2_V1_MAC_KEY_F15
};

#define DM2_V1_MAC_MOD_COMMAND (1u << 0)

typedef enum {
    DM2_V1_MAC_INPUT_GAMEPLAY = 0,
    DM2_V1_MAC_INPUT_ENTRANCE,
    DM2_V1_MAC_INPUT_CREDITS
} DM2_V1_MacInputPhase;

typedef enum {
    DM2_V1_MAC_ACTION_NONE = 0,
    DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_0,
    DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_1,
    DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_2,
    DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_3,
    DM2_V1_MAC_ACTION_TOGGLE_LEADER,
    DM2_V1_MAC_ACTION_FREEZE,
    DM2_V1_MAC_ACTION_WAKE,
    DM2_V1_MAC_ACTION_TURN_LEFT,
    DM2_V1_MAC_ACTION_MOVE_FORWARD,
    DM2_V1_MAC_ACTION_TURN_RIGHT,
    DM2_V1_MAC_ACTION_MOVE_LEFT,
    DM2_V1_MAC_ACTION_MOVE_BACKWARD,
    DM2_V1_MAC_ACTION_MOVE_RIGHT,
    DM2_V1_MAC_ACTION_WALL_LEFT,
    DM2_V1_MAC_ACTION_WALL_CENTER,
    DM2_V1_MAC_ACTION_WALL_RIGHT,
    DM2_V1_MAC_ACTION_OPEN_GAME,
    DM2_V1_MAC_ACTION_SAVE_GAME,
    DM2_V1_MAC_ACTION_QUIT,
    DM2_V1_MAC_ACTION_NEW_GAME,
    DM2_V1_MAC_ACTION_CLOSE_CREDITS
} DM2_V1_MacInputAction;

typedef struct {
    int accepted;
    DM2_V1_MacInputAction action;
    int runtime_command;
} DM2_V1_MacInputReceipt;

/* Resolve the English Macintosh retail keyboard table without guessing. */
int dm2_v1_mac_input_resolve(uint32_t key,
                             uint32_t modifiers,
                             DM2_V1_MacInputPhase phase,
                             DM2_V1_MacInputReceipt *out);

/* Send only actions represented by the existing source-compatible queue.
 * Mac-only menu/wall actions remain explicit for the native Mac dispatcher. */
int dm2_v1_mac_input_enqueue(const DM2_V1_MacInputReceipt *receipt,
                             struct Dm1V1InputCommandQueuePc34Compat *queue);

const char *dm2_v1_mac_input_source_evidence(void);

#endif
