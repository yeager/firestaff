#include "dm1_v2_input_remap_pc34.h"

/* PC34 commands are source-owned command records.  This compatibility API
 * must not add a second keyboard table, gamepad map or host-side binding file. */
void v2_input_init_defaults(void) {}
void v2_input_remap(M11_V2_GameAction action, int key) { (void)action; (void)key; }
int v2_input_get_action(int scancode) { (void)scancode; return -1; }
bool v2_input_save(const char* path) { (void)path; return false; }
bool v2_input_load(const char* path) { (void)path; return false; }
void v2_input_reset_defaults(void) {}
