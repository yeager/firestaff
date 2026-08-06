#include "dm1_v1_fmtowns_startup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *root, const char *name, uint8_t **out, size_t *size) {
    char path[1024]; FILE *f; long n;
    snprintf(path,sizeof(path),"%s/%s",root,name); f=fopen(path,"rb"); if(!f) return 0;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;} n=ftell(f); if(n<0){fclose(f);return 0;} rewind(f);
    *out=(uint8_t*)malloc((size_t)n); if(!*out || fread(*out,1,(size_t)n,f)!=(size_t)n){free(*out);*out=NULL;fclose(f);return 0;} fclose(f); *size=(size_t)n; return 1;
}
int main(void) {
    const char *root=getenv("FIRESTAFF_DM1_FMTOWNS_RUNTIME_ROOT");
    uint8_t *a=NULL,*g=NULL,*m=NULL,*i=NULL,*n=NULL; size_t as=0,gs=0,ms=0,is=0,ns=0; DM1_V1_FmtownsStartupReceipt r;
    if(!root || !root[0]) { puts("SKIP: FIRESTAFF_DM1_FMTOWNS_RUNTIME_ROOT is not set"); return 0; }
    if(!read_file(root,"AUTOEXEC.BAT",&a,&as)||!read_file(root,"EDM.EXP",&g,&gs)||!read_file(root,"TMENU.EXP",&m,&ms)||!read_file(root,"TMENU.ICN",&i,&is)||!read_file(root,"TMENU.INF",&n,&ns)) { fprintf(stderr,"FAIL: FM Towns runtime cache is missing original startup files\n"); return 1; }
    if(!dm1_v1_fmtowns_startup_receipt(a,as,g,gs,m,ms,i,is,n,ns,&r)||!dm1_v1_fmtowns_startup_receipt_is_native(&r)||!dm1_v1_fmtowns_startup_receipt_has_native_owners(&r)||r.language!=DM1_FMTOWNS_LANG_EN||strcmp(r.game_program_name,"EDM.EXP")||r.game_p3_header_size!=0x180u||r.game_p3_load_image_offset!=0x200u||r.game_p3_initial_eip!=0x42a48u||r.title_track!=2||r.hall_track!=3||r.entrance_track!=5) { fprintf(stderr,"FAIL: original FM Towns startup receipt\n"); return 1; }
    free(a);free(g);free(m);free(i);free(n); puts("PASS: original FM Towns startup owner and menu media"); return 0;
}
