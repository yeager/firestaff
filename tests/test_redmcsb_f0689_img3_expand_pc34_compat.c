#include "redmcsb_f0689_img3_expand_pc34_compat.h"
int main(void){const uint8_t s[]={2,0,1,0,0x09,0x80,0,0x12};uint8_t d[1]={0};uint16_t w,h;if(!redmcsb_f0689_img3_expand_even_pc34_compat(s,sizeof s,d,sizeof d,&w,&h)||w!=2||h!=1||d[0]!=0x98)return 1;return 0;}
