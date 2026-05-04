#ifndef FIRESTAFF_DM1_V2_AUTO_SAVE_PC34_H
#define FIRESTAFF_DM1_V2_AUTO_SAVE_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool enabled;
    uint32_t interval_ticks;
    int max_slots;
    int current_slot;
    uint32_t last_save_tick;
} M11_V2_AutoSaveConfig;

void v2_autosave_init(int max_slots, uint32_t interval);
bool v2_autosave_check(uint32_t current_tick);
void v2_autosave_get_path(char *buf, int bufsize);
void v2_autosave_advance_slot(void);
void v2_autosave_enable(void);
void v2_autosave_disable(void);
void v2_autosave_set_interval(uint32_t ticks);

#ifdef __cplusplus
}
#endif

#endif
