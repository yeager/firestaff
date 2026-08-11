#ifndef FIRESTAFF_CSB_HINT_ORACLE_GRAPHICS_SURFACE_H
#define FIRESTAFF_CSB_HINT_ORACLE_GRAPHICS_SURFACE_H
#include "csb_hint_oracle_dat_real_scan.h"
#define CSB_HINT_ORACLE_GRAPHICS_SURFACE_WIDTH 320u
#define CSB_HINT_ORACLE_GRAPHICS_SURFACE_HEIGHT 200u
typedef struct { CSB_HintOracleDAT_RealCache source; uint8_t *pixels; uint8_t rgb4[48]; uint16_t width,height; } CSB_HintOracleGraphicsSurface;
void csb_hint_oracle_graphics_surface_init(CSB_HintOracleGraphicsSurface *surface);
void csb_hint_oracle_graphics_surface_free(CSB_HintOracleGraphicsSurface *surface);
int csb_hint_oracle_graphics_surface_load(CSB_HintOracleGraphicsSurface *surface,const char *data_dir,int max_depth,const char *expected_md5);
#endif
