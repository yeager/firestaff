#include <stdint.h>
#include <string.h>
#include "redmcsb_f8137_pixels_nec16_compat.h"
int main(void){uint8_t v[3]={0xaa,0xaa,0xaa};redmcsb_f8137_set_multiple_pixels_nec16_compat(v,3,1,3,4);return memcmp(v,(uint8_t[]){0xa3,0x33,0x3a},3)!=0;}
