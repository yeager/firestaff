#include "csb_hint_oracle_graphics_surface.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv){const char *d=argc>1?argv[1]:getenv("FIRESTAFF_CSB_HCSB_DAT_DATA");CSB_HintOracleGraphicsSurface s;int rc;if(!d||!*d){puts("SKIP: set FIRESTAFF_CSB_HCSB_DAT_DATA.");return 0;}csb_hint_oracle_graphics_surface_init(&s);rc=csb_hint_oracle_graphics_surface_load(&s,d,6,NULL);if(rc){fprintf(stderr,"FAIL: surface load %d\n",rc);return 1;}if(!s.pixels||s.width!=320u||s.height!=200u){fprintf(stderr,"FAIL: invalid surface\n");csb_hint_oracle_graphics_surface_free(&s);return 1;}printf("surface %ux%u md5=%s\n",s.width,s.height,s.source.matched_md5);csb_hint_oracle_graphics_surface_free(&s);return 0;}
