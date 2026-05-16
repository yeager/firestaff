
#ifndef FIRESTAFF_DM2_V1_SAVE_LOAD_H
#define FIRESTAFF_DM2_V1_SAVE_LOAD_H
#include <stdint.h>
int dm2_v1_save_game(const char *path, const void *state, int size);
int dm2_v1_load_game(const char *path, void *state, int max_size);
const char *dm2_v1_save_source_evidence(void);
#endif

