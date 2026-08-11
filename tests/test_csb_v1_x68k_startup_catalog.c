#include "csb_v1_x68k_startup_catalog.h"
#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *p, uint8_t **o, size_t *n) { FILE *f=fopen(p,"rb"); long z; if(!f||fseek(f,0,SEEK_END)||(z=ftell(f))<=0||fseek(f,0,SEEK_SET)||!(*o=malloc((size_t)z))||fread(*o,1,(size_t)z,f)!=(size_t)z){if(f)fclose(f);free(*o);return 0;}fclose(f);*n=(size_t)z;return 1; }
int main(void) { const char *p=getenv("FIRESTAFF_CSB_X68K_HDM"); uint8_t bad[8]={0}; CSB_V1_X68kStartupCatalog c; if(csb_v1_x68k_hdm_startup_catalog(bad,sizeof(bad),&c)||csb_v1_x68k_hdm_startup_catalog(NULL,0,NULL))return 1; if(!p||!*p){puts("test_csb_v1_x68k_startup_catalog: SKIP FIRESTAFF_CSB_X68K_HDM unset");return 0;} {uint8_t *h=NULL;size_t n=0;if(!read_file(p,&h,&n)||!csb_v1_x68k_hdm_startup_catalog(h,n,&c)||c.root_file_count!=27u||c.program_bytes!=12284u||c.graphics_bytes!=373583u||c.dungeon_bytes!=2066u||c.title_bytes!=16830u||c.animation_bytes!=286946u||c.entrance_music_bytes!=14722u||c.animation_script_bytes!=6366u||c.mini_dungeon_bytes!=43222u||!c.x68000_identity_bound||c.native_runtime_launch_permitted){free(h);return 1;}free(h);}puts("test_csb_v1_x68k_startup_catalog: PASS");return 0;}
