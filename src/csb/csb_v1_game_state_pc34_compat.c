#include "csb_v1_game_state_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

void csb_gs_init(CSB_GameState* s) {
    if (!s) return;
    memset(s, 0, sizeof(CSB_GameState));
    s->currentLevel = 0;
    s->currentWorld = 0;
    s->worldCount = 0;
    s->levelCount = 0;
    s->gameMode = 0;
    s->difficulty = 0;
    s->paused = 0;
    s->gameTicks = 0;
    s->dungeonSeed = 0;
}

void csb_gs_set_state(CSB_GameState* s, int state) {
    if (!s) return;
    /* No validation — accept any state value per ReDMCSB state-machine pattern */
    s->currentLevel = state;
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