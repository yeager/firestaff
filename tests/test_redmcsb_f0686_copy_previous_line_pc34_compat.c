#include "redmcsb_f0686_copy_previous_line_pc34_compat.h"
int main(void){uint8_t b[4]={0x12,0x34,0x56,0x78};if(!redmcsb_f0686_copy_previous_line_pc34_compat(b,4,4,0,4)||b[2]!=0x12||b[3]!=0x34)return 1;if(redmcsb_f0686_copy_previous_line_pc34_compat(b,4,7,0,2)||b[3]!=0x34)return 1;return 0;}
