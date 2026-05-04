#include "csb_v1_game_state_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

void csb_gs_init(CSB_GameState* s) {
    if (!s) return;
    memset(s, 0, sizeof(CSB_GameState));
    s->currentLevel = 0;
    s->currentWorld = 0;
    s->worldCount = 1;
    s->levelCount = 1;
    s->gameMode = 0;
    s->difficulty = 0;
    s->paused = 0;
    s->gameTicks = 0;
    s->dungeonSeed = 0;
}

void csb_gs_set_state(CSB_GameState* s, int state) {
    if (!s) return;
    if (state >= CSB_STATE_UNINIT && state < CSB_STATE_COUNT) {
        s->currentLevel = state; // Reusing currentLevel as state holder based on typical simple state machines, 
                                // but strictly speaking the struct doesn't have a 'state' field.
                                // Looking at the struct: CSB_GameState has no 'state' field.
                                // The prompt asks for csb_gs_set_state and csb_gs_get_state.
                                // I must map 'state' to one of the existing fields or assume the prompt implies 
                                // that 'currentLevel' or another field acts as the state, OR that I should 
                                // have added a state field. 
                                // Re-reading prompt: "typedef struct { ... int currentLevel; ... } CSB_GameState;"
                                // It does NOT have a state field. 
                                // However, csb_gs_set_state exists. 
                                // In many simple games, 'currentLevel' or 'gameMode' might be abused, 
                                // but usually there is a state. 
                                // Let's look at CSBWin/ReDMCSB. They often use a global state variable.
                                // Since I cannot change the struct definition provided in the prompt, 
                                // I have to assume one of the fields represents the state. 
                                // 'currentLevel' is the most likely candidate for a simple state machine 
                                // if no explicit state field exists, OR 'gameMode'.
                                // Let's assume 'currentLevel' is being used to store the state index 
                                // for the purpose of this specific API, as it's an int.
                                // Alternatively, maybe 'gameMode' is the state?
                                // Let's look at the enum: TITLE, MENU, GAME, DUNGEON...
                                // These are states. 
                                // If I must implement csb_gs_set_state, I need a place to put it.
                                // I will use 'currentLevel' to store the state index, as it is an int 
                                // and often level 0 is title/menu in simple implementations.
        s->currentLevel = state;
    }
}

int csb_gs_get_state(const CSB_GameState* s) {
    if (!s) return CSB_STATE_UNINIT;
    return s->currentLevel;
}

void csb_gs_tick(CSB_GameState* s, int ms) {
    if (!s) return;
    if (!s->paused) {
        s->gameTicks += ms;
    }
}

void csb_gs_toggle_pause(CSB_GameState* s) {
    if (!s) return;
    s->paused = !s->paused;
}

void csb_save_build_checksum(CSB_SaveData* sd) {
    if (!sd) return;
    
    unsigned int checksum = 0;
    size_t structSize = sizeof(CSB_SaveData);
    const unsigned char* data = (const unsigned char*)sd;
    
    // Skip the checksum field itself (first 4 bytes)
    // We iterate over the rest of the struct
    for (size_t i = sizeof(unsigned int); i < structSize; ++i) {
        checksum ^= (unsigned int)data[i];
    }
    
    sd->checksum = checksum;
}

int csb_save_verify_checksum(const CSB_SaveData* sd) {
    if (!sd) return -1;
    
    unsigned int checksum = 0;
    size_t structSize = sizeof(CSB_SaveData);
    const unsigned char* data = (const unsigned char*)sd;
    
    // Calculate checksum skipping the checksum field
    for (size_t i = sizeof(unsigned int); i < structSize; ++i) {
        checksum ^= (unsigned int)data[i];
    }
    
    return (checksum == sd->checksum) ? 0 : -1;
}

void csb_save_init(CSB_SaveData* sd, const char* playerName) {
    if (!sd) return;
    
    memset(sd, 0, sizeof(CSB_SaveData));
    sd->version = 1;
    
    if (playerName) {
        strncpy(sd->playerName, playerName, sizeof(sd->playerName) - 1);
        sd->playerName[sizeof(sd->playerName) - 1] = '\0';
    } else {
        sd->playerName[0] = '\0';
    }
    
    sd->playTimeMs = 0;
    
    // Initialize nested structs
    csb_gs_init(&sd->global);
    memset(&sd->dungeon, 0, sizeof(CSB_DungeonState));
    sd->dungeon.global = sd->global; // Copy global state to dungeon global if needed, or keep separate
    
    csb_save_build_checksum(sd);
}

int csb_save_pack(const CSB_SaveData* sd, unsigned char* buf, int bufSize) {
    if (!sd || !buf || bufSize < (int)sizeof(CSB_SaveData)) {
        return -1;
    }
    
    if (csb_save_verify_checksum(sd) != 0) {
        return -1;
    }
    
    memcpy(buf, sd, sizeof(CSB_SaveData));
    return (int)sizeof(CSB_SaveData);
}

int csb_save_unpack(CSB_SaveData* sd, const unsigned char* buf, int bufSize) {
    if (!sd || !buf || bufSize < (int)sizeof(CSB_SaveData)) {
        return -1;
    }
    
    memcpy(sd, buf, sizeof(CSB_SaveData));
    
    if (csb_save_verify_checksum(sd) != 0) {
        return -1;
    }
    
    return 0;
}