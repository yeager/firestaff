#include "redmcsb_f8137_pixels_nec16_compat.h"
void redmcsb_f8137_set_multiple_pixels_nec16_compat(uint8_t *v,size_t n,uint16_t p,uint8_t c,uint16_t count){
 size_t i; if(!v)return; c&=15U; for(i=0;i<count;i++){size_t q=(size_t)p+i,b=q/2U;if(b>=n)break;if(q&1U)v[b]=(uint8_t)((v[b]&0xf0U)|c);else v[b]=(uint8_t)((v[b]&0x0fU)|(c<<4));}}
