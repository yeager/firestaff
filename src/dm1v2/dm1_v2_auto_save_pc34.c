#include "dm1_v2_auto_save_pc34.h"

/* PC34 saves through the explicit F0433/F0435 path.  There is no rotating
 * autosave slot, periodic trigger or parallel serialized configuration. */
void v2_autosave_init(int max_slots, uint32_t interval) { (void)max_slots; (void)interval; }
bool v2_autosave_check(uint32_t current_tick) { (void)current_tick; return false; }
void v2_autosave_get_path(char *buf, int bufsize) { (void)buf; (void)bufsize; }
void v2_autosave_advance_slot(void) {}
void v2_autosave_enable(void) {}
void v2_autosave_disable(void) {}
void v2_autosave_set_interval(uint32_t ticks) { (void)ticks; }
int v2_autosave_serialize_state(const M11_V2_AutoSaveConfig *cfg, unsigned char *buf, int bufsize) { (void)cfg; (void)buf; (void)bufsize; return -1; }
int v2_autosave_deserialize_state(M11_V2_AutoSaveConfig *cfg, const unsigned char *buf, int bufsize) { (void)cfg; (void)buf; (void)bufsize; return -1; }
bool v2_autosave_is_enabled(void) { return false; }
int v2_autosave_current_slot(void) { return -1; }
