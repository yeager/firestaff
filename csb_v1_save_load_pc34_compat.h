
#ifndef FIRESTAFF_CSB_V1_SAVE_LOAD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_SAVE_LOAD_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Save/Load System
 * Source: CSBWin/SaveGame.cpp (2953 lines)
 */

int csb_v1_save_game(const char *path, const void *state, int state_size);
int csb_v1_load_game(const char *path, void *state, int max_size);
const char *csb_v1_save_source_evidence(void);

#endif

