#ifndef FIRESTAFF_DM1_V2_ACHIEVEMENTS_PC34_H
#define FIRESTAFF_DM1_V2_ACHIEVEMENTS_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int id;
    char name[64];
    char desc[128];
    bool unlocked;
    uint32_t unlock_time;
    int icon_idx;
} M11_V2_Achievement;

void v2_achievement_init(void);
void v2_achievement_define(int id, const char* name, const char* desc, int icon);
void v2_achievement_unlock(int id);
bool v2_achievement_is_unlocked(int id);
void v2_achievement_save(const char* path);
void v2_achievement_load(const char* path);
const char* v2_achievement_get_notification(void);

#ifdef __cplusplus
}
#endif

#endif
