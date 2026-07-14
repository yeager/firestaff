#include "redmcsb_f0685_img3_line_fill_pc34_compat.h"
bool redmcsb_f0685_img3_line_fill_pc34_compat(uint8_t *d,size_t n,size_t p,uint8_t c,size_t count) {
    size_t i; if(d==NULL || p>n*2U || count>n*2U-p)return false;
    for(i=0;i<count;i++,p++) { size_t b=p>>1; if((p&1U)==0U)d[b]=(uint8_t)((d[b]&0x0FU)|((c&0x0FU)<<4)); else d[b]=(uint8_t)((d[b]&0xF0U)|(c&0x0FU)); }
    return true;
}
const char *redmcsb_f0685_img3_line_fill_source_evidence_pc34(void) { return "ReDMCSB IMAGE4.C F0685_IMG3_LineColorFilling (48-67), PC I34E/I34M packed-nibble destination fill"; }
