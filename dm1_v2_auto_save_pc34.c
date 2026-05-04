#include "dm1_v2_auto_save_pc34.h"

static M11_V2_AutoSaveConfig g_autosave = {false, 0, 0, 0, 0};

void v2_autosave_init(int max_slots, uint32_t interval) {
    g_autosave.enabled = false;
    g_autosave.interval_ticks = interval;
    g_autosave.max_slots = max_slots > 0 ? max_slots : 5;
    g_autosave.current_slot = 0;
    g_autosave.last_save_tick = 0;
}

bool v2_autosave_check(uint32_t current_tick) {
    if (!g_autosave.enabled) return false;
    if ((current_tick - g_autosave.last_save_tick) >= g_autosave.interval_ticks) {
        g_autosave.last_save_tick = current_tick;
        return true;
    }
    return false;
}

void v2_autosave_get_path(char *buf, int bufsize) {
    if (!buf || bufsize <= 0) return;
    snprintf(buf, bufsize, "autosave_slot%d.dat", g_autosave.current_slot);
}

void v2_autosave_advance_slot(void) {
    if (g_autosave.max_slots > 0) {
        g_autosave.current_slot = (g_autosave.current_slot + 1) % g_autosave.max_slots;
    }
}

void v2_autosave_enable(void) {
    g_autosave.enabled = true;
}

void v2_autosave_disable(void) {
    g_autosave.enabled = false;
}

void v2_autosave_set_interval(uint32_t ticks) {
    g_autosave.interval_ticks = ticks;
}
