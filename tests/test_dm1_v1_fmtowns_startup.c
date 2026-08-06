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
    uint8_t *a=NULL,*g=NULL,*m=NULL,*i=NULL,*n=NULL; size_t as=0,gs=0,ms=0,is=0,ns=0; DM1_V1_FmtownsStartupReceipt r, directoryReceipt; DM1_V1_FmtownsMenuReceipt menuReceipt;
    if(!root || !root[0]) { puts("SKIP: FIRESTAFF_DM1_FMTOWNS_RUNTIME_ROOT is not set"); return 0; }
    if(!read_file(root,"AUTOEXEC.BAT",&a,&as)||!read_file(root,"EDM.EXP",&g,&gs)||!read_file(root,"TMENU.EXP",&m,&ms)||!read_file(root,"TMENU.ICN",&i,&is)||!read_file(root,"TMENU.INF",&n,&ns)) { fprintf(stderr,"FAIL: FM Towns runtime cache is missing original startup files\n"); return 1; }
    if(!dm1_v1_fmtowns_startup_receipt(a,as,g,gs,m,ms,i,is,n,ns,&r)||!dm1_v1_fmtowns_startup_receipt_is_native(&r)||!dm1_v1_fmtowns_startup_receipt_has_native_owners(&r)||r.language!=DM1_FMTOWNS_LANG_EN||strcmp(r.game_program_name,"EDM.EXP")||r.game_p3_header_size!=0x180u||r.game_p3_load_image_offset!=0x200u||r.game_p3_initial_eip!=0x42a48u||!r.game_symbol_table_verified||r.game_symbol_table_entry_count!=1174u||r.game_do_title_animation_entry!=0xc3b0u||r.game_title_presents_entry!=0x28f4au||r.game_title_dungeon_entry!=0x28f4cu||r.game_draw_dmenu_entry!=0x4620u||r.game_dynamenu_entry!=0x2418cu||r.game_menu_icons_entry!=0x2415cu||r.game_cd_level_song_entry!=0x211d8u||!r.game_title_animation_plan_verified||r.game_title_graphic_index!=1u||r.game_title_presents_source_y!=137u||r.game_title_master_source_y!=80u||r.game_title_zoom_step_count!=18u||r.game_title_zoom_width_step!=16u||r.game_title_zoom_height_step!=4u||r.game_title_swoosh_rect[2]!=0u||r.game_title_swoosh_rect[3]!=56u||r.game_title_presents_rect[2]!=90u||r.game_title_presents_rect[3]!=105u||r.game_title_master_rect[2]!=118u||r.game_title_master_rect[3]!=174u||r.game_action_name_count!=44u||strcmp(r.game_action_names[6],"PUNCH")||strcmp(r.game_action_names[8],"WAR CRY")||strcmp(r.game_action_names[43],"FUSE")||r.title_track!=2||r.hall_track!=3||r.entrance_track!=5) { fprintf(stderr,"FAIL: original FM Towns startup receipt\n"); return 1; }
    if (!dm1_v1_fmtowns_startup_receipt_from_directory(root, 0, &directoryReceipt) ||
        !directoryReceipt.valid || !directoryReceipt.game_title_animation_plan_verified) {
        fprintf(stderr,"FAIL: FM Towns directory startup handoff\n"); return 1;
    }
    if (!dm1_v1_fmtowns_menu_receipt(n, ns, &menuReceipt) || !menuReceipt.valid ||
        !menuReceipt.entries[0].valid || !menuReceipt.entries[1].valid ||
        menuReceipt.entries[0].language != DM1_FMTOWNS_LANG_JP ||
        menuReceipt.entries[1].language != DM1_FMTOWNS_LANG_EN ||
        strcmp(menuReceipt.entries[0].program_name, "JDM     .EXP") ||
        strcmp(menuReceipt.entries[1].program_name, "EDM     .EXP") ||
        strcmp(menuReceipt.entries[0].program_path, "\\JDM.EXP") ||
        strcmp(menuReceipt.entries[1].program_path, "\\EDM.EXP")) {
        fprintf(stderr,"FAIL: original TMENU.INF launch records\n"); return 1;
    }
    free(a);free(g);free(m);free(i);free(n); puts("PASS: original FM Towns startup owner and menu media"); return 0;
}
