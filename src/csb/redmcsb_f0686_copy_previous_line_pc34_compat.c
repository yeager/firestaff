#include "redmcsb_f0686_copy_previous_line_pc34_compat.h"
static uint8_t getp(const uint8_t*b,size_t p){return (uint8_t)(((p&1U)?b[p>>1]:(b[p>>1]>>4))&15U);}static void setp(uint8_t*b,size_t p,uint8_t v){size_t q=p>>1;if((p&1U)==0U)b[q]=(uint8_t)((b[q]&15U)|(v<<4));else b[q]=(uint8_t)((b[q]&240U)|v);}
bool redmcsb_f0686_copy_previous_line_pc34_compat(uint8_t*b,size_t n,size_t d,size_t s,size_t c){size_t i;if(b==0||d>n*2U||s>n*2U||c>n*2U-d||c>n*2U-s)return false;for(i=0;i<c;i++)setp(b,d+i,getp(b,s+i));return true;}
const char *redmcsb_f0686_copy_previous_line_source_evidence_pc34(void){return "ReDMCSB IMAGE2.C F0686_IMG_CopyFromPreviousLine (19-37), PC I34E/I34M packed pixel copy";}
