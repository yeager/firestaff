#include "csb_v1_f0786_f0805_panel_layout_raw_pc34_compat.h"
#include <string.h>
typedef struct Spec { int f,g,z,d,s,p; const char *e; } Spec;
static const Spec specs[] = {
 {786,0,1,0,0,1,"ReDMCSB SELECTOR.C F0786 selector input"},
 {787,0,1,0,0,0,"ReDMCSB COORD.C F0787 GetZoneInitializedFromCoordinates"},
 {788,1,1,0,0,1,"ReDMCSB COORD.C F0788 bitmap zone copy"},
 {789,0,1,0,0,0,"ReDMCSB COORD.C F0789 AllocateLayoutRange"},
 {790,1,0,0,0,1,"ReDMCSB IMAGE4.C F0790 image layout"},
 {791,1,1,1,0,1,"ReDMCSB DUNVIEW.C F0791 viewport bitmap"},
 {792,1,1,1,0,1,"ReDMCSB DUNVIEW.C F0792 viewport bitmap"},
 {797,1,1,1,0,1,"ReDMCSB ENTRANCE.C F0797 DrawEntranceMicroDungeon"},
 {798,0,1,0,0,0,"ReDMCSB COMMAND.C F0798 IsPointInZone"},
 {799,0,0,0,1,1,"ReDMCSB SOUND.C F0799 DisableUnavailableSounds"},
 {802,0,1,1,0,1,"ReDMCSB PANEL.C F0802 IsMagicMap"},
 {803,1,1,1,0,1,"ReDMCSB PANEL.C F0803 DrawMagicMapIcon"},
 {804,1,1,1,0,1,"ReDMCSB PANEL.C F0804 DrawMagicMap"},
 {805,0,1,1,0,1,"ReDMCSB PANEL.C F0805 creature name scroll"}
};
static int has(const uint8_t *p,size_t n,uint32_t id){return p!=NULL&&n!=0&&id!=0;}
int csb_v1_f0786_f0805_panel_layout_audit_pc34(const CSB_V1_PanelLayoutRawPc34 *r,int f,CSB_V1_PanelLayoutReceiptPc34 *o){
 const Spec *s=NULL;size_t i;if(o==NULL)return 0;memset(o,0,sizeof(*o));
 for(i=0;i<sizeof(specs)/sizeof(specs[0]);++i)if(specs[i].f==f){s=&specs[i];break;}
 if(s==NULL||r==NULL||!r->authenticated_pc34||(s->g&&!has(r->graphics,r->graphics_size,r->graphics_identity))||(s->z&&!has(r->zone,r->zone_size,r->zone_identity))||(s->d&&!has(r->dungeon,r->dungeon_size,r->dungeon_identity))||(s->s&&!has(r->sound,r->sound_size,r->sound_identity))||(s->p&&!has(r->package,r->package_size,r->package_identity)))return 0;
 o->raw_material_admitted=1;o->existing_runtime_owner_preserved=1;o->graphics_required=s->g;o->zone_required=s->z;o->dungeon_required=s->d;o->sound_required=s->s;o->package_required=s->p;o->read_only_query=1;o->runtime_execution_blocked=1;o->platform_behavior_fail_closed=1;o->function_number=f;o->source_evidence=s->e;return 1;
}
