#include "csb_v1_f0906_f0925_swoosh_primitive_raw_pc34_compat.h"
#include <string.h>
typedef struct Spec{int f,p,g,s,u;const char*e;}Spec;
static const Spec specs[]={
 {906,1,0,0,0,"ReDMCSB SWSH.C F0906 CheckOriginalDisk"},{907,1,0,0,0,"ReDMCSB SWSH.C F0907 CheckBootSector"},
 {908,1,0,1,0,"ReDMCSB SWSHSND.C F0908 InitSound"},{909,1,0,1,0,"ReDMCSB SWSH.C F0909 PlaySwooshSound"},{910,1,0,1,0,"ReDMCSB SWSH.C F0910 ReleaseSwooshSound"},
 {913,1,0,0,0,"ReDMCSB DECOMPCO.C F0913 DecompressPAK"},{914,1,1,0,0,"ReDMCSB GRAPH21.C F0914 graphic metadata"},{915,1,1,0,0,"ReDMCSB GRAPH538.C F0915 graphic metadata"},
 {917,0,0,0,1,"ReDMCSB PRIM1.C F0917 PRIM Memory Allocate"},{918,0,0,0,1,"ReDMCSB PRIM1.C F0918 PRIM Memory Free"},{919,0,0,0,1,"ReDMCSB PRIM1.C F0919 PRIM File Seek"},{920,1,0,0,1,"ReDMCSB PRIM1.C F0920 PRIM File Read"},
 {922,0,0,0,1,"ReDMCSB PRIM1.C F0922 Custom_strcpy"},{924,0,0,0,1,"ReDMCSB PRIM1.C F0924 SetCriticalErrorHandler"},{925,0,0,0,1,"ReDMCSB PRIM1.C F0925 CheckUtilityDiskInDrive"}};
static int has(const uint8_t*p,size_t n,uint32_t i){return p!=NULL&&n!=0&&i!=0;}
int csb_v1_f0906_f0925_swoosh_primitive_audit_pc34(const CSB_V1_SwooshPrimitiveRawPc34*r,int f,CSB_V1_SwooshPrimitiveReceiptPc34*o){const Spec*s=NULL;size_t i;if(o==NULL)return 0;memset(o,0,sizeof(*o));for(i=0;i<sizeof(specs)/sizeof(specs[0]);++i)if(specs[i].f==f){s=&specs[i];break;}if(s==NULL||r==NULL||!r->authenticated_pc34||(s->p&&!has(r->package,r->package_size,r->package_identity))||(s->g&&!has(r->graphics,r->graphics_size,r->graphics_identity))||(s->s&&!has(r->sound,r->sound_size,r->sound_identity))||(s->u&&!has(r->utility,r->utility_size,r->utility_identity)))return 0;o->raw_material_admitted=1;o->existing_runtime_owner_preserved=1;o->package_required=s->p;o->graphics_required=s->g;o->sound_required=s->s;o->utility_required=s->u;o->read_only_query=1;o->runtime_execution_blocked=1;o->platform_behavior_fail_closed=1;o->function_number=f;o->source_evidence=s->e;return 1;}
