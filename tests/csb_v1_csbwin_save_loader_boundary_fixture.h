#ifndef CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_FIXTURE_H
#define CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_FIXTURE_H

#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"

size_t csb_v1_csbwin_save_loader_boundary_build_fixture(
    CSB_V1_CSBWinSaveShape shape, uint8_t *out_buf, size_t buf_capacity);
int csb_v1_csbwin_save_loader_boundary_check_shape(
    CSB_V1_CSBWinSaveShape shape, CSB_V1_CSBWinLoaderBoundaryResult *out);

#endif
