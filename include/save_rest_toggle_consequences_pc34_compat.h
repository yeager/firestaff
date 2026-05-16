#ifndef SAVE_REST_TOGGLE_CONSEQUENCES_PC34_COMPAT_H
#define SAVE_REST_TOGGLE_CONSEQUENCES_PC34_COMPAT_H

typedef struct SaveRestToggleInputPc34Compat {
    unsigned int command;
    unsigned int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
} SaveRestToggleInputPc34Compat;

typedef struct SaveRestToggleResultPc34Compat {
    unsigned int handled;
    unsigned int toggleInventoryChampionIndex;
    unsigned int closeInventory;
    unsigned int drawDisabledMenus;
    unsigned int drawRestScreen;
    unsigned int drawViewportBeforeRestOrFreeze;
    unsigned int partyIsResting;
    unsigned int waitForInputMaxVbl;
    unsigned int primaryMouseInputPartyResting;
    unsigned int primaryKeyboardInputPartyResting;
    unsigned int discardAllInput;
    unsigned int processSaveGame;
    unsigned int wakeUp;
} SaveRestToggleResultPc34Compat;

#define SAVE_REST_TOGGLE_PC34_COMMAND_TOGGLE_INVENTORY_CHAMPION_0 7u
#define SAVE_REST_TOGGLE_PC34_COMMAND_CLOSE_INVENTORY 11u
#define SAVE_REST_TOGGLE_PC34_COMMAND_SAVE_GAME 140u
#define SAVE_REST_TOGGLE_PC34_COMMAND_REST 145u
#define SAVE_REST_TOGGLE_PC34_COMMAND_WAKE_UP 146u
#define SAVE_REST_TOGGLE_PC34_CHAMPION_CLOSE_INVENTORY 4u

const char* save_rest_toggle_consequences_GetEvidence(void);
SaveRestToggleResultPc34Compat save_rest_toggle_consequences_Evaluate(SaveRestToggleInputPc34Compat input);
unsigned int save_rest_toggle_consequences_GetInvariant(void);

#endif
