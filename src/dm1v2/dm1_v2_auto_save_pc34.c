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


/* V2 auto-save state serialization */
int v2_autosave_serialize_state(const M11_V2_AutoSaveConfig *cfg,
    unsigned char *buf, int bufsize)
{
    if (!cfg || !buf || bufsize < 20) return -1;
    buf[0] = cfg->enabled ? 1 : 0;
    buf[1] = (unsigned char)(cfg->max_slots & 0xFF);
    buf[2] = (unsigned char)(cfg->current_slot & 0xFF);
    buf[3] = 0; /* reserved */
    /* interval_ticks: little-endian uint32 */
    buf[4] = (unsigned char)(cfg->interval_ticks & 0xFF);
    buf[5] = (unsigned char)((cfg->interval_ticks >> 8) & 0xFF);
    buf[6] = (unsigned char)((cfg->interval_ticks >> 16) & 0xFF);
    buf[7] = (unsigned char)((cfg->interval_ticks >> 24) & 0xFF);
    /* last_save_tick */
    buf[8] = (unsigned char)(cfg->last_save_tick & 0xFF);
    buf[9] = (unsigned char)((cfg->last_save_tick >> 8) & 0xFF);
    buf[10] = (unsigned char)((cfg->last_save_tick >> 16) & 0xFF);
    buf[11] = (unsigned char)((cfg->last_save_tick >> 24) & 0xFF);
    return 12;
}

int v2_autosave_deserialize_state(M11_V2_AutoSaveConfig *cfg,
    const unsigned char *buf, int bufsize)
{
    if (!cfg || !buf || bufsize < 12) return -1;
    cfg->enabled = buf[0] != 0;
    cfg->max_slots = buf[1];
    cfg->current_slot = buf[2];
    cfg->interval_ticks = (uint32_t)buf[4]
        | ((uint32_t)buf[5] << 8)
        | ((uint32_t)buf[6] << 16)
        | ((uint32_t)buf[7] << 24);
    cfg->last_save_tick = (uint32_t)buf[8]
        | ((uint32_t)buf[9] << 8)
        | ((uint32_t)buf[10] << 16)
        | ((uint32_t)buf[11] << 24);
    return 0;
}

bool v2_autosave_is_enabled(void) {
    return g_autosave.enabled;
}

int v2_autosave_current_slot(void) {
    return g_autosave.current_slot;
}

