#include "redmcsb_f0685_img3_line_fill_pc34_compat.h"
int main(void){uint8_t b[3]={0x12,0x34,0x56}; if(!redmcsb_f0685_img3_line_fill_pc34_compat(b,3,1,0xA,4)||b[0]!=0x1A||b[1]!=0xAA||b[2]!=0xA6)return 1; if(redmcsb_f0685_img3_line_fill_pc34_compat(b,3,5,1,2)||b[2]!=0xA6)return 1; return 0;}
