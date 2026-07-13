#include "theron_v1_srm_classifier.h"
int main(void){unsigned char ok[16]={0x1f,0x8b,8,28,0,0,0,0,0,0,1,0,'x',0,'y',0};unsigned char bad[12]={0x1f,0x8b,8,4,0,1};Theron_V1SrmHeaderReceipt r;return theron_v1_srm_header_receipt(ok,16,&r)&&r.valid&&!theron_v1_srm_header_receipt(bad,12,&r)?0:1;}
