#ifndef FIRESTAFF_DM1_V1_SAVE_PATH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SAVE_PATH_PC34_COMPAT_H

#include <stddef.h>

/* Creates only the parent directory for a caller-owned, real save path. */
int dm1_v1_save_prepare_parent_directory_pc34(const char *save_path);

#endif
