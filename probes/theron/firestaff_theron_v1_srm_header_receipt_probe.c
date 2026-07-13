#include "theron_v1_srm_classifier.h"
int main(void){unsigned char ok[10]={0x1f,0x8b,8};unsigned char bad[10]={0};Theron_V1SrmHeaderReceipt r;bad[0]=0x1f;bad[1]=0x8b;bad[2]=8;bad[3]=0xe0;return theron_v1_srm_header_receipt(ok,10,&r)&&r.valid&&r.container_bytes==10&&!theron_v1_srm_header_receipt(ok,2,&r)&&!theron_v1_srm_header_receipt(bad,10,&r)?0:1;}
