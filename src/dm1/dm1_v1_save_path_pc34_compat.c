#include "dm1_v1_save_path_pc34_compat.h"

#include "fs_portable_compat.h"

int dm1_v1_save_prepare_parent_directory_pc34(const char *save_path)
{
    char parent[FSP_PATH_MAX];

    if (!save_path || save_path[0] == '\0') {
        return 0;
    }
    /* A bare filename is deliberately relative to the user-selected cwd. */
    if (!FSP_ParentDir(parent, sizeof(parent), save_path)) {
        return 1;
    }
    return FSP_CreateDirectoryRecursive(parent);
}
