#include "dm1_v1_save_path_pc34_compat.h"

#include "fs_portable_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *root = "dm1-save-path-test";
    char path[FSP_PATH_MAX];
    char parent[FSP_PATH_MAX];

    if (snprintf(path, sizeof(path), "%s/one/two/real-save.sav", root) >=
            (int)sizeof(path) ||
        !dm1_v1_save_prepare_parent_directory_pc34(path) ||
        snprintf(parent, sizeof(parent), "%s/one/two", root) >=
            (int)sizeof(parent) ||
        !FSP_DirExists(parent) ||
        !dm1_v1_save_prepare_parent_directory_pc34("real-save.sav") ||
        dm1_v1_save_prepare_parent_directory_pc34("") ||
        dm1_v1_save_prepare_parent_directory_pc34(0)) {
        return 1;
    }
    return 0;
}
