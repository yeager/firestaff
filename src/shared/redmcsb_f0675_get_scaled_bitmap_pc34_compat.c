#include "redmcsb_f0675_get_scaled_bitmap_pc34_compat.h"
const RedmcsbF0675BitmapPc34Compat *redmcsb_f0675_get_scaled_bitmap_pc34_compat(void *c,const RedmcsbF0675BitmapOpsPc34Compat *o,uint16_t index,int16_t derived,int16_t sx,int16_t sy,const uint8_t *pal) {
 const RedmcsbF0675BitmapPc34Compat *native; RedmcsbF0675BitmapPc34Compat *out; bool cached=true;
 if(!o||!o->get_native)return 0;
 if(sx==32&&sy==32&&!pal)return o->get_native(c,index);
 if(derived>=0){if(!o->is_cached||!o->get_derived)return 0; cached=o->is_cached(c,derived); if(cached)return o->get_derived(c,derived); out=o->get_derived(c,derived);} else {if(!o->get_temporary)return 0; out=o->get_temporary(c);}
 native=o->get_native(c,index);
 if(!native||!out||!o->shrink||!o->shrink(c,native,out,sx,sy,pal))return 0;
 if(!cached&&o->add_cache)o->add_cache(c,derived);
 return out;
}
const char *redmcsb_f0675_get_scaled_bitmap_source_evidence_pc34(void){return "ReDMCSB DUNVIEW.C:3308-3392 F0675 native/derived/temporary selection and F0129 scaling.";}
