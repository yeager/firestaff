#ifndef FIRESTAFF_CSB_V1_GAME_STATE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_GAME_STATE_PC34_COMPAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_STATE_UNINIT = 0,
    CSB_STATE_TITLE,
    CSB_STATE_MENU,
    CSB_STATE_GAME,
    CSB_STATE_DUNGEON,
    CSB_STATE_VICTORY,
    CSB_STATE_GAMEOVER,
    CSB_STATE_COUNT
};

typedef struct {
    unsigned int dungeonSeed;
    int currentLevel;
    int currentWorld;
    int worldCount;
    int levelCount;
    int gameMode;
    int difficulty;
    int paused;
    int gameTicks;
} CSB_GameState;

typedef struct {
    CSB_GameState global;
    unsigned int dungeonSeed;
    int dungeonLevel;
    int dungeonFloor;
} CSB_DungeonState;

typedef struct {
    unsigned int checksum;
    int version;
    CSB_GameState global;
    CSB_DungeonState dungeon;
    char playerName[32];
    int playTimeMs;
} CSB_SaveData;

void csb_gs_init(CSB_GameState* s);
void csb_gs_set_state(CSB_GameState* s, int state);
int csb_gs_get_state(const CSB_GameState* s);
void csb_gs_tick(CSB_GameState* s, int ms);
void csb_gs_toggle_pause(CSB_GameState* s);
void csb_save_build_checksum(CSB_SaveData* sd);
int csb_save_verify_checksum(const CSB_SaveData* sd);
void csb_save_init(CSB_SaveData* sd, const char* playerName);
int csb_save_pack(const CSB_SaveData* sd, unsigned char* buf, int bufSize);
int csb_save_unpack(CSB_SaveData* sd, const unsigned char* buf, int bufSize);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_GAME_STATE_PC34_COMPAT_H */