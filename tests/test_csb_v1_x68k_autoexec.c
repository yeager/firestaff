#include "csb_v1_x68k_autoexec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int read_file(const char*p,uint8_t**o,size_t*n){FILE*f=fopen(p,"rb");long z;if(!f||fseek(f,0,SEEK_END)||(z=ftell(f))<=0||fseek(f,0,SEEK_SET)||!(*o=malloc((size_t)z))||fread(*o,1,(size_t)z,f)!=(size_t)z){if(f)fclose(f);free(*o);return 0;}fclose(f);*n=(size_t)z;return 1;}
int main(void){const char*p=getenv("FIRESTAFF_CSB_X68K_HDM");CSB_V1_X68kAutoexecReceipt r;if(!p||!*p){puts("test_csb_v1_x68k_autoexec: SKIP FIRESTAFF_CSB_X68K_HDM unset");return 0;}{uint8_t*h=0;size_t n=0;if(!read_file(p,&h,&n)||!csb_v1_x68k_autoexec_receipt(h,n,&r)||r.command_count!=3u||strcmp(r.commands[0],"CK")||strcmp(r.commands[1],"VIDSET")||strcmp(r.commands[2],"CHAOS_STRIKES_BACK")||!r.dos_eof_terminated||r.host_execution_permitted){free(h);return 1;}free(h);}puts("test_csb_v1_x68k_autoexec: PASS");return 0;}
