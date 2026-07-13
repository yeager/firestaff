#include "theron_v1_srm_classifier.h"
int main(void){unsigned char ok[12]={0x1f,0x8b,8,2};unsigned char bad[11]={0x1f,0x8b,8,2};Theron_V1SrmHeaderReceipt r;return theron_v1_srm_header_receipt(ok,12,&r)&&r.valid&&!theron_v1_srm_header_receipt(bad,11,&r)?0:1;}
