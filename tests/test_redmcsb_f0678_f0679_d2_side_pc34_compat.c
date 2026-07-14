#include "redmcsb_f0678_f0679_d2_side_pc34_compat.h"
#include <string.h>
typedef struct { char log[4]; int n, index, zone, aspect, flip; } fxt;
static void wall(void*p,int s,int i,int z,int f){fxt*x=p;(void)s;x->log[x->n++]='W';x->index=i;x->zone=z;x->flip=f;} static void field(void*p,int s,int a,int z){fxt*x=p;(void)s;x->log[x->n++]='F';x->aspect=a;x->zone=z;}
int main(void){fxt x; redmcsb_f0678_f0679_runtime_pc34_compat r={wall,field,&x}; memset(&x,0,sizeof x); if(!redmcsb_f0678_f0679_draw_d2_side_pc34_compat(0,0,0,&r)||x.log[0]!='W'||x.index!=6||x.zone!=707||x.flip)return 1; memset(&x,0,sizeof x); if(!redmcsb_f0678_f0679_draw_d2_side_pc34_compat(1,0,1,&r)||x.index!=6||x.zone!=708||x.flip!=1)return 1; memset(&x,0,sizeof x); if(!redmcsb_f0678_f0679_draw_d2_side_pc34_compat(0,5,0,&r)||x.log[0]!='F'||x.aspect!=5||x.zone!=707)return 1; memset(&x,0,sizeof x); if(!redmcsb_f0678_f0679_draw_d2_side_pc34_compat(1,1,0,&r)||x.n)return 1; return 0;}
